#include <array>
#include <iomanip>
#include <iostream>
#include <map>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <string>

int dictInput();
char colorInput();
auto calculatePose();
void detectMarkers();
void maskFrame(const cv::Mat &inFrame, cv::Mat &outFrame, char targetClr, int delta = 12);
void processFrame(const cv::Mat &inFrame, cv::Mat &outFrame, char targetClr, cv::Size kSize = cv::Size(10, 10));
template <typename T, size_t n, size_t m>
using matrix = std::array<std::array<T, m>, n>;

// Global variables
const matrix<float, 3, 3> cameraMatrix = {752.461885, 0, 363.097359, 0, 513.308335, 242.851570, 0, 0, 1};
const matrix<float, 1, 5> distCoeffs = {0.050106, 0.045766, -0.019956, 0.022466, 0.000000};

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

    auto arucoDict = cv::aruco::getPredefinedDictionary(supportedArucoTypes.at(parser.get<std::string>("dict")));
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

        processFrame(frame, maskedFrame, targetColorCh);
        detectMarkers();

        cv::imshow("Live", frame);
        cv::imshow("ProcessedFrame", maskedFrame);
        

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
            for (auto pair : supportedArucoTypes)
                std::cout << " -- \"" << pair.first << "\"" << std::endl;
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    return supportedArucoTypes.at(userInput);
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

void maskFrame(const cv::Mat &inFrame, cv::Mat &outFrame, char targetClr, int delta) {
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

void processFrame(const cv::Mat &inFrame, cv::Mat &outFrame, char targetClr, cv::Size kSize) {
    // run a bilateralFilter to blur the original image - helps reducing noise for future masking
    cv::bilateralFilter(inFrame, outFrame, 15, 75, 90);

    // if the target color is "white", image is only converted to grayscale -- no further masking is needed
    if (targetClr == 'w')
        return cv::cvtColor(outFrame, outFrame, cv::COLOR_BGR2GRAY);

    // threshold image relative to the selected color channel
    // masked_image = mask_frame(blurred_image, target_color_channel)
    maskFrame(outFrame, outFrame, targetClr);

    // cv::getStructuringElement(cv::MORPH_ELLIPSE, kernelSize);
    cv::Mat dilKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, kSize);
    cv::Mat erKernel = cv::getStructuringElement(cv::MORPH_RECT, kSize);

    // image dilation with eliptical kernel will help close possible black spots inside the code squares
    // created due to excessive glow in the center of the pixel (where the led is located)
    cv::dilate(outFrame, outFrame, dilKernel);

    // image erosion with rectangular kernel to try and correct the proportions of black and white squares
    // in the masked/thresholded image -- white squares tend have bigger area than the black squares
    cv::erode(outFrame, outFrame, erKernel);
}

// void detectMarkers() {
// }
