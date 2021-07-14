#include <string>
#include <iostream>

#include "../include/settings.hpp"

using namespace std;

Settings::Settings(string filepath) {
    if (filepath.find(".yml") == string::npos or filepath.find(".xml") == string::npos) {
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