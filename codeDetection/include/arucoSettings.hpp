#include <map>
#include <string>

#include <opencv2/aruco.hpp>
class CameraSettings {
   public:
    bool OK = true;
    cv::Mat cameraMatrix;
    cv::Mat distortionCoeffs;

    CameraSettings();
    CameraSettings(std::string filepath);
};

class ArucoSettings {
   public:
    bool OK = true;
    float squareSize;

    cv::Ptr<cv::aruco::Dictionary> arucoDict;
    cv::Ptr<cv::aruco::DetectorParameters> arucoParams;

    const std::map<std::string, int> supportedArucoDictionaries{
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

    ArucoSettings();
};