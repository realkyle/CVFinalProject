# Parking Lot Occupancy Detector

A classical computer vision program that classifies parking spaces as **OCCUPIED** or **EMPTY**
from overhead security-camera images. Built with C++ and OpenCV 4.11.0 — no machine learning.

---

## Requirements

- Visual Studio 2022
- OpenCV 4.11.0 installed at `C:\opencv\`
- `C:\opencv\build\x64\vc16\bin` on your system PATH (for the DLL at runtime)

---

## Project Structure

```
Project/                       # Visual Studio solution root
├── Project.sln
├── README.md
├── images/                    # Input test images (JPG/PNG)
├── output/                    # Annotated result images saved here
├── docs/                      # make_writeup.py → writeup.docx, make_slides.py → slides.pptx
├── scripts/
│   └── run.bat                # Batch script to run detector on all images
├── Project/                   # C++ source
│   ├── main.cpp
│   ├── parking.cpp
│   └── parking.h
└── label_tool/
    └── label_tool.cpp         # Interactive polygon ROI tracer
```

---

## How It Works

### Pipeline

```
Input image (640×640 overhead JPEG)
    └─ Grayscale            (cv::cvtColor)
    └─ Gaussian Blur 5×5    (cv::GaussianBlur, σ=1.5)
    └─ For each parking space polygon ROI:
          └─ Build polygon mask     (cv::fillConvexPoly)
          └─ Canny edge detection   (cv::Canny, low=50, high=150)
          └─ Count edge pixels      (cv::countNonZero)  → edgeCount
          └─ Intensity stddev       (cv::meanStdDev)    → σ
          └─ OCCUPIED if edgeCount > edgeThreshold  OR  σ > varianceThreshold
    └─ Draw red (OCC) / green (EMP) polygon outlines
    └─ Save annotated image to output/
    └─ Display result window
```

### Dual-Signal Classifier

Two independent signals are combined with OR logic:

| Signal | Occupied indicator | Empty indicator |
|---|---|---|
| Canny edge count | High (car body, windows, shadow edges) | Low (uniform asphalt) |
| Intensity stddev σ | High (texture, color contrast) | Low (spectrally uniform pavement) |

A space is **OCCUPIED** if either signal exceeds its per-lot threshold.

### ROI Definition

Each parking space is defined as a **quadrilateral polygon** (four corner points) traced
interactively with `label_tool.exe`:

1. Run `label_tool.exe <empty_lot_image.jpg>`
2. Click four corners per space — the polygon auto-completes after the 4th click
3. **U** to undo, **S** to print C++ code to the console, **ESC** to quit
4. Paste the printed block into `getROIs_PUCPR()` or `getROIs_UFPR04()` in `parking.cpp`

Lot routing is automatic based on filename prefix:
- `UFPR*` or `2012-09*`, `2012-10*`, `2012-11*` → UFPR04 lot (257 spaces)
- `ANG*` → ANG lot (34 spaces)
- Everything else → PUCPR lot (43 spaces)

### Threshold Calibration

Thresholds are calibrated per lot on an empty reference image:

| Lot | Edge threshold | Variance threshold | Calibration image |
|---|---|---|---|
| PUCPR  | 137 | 35.0 | PUCPR3.jpg |
| UFPR04 | 100 | 25.0 | UFPR1.jpg  |
| ANG    | 170 | 35.0 | ANG1.jpg   |

To recalibrate: temporarily add `std::cout << "edge=" << edgeCount << " sd=" << sd << "\n";`
inside `classifySpaces()`, run on the empty reference, find the max values, and set
`threshold = max + margin`.

---

## Build

**Visual Studio:**
1. Open `Project.sln`
2. Set platform to **x64**
3. `Ctrl+Shift+B`

**PowerShell:**
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  "Project.sln" /p:Configuration=Debug /p:Platform=x64 /t:Build /v:minimal
```

---

## Run

```powershell
# Add OpenCV to PATH (once per terminal session)
$env:Path += ";C:\opencv\build\x64\vc16\bin"

cd Project\x64\Debug

# Single image
.\Project.exe "..\..\images\PUCPR1.jpg"

# All images at once
cd ..\..\scripts
.\run.bat
```

Press any key in the display window to advance to the next image when running the batch script.

---

## Results

| Image | Lot | Condition | Occupied | Empty | Total |
|---|---|---|---|---|---|
| PUCPR3.jpg | PUCPR  | Empty reference | 0   | 43  | 43  |
| PUCPR1.jpg | PUCPR  | Partial         | 26  | 17  | 43  |
| PUCPR2.jpg | PUCPR  | Heavy           | 32  | 11  | 43  |
| UFPR1.jpg  | UFPR04 | Empty reference | 0   | 257 | 257 |
| UFPR2.jpg  | UFPR04 | Partial         | 120 | 137 | 257 |
| UFPR3.jpg  | UFPR04 | Heavy           | 212 | 45  | 257 |
| ANG1.jpg   | ANG    | Empty reference | 0   | 34  | 34  |
| ANG2.jpg   | ANG    | Partial         | 8   | 26  | 34  |
| ANG3.jpg   | ANG    | Heavy           | 29  | 5   | 34  |

---

## Dataset

Test images are from the **PKLot dataset** (De Almeida et al., 2015) — overhead parking lot
images from PUCPR and UFPR (Brazil) captured under varied lighting and occupancy conditions.

---

## Key OpenCV Functions

| Function | Purpose |
|---|---|
| `cv::imread` | Load image from disk |
| `cv::cvtColor` | BGR → grayscale |
| `cv::GaussianBlur` | Smooth image before edge detection |
| `cv::fillConvexPoly` | Build polygon mask for each space |
| `cv::Canny` | Detect edges within masked ROI |
| `cv::countNonZero` | Count edge pixels |
| `cv::meanStdDev` | Compute intensity variance within mask |
| `cv::polylines` | Draw polygon outlines on result image |
| `cv::putText` | Overlay OCC/EMP labels and count |
| `cv::imwrite` | Save annotated result image |
| `cv::imshow` | Display result window |
