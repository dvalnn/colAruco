# USAGE
# python detect_aruco_video.py -t [dictType]

# import the necessary packages
import argparse

import imutils
from imutils.video import VideoStream

import cv2
import numpy as np

import sys

# construct the argument parser and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-c", "--camera", type=int, required=False,
                default=0,
                help="webcam index")
ap.add_argument("-t", "--type", type=str, required=False,
                default="dict6_100",
                help="type of ArUCo tag to detect")
args = vars(ap.parse_args())

# define names of each possible ArUco tag OpenCV supports
ARUCO_DICT = {
    "dict4_50": cv2.aruco.DICT_4X4_50,
    "dict4_100": cv2.aruco.DICT_4X4_100,
    "dict4_250": cv2.aruco.DICT_4X4_250,
    "dict4_1000": cv2.aruco.DICT_4X4_1000,
    "dict5_50": cv2.aruco.DICT_5X5_50,
    "dict5_100": cv2.aruco.DICT_5X5_100,
    "dict5_250": cv2.aruco.DICT_5X5_250,
    "dict5_1000": cv2.aruco.DICT_5X5_1000,
    "dict6_50": cv2.aruco.DICT_6X6_50,
    "dict6_100": cv2.aruco.DICT_6X6_100,
    "dict6_250": cv2.aruco.DICT_6X6_250,
    "dict6_1000": cv2.aruco.DICT_6X6_1000,
    # "dict7_50": cv2.aruco.DICT_7X7_50,  Not in use due to hardware limitations
    # "dict7_100": cv2.aruco.DICT_7X7_100,
    # "dict7_250": cv2.aruco.DICT_7X7_250,
    # "dict7_1000": cv2.aruco.DICT_7X7_1000,
    "original": cv2.aruco.DICT_ARUCO_ORIGINAL,
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
vs = VideoStream(src=args["camera"], resolution=(1920, 1080)).start()


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
    mask_chnl, ch2, ch3 = channels[colors[selected]], channels[colors[(
        selected + 1) % 3]], channels[colors[(selected + 1) % 3]]

    zeros = np.zeros(r.shape, dtype="uint8")

    colorMask = (mask_chnl > (ch2 + delta)) & (mask_chnl > (ch3 + delta))

    #  filter false positives that come up if _ + delta > 255

    falsePositives = (ch2 < 255 - delta) & (ch3 < 255 - delta)
    masked_image = zeros.copy()
    masked_image[colorMask & falsePositives] = 255

    if color == "g":
        kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (30, 30))
        masked_image = cv2.morphologyEx(masked_image, cv2.MORPH_CLOSE, kernel)
        #tentar dilate
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


def dict_inpt_parser() -> int:
    user_input = 0
    while user_input not in ARUCO_DICT.keys():
        user_input = input(
            "Input a aruco dictionary type do detect (suported types: -h / --help): ")
        if user_input.lower() in ["-h", "--help"]:
            print(str(ARUCO_DICT.keys())[str(ARUCO_DICT.keys()).index("(")+1: -1])
    return ARUCO_DICT[user_input]


def clr_inpt_parser() -> str:
    allowed_colors = ["r", "g", "b", "w"]
    user_input = 0
    while user_input not in allowed_colors:
        try:
            user_input = input("Input a color channel to mask (r/g/b/w): ")
        except EOFError:
            print("[WARN] Invalid input")

    return user_input


color = clr_inpt_parser()
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

    # if the 'i' key was pressed, pause the loop and parse the color mask input
    if key == ord("d"):
        dictType = dict_inpt_parser()
        arucoDict = cv2.aruco.Dictionary_get(dictType)
    # if the 'i' key was pressed, pause the loop and parse the color mask input
    if key == ord("i"):
        color = clr_inpt_parser()
    # if the 'q' key was pressed, break from the loop
    if key == ord("q"):
        break

# do a bit of cleanup
cv2.destroyAllWindows()
vs.stop()
