#include <iostream>

#include "../include/settings.hpp"

using namespace std;

CameraSettings::CameraSettings(string filepath) {
    if (filepath.find(".yml") == string::npos and filepath.find(".xml") == string::npos) {
        OK = false;
        cout << "[ERROR] Camera settings file must be supported by opencv (.yml | .xml | .json)" << endl;
        return;
    }

    cv::FileStorage fs(filepath, cv::FileStorage::READ);

    fs["Camera_Matrix"] >> cameraMatrix;
    fs["Distortion_Coefficients"] >> distortionCoeffs;

    if (cameraMatrix.empty() or distortionCoeffs.empty()) {
        OK = false;
        cout << "[ERROR] Invalid data (" << filepath << ")" << endl;
    }
}

ArucoSettings::ArucoSettings() {
    arucoParams = cv::aruco::DetectorParameters::create();
}