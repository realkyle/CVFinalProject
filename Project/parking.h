//----------------------------------------------------------------------------
// PARKING.H
// Author: Jingyi Zhong, Kyle Hale
// ----------------------------------------------------------------------------
// Header file for parking lot occupancy detection system. Defines ParkingSpace
// data structure and declares all public functions used by main.cpp to run the
// detection pipeline.
// Each space is represented as a hand traced quadrilateral polygon to accurately
// cover angled and diagonal spots. Classification uses Canny edge density signal
// and pixel intenssity standard deviation.
// 
// Assumptions: 1. Input images are taken from a fixed overhead camera position.
//              2. Polygon ROIs were traced on an empty reference image and are
//                  reused for all images from the same lot and camera angle.
//              3. The correct lot is identified by the image filename prefix.
//              4. Images are expected to be 640x640 pixels
// ----------------------------------------------------------------------------

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// ----- struct ParkingSpace ---------------------------------------------------------
// Represents a single parking space in the lot. Stores the polygon boundary of the
// space and its current occupancy classification result.
//     poly     - Four corner points defining the boundary of this parking space.
//     occupied - True if a vehicle is detected; set by classifySpaces().
// ----------------------------------------------------------------------------

struct ParkingSpace {
    std::vector<cv::Point> poly;
    bool occupied = false;
};

// -------getROIs----------------------------------------------------
// Returns parking space ROIs for the given image (selects lot layout by filename)
// Pre-conditions: 1. imagePath is a non-empty string of filename
//                 2. Polygon ROIs for the target lot have already defined in
//                    parking.cpp using label_tool onan empty reference image of lot
// Post-conditions:
//  1. Returns a vector of ParkingSpace objects with poly fields set to the correct
//     pixel coordinates for that lot. All occupied flags are false.
//  2. If the filename prefix does not match, PUCPR layout is returned as the default.
// --------------------------------------------------------------------------

std::vector<ParkingSpace> getROIs(const std::string& imagePath);

// ------getThreshold--------------------------------------------------------
// Returns the Canny edge-count threshold calibrated for the lot in the given image
// Pre-conditions:
//   - imagePath is a non-empty string. Only the filename is inspected.
//   - Thresholds were determined by running the detector on a known-empty
//     image of each lot and recording the maximum edge count observed.
//
// Post-conditions:
//   - Returns a positive integer threshold value specific to the lot.
//   - Does not modify any state.
// Returns:
//   int � edge pixel count threshold.
//         ANG: 170 | UFPR04: 100 | PUCPR (default): 137
// --------------------------------------------------------------------------

int getThreshold(const std::string& imagePath);

// -----getVarianceThreshold-------------------------------------------------
// Returns the pixel stddev threshold calibrated for the lot in the given image
// Pre-conditions:
//   - imagePath is a non-empty string. Only the filename is inspected.
//   - Thresholds were determined by measuring stddev on empty spaces across
//     reference images for each lot.
//
// Post-conditions:
//   - Returns a positive double threshold value specific to the lot.
//   - Does not modify any state.
// 
// Returns:
//   double � pixel standard deviation threshold.
//            ANG: 35.0 | UFPR04: 25.0 | PUCPR (default): 35.0
// --------------------------------------------------------------------------

double getVarianceThreshold(const std::string& imagePath);

// -----classifySpaces-------------------------------------------------------
// Pre-conditions:
//   - blurred is a single-channel (grayscale) image produced by GaussianBlur.
//     It must be the same size and coordinate space as the image used to
//     define the polygon ROIs in spaces.
//   - spaces is a non-empty vector of ParkingSpace objects with valid poly
//     fields (as returned by getROIs()).
//   - edgeThreshold >= 0 and varianceThreshold >= 0.0.
//   - Any polygon that extends outside the image bounds is clamped safely;
//     entirely out-of-bounds polygons are marked empty and skipped.
//
// Post-conditions:
//   - Each space.occupied flag is set to true (car detected) or false (empty).
//   - blurred is not modified.
//   - The polygon coordinates in spaces are not modified.
// Classifies each space as occupied/empty using edge density + intensity variance.
// A space is occupied if edgeCount > edgeThreshold OR stddev > varianceThreshold.
// --------------------------------------------------------------------------

void classifySpaces(const cv::Mat& blurred, std::vector<ParkingSpace>& spaces,
                    int edgeThreshold = 93, double varianceThreshold = 10.0);

// ----drawResults----------------------------------------------------------------------
// Pre-conditions:
//   - image is a valid non-empty 3-channel BGR color image.
//   - spaces contains ParkingSpace objects with valid poly fields and
//     occupied flags already set by classifySpaces().
//   - All polygon coordinates in spaces lie within the bounds of image
//     (or at least partially overlap; OpenCV clips drawing at image edges).
//
// Post-conditions:
//   - image is modified in place with polygon outlines and text labels drawn.
//   - The poly and occupied fields of spaces are not modified.
// Draws colored bounding boxes and OCC/EMP labels onto the image
// --------------------------------------------------------------------------

void drawResults(cv::Mat& image, const std::vector<ParkingSpace>& spaces);

