#include <opencv2/aruco.hpp>

class Settings {
   public:
    bool OK = true;
    cv::Mat cameraMatrix;
    cv::Mat distortionCoeffs;
    Settings(std::string filepath);
};