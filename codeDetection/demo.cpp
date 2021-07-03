#include <iostream>
#include <map>
#include <opencv2/aruco.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgcodecs.hpp>
#include <string>

int main(int argc, char **argv) {
    const std::string keys =
        "{help h usage ?   |            | print this message                                }"
        "{camera c         |      0     | webcam index                                      }"
        "{dict d           |   6_1000   | Type and size of aruco dict to use for detection  }";

    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("opencv video stream aruco detection");

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::map<std::string, int> ARUCO_DICT{
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

    if (!ARUCO_DICT.contains(parser.get<std::string>("type"))) {
        std::cout << "[FATAL] aruco tag type {dict" << parser.get<std::string>("type") << "} is not supported\n";
        return 0;
    }

    cv::Mat markerImage;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::drawMarker(dictionary, 23, 200, markerImage, 1);
    cv::imwrite("marker23.png", markerImage);
}