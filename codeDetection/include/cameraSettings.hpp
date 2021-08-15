#include <map>
#include <string>

#include <opencv2/aruco.hpp>
class CameraSettings {
   public:
    bool OK = false;
    cv::Mat cameraMatrix;
    cv::Mat distortionCoeffs;

    CameraSettings(std::string filepath);
    void createFile(std::string filepath, cv::Mat camMatrix, cv::Mat distCoeffs);
};
