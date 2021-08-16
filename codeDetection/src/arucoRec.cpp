#include <map>
#include <array>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>

#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>

#include "../include/cameraSettings.hpp"
#include "../include/arucoSettings.hpp"

// ####################################################################################################################

#define DELTA 12

auto ARUCO_PARAMS = cv::aruco::DetectorParameters::create();

const std::map<std::string, int> supportedArucoTypes{
    {"4_50", cv::aruco::DICT_4X4_50},
    {"4_100", cv::aruco::DICT_4X4_100},
    {"4_250", cv::aruco::DICT_4X4_250},
    {"4_1000", cv::aruco::DICT_4X4_1000},
    {"5_50", cv::aruco::DICT_5X5_50},
    {"5_100", cv::aruco::DICT_5X5_100},
    {"5_250", cv::aruco::DICT_5X5_250},
    {"5_1000", cv::aruco::DICT_5X5_1000},
    {"6_50", cv::aruco::DICT_6X6_50},
    {"6_100", cv::aruco::DICT_6X6_100},
    {"6_250", cv::aruco::DICT_6X6_250},
    {"6_1000", cv::aruco::DICT_6X6_1000},
    {"original", cv::aruco::DICT_ARUCO_ORIGINAL},
};

// ####################################################################################################################

int dictInput(int currDict) {
    std::string userInput;
    while (not supportedArucoTypes.contains(userInput)) {
        std::cout << "Input a aruco dictionary type do detect (suported types: -h / --help): ";
        std::cin >> std::setw(10) >> userInput;

        if (std::cin.eof()) {
            std::cin.clear();
            std::cin.putback('\n');
            std::cout << "\u001b[2K\r";
        }

        if ((userInput == "-h" or userInput == "-help") and std::cin.peek() == '\n' and std::cin.good()) {
            std::cout << "Supported dict values (usage: -d=<value>)\n";
            for (auto pair : supportedArucoTypes)
                std::cout << " -- \"" << pair.first << "\"\n";
            continue;
        }

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return supportedArucoTypes.at(userInput);
}

char colorInput(char currClr) {
    char userInput = '0';
    const std::string allowedColors = "rgbw";
    while (allowedColors.find(userInput) == std::string::npos) {
        std::cout << "Input a color channel to mask (r/g/b/w): ";
        std::cin >> userInput;

        if (std::cin.eof()) {
            std::cin.clear();
            std::cin.putback('\n');
            std::cout << "\u001b[2K\r";
        }

        if (not std::cin.peek() == '\n' and not(std::cin.good())) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return userInput;
}

void maskFrame(const cv::Mat &inFrame, cv::Mat &outFrame, char targetClr, int delta = DELTA) {
    cv::Mat bgr[3];
    cv::split(inFrame, bgr);

    std::map<char, cv::Mat> clrChannels{{'r', bgr[2]}, {'g', bgr[1]}, {'b', bgr[0]}};

    cv::Mat targetCh = clrChannels[targetClr];
    cv::Mat betaCh, gammaCh;

    for (auto pair : clrChannels) {
        if (pair.first == targetClr)
            continue;
        if (betaCh.empty())
            betaCh = clrChannels[pair.first];
        else
            gammaCh = clrChannels[pair.first];
    }

    cv::Mat colorMask = (targetCh > (betaCh + delta)) & (targetCh > (gammaCh + delta));
    cv::Mat falsePositives = (betaCh < 255 - delta) & (gammaCh < 255 - delta);

    outFrame = cv::Mat::zeros(targetCh.rows, targetCh.cols, targetCh.type());
    outFrame = (255 * (colorMask & falsePositives));
}

void processFrame(const cv::Mat &inFrame, cv::Mat &outFrame, char targetClr, cv::Size kSize = cv::Size(10, 10)) {
    // normal color for the codes -- handled by default by openCV
    if (targetClr == 'w')
        return cv::cvtColor(inFrame, outFrame, cv::COLOR_BGR2GRAY);

    // run a bilateralFilter to blur the original image - helps reducing noise for future masking
    cv::bilateralFilter(inFrame, outFrame, 5, 75, 90);  //! needs revision

    // threshold image relative to the selected color channel
    maskFrame(outFrame, outFrame, targetClr);

    // kernels for dilation and erosion operations
    cv::Mat dilKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kSize);
    cv::Mat erKernel = cv::getStructuringElement(cv::MORPH_RECT, kSize);

    //image dilation and erosion for eliminating noise created by the color mask
    cv::dilate(outFrame, outFrame, dilKernel);
    cv::erode(outFrame, outFrame, erKernel);
}

void detectMarkers(CameraSettings cs, cv::Mat &original, cv::Mat &masked, cv::Ptr<cv::aruco::Dictionary> dict, float mLen) {
    std::vector<int> ids;
    std::vector<cv::Vec3d> rvecs, tvecs;
    std::vector<std::vector<cv::Point2f> > corners, rejected;

    cv::aruco::detectMarkers(masked, dict, corners, ids, ARUCO_PARAMS, rejected, cs.cameraMatrix, cs.distortionCoeffs);

    if (not corners.empty()) {
        cv::aruco::drawDetectedMarkers(original, corners, ids);
        cv::aruco::estimatePoseSingleMarkers(corners, mLen, cs.cameraMatrix, cs.distortionCoeffs, rvecs, tvecs);

        for (int i = 0; i < rvecs.size(); i++) {
            auto rvec = rvecs[i];
            auto tvec = tvecs[i];
            cv::aruco::drawAxis(original, cs.cameraMatrix, cs.distortionCoeffs, rvec, tvec, mLen / 3);
        }
    }
}

// ####################################################################################################################

void arucoRecLoop(CameraSettings cs, cv::VideoCapture &vidCap, std::string dict, float mLen) {
    cv::Mat frame, maskedFrame;

    char targetColorCh = colorInput(targetColorCh);
    int dictIndex = supportedArucoTypes.at(dict);
    auto arucoDict = cv::aruco::getPredefinedDictionary(dictIndex);
    std::cout << "Grabbing frames ... " << std::endl;

    while (true) {
        vidCap.read(frame);

        if (frame.empty()) {
            std::cout << "[FATAL] blank frame grabbed.\n";
            break;
        }

        processFrame(frame, maskedFrame, targetColorCh);
        detectMarkers(cs, frame, maskedFrame, arucoDict, mLen);

        cv::imshow("Live", frame);
        cv::imshow("Color Mask", maskedFrame);

        //----------------------- input waitkeys -----------------------
        int key = cv::waitKey(1) & 0xff;
        switch (key) {
            case 'd':
                dictIndex = dictInput(dictIndex);
                arucoDict = cv::aruco::getPredefinedDictionary(dictIndex);
                break;

            case 'c':
                targetColorCh = colorInput(targetColorCh);
                break;

            case 'q':
                return;
        }
    }
}

//* replacement for arucoCodeGen folder
// void createArucoBinary(int id) {
//     cv::Mat outputMarker;

//     auto markerDictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
//     cv::aruco::drawMarker(markerDictionary, id, 6, outputMarker);

//     short *markerBits;
//     markerBits = new short[6];

//     for (int i = 0; i < outputMarker.rows; i++) {
//         for (int j = 0; j < outputMarker.cols; j++) {
//             if (outputMarker.at<uchar>(i, j))
//                 markerBits[i] += (1 << outputMarker.cols - j - 1);
//         }
//     }

//     cout << "marker\n"
//          << outputMarker << endl
//          << "markerBits\n";

//     for (int i = 0; i < 6; i++) {
//         cout << markerBits[i] << endl;
//     }
// }

int main(int argc, char **argv) {
    const std::string keys =
        "{help h                          |      | print this message                                                 }"
        "{dict d                          | 4_50 | dictionary used for code detection                                 }"
        "{camera c                        |      | manually set webcam path in case it isn't being found by default   }"
        "{markerSquareSize ms             |      | aruco marker side lenght (in meters)                               }"
        "{calibrationSquareSize cs        | 0.02 | side lenght (in meters) of the chessboard squares (for calibration)}"
        "{calibrationVerticalCorners vc   |  7   | number of inner corners - vertical (chessboard for calibration)    }"
        "{calibrationHorizontalCorners hc |  12  | number of inner corners - horizontal (chessboard for calibration)  }";

    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("opencv video stream aruco detection");

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    if (not parser.has("markerSquareSize")) {
        parser.printMessage();
        return 0;
    }

    if (!supportedArucoTypes.contains(parser.get<std::string>("dict"))) {
        std::cout << "[FATAL] aruco tag of type {dict" << parser.get<std::string>("dict") << "} is not supported\n";
        return -1;
    }

    CameraSettings cs("../resources/calib_results.json");

    const cv::Size chessboardSize = cv::Size(parser.get<int>("vc"), parser.get<int>("hc"));
    const float calibrationSquareSize = parser.get<float>("calibrationSquareSize");

    if (not cs.OK) {
        cs.runCalibrationAndSave(chessboardSize, calibrationSquareSize);
        return 0;
    }

    cv::VideoCapture vidCap;

    if (parser.has("camera")) {
        std::cout << "[INFO] Using specified webcam path {" << parser.get<std::string>("camera") << "}\n";
        vidCap.open(parser.get<std::string>("camera"), cv::CAP_ANY);
    } else {
        for (short i = 0; i <= 10; i++) {
            std::cout << "[INFO] Searching for available video capture device on index " << i << "\n";
            vidCap.open(i);

            if (vidCap.isOpened())
                break;

            cout << "\u001b[1A"   //move cursor up one line
                 << "\r"          //move cursor to the beginning of the line
                 << "\u001b[2K";  //clear line

            if (i == 10)
                return 0;
        }
    }

    arucoRecLoop(cs, vidCap, parser.get<std::string>("dict"), std::abs(parser.get<float>("markerSquareSize")));

    return 0;
}