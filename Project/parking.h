#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct ParkingSpace {
    cv::Rect roi;       // bounding box in the full image
    bool occupied;
};

// Returns parking space ROIs for the given image (selects lot layout by filename)
std::vector<ParkingSpace> getROIs(const std::string& imagePath);

// Runs Canny edge detection on each ROI and sets occupied flag
// threshold: edge pixel count above which a space is considered occupied
void classifySpaces(const cv::Mat& blurred, std::vector<ParkingSpace>& spaces, int threshold = 150);

// Draws colored bounding boxes and OCC/EMP labels onto the image
void drawResults(cv::Mat& image, const std::vector<ParkingSpace>& spaces);
