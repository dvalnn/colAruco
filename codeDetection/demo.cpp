#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <string>

int main(int argc, char **argv) {
    const std::string keys = "{help h usage ?   |           | print this message          }" 
                             "{camera c         |     0     | webcam index                }"
                             "{type t           | dict6_100 | type of Aruco tag to detect }";
    
    
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("open cv cpp demo");
    
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    cv::Mat markerImage;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::drawMarker(dictionary, 23, 200, markerImage, 1);
    cv::imwrite("marker23.png", markerImage);
}