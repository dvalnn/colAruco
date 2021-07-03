#include <iomanip>
#include <iostream>
#include <map>
#include <opencv2/aruco.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <string>

int dictInput();
char colorInput();
auto calculatePose();
auto maskFrame();
void detectMarkers();
cv::Mat processFrame();

// Global variables
std::map<std::string, int> supportedArucoTypes{
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

    if (!supportedArucoTypes.contains(parser.get<std::string>("dict"))) {
        std::cout << "[FATAL] aruco tag type {dict" << parser.get<std::string>("dict") << "} is not supported\n";
        return -1;
    }

    auto arucoDict = cv::aruco::getPredefinedDictionary(supportedArucoTypes[parser.get<std::string>("dict")]);
    auto ARUCO_PARAMS = cv::aruco::DetectorParameters::create();

    cv::Mat frame, maskedFrame;
    cv::VideoCapture vidCap;

    std::cout << "[INFO] starting video stream. Cam index " << parser.get<int>("camera") << "\n";
    vidCap.open(parser.get<int>("camera"), cv::CAP_ANY);

    if (!vidCap.isOpened()) {
        std::cout << "[FATAL] unable to open video capture\n";
        return -1;
    }

    char targetColorCh = colorInput();
    bool verbose = false;

    while (true) {
        vidCap.read(frame);

        if (frame.empty()) {
            std::cout << "[FATAL] blank frame grabbed\n";
            break;
        }

        // maskedFrame = processFrame();
        // detectMarkers();

        cv::imshow("Live", frame);
        // cv::imshow("Processed Image", maskedFrame);

        //----------------------- input waitkeys -----------------------
        int key = cv::waitKey(1) & 0xff;
        switch (key) {
            case 'd':
                arucoDict = cv::aruco::getPredefinedDictionary(dictInput());
                break;

            case 'c':
                targetColorCh = colorInput();
                break;

            case 'v':
                verbose = verbose != true;
                break;

            case 'q':
                return 0;
        }
    }

    return 0;
}

int dictInput() {
    std::string userInput;
    while (not supportedArucoTypes.contains(userInput)) {
        std::cout << "Input a aruco dictionary type do detect (suported types: -h / --help): ";
        std::cin >> std::setw(10) >> userInput;

        if ((userInput == "-h" or userInput == "-help") and std::cin.peek() == '\n' and std::cin.good()) {
            std::cout << "Supported dict values (usage: -d=<value>)\n";
            for (auto const pair : supportedArucoTypes)
                std::cout << pair.first << std::endl;
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return supportedArucoTypes[userInput];
}

char colorInput() {
    char userInput = '0';
    const std::string allowedColors = "rgbw";
    while (allowedColors.find(userInput) == std::string::npos) {
        std::cout << "Input a color channel to mask (r/g/b/w): ";
        std::cin >> userInput;

        if (not std::cin.peek() == '\n' and not(std::cin.good())) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return userInput;
}

// void detectMarkers() {
// }

// cv::Mat processFrame() {
// }
