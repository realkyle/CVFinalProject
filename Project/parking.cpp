#include "parking.h"

// ---------------------------------------------------------------------------
// TODO: Replace these rectangles with your actual parking space coordinates.
//
// How to find coordinates:
//   1. Open your test image in MS Paint
//   2. Hover over each corner of a parking space — Paint shows pixel coords
//      in the bottom-left status bar (e.g. "128, 245")
//   3. cv::Rect(x, y, width, height) where (x, y) is the TOP-LEFT corner
//
// These placeholders assume a ~1280x720 PKLot-style overhead image.
// ---------------------------------------------------------------------------
std::vector<ParkingSpace> getROIs() {
    std::vector<cv::Rect> rects = {
        cv::Rect( 50, 100, 80, 140),
        cv::Rect(140, 100, 80, 140),
        cv::Rect(230, 100, 80, 140),
        cv::Rect(320, 100, 80, 140),
        cv::Rect(410, 100, 80, 140),
        cv::Rect( 50, 280, 80, 140),
        cv::Rect(140, 280, 80, 140),
        cv::Rect(230, 280, 80, 140),
        cv::Rect(320, 280, 80, 140),
        cv::Rect(410, 280, 80, 140),
    };

    std::vector<ParkingSpace> spaces;
    for (auto& r : rects)
        spaces.push_back({ r, false });
    return spaces;
}

void classifySpaces(const cv::Mat& blurred, std::vector<ParkingSpace>& spaces, int threshold) {
    for (auto& space : spaces) {
        cv::Mat roi = blurred(space.roi);
        cv::Mat edges;
        // Low/high thresholds for Canny — tune if needed
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
