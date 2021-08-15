#include "../include/arucoSettings.hpp"

using namespace std;

ArucoSettings::ArucoSettings() {
    arucoParams = cv::aruco::DetectorParameters::create();
}