#include "parking.h"
#include <algorithm>

// ---------------------------------------------------------------------------
// UFPR04 lot — large multi-row horizontal lot
// Calibrated from 2012-10-13 (mostly empty). Image size: 640x640.
//
// Layout: 4 row-pairs, each pair = upper row + green median + lower row.
// 8 spaces per row, x from 50 to 432, w=46. 8 rows * 8 spaces = 64 ROIs.
//
// To retune: open 2012-10-13 in MS Paint, hover each space corner, read coords.
// ---------------------------------------------------------------------------
static std::vector<ParkingSpace> getROIs_UFPR04() {
    const int w = 46, h = 28;
    const int xs[] = { 50, 98, 146, 194, 242, 290, 338, 386 };
    // 4 band-pairs, 2 rows each: upper y, lower y
    const int ys[] = { 28, 65, 128, 168, 233, 270, 343, 383 };

    std::vector<ParkingSpace> spaces;
    for (int y : ys)
        for (int x : xs)
            spaces.push_back({ cv::Rect(x, y, w, h), false });
    return spaces;
}

// ---------------------------------------------------------------------------
// PUCPR lot — smaller diagonal/angled lot with landscaping
// Calibrated from 2012-12-08 (mostly empty). Image size: 640x640.
//
// Rectangular ROIs approximate the angled space boundaries.
// Expect some mis-alignment — tune x/y/w/h against the actual images.
// ---------------------------------------------------------------------------
static std::vector<ParkingSpace> getROIs_PUCPR() {
    std::vector<cv::Rect> rects = {
        // Right column (diagonal spaces, stepping diagonally down-left)
        cv::Rect(440, 100, 80, 45), cv::Rect(455, 152, 80, 45),
        cv::Rect(470, 204, 80, 45), cv::Rect(485, 256, 80, 45),
        cv::Rect(500, 308, 80, 45),
        // Centre-right column
        cv::Rect(318, 100, 75, 45), cv::Rect(333, 152, 75, 45),
        cv::Rect(348, 204, 75, 45), cv::Rect(363, 256, 75, 45),
        // Left-side lower column
        cv::Rect(72, 240, 70, 45),  cv::Rect(92,  292, 70, 45),
        cv::Rect(112, 344, 70, 45), cv::Rect(50,  450, 75, 50),
        cv::Rect(130, 470, 75, 50),
    };

    std::vector<ParkingSpace> spaces;
    for (auto& r : rects)
        spaces.push_back({ r, false });
    return spaces;
}

// ---------------------------------------------------------------------------
// Route to the correct lot based on image filename date prefix.
// UFPR04: filenames starting 2012-09, 2012-10, or 2012-11
// PUCPR:  everything else (2012-12, 2013-*)
// ---------------------------------------------------------------------------
std::vector<ParkingSpace> getROIs(const std::string& imagePath) {
    // Extract just the filename (handle both / and \ separators)
    size_t sep = imagePath.find_last_of("/\\");
    std::string fname = (sep == std::string::npos) ? imagePath : imagePath.substr(sep + 1);

    bool isUFPR04 = (fname.find("2012-09") == 0 ||
                     fname.find("2012-10") == 0 ||
                     fname.find("2012-11") == 0);

    return isUFPR04 ? getROIs_UFPR04() : getROIs_PUCPR();
}

void classifySpaces(const cv::Mat& blurred, std::vector<ParkingSpace>& spaces, int threshold) {
    cv::Rect imgBounds(0, 0, blurred.cols, blurred.rows);
    for (auto& space : spaces) {
        // Clamp to image bounds — prevents crash if any ROI extends outside
        cv::Rect safe = space.roi & imgBounds;
        if (safe.empty()) { space.occupied = false; continue; }

        cv::Mat roi = blurred(safe);
        cv::Mat edges;
        // Low=30/High=90: more sensitive than default, catches lower-contrast cars
        cv::Canny(roi, edges, 50, 150);
        int edgeCount = cv::countNonZero(edges);
        space.occupied = (edgeCount > threshold);
    }
}

void drawResults(cv::Mat& image, const std::vector<ParkingSpace>& spaces) {
    for (const auto& space : spaces) {
        cv::Scalar color = space.occupied
            ? cv::Scalar(0, 0, 255)   // red  = occupied
            : cv::Scalar(0, 255, 0);  // green = empty
        cv::rectangle(image, space.roi, color, 2);
        std::string label = space.occupied ? "OCC" : "EMP";
        cv::putText(image, label,
            cv::Point(space.roi.x + 4, space.roi.y + 16),
            cv::FONT_HERSHEY_SIMPLEX, 0.45, color, 1);
    }
}
