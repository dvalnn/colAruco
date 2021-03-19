from imutils.video import VideoStream
from os import chdir
import argparse
import numpy as np
import cv2
# import glob

ap = argparse.ArgumentParser()
ap.add_argument("-c", "--camera", type=int, required=False,
                default=2,
                help="webcam index")
args = vars(ap.parse_args())

path = "/home/dvalinn/prog/colAruco/cameraCalibration/calibrationImages"
chdir(path)

# termination criteria
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

# prepare object points, like (0,0,0), (1,0,0), (2,0,0) ....,(6,5,0)
objp = np.zeros((6*7, 3), np.float32)
objp[:, :2] = np.mgrid[0:7, 0:6].T.reshape(-1, 2)

# Arrays to store object points and image points from all the images.
objpoints = []  # 3d point in real world space
imgpoints = []  # 2d points in image plane.

print("[INFO] starting video stream...")
vs = VideoStream(src=args["camera"], resolution=(1920, 1080)).start()
counter = 0

while True:
    img = vs.read()

    if img is None:
        print("[FATAL] camera feed not found")
        vs.stop()
        exit(0)

    cv2.imshow("img", img)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Find the chess board corners
    ret, corners = cv2.findChessboardCorners(gray, (12, 7), None)

    # If found, add object points, image points (after refining them)
    if ret == True:
        objpoints.append(objp)

        corners2 = cv2.cornerSubPix(
            gray, corners, (11, 11), (-1, -1), criteria)
        imgpoints.append(corners2)

        # Draw and display the corners
        key = cv2.waitKey(300) & 0xFF
        if key == ord('s') or key == 13:
            counter += 1
            print("images found: ", counter)
            cv2.imwrite(f"detectionImage{counter}.jpg", img)

        img = cv2.drawChessboardCorners(img, (7, 6), corners2, ret)
        cv2.imshow('FOUND', img)

    key = cv2.waitKey(1) & 0xFF

    if key == ord("q") or counter == 20:
        break

cv2.destroyAllWindows()
vs.stop()
