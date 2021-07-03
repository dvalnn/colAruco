#include <map>
#include <string>
#include <iostream>

#include <opencv2/aruco.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgcodecs.hpp>

int main(int argc, char **argv) {
    const std::string keys =
        "{help h usage ?   |           | print this message          }"
        "{camera c         |     0     | webcam index                }"
        "{type t           | dict6_100 | type of Aruco tag to detect }";

    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("opencv video stream aruco detection");

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::map<std::string, int> ARUCO_DICT{
        {"dict4_50", cv::aruco::DICT_4X4_50},
        {"dict4_100", cv::aruco::DICT_4X4_100},
        {"dict4_250", cv::aruco::DICT_4X4_250},
        {"dict4_1000", cv::aruco::DICT_4X4_1000},
        {"dict5_50", cv::aruco::DICT_5X5_50},
        {"dict5_100", cv::aruco::DICT_5X5_100},
        {"dict5_250", cv::aruco::DICT_5X5_250},
        {"dict5_1000", cv::aruco::DICT_5X5_1000},
        {"dict6_50", cv::aruco::DICT_6X6_50},
        {"dict6_100", cv::aruco::DICT_6X6_100},
        {"dict6_250", cv::aruco::DICT_6X6_250},
        {"dict6_1000", cv::aruco::DICT_6X6_1000},
        {"original", cv::aruco::DICT_ARUCO_ORIGINAL},
    };

    if (!ARUCO_DICT.contains(parser.get<std::string>("type"))) {
        std::cout << "[FATAL] aruco tag type {" << parser.get<std::string>("type") << "} is not supported\n";
        return 0;
    }

    cv::Mat markerImage;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::drawMarker(dictionary, 23, 200, markerImage, 1);
    cv::imwrite("marker23.png", markerImage);
}