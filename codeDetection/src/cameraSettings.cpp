#include <iostream>
#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

#include "../include/cameraSettings.hpp"

using namespace std;

#define MAX_VIDEO_CAPTURE 64
#define INVALID_PATH_ERROR_MSG "[ERROR] Camera settings file must be supported by opencv (.yml | .xml | .json)"

bool filenameIsValid(const string filename) {
    return filename.ends_with(".yml") or filename.ends_with(".xml") or filename.ends_with(".json");
}

/**
 * @brief Import cameraSettings from previously existing file
 * 
 * @param filepath 
 */
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

    fs["Device_Name"] >> deviceName;
    fs["Camera_Matrix"] >> cameraMatrix;
    fs["Distortion_Coefficients"] >> distortionCoeffs;

    if (cameraMatrix.empty() or distortionCoeffs.empty()) {
        cout << "[ERROR] Invalid calibration data (loaded from: " << filepath << ")" << endl;
        return;
    }

    OK = true;
}

bool CameraSettings::saveCalibrationResults(string filepath, cv::Mat camMatrix, cv::Mat distCoeffs, int camIndex) {
    if (not filenameIsValid(filepath)) {
        cout << INVALID_PATH_ERROR_MSG << endl;
        return false;
    }
    //? vid.getBackendName ??
    cv::FileStorage fs(filepath, cv::FileStorage::WRITE);
    if (not fs.isOpened()) {
        cout << "[ERROR] could not open specified file (" << filepath << ") " << endl;
        return false;
    }

    fstream videoDevice;
    videoDevice.open("/sys/class/video4linux/video" + std::to_string(camIndex) + "/name", ios::in);

    char videoDeviceName[1024];
    videoDevice.getline(videoDeviceName, 1024);

    fs << "Device_Name" << videoDeviceName;
    fs << "Camera_Matrix" << camMatrix;
    fs << "Distortion_Coefficients" << distCoeffs;

    return true;
}

void CameraSettings::createKnownBoardPositions(cv::Size boardSize, float squareEdgelength,
                                               vector<cv::Point3f>& corners) {
    for (int i = 0; i < boardSize.height; i++) {
        for (int j = 0; j < boardSize.width; j++) {
            corners.push_back(cv::Point3f(j * squareEdgelength, i * squareEdgelength, 0.0f));  // z value is always 0
        }
    }
}

/**
 * @brief Get all the chessboard corners from a list of images
 * 
 * @param images 
 * @param allFoundCorners 
 * @param showResults 
 */
void CameraSettings::getChessboardCorners(vector<cv::Mat> images, vector<vector<cv::Point2f>>& allFoundCorners,
                                          cv::Size chessboardSize, bool showResults) {
    for (vector<cv::Mat>::iterator iter = images.begin(); iter != images.end(); iter++) {
        vector<cv::Point2f> pointBuffer;
        auto detectionFlags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;
        bool found = cv::findChessboardCorners(*iter, chessboardSize, pointBuffer, detectionFlags);

        if (found)
            allFoundCorners.push_back(pointBuffer);

        if (showResults) {
            cv::drawChessboardCorners(*iter, chessboardSize, pointBuffer, found);
            cv::imshow("looking for corners", *iter);
            cv::waitKey(0);
        }
    }
}

void CameraSettings::cameraCalibration(vector<cv::Mat> calibrationImages, cv::Size boardSize, float squareEdgeLength,
                                       cv::Mat& cameraMatrix, cv::Mat& distanceCoefficients) {
    vector<vector<cv::Point2f>> chessboardImageSpacePoints;
    getChessboardCorners(calibrationImages, chessboardImageSpacePoints, boardSize, false);

    vector<vector<cv::Point3f>> worldSpaceCornerPoints(1);

    createKnownBoardPositions(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
    worldSpaceCornerPoints.resize(chessboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

    vector<cv::Mat> rVectors, tVectors;
    distanceCoefficients = cv::Mat::zeros(8, 1, CV_64F);

    cv::calibrateCamera(worldSpaceCornerPoints, chessboardImageSpacePoints, boardSize,
                        cameraMatrix, distanceCoefficients, rVectors, tVectors);
}

bool CameraSettings::runCalibrationAndSave(const cv::Size chessboardSize, const float calibrationSquareSize) {
    cv::Mat frame;

    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distortionCoefficients;

    vector<cv::Mat> savedImages;
    vector<vector<cv::Point2f>> markerCorners, rejectedCandidates;

    cv::VideoCapture vid;
    for (int i = 0; i < MAX_VIDEO_CAPTURE; i++) {
        vid.open(i);
        if (vid.isOpened())
            break;

        cout << "\u001b[1A"   //move cursor up one line
             << "\r"          //move cursor to the beginning of the line
             << "\u001b[2K";  //clear line
    }

    if (!vid.isOpened()) {
        cout << "webcam not found" << endl;
        return 0;
    }

    int framesPerSecond = 30;
    int nImages = 0;

    cv::namedWindow("Webcam - calibration mode", cv::WINDOW_AUTOSIZE);

    while (true) {
        if (!vid.read(frame))
            break;

        vector<cv::Vec2f> foundPoints;
        auto detectionFlags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;
        bool found = cv::findChessboardCorners(frame, chessboardSize, foundPoints, detectionFlags);

        cv::Mat drawToFrame;
        frame.copyTo(drawToFrame);
        cv::drawChessboardCorners(drawToFrame, chessboardSize, foundPoints, found);

        if (found)
            cv::imshow("Webcam - calibration mode", drawToFrame);
        else
            cv::imshow("Webcam - calibration mode", frame);

        char character = cv::waitKey(1000 / framesPerSecond);

        switch (character) {
            case 13:  // ENTER - starting calibration
                if (savedImages.size() >= 30) {
                    string filename = "../resources/calib_results.json";

                    cameraCalibration(savedImages, chessboardSize, calibrationSquareSize, cameraMatrix, distortionCoefficients);
                    cout << "Calibration successful\nSaving results to " << filename << endl;

                    this->cameraMatrix = cameraMatrix;
                    this->distortionCoeffs = distortionCoefficients;

                    if (!saveCalibrationResults(filename, cameraMatrix, distortionCoefficients, 1))  //! change to actuall id
                        cout << "Failed to save calibration results to " << filename << endl;

                    cout << "Press any key to end calibration" << endl;

                    cv::waitKey(0);
                    return true;
                }

            case 27:  // ESC - exiting calibration
                return 0;

            case 32:  // SPACE - saving image
                cv::Mat temp;
                frame.copyTo(temp);
                savedImages.push_back(temp);
                cout << "image saved (" << ++nImages << "/30)\n";
                break;
        }
    }

    return 0;
}