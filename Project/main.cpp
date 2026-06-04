// ---------------------------------------------------------------------------
// MAIN.CPP
// Authors: Jingyi Zhong, Kyle Hale
//
// Description:
//   Main file for the parking lot occupancy detector. Runs the full
//   detection pipeline on single input image provided as a command-line
//   argument. The pipeline includes the following steps:
//
//     1. Load input image from disk
//     2. Preprocess: convert grayscale and apply Gaussian blur
//     3. Load correct polygon ROIs for detected lot (via filename)
//     4. Classify each space as occupied or empty using edge density and
//        pixel variance (per-lot thresholds applied automatically)
//     5. Annotate image with colored polygon outlines and OCC/EMP labels
//     6. Overlay total occupied/empty count as text
//     7. Save annotated image to output/ folder
//     8. Display result in a window
//
// Usage:
//   Project.exe <image_path>
//   Example: Project.exe ..\images\UFPR1.jpg
//
// Assumptions:
//   - The output directory exists at <image_dir>\..\output\ relative to the
//     input image. The program will warn but not crash if it does not exist.
//   - The input image filename prefix matches one of the supported lots
//     (ANG, UFPR, 2012-09/10/11, or PUCPR as default). See parking.cpp.
//   - Only one image is processed per invocation. For batch processing,
//     call the executable in a loop from a script.
// ---------------------------------------------------------------------------

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "parking.h"

// ----------------------------------------------------------------------------
// main
// Pre-conditions:
//   1. Exactly one command-line argument is provided: the path to a valid
//     image file readable by OpenCV (JPEG, PNG, BMP, etc.).
//   2. The output directory (..\output\ relative to the image directory)
//     exists and is writable.
//   3. parking.cpp contains ROI definitions for the lot identified by the
//     image filename prefix.
//
// Post-conditions:
//   1. An annotated result image is saved to:
//       <image_dir>\..\output\result_<filename>
//   2. Occupied, empty, and total space counts are printed to stdout.
//   3. A display window shows the annotated image until any key is pressed.
//   4. Returns 0 on success, 1 on argument or image-load error.
// ----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: Project.exe <image_path>" << std::endl;
        std::cerr << "Example: Project.exe ..\\images\\test1.jpg" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Error: Could not load image: " << imagePath << std::endl;
        return 1;
    }

    // Preprocessing
    cv::Mat gray, blurred;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);

    // Classification
    ParkingLot lot(imagePath);
    lot.classify(blurred);

    // Draw results onto original color image
    lot.drawResults(image);

    // Count occupied / empty
    int occupied = lot.occupiedCount();
    int empty    = lot.emptyCount();

    // Draw count text drawn twice for simple white-outline effect
    std::string countText = "Occupied: " + std::to_string(occupied) +
                            "   Empty: " + std::to_string(empty);
    cv::putText(image, countText, cv::Point(10, 35),
        cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(255, 255, 255), 3);
    cv::putText(image, countText, cv::Point(10, 35),
        cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0), 1);

    // Save output
    size_t sep = imagePath.find_last_of("/\\");
    std::string filename = (sep == std::string::npos) ? imagePath  : imagePath.substr(sep + 1);
    std::string imageDir = (sep == std::string::npos) ? "."        : imagePath.substr(0, sep);
    std::string outputPath = imageDir + "\\..\\output\\result_" + filename;

    if (!cv::imwrite(outputPath, image))
        std::cerr << "Warning: Could not save to: " << outputPath << std::endl;
    else
        std::cout << "Saved: " << outputPath << std::endl;

    std::cout << "Occupied: " << occupied << " | Empty: " << empty
              << " | Total: " << lot.totalCount() << std::endl;

    // Display
    cv::imshow("Parking Lot Detector", image);
    cv::waitKey(0);

    return 0;
}
