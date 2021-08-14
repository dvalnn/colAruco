#include <sstream>
#include <iostream>
#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

const cv::Size chessboardSize = cv::Size(7, 12);
const float calibrationSquareSize = 0.020f;  //meters
const float arucoMarkerSize = 0.0905f;       //meters

using namespace std;

void createArucoMarkers() {
    cv::Mat outputMarker;

    cv::Ptr<cv::aruco::Dictionary> markerDictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    for (int i = 0; i < 50; i++) {
        cv::aruco::drawMarker(markerDictionary, i, 500, outputMarker);

        ostringstream fileName;
        string imageName = "4x4Marker_";
        string directoryName = "../resources/";
        fileName << directoryName << imageName << i << ".jpg";
        imwrite(fileName.str(), outputMarker);
    }
}

void createKnownBoardPositions(cv::Size boardSize, float squareEdgelength, vector<cv::Point3f>& corners) {
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
void getChessboardCorners(vector<cv::Mat> images, vector<vector<cv::Point2f>>& allFoundCorners, bool showResults) {
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

void cameracalibration(vector<cv::Mat> calibrationImages,
                       cv::Size boardSize,
                       float squareEdgeLength,
                       cv::Mat& distanceCoefficients,
                       cv::Mat& cameraMatrix) {
    vector<vector<cv::Point2f>> chessboardImageSpacePoints;
    getChessboardCorners(calibrationImages, chessboardImageSpacePoints, false);

    vector<vector<cv::Point3f>> worldSpaceCornerPoints(1);

    createKnownBoardPositions(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
    worldSpaceCornerPoints.resize(chessboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

    vector<cv::Mat> rVectors, tVectors;
    distanceCoefficients = cv::Mat::zeros(8, 1, CV_64F);

    cv::calibrateCamera(worldSpaceCornerPoints,
                        chessboardImageSpacePoints,
                        boardSize,
                        cameraMatrix,
                        distanceCoefficients,
                        rVectors,
                        tVectors);
}

int main(int argc, char** argv) {
    cv::Mat frame;

    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distanceCoefficients;

    vector<cv::Mat> savedImages;
    vector<vector<cv::Point2f>> markerCorners, rejectedCandidates;

    cv::VideoCapture vid(1);

    if (!vid.isOpened())
        return 0;

    int framesPerSecond = 30;

    cv::namedWindow("Webcam", cv::WINDOW_AUTOSIZE);

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
            cv::imshow("Webcam", drawToFrame);
        else
            cv::imshow("Webcam", frame);

        char character = cv::waitKey(1000 / framesPerSecond);

        switch (character) {
            case 13:  // ENTER
            //starting calibration
                break;

            case 27:  // ESC
            //exiting program
                break;

            case 32:  // SPACE
            //saving image
                break;
        }
    }

    return 0;
}
