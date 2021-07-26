#include <iostream>

#include "../include/cameraSettings.hpp"

using namespace std;

CameraSettings::CameraSettings(){};

CameraSettings::CameraSettings(string filepath) {
    if (filepath.find(".yml") == string::npos and filepath.find(".xml") == string::npos) {
        cout << "[ERROR] Camera settings file must be supported by opencv (.yml | .xml | .json)" << endl;
        return;
    }

    cv::FileStorage fs(filepath, cv::FileStorage::READ);

    fs["Camera_Matrix"] >> cameraMatrix;
    fs["Distortion_Coefficients"] >> distortionCoeffs;

    if (cameraMatrix.empty() or distortionCoeffs.empty()) {
        cout << "[ERROR] Invalid data (" << filepath << ")" << endl;
    }

    OK = true;
}
