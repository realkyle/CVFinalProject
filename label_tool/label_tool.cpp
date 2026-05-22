#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

struct State {
    cv::Mat display;
    cv::Mat original;
    std::vector<std::vector<cv::Point>> polys;  // completed spaces
    std::vector<cv::Point> current;             // corners clicked so far
};

void redraw(State* s) {
    s->display = s->original.clone();

    // Instructions
    cv::putText(s->display, "Click 4 corners per space (any order, go around the shape)",
        { 8, 20 }, cv::FONT_HERSHEY_SIMPLEX, 0.5, { 255,255,255 }, 2);
    cv::putText(s->display, "U = undo last space  |  S = print code  |  ESC = quit",
        { 8, 40 }, cv::FONT_HERSHEY_SIMPLEX, 0.45, { 200,200,200 }, 1);

    // Completed polys
    for (int i = 0; i < (int)s->polys.size(); i++) {
        cv::polylines(s->display, s->polys[i], true, { 0,255,0 }, 2);
        cv::putText(s->display, std::to_string(i + 1),
            { s->polys[i][0].x + 3, s->polys[i][0].y + 14 },
            cv::FONT_HERSHEY_SIMPLEX, 0.45, { 0,255,0 }, 1);
    }

    // In-progress corners
    for (int i = 0; i < (int)s->current.size(); i++) {
        cv::circle(s->display, s->current[i], 4, { 255,255,0 }, -1);
        if (i > 0)
            cv::line(s->display, s->current[i - 1], s->current[i], { 255,255,0 }, 1);
    }

    cv::imshow("Label Tool", s->display);
}

void onMouse(int event, int x, int y, int, void* userdata) {
    State* s = reinterpret_cast<State*>(userdata);

    if (event == cv::EVENT_LBUTTONDOWN) {
        s->current.push_back({ x, y });

        if (s->current.size() == 4) {
            s->polys.push_back(s->current);
            s->current.clear();
        }
        redraw(s);
    }

    // Rubber band: show line from last clicked point to mouse
    if (event == cv::EVENT_MOUSEMOVE && !s->current.empty()) {
        redraw(s);
        cv::line(s->display,
            s->current.back(), { x, y },
            { 255,255,0 }, 1);
        cv::imshow("Label Tool", s->display);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: label_tool.exe <empty_lot_image>" << std::endl;
        return 1;
    }

    State s;
    s.original = cv::imread(argv[1]);
    if (s.original.empty()) {
        std::cerr << "Could not load: " << argv[1] << std::endl;
        return 1;
    }

    cv::namedWindow("Label Tool");
    cv::setMouseCallback("Label Tool", onMouse, &s);
    redraw(&s);

    while (true) {
        int key = cv::waitKey(20);
        if (key == 27) break;  // ESC

        if (key == 'u' || key == 'U') {
            if (!s.current.empty()) {
                s.current.pop_back();   // undo last corner mid-polygon
            }
            else if (!s.polys.empty()) {
                s.polys.pop_back();     // undo last completed polygon
            }
            redraw(&s);
        }

        if (key == 's' || key == 'S') {
            std::cout << "\n// --- paste into parking.cpp ---\n";
            std::cout << "std::vector<ParkingSpace> spaces = {\n";
            for (const auto& poly : s.polys) {
                std::cout << "    {{"
                    << "{" << poly[0].x << "," << poly[0].y << "},"
                    << "{" << poly[1].x << "," << poly[1].y << "},"
                    << "{" << poly[2].x << "," << poly[2].y << "},"
                    << "{" << poly[3].x << "," << poly[3].y << "}"
                    << "}, false},\n";
            }
            std::cout << "};\n";
            std::cout << "// " << s.polys.size() << " spaces total\n";
        }
    }

    return 0;
}