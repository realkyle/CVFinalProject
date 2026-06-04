// ---------------------------------------------------------------------------
// PARKING.H
// Authors: Jingyi Zhong, Kyle Hale
// ---------------------------------------------------------------------------
// Declares ParkingLot class, which encapsulates polygon ROIs,
// per-lot detection thresholds, and occupancy classification for
// overhead parking lot image. Correct lot layout and thresholds are
// selected automatically from image filename prefix.
// Assumptions:
//   1. Input images are taken from fixed overhead camera.
//   2. Polygon ROIs were traced on empty reference image and are
//      reused for all images from same lot and camera angle.
//   3. Correct lot is identified by image filename prefix.
//   4. Images are expected to be 640x640 pixels.
// ---------------------------------------------------------------------------

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// ---------------------------------------------------------------------------
// class ParkingLot
// Represents single parking lot loaded from named image file.
// Encapsulates polygon ROIs, per-lot thresholds, and classification
// state. Construct with image path correct lot layout and thresholds
// are selected automatically from filename prefix (ANG, UFPR, or PUCPR).
// ---------------------------------------------------------------------------
class ParkingLot {
public:

    // -----------------------------------------------------------------------
    // ParkingLot constructor
    // Pre-conditions:
    //   - imagePath is non-empty string. Only filename prefix used
    //     to identify lot (ANG, UFPR04, or PUCPR as default).
    // Post-conditions:
    //   - spaces_ is populated with polygon ROIs for identified lot.
    //   - edgeThreshold_ and varianceThreshold_ are set to per-lot thresholds.
    // -----------------------------------------------------------------------
    explicit ParkingLot(const std::string& imagePath);

    // -----------------------------------------------------------------------
    // ParkingLot::classify
    // Classifies each space as occupied or empty using canny edge density
    // and pixel intensity standard deviation. A space is occupied if either
    // signal exceeds its per-lot threshold.
    // Pre-conditions:
    //   - blurred is single-channel (grayscale) image produced by
    //     GaussianBlur, same size and coordinate space as ROI reference.
    //   - Any polygon extending outside image bounds is clamped safely,
    //     entirely out-of-bounds polygons are marked empty and skipped.
    // Post-conditions:
    //   - Each space's occupied flag is set. Blurred is not modified.
    // -----------------------------------------------------------------------
    void classify(const cv::Mat& blurred);

    // -----------------------------------------------------------------------
    // ParkingLot::drawResults
    // Draws colored polygon outlines and OCC/EMP labels onto image.
    // Pre-conditions:
    //   - image is valid non-empty 3-channel BGR image, same dimensions
    //     as image used to define polygon ROIs.
    //   - classify() has been called to set occupancy flags.
    // Post-conditions:
    //   - Image is annotated in place with red (OCC) or green (EMP) outlines.
    //   - Spaces_ is not modified.
    // -----------------------------------------------------------------------
    void drawResults(cv::Mat& image) const;

    // OccupiedCount
    // Returns number of spaces classified as occupied by classify().
    int occupiedCount() const;

    // EmptyCount
    // Returns number of spaces classified as empty by classify().
    int emptyCount() const;

    // TotalCount
    // Returns total number of polygon ROIs in this lot.
    int totalCount() const;

private:

    // -----------------------------------------------------------------------
    // Internal struct ParkingSpace
    // Internal data structure representing single parking space.
    //   poly - Four corner points defining boundary of space.
    //   occupied - True if vehicle is detected; set by classify().
    // -----------------------------------------------------------------------
    struct ParkingSpace {
        std::vector<cv::Point> poly;
        bool occupied = false;
    };

    std::vector<ParkingSpace> spaces_;
    int    edgeThreshold_;
    double varianceThreshold_;

    // Hardcoded polygon ROI sets for each supported lot.
    static std::vector<ParkingSpace> loadROIs_UFPR04();
    static std::vector<ParkingSpace> loadROIs_ANG();
    static std::vector<ParkingSpace> loadROIs_PUCPR();
};