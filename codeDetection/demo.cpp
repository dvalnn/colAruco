#include <opencv2/core/core.hpp>
#include <opencv2/viz/vizcore.hpp>

int main()
{
    cv::viz::Viz3d window = cv::viz::Viz3d("Viz demonstration");

    cv::Point3d min(0.25, 0.0, 0.25);
    cv::Point3d max(0.75, 0.5, 0.75);

    cv::viz::WCube cube(min, max, true, cv::viz::Color::blue());
    cube.setRenderingProperty(cv::viz::LINE_WIDTH, 4.0);

    window.showWidget("Axis widget", cv::viz::WCoordinateSystem());
    window.showWidget("Cube widget", cube);

    while(!window.wasStopped()) {
        window.spinOnce(1, true);
    }

    return 0;
}

// #include <opencv2/aruco.hpp>
// #include <opencv2/opencv.hpp>

// /*
//     g++ -I/usr/local/include/opencv4 \
//         -L/usr/local/lib \
//         -lopencv_aruco \
//         -lopencv_core \
//         -lopencv_imgproc \
//         -lopencv_highgui \
//         cv2test.cpp -o test
// */

// int main(int argc, char **argv) {
//     cv::Mat markerImage;
//     cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
//     cv::aruco::drawMarker(dictionary, 23, 200, markerImage, 1);
//     cv::imwrite("marker23.png", markerImage);
// }