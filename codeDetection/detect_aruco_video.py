# USAGE
# python detect_aruco_video.py -t [dictType]

# import the necessary packages
from imutils.video import VideoStream
import argparse
import imutils
import cv2
import numpy as np
import sys

# construct the argument parser and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-t", "--type", type=str, required=True,
                default="DICT_ARUCO_ORIGINAL",
                help="type of ArUCo tag to detect")
args = vars(ap.parse_args())

# define names of each possible ArUco tag OpenCV supports
ARUCO_DICT = {
    "DICT_4X4_50": cv2.aruco.DICT_4X4_50,
    "DICT_4X4_100": cv2.aruco.DICT_4X4_100,
    "DICT_4X4_250": cv2.aruco.DICT_4X4_250,
    "DICT_4X4_1000": cv2.aruco.DICT_4X4_1000,
    "DICT_5X5_50": cv2.aruco.DICT_5X5_50,
    "DICT_5X5_100": cv2.aruco.DICT_5X5_100,
    "DICT_5X5_250": cv2.aruco.DICT_5X5_250,
    "DICT_5X5_1000": cv2.aruco.DICT_5X5_1000,
    "DICT_6X6_50": cv2.aruco.DICT_6X6_50,
    "DICT_6X6_100": cv2.aruco.DICT_6X6_100,
    "DICT_6X6_250": cv2.aruco.DICT_6X6_250,
    "DICT_6X6_1000": cv2.aruco.DICT_6X6_1000,
    # "DICT_7X7_50": cv2.aruco.DICT_7X7_50,
    # "DICT_7X7_100": cv2.aruco.DICT_7X7_100,
    # "DICT_7X7_250": cv2.aruco.DICT_7X7_250,
    # "DICT_7X7_1000": cv2.aruco.DICT_7X7_1000,
    "DICT_ARUCO_ORIGINAL": cv2.aruco.DICT_ARUCO_ORIGINAL,
}

ARUCO_SIZES = {
    "DICT_4X4_50":  4,
    "DICT_4X4_100": 4,
    "DICT_4X4_250": 4,
    "DICT_4X4_1000": 4,
    "DICT_5X5_50": 5,
    "DICT_5X5_100": 5,
    "DICT_5X5_250": 5,
    "DICT_5X5_1000": 5,
    "DICT_6X6_50": 6,
    "DICT_6X6_100": 6,
    "DICT_6X6_250": 6,
    "DICT_6X6_1000": 6,
    # "DICT_7X7_50": 7,
    # "DICT_7X7_100": 7,
    # "DICT_7X7_250": 7,
    # "DICT_7X7_1000": 7,
    "DICT_ARUCO_ORIGINAL": 5
}

if args["type"] not in ARUCO_DICT:
    print("[FATAL] ArUCo tag of '{}' is not supported".format(
        args["type"]))
    sys.exit(0)

# load the ArUCo dictionary and grab the ArUCo parameters
print("[INFO] detecting '{}' tags...".format(args["type"]))
arucoDict = cv2.aruco.Dictionary_get(ARUCO_DICT[args["type"]])
arucoParams = cv2.aruco.DetectorParameters_create()

# initialize the video stream and allow the camera sensor to warm up
print("[INFO] starting video stream...")
vs = VideoStream(src=1, resolution=(1920, 1080)).start()


def mask(frame: np.ndarray, color: str, delta: int) -> np.ndarray:

    if color == "w":
        return cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    b, g, r = frame[:, :, 0], frame[:, :, 1], frame[:, :, 2]

    # if color == "w":
    #     return b + g + r

    channels = {"r": r,
                "g": g,
                "b": b}

    colors = ["r", "g", "b"]
    selected = colors.index(color)
    x, y, z = channels[colors[selected]], channels[colors[(
        selected + 1) % 3]], channels[colors[(selected + 1) % 3]]

    zeros = np.zeros(r.shape, dtype="uint8")

    colorMask = (x > (y + delta)) & (x > (z + delta))

    #  filter false positives that come up if _ + delta > 255

    falsePositives = (y < 255 - delta) & (z < 255 - delta)
    masked_image = zeros.copy()
    masked_image[colorMask & falsePositives] = 255

    if color == "g":
        kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (30, 30))
        masked_image = cv2.morphologyEx(masked_image, cv2.MORPH_CLOSE, kernel)

    return masked_image


def drawDetectionLines(frame, corners, ids):
    # loop over the detected ArUCo corners
    for (markerCorner, markerID) in zip(corners, ids):
        # extract the marker corners (which are always returned
        # in top-left, top-right, bottom-right, and bottom-left
        # order)
        corners = markerCorner.reshape((4, 2))
        (topLeft, topRight, bottomRight, bottomLeft) = corners

        # convert each of the (x, y)-coordinate pairs to integers
        topRight = (int(topRight[0]), int(topRight[1]))
        bottomRight = (int(bottomRight[0]), int(bottomRight[1]))
        bottomLeft = (int(bottomLeft[0]), int(bottomLeft[1]))
        topLeft = (int(topLeft[0]), int(topLeft[1]))

        # draw the bounding box of the ArUCo detection
        cv2.line(frame, topLeft, topRight, (0, 255, 0), 5)
        cv2.line(frame, topRight, bottomRight, (0, 255, 0), 5)
        cv2.line(frame, bottomRight, bottomLeft, (0, 255, 0), 5)
        cv2.line(frame, bottomLeft, topLeft, (0, 255, 0), 5)

        # compute and draw the center (x, y)-coordinates of the
        # ArUco marker
        cX = int((topLeft[0] + bottomRight[0]) / 2.0)
        cY = int((topLeft[1] + bottomRight[1]) / 2.0)
        cv2.circle(frame, (cX, cY), 4, (0, 0, 255), -1)

        # draw the ArUco marker ID on the frame
        cv2.putText(frame, "  id = " + str(markerID),
                    (topLeft[0], topLeft[1] - 15),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    1, (0, 255, 0), 3)


def input_parser() -> str:
    allowed_colors = ["r", "g", "b", "w"]
    user_input = 0
    while user_input not in allowed_colors:
        try:
            user_input = input("Input a color channel to mask (r/g/b/w): ")
        except EOFError:
            print("[WARN] Invalid input")

    return user_input


color = input_parser()
# loop over the frames from the video stream
while True:
    # grab the frame from the threaded video stream and resize it
    # to have a maximum width of 600 pixels
    frame = vs.read()

    frame = imutils.resize(frame, width=1000)
    frame = cv2.bilateralFilter(frame, 15, 75, 90)

    if frame is None:
        print("[FATAL] camera feed not found")
        sys.exit(0)

    masked_image = mask(frame, color, 20)
    masked_image = cv2.bilateralFilter(masked_image, 15, 75, 90)

    # detect ArUco markers in the input frame
    (corners, ids, rejected) = cv2.aruco.detectMarkers(
        masked_image, arucoDict, parameters=arucoParams)

    # verify *at least* one ArUco marker was detected
    if len(corners) > 0:
        # flatten the ArUco IDs list
        ids = ids.flatten()
        drawDetectionLines(frame, corners, ids)
        # drawDetectionLines(masked_image, corners, ids)

    # show the output frame
    cv2.imshow("Frame", frame)
    cv2.imshow("Masked Image", masked_image)
    key = cv2.waitKey(1) & 0xFF
    if key == ord("i"):
        color = input_parser()
    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break

# do a bit of cleanup
cv2.destroyAllWindows()
vs.stop()
