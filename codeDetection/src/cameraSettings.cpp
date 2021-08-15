#include <iostream>

#include "../include/cameraSettings.hpp"

using namespace std;

#define INVALID_PATH_ERROR_MSG "[ERROR] Camera settings file must be supported by opencv (.yml | .xml | .json)"

bool filenameIsValid(const string filename) {
    return filename.ends_with(".yml") or filename.ends_with(".xml") or filename.ends_with(".json");
}

void CameraSettings::createFile(string filepath, cv::Mat camMatrix, cv::Mat distCoeffs) {
    if (not filenameIsValid(filepath)) {
        cout << INVALID_PATH_ERROR_MSG << endl;
        return;
    }

    cv::FileStorage fs(filepath, cv::FileStorage::WRITE);
    if (not fs.isOpened()) {
        cout << "[ERROR] could not open specified file (" << filepath << ") " << endl;
        return;
    }

    fs << "Camera_Matrix" << camMatrix;
    fs << "Distortion_Coefficients" << distCoeffs;
}

CameraSettings::CameraSettings(string filepath) {
    if (not filenameIsValid(filepath)) {
        cout << INVALID_PATH_ERROR_MSG << endl;
        return;
    }

    cv::FileStorage fs(filepath, cv::FileStorage::READ);

    if (not fs.isOpened()) {
        cout << "[ERROR] could not open specified file (" << filepath << ") " << endl;
        return;
    }

    fs["Camera_Matrix"] >> cameraMatrix;
    fs["Distortion_Coefficients"] >> distortionCoeffs;

    cout << "Camera_Matrix" << cameraMatrix << endl;
    cout << "Distortion_Coefficients" << distortionCoeffs << endl;

    if (cameraMatrix.empty() or distortionCoeffs.empty()) {
        cout << "[ERROR] Invalid data (" << filepath << ")" << endl;
        OK = false;
        return;
    }

    OK = true;
}
