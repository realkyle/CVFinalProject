# Parking Lot Occupancy Detector

A classical computer vision program that classifies parking spaces as **OCCUPIED** or **EMPTY**
from overhead or security-camera images. Built with C++ and OpenCV 4 — no machine learning.

---

## Requirements

- Visual Studio 2022
- OpenCV 4 installed at `C:\opencv` (tested with 4.11.0)
- `C:\opencv\build\x64\vc16\bin` must be on your system PATH so the DLL is found at runtime

---

## Project Structure

```
Project/                       # Visual Studio solution root (this folder)
├── Project.sln
├── README.md
├── images/                    # Input test images (JPG/PNG)
├── output/                    # Annotated result images saved here
├── docs/                      # Write-up and slides
├── scripts/
│   └── run.bat                # Batch script to run detector on all images
└── Project/                   # C++ source
    ├── main.cpp
    ├── parking.cpp
    └── parking.h
```

---

## How It Works

The detector uses **Canny edge density** to decide whether a parking space is occupied.
The idea: a car sitting in a space creates edges from its body, windows, and shadow, while
an empty asphalt space is visually uniform and produces very few edges.

### Pipeline

```
Input image
    └─ Grayscale conversion        (cv::cvtColor)
    └─ Gaussian Blur 5x5           (cv::GaussianBlur, reduces noise before edge detection)
    └─ For each parking space ROI:
          └─ Extract sub-image     (cv::Mat roi = blurred(space.roi))
          └─ Canny edge detection  (cv::Canny, low=50 high=150)
          └─ Count edge pixels     (cv::countNonZero)
          └─ edge count > 150?
                ├─ YES → OCCUPIED  (red bounding box + "OCC" label)
                └─ NO  → EMPTY     (green bounding box + "EMP" label)
    └─ Overlay "Occupied: X  Empty: Y" count text
    └─ Save annotated image to output/
    └─ Display result window
```

### Why Edge Density?

- Parked cars have strong structural edges (roof lines, windows, wheel arches, shadows).
- Empty spaces are bare asphalt — low texture, low edge count after blurring.
- Canny with Gaussian pre-blur suppresses lighting noise while preserving real object boundaries,
  making the threshold relatively stable across different lighting conditions.

### ROI Definition

Parking space regions are **manually defined** as `cv::Rect(x, y, width, height)` coordinates
hardcoded in `parking.cpp`. The function `getROIs(imagePath)` automatically selects the correct
layout based on the image filename: UFPR04 lot for filenames starting with `2012-09`, `2012-10`,
or `2012-11`; PUCPR lot otherwise.

To find coordinates: open the image in MS Paint, hover over the corners of each space —
the pixel coordinate is shown in the bottom-left status bar.

### Threshold Tuning

The occupancy threshold (default **150 edge pixels**) can be adjusted in `parking.h`:

```cpp
void classifySpaces(const cv::Mat& blurred, std::vector<ParkingSpace>& spaces, int threshold = 150);
```

To tune: add a `std::cout << edgeCount` print inside `classifySpaces`, run on a known-empty
and known-occupied space, and pick a value between the two readings.

---

## Build Instructions

1. Open `Project.sln` in Visual Studio 2022
2. Set the platform to **x64** (top toolbar dropdown)
3. Set configuration to **Debug** or **Release**
4. Press `Ctrl+Shift+B` to build

The executable is output to `Project/x64/Debug/Project.exe` (or `Release`).

---

## Running the Program

### Single image (from Visual Studio)

Go to **Project Properties → Debugging → Command Arguments** and enter:

```
..\images\your_image.jpg
```

Then press `Ctrl+F5` to run.

### Single image (from terminal)

```cmd
cd Project\x64\Debug
Project.exe ..\..\images\test1.jpg
```

### All images at once (batch script)

```cmd
cd Project\scripts
run.bat
```

This loops over every `.jpg` and `.png` in `images\` and saves annotated results to `output\`.

---

## Output

Each run produces:
- A result image saved to `output\result_<original_filename>`
- Console output: `Occupied: X | Empty: Y | Total: Z`
- A display window showing the annotated image (press any key to close)

Red boxes = occupied spaces, green boxes = empty spaces.

---

## Dataset

Test images are from the **PKLot dataset** — overhead parking lot images captured under
varied conditions (sunny, overcast, shadowed, different fill levels).

---

## Key OpenCV Functions Used

| Function | Purpose |
|---|---|
| `cv::imread` | Load image from disk |
| `cv::cvtColor` | Convert BGR to grayscale |
| `cv::GaussianBlur` | Smooth image to reduce noise |
| `cv::Canny` | Detect edges in each parking space ROI |
| `cv::countNonZero` | Count edge pixels to measure texture density |
| `cv::rectangle` | Draw colored bounding boxes |
| `cv::putText` | Overlay OCC/EMP labels and count text |
| `cv::imwrite` | Save annotated result image |
| `cv::imshow` | Display result in a window |
