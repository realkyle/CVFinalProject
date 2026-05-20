#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "parking.h"

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

    // --- Preprocessing ---
    cv::Mat gray, blurred;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);

    // --- Classification ---
    std::vector<ParkingSpace> spaces = getROIs();
    classifySpaces(blurred, spaces);  // tune threshold in parking.cpp if needed

    // --- Draw results onto original color image ---
    drawResults(image, spaces);

    // --- Count occupied / empty ---
    int occupied = 0, empty = 0;
    for (const auto& s : spaces)
        s.occupied ? ++occupied : ++empty;

    // Draw count text -- drawn twice for a simple white-outline effect
    std::string countText = "Occupied: " + std::to_string(occupied) +
                            "   Empty: " + std::to_string(empty);
    cv::putText(image, countText, cv::Point(10, 35),
        cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(255, 255, 255), 3);
    cv::putText(image, countText, cv::Point(10, 35),
        cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0), 1);

    // --- Save output ---
    size_t sep = imagePath.find_last_of("/\\");
    std::string filename = (sep == std::string::npos) ? imagePath : imagePath.substr(sep + 1);
    std::string outputPath = "..\\output\\result_" + filename;

    if (!cv::imwrite(outputPath, image))
        std::cerr << "Warning: Could not save to: " << outputPath << std::endl;
    else
        std::cout << "Saved: " << outputPath << std::endl;

    std::cout << "Occupied: " << occupied << " | Empty: " << empty
              << " | Total: " << spaces.size() << std::endl;

    // --- Display ---
    cv::imshow("Parking Lot Detector", image);
    cv::waitKey(0);

    return 0;
}
