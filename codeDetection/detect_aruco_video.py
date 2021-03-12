# USAGE
# python detect_aruco_video.py

# import the necessary packages
from os import pipe
from imutils.video import VideoStream
import argparse
import imutils
import time
import cv2
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
    "DICT_7X7_50": cv2.aruco.DICT_7X7_50,
    "DICT_7X7_100": cv2.aruco.DICT_7X7_100,
    "DICT_7X7_250": cv2.aruco.DICT_7X7_250,
    "DICT_7X7_1000": cv2.aruco.DICT_7X7_1000,
    "DICT_ARUCO_ORIGINAL": cv2.aruco.DICT_ARUCO_ORIGINAL,
    # "DICT_APRILTAG_16h5": cv2.aruco.DICT_APRILTAG_16h5,
    # "DICT_APRILTAG_25h9": cv2.aruco.DICT_APRILTAG_25h9,
    # "DICT_APRILTAG_36h10": cv2.aruco.DICT_APRILTAG_36h10,
    # "DICT_APRILTAG_36h11": cv2.aruco.DICT_APRILTAG_36h11
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
    "DICT_7X7_50": 7,
    "DICT_7X7_100": 7,
    "DICT_7X7_250": 7,
    "DICT_7X7_1000": 7,
    "DICT_ARUCO_ORIGINAL": 5
}

if args["type"] not in ARUCO_DICT:
    print("[FATAL] ArUCo tag of '{}' is not supported".format(
        args["type"]))
    sys.exit(0)

# completed_process = __import__("subprocess").run(
#     ["/home/dvalinn/prog/arucoOpenCV/colAruco_code_gen/arucoDictTranslator", str(args["id"]), str(ARUCO_SIZES[args["type"]])], capture_output=True, text=True)
# print("aruco input: ", "code", ARUCO_SIZES[args["type"]] ,completed_process.stdout[completed_process.stdout.index("ar") + 3 :])

# load the ArUCo dictionary and grab the ArUCo parameters
print("[INFO] detecting '{}' tags...".format(args["type"]))
arucoDict = cv2.aruco.Dictionary_get(ARUCO_DICT[args["type"]])
arucoParams = cv2.aruco.DetectorParameters_create()

# initialize the video stream and allow the camera sensor to warm up
print("[INFO] starting video stream...")
vs = VideoStream(src=0).start()
time.sleep(2.0)

#daqui para baixo acho que deve estar igual ao pyImageSearch e que n mudei nada
cam = cv2.VideoCapture(0)

# loop over the frames from the video stream
while True:
    # grab the frame from the threaded video stream and resize it
    # to have a maximum width of 600 pixels

    frame = vs.read()

    if frame is None:
        print("[FATAL] camera feed not found")
        sys.exit(0)

    frame = imutils.resize(frame, width=1000)

    # flag, srcframe = cam.read()
    # frame = cv2.cvtColor(srcframe,cv2.COLOR_BGR2GRAY)

    # detect ArUco markers in the input frame
    (corners, ids, rejected) = cv2.aruco.detectMarkers(frame, arucoDict, parameters=arucoParams)

    # verify *at least* one ArUco marker was detected
    if len(corners) > 0:
        # flatten the ArUco IDs list
        ids = ids.flatten()

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

    # show the output frame
    cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF

    # if the `q` key was pressed, break from the loop
    if key == ord("q"):
        break

# do a bit of cleanup
cv2.destroyAllWindows()
vs.stop()
