#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>

using namespace std;

class CameraSettings {
   public:
    bool OK = false;
    cv::Mat cameraMatrix;
    cv::Mat distortionCoeffs;

    CameraSettings(string filepath);
    bool runCalibrationAndSave(const cv::Size chessboardSize, const float calibrationSquareSize);

   private:
    string deviceName;

    bool saveCalibrationResults(string filepath, cv::Mat camMatrix, cv::Mat distCoeffs, int camIndex);
    void createKnownBoardPositions(cv::Size boardSize, float squareEdgelength, vector<cv::Point3f>& corners);
    void getChessboardCorners(vector<cv::Mat> images, vector<vector<cv::Point2f>>& allFoundCorners,
                              cv::Size chessboardSize, bool showResults);
    void cameraCalibration(vector<cv::Mat> calibrationImages, cv::Size boardSize, float squareEdgeLength,
                           cv::Mat& cameraMatrix, cv::Mat& distanceCoefficients);
};
