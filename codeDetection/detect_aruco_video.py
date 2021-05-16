#!/usr/bin/python3.8

# USAGE
# python detect_aruco_video.py -t [dictType]

# import the necessary packages
import argparse

from imutils import resize
from imutils.video import VideoStream

import cv2
import numpy as np

from sys import exit


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


def maskFrame(frame: np.ndarray, color: str, delta: int = 12):

    # Extract primary color channels from incoming frame.
    b, g, r = frame[:, :, 0], frame[:, :, 1], frame[:, :, 2]

    channels = {"r": r, "g": g, "b": b}

    colors = ["r", "g", "b"]

    selected = colors.index(color)

    mask_chnl, ch2, ch3 = (
        channels[colors[selected]],
        channels[colors[(selected + 1) % 3]],
        channels[colors[(selected + 2) % 3]],
    )

    colorMask = (mask_chnl > (ch2 + delta)) & (mask_chnl > (ch3 + delta))

    #  filter false positives that come up if _ + delta > 255
    falsePositives = (ch2 < 255 - delta) & (ch3 < 255 - delta)
    masked_image = np.zeros(r.shape, dtype="uint8")
    masked_image[colorMask & falsePositives] = 255

    return masked_image


def processFrame(original_image, target_color_channel, morphology_kernel_size: tuple = (12, 12)):
    if target_color_channel == "w":
        return original_image, cv2.cvtColor(original_image, cv2.COLOR_BGR2GRAY)

    # run a bilateralFilter to blur the original image and then mask the target color channel
    masked_image = maskFrame(cv2.bilateralFilter(original_image, 15, 75, 90), target_color_channel)

    # create a rectangular kernel and apply an erosion transformation to the masked frame
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, morphology_kernel_size)
    # cv2.erode (using rectangular kernel) will expand the black rectangles to counteract deformation caused by the masking
    masked_image = cv2.erode(masked_image, kernel, iterations=1)

    # return both the unaltered original frame as well as the treated version for cv2 detection
    return original_image, masked_image


###################################################################################################
######################################### MAIN CODE ###############################################


def mainLoop(arucoDict, arucoParams, videoSource, cameraMatrix, distCoeffs):

    target_color_channel = clrInputParser()
    verbose = False  # flag to toggle marker info printing

    # main code loop --- loop over the frames from the video stream
    while True:
        # grab the frame from the threaded video stream and resize it
        # to have a maximum width of 600 pixels
        original_image, masked_image = processFrame(resize(videoSource.read(), width=1000), target_color_channel)

        # detect ArUco markers in the input frame
        #! verificar relação entre rejeições e deteção -->
        (corners, ids, rejected) = cv2.aruco.detectMarkers(
            masked_image, arucoDict, parameters=arucoParams, cameraMatrix=cameraMatrix, distCoeff=distCoeffs
        )

        # ,cameraMatrix=cameraMatrix, distCoeff=distCoeffs
        # verify *at least* one ArUco marker was detected
        if len(corners) > 0:
            cv2.aruco.drawDetectedMarkers(original_image, corners, ids)

            for i in range(len(ids)):
                rvec, tvec, markerPoints = cv2.aruco.estimatePoseSingleMarkers(
                    corners[i], 0.02, cameraMatrix, distCoeffs
                )
                (rvec - tvec).any()
                cv2.aruco.drawAxis(original_image, cameraMatrix, distCoeffs, rvec, tvec, 0.01)
            if verbose:
                print(f"Aruco Marker id: {ids[i]}")
                print(f"\trotation vector: [{rvec[ 0, 0, 0]} {rvec[ 0, 0, 1]} {rvec[ 0, 0, 2]}]")
                print(f"\ttranslation vector: [{tvec[ 0, 0, 0]} {tvec[ 0, 0, 1]} {tvec[ 0, 0, 2]}]")
        # show the output frame
        cv2.imshow("Frame", original_image)
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

    # final cleanup
    cv2.destroyAllWindows()
    videoSource.stop()


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

    if args["type"] not in ARUCO_DICT:
        print("[FATAL] ArUCo tag of '{}' is not supported".format(args["type"]))
        exit(0)

    # load the aruco dictionary and create detection parameters
    print("[INFO] detecting '{}' tags...".format(args["type"]))
    arucoDict = cv2.aruco.Dictionary_get(ARUCO_DICT[args["type"]])
    arucoParams = cv2.aruco.DetectorParameters_create()

    print("[INFO] starting video stream...")
    videoSource = VideoStream(src=args["camera"], resolution=(1920, 1080)).start()

    testFrame = videoSource.read()
    if testFrame is None:
        print("[FATAL] camera feed not found")
        videoSource.stop()
        exit(0)

    # with np.load("../cameraCalibration/calib_results.npz") as npzfile:
    #     cameraMatrix, distCoeffs, rvecs, tvecs = [
    #         npzfile[i] for i in ["mtx", "dist", "rvecs", "tvecs"]]

    # camera matrix and distance coefficients outputed by ros camera_calibration module -- needs propper integration
    cameraMatrix = np.ndarray(
        shape=(3, 3),
        buffer=np.array([874.7624752186383, 0, 282.6009074642533, 0, 874.5379489806799, 218.1223179333145, 0, 0, 1]),
    )

    distCoeffs = np.ndarray(
        shape=(1, 5),
        buffer=np.array([0.05363329676093317, 0.3372325263081464, -0.005382727611648226, -0.02717982394149372, 0]),
    )

    mainLoop(arucoDict, arucoParams, videoSource, cameraMatrix, distCoeffs)
