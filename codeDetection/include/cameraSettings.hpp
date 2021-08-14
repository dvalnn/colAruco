#include <map>
#include <string>

#include <opencv2/aruco.hpp>
class CameraSettings {
   private:
    bool OK = false;
    cv::Mat cameraMatrix;
    cv::Mat distortionCoeffs;

   public:
    bool isValid() const;
    cv::Mat getCamMatrix() const;
    cv::Mat getDistCoeffs() const;

    CameraSettings(std::string filepath);
    void createFile(std::string filepath, cv::Mat camMatrix, cv::Mat distCoeffs);
};
