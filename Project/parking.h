//----------------------------------------------------------------------------
// PARKING.H
// Authors: Jingyi Zhong, Kyle Hale
//----------------------------------------------------------------------------
// Declares the ParkingLot class, which encapsulates all polygon ROIs,
// per-lot detection thresholds, and occupancy classification for a single
// overhead parking lot image. The correct lot layout and thresholds are
// selected automatically from the image filename prefix.
//
// Assumptions:
//   1. Input images are taken from a fixed overhead camera position.
//   2. Polygon ROIs were traced on an empty reference image and are
//      reused for all images from the same lot and camera angle.
//   3. The correct lot is identified by the image filename prefix.
//   4. Images are expected to be 640x640 pixels.
//----------------------------------------------------------------------------

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

//----------------------------------------------------------------------------
// class ParkingLot
// Represents a single parking lot loaded from a named image file.
// Encapsulates all polygon ROIs, per-lot thresholds, and classification
// state. Construct with an image path — the correct lot layout and thresholds
// are selected automatically from the filename prefix (ANG, UFPR, or PUCPR).
//----------------------------------------------------------------------------
class ParkingLot {
public:

    // -----------------------------------------------------------------------
    // ParkingLot (constructor)
    // Pre-conditions:
    //   - imagePath is a non-empty string. Only the filename prefix is used
    //     to identify the lot (ANG, UFPR04, or PUCPR as default).
    // Post-conditions:
    //   - spaces_ is populated with polygon ROIs for the identified lot.
    //   - edgeThreshold_ and varianceThreshold_ are set to per-lot values.
    // -----------------------------------------------------------------------
    explicit ParkingLot(const std::string& imagePath);

    // -----------------------------------------------------------------------
    // classify
    // Classifies each space as occupied or empty using Canny edge density
    // and pixel intensity standard deviation. A space is occupied if either
    // signal exceeds its per-lot threshold.
    // Pre-conditions:
    //   - blurred is a single-channel (grayscale) image produced by
    //     GaussianBlur, same size and coordinate space as the ROI reference.
    //   - Any polygon extending outside image bounds is clamped safely;
    //     entirely out-of-bounds polygons are marked empty and skipped.
    // Post-conditions:
    //   - Each space's occupied flag is set. blurred is not modified.
    // -----------------------------------------------------------------------
    void classify(const cv::Mat& blurred);

    // -----------------------------------------------------------------------
    // drawResults
    // Draws colored polygon outlines and OCC/EMP labels onto image.
    // Pre-conditions:
    //   - image is a valid non-empty 3-channel BGR image, same dimensions
    //     as the image used to define the polygon ROIs.
    //   - classify() has already been called to set occupancy flags.
    // Post-conditions:
    //   - image is annotated in place with red (OCC) or green (EMP) outlines.
    //   - spaces_ is not modified.
    // -----------------------------------------------------------------------
    void drawResults(cv::Mat& image) const;

    // Returns the number of spaces classified as occupied by classify().
    int occupiedCount() const;

    // Returns the number of spaces classified as empty by classify().
    int emptyCount() const;

    // Returns the total number of polygon ROIs in this lot.
    int totalCount() const;

private:

    //------------------------------------------------------------------------
    // ParkingSpace
    // Internal data structure representing a single parking space.
    //   poly     - Four corner points defining the boundary of the space.
    //   occupied - True if a vehicle is detected; set by classify().
    //------------------------------------------------------------------------
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
