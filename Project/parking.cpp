#include "parking.h"
#include <algorithm>

// ---------------------------------------------------------------------------
// UFPR04 lot — large multi-row horizontal lot
// Calibrated from 2012-10-13 (mostly empty). Image size: 640x640.
//
// Layout: 4 row-pairs, each pair = upper row + green median + lower row.
// 8 spaces per row, x from 50 to 432, w=46. 8 rows * 8 spaces = 64 ROIs.
//
// To retune: open 2012-10-13 in MS Paint, hover each space corner, read coords.
// ---------------------------------------------------------------------------
static std::vector<ParkingSpace> getROIs_UFPR04() {
    const int w = 46, h = 28;
    const int xs[] = { 50, 98, 146, 194, 242, 290, 338, 386 };
    // 4 band-pairs, 2 rows each: upper y, lower y
    const int ys[] = { 28, 65, 128, 168, 233, 270, 343, 383 };

    std::vector<ParkingSpace> spaces;
    for (int y : ys)
        for (int x : xs)
            spaces.push_back({ {
                {x,     y},
                {x + w,   y},
                {x + w,   y + h},
                {x,     y + h}
            }, false });
    return spaces;
}

// ---------------------------------------------------------------------------
// PUCPR lot — smaller diagonal/angled lot with landscaping
// Calibrated from 2012-12-08 (mostly empty). Image size: 640x640.
//
// Rectangular ROIs approximate the angled space boundaries.
// Expect some mis-alignment — tune x/y/w/h against the actual images.
// ---------------------------------------------------------------------------
static std::vector<ParkingSpace> getROIs_PUCPR() {
    std::vector<ParkingSpace> polys = {
    {{{132,55},{179,67},{167,90},{131,81}}, false},
    {{{135,82},{190,94},{179,120},{136,106}}, false},
    {{{155,114},{203,128},{193,154},{151,143}}, false},
    {{{168,150},{220,164},{207,193},{168,180}}, false},
    {{{176,187},{237,207},{224,238},{176,222}}, false},
    {{{189,229},{253,251},{242,285},{182,265}}, false},
    {{{205,281},{276,302},{263,344},{200,324}}, false},
    {{{224,338},{303,364},{291,412},{220,384}}, false},
    {{{252,401},{333,434},{320,489},{250,458}}, false},
    {{{274,475},{369,517},{359,583},{270,535}}, false},
    {{{176,56},{187,41},{230,50},{225,69}}, false},
    {{{201,67},{238,77},{234,98},{191,84}}, false},
    {{{216,96},{263,108},{254,132},{205,118}}, false},
    {{{227,130},{281,144},{275,169},{219,150}}, false},
    {{{246,166},{302,178},{297,210},{236,189}}, false},
    {{{267,207},{325,223},{314,256},{255,235}}, false},
    {{{287,251},{347,269},{337,302},{277,284}}, false},
    {{{311,302},{389,326},{376,371},{301,342}}, false},
    {{{338,362},{428,396},{417,442},{329,408}}, false},
    {{{373,433},{456,466},{444,520},{363,487}}, false},
    {{{324,20},{348,24},{334,64},{312,55}}, false},
    {{{345,43},{366,48},{353,89},{333,80}}, false},
    {{{364,71},{389,77},{377,122},{354,113}}, false},
    {{{387,101},{412,107},{401,155},{378,145}}, false},
    {{{412,132},{440,141},{429,192},{404,182}}, false},
    {{{439,168},{467,178},{462,237},{431,222}}, false},
    {{{468,213},{503,221},{498,288},{464,275}}, false},
    {{{506,254},{545,265},{537,342},{501,330}}, false},
    {{{547,309},{585,321},{584,387},{542,375}}, false},
    {{{357,2},{395,7},{392,20},{351,15}}, false},
    {{{371,21},{422,28},{418,50},{367,37}}, false},
    {{{389,46},{447,56},{445,80},{386,64}}, false},
    {{{412,74},{475,85},{473,108},{411,91}}, false},
    {{{439,103},{491,114},{488,139},{437,124}}, false},
    {{{465,136},{530,151},{529,176},{464,159}}, false},
    {{{499,174},{565,189},{562,217},{499,197}}, false},
    {{{533,215},{607,233},{602,262},{534,243}}, false},
    {{{573,264},{632,278},{630,313},{574,297}}, false},
    {{{477,9},{502,16},{498,52},{472,45}}, false},
    {{{504,34},{528,41},{527,80},{501,72}}, false},
    {{{533,58},{558,62},{556,109},{531,102}}, false},
    {{{561,87},{591,94},{591,144},{565,137}}, false},
    {{{596,116},{625,125},{632,185},{600,175}}, false},
    };
    // 43 spaces total
    // 43 spaces total

    std::vector<ParkingSpace> spaces;
    for (auto& p : polys)
        spaces.push_back({ p});
    return spaces;
}

// ---------------------------------------------------------------------------
// Route to the correct lot based on image filename date prefix.
// UFPR04: filenames starting 2012-09, 2012-10, or 2012-11
// PUCPR:  everything else (2012-12, 2013-*)
// ---------------------------------------------------------------------------
std::vector<ParkingSpace> getROIs(const std::string& imagePath) {
    // Extract just the filename (handle both / and \ separators)
    size_t sep = imagePath.find_last_of("/\\");
    std::string fname = (sep == std::string::npos) ? imagePath : imagePath.substr(sep + 1);

    bool isUFPR04 = (fname.find("2012-09") == 0 ||
                     fname.find("2012-10") == 0 ||
                     fname.find("2012-11") == 0);

    return isUFPR04 ? getROIs_UFPR04() : getROIs_PUCPR();
}

void classifySpaces(const cv::Mat& blurred, std::vector<ParkingSpace>& spaces, int threshold) {
    cv::Rect imgBounds(0, 0, blurred.cols, blurred.rows);

    for (auto& space : spaces) {
        
        // get bounding rect of the polygon
        cv::Rect bbox = cv::boundingRect(space.poly);
        
        // Clamp to image bounds — prevents crash if any ROI extends outside
        cv::Rect safe = bbox & imgBounds;
        if (safe.empty()) { space.occupied = false; continue; }

        // build a mask for the polygon within the bounding rect
        cv::Mat mask = cv::Mat::zeros(safe.height, safe.width, CV_8UC1);
        std::vector<cv::Point> shifted;
        for (const auto& p : space.poly)
            shifted.push_back(p - cv::Point(safe.x, safe.y));
        cv::fillConvexPoly(mask, shifted, cv::Scalar(255));


        // Canny
        cv::Mat roi = blurred(safe);
        cv::Mat edges;
        // Low=30/High=90: more sensitive than default, catches lower-contrast cars
        cv::Canny(roi, edges, 50, 150);
        cv::bitwise_and(edges, mask, edges);

        int edgeCount = cv::countNonZero(edges);
        space.occupied = (edgeCount > threshold);
    }
}

void drawResults(cv::Mat& image, const std::vector<ParkingSpace>& spaces) {
    for (const auto& space : spaces) {
        cv::Scalar color = space.occupied
            ? cv::Scalar(0, 0, 255)   // red  = occupied
            : cv::Scalar(0, 255, 0);  // green = empty

        // draw polygon outline
        const std::vector<cv::Point>* pts = &space.poly;
        cv::polylines(image, *pts, true, color, 2);

        // add label
        std::string label = space.occupied ? "OCC" : "EMP";
        cv::putText(image, label,
            cv::Point(space.poly[0].x + 4, space.poly[0].y + 14),
            cv::FONT_HERSHEY_SIMPLEX, 0.45, color, 1);
    }
}
