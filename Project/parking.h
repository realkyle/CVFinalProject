#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct ParkingSpace {
    std::vector<cv::Point> poly;       // bounding box in the full image
    bool occupied;
};

// Returns parking space ROIs for the given image (selects lot layout by filename)
std::vector<ParkingSpace> getROIs(const std::string& imagePath);

// Returns the Canny edge-count threshold calibrated for the lot in the given image
int getThreshold(const std::string& imagePath);

// Returns the pixel stddev threshold calibrated for the lot in the given image
double getVarianceThreshold(const std::string& imagePath);

// Classifies each space as occupied/empty using edge density + intensity variance.
// A space is occupied if edgeCount > edgeThreshold OR stddev > varianceThreshold.
void classifySpaces(const cv::Mat& blurred, std::vector<ParkingSpace>& spaces,
                    int edgeThreshold = 93, double varianceThreshold = 10.0);

// Draws colored bounding boxes and OCC/EMP labels onto the image
void drawResults(cv::Mat& image, const std::vector<ParkingSpace>& spaces);
