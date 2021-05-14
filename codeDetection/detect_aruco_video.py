#!/usr/bin/python3.8

# USAGE
# python detect_aruco_video.py -t [dictType]

# import the necessary packages
import argparse

import imutils
from imutils.video import VideoStream

import cv2
import numpy as np

from sys import exit

#! REFATORAR O CÓDIGO -- confusão total

###################################################################################################
################################## FUNCTION DECLARATION ###########################################


def dictInputParser() -> int:
    user_input = 0
    while user_input not in ARUCO_DICT.keys():
        user_input = input("Input a aruco dictionary type do detect (suported types: -h / --help): ")
        if user_input.lower() in ["-h", "--help"]:
            print(str(ARUCO_DICT.keys())[str(ARUCO_DICT.keys()).index("(") + 1 : -1])
    return ARUCO_DICT[user_input]


def clrInputParser() -> str:
    allowed_colors = ["r", "g", "b", "w"]
    user_input = 0
    while user_input not in allowed_colors:
        try:
            user_input = input("Input a color channel to mask (r/g/b/w): ")
        except EOFError:
            print("[WARN] Invalid input")

    return user_input


def mask(frame: np.ndarray, color: str, delta: int, dilate: bool = False, kernel_size: tuple = (12, 12)) -> np.ndarray:

    if color == "w":
        return cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    b, g, r = frame[:, :, 0], frame[:, :, 1], frame[:, :, 2]

    channels = {"r": r, "g": g, "b": b}

    colors = ["r", "g", "b"]

    selected = colors.index(color)
    mask_chnl, ch2, ch3 = (
        channels[colors[selected]],
        channels[colors[(selected + 1) % 3]],
        channels[colors[(selected + 1) % 3]],
    )

    zeros = np.zeros(r.shape, dtype="uint8")

    colorMask = (mask_chnl > (ch2 + delta)) & (mask_chnl > (ch3 + delta))

    #  filter false positives that come up if _ + delta > 255
    falsePositives = (ch2 < 255 - delta) & (ch3 < 255 - delta)
    masked_image = zeros.copy()
    masked_image[colorMask & falsePositives] = 255

    # if dilate:
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, kernel_size)
    masked_image = cv2.erode(masked_image, kernel, iterations=1)

    return masked_image


###################################################################################################
######################################### MAIN CODE ###############################################


def main(args):
    if args["type"] not in ARUCO_DICT:
        print("[FATAL] ArUCo tag of '{}' is not supported".format(args["type"]))
        exit(0)

    # load the ArUCo dictionary and grab the ArUCo parameters
    print("[INFO] detecting '{}' tags...".format(args["type"]))
    arucoDict = cv2.aruco.Dictionary_get(ARUCO_DICT[args["type"]])
    arucoParams = cv2.aruco.DetectorParameters_create()

    # initialize the video stream and allow the camera sensor to warm up
    print("[INFO] starting video stream...")
    vs = VideoStream(src=args["camera"], resolution=(1920, 1080)).start()

    # with np.load("../cameraCalibration/calib_results.npz") as npzfile:
    #     cameraMatrix, distCoeffs, rvecs, tvecs = [
    #         npzfile[i] for i in ["mtx", "dist", "rvecs", "tvecs"]]

    # camera matrix and distance coefficients outputed by ros camera_calibration module -- propper integration in progress
    cameraMatrix = np.ndarray(
        shape=(3, 3),
        buffer=np.array([874.7624752186383, 0, 282.6009074642533, 0, 874.5379489806799, 218.1223179333145, 0, 0, 1]),
    )
    distCoeffs = np.ndarray(
        shape=(1, 5),
        buffer=np.array([0.05363329676093317, 0.3372325263081464, -0.005382727611648226, -0.02717982394149372, 0]),
    )
    color = clrInputParser()

    verbose = False  # flag to toggle marker info printing

    # main code loop --- loop over the frames from the video stream
    while True:
        # grab the frame from the threaded video stream and resize it
        # to have a maximum width of 600 pixels
        frame = vs.read()

        if frame is None:
            print("[FATAL] camera feed not found")
            vs.stop()
            exit(0)

        frame = imutils.resize(frame, width=1000)
        masked_image = cv2.bilateralFilter(frame, 15, 75, 90)
        DELTA = 10
        masked_image = mask(masked_image, color, DELTA)

        # detect ArUco markers in the input frame
        #! verificar relação entre rejeições e deteção --> 
        (corners, ids, rejected) = cv2.aruco.detectMarkers(
            masked_image, arucoDict, parameters=arucoParams, cameraMatrix=cameraMatrix, distCoeff=distCoeffs)

        # ,cameraMatrix=cameraMatrix, distCoeff=distCoeffs
        # verify *at least* one ArUco marker was detected
        if len(corners) > 0:
            cv2.aruco.drawDetectedMarkers(frame, corners, ids)

            for i in range(len(ids)):
                rvec, tvec, markerPoints = cv2.aruco.estimatePoseSingleMarkers(
                    corners[i], 0.02, cameraMatrix, distCoeffs
                )
                (rvec - tvec).any()
                cv2.aruco.drawAxis(frame, cameraMatrix, distCoeffs, rvec, tvec, 0.01)
            if verbose:
                print(f"Aruco Marker id: {ids[i]}")
                print(f"\trotation vector: [{rvec[ 0, 0, 0]} {rvec[ 0, 0, 1]} {rvec[ 0, 0, 2]}]")
                print(f"\ttranslation vector: [{tvec[ 0, 0, 0]} {tvec[ 0, 0, 1]} {tvec[ 0, 0, 2]}]")
        # show the output frame
        cv2.imshow("Frame", frame)
        cv2.imshow("Masked Image", masked_image)

        ################################ input waitKeys ################################
        key = cv2.waitKey(1) & 0xFF

        # if the 'i' key was pressed, pause the loop and parse the color mask input
        if key == ord("d"):
            dictType = dictInputParser()
            arucoDict = cv2.aruco.Dictionary_get(dictType)
        # if the 'i' key was pressed, pause the loop and parse the color mask input
        if key == ord("i"):
            color = clrInputParser()
        if key == ord("p"):
            verbose = verbose != True
        # if the 'q' key was pressed, break from the loop
        if key == ord("q"):
            break

    # do a bit of cleanup
    cv2.destroyAllWindows()
    vs.stop()


###################################################################################################
######################################## DRIVER CODE ##############################################


if __name__ == "__main__":
    # construct the argument parser and parse the arguments
    ap = argparse.ArgumentParser()
    ap.add_argument("-c", "--camera", type=int, required=False, default=0, help="webcam index")
    ap.add_argument("-t", "--type", type=str, required=False, default="dict6_100", help="type of ArUCo tag to detect")
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

    main(args)
