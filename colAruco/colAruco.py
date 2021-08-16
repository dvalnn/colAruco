#!/usr/bin/python3

import cv2
import serial
import argparse
from time import time


###################################################################################################
################################## FUNCTION DECLARATION ###########################################


def arduino_write(serial_in: str):
    arduino.write(bytes(serial_in, "utf-8"))
    arduino.flush()


def arduino_read():
    serial_out = arduino.readlines()
    return [str(value) for value in serial_out]


def fetch_aruco(id, dictionary) -> str:

    marker_dictionary = cv2.aruco.Dictionary_get(dictionary)
    marker = cv2.aruco.drawMarker(marker_dictionary, 0, 6)

    marker_bits = [0 for _ in range(6)]

    for i in range(len(marker)):
        for j in range(len(marker[i])):
            if marker[i][j]:
                marker_bits[i] += 1 << (len(marker[i]) - j - 1)

    code_raw = " ".join(str(marker_bits[i]) for i in range(len(marker_bits)))

    return f"code {len(marker_bits)} {code_raw}"


def man():
    with open("colAruco_manual.txt", "r") as man:
        print("\n" + man.read() + "\n")


def input_parser(usr_input: list) -> tuple:
    # text input is always lowercase
    error = False
    input_flag = ""
    formated_input = ""

    if "save" in usr_input:
        input_flag = "save"

    elif "load" in usr_input:
        input_flag = "load"

    elif "test" in usr_input:
        input_flag = "test"

    elif "br" in usr_input:
        error = True
        try:
            tmp = int(usr_input[usr_input.index("br") + 1])
        except ValueError:
            print("[ERROR] invalid brightness input - not integer value")
        except IndexError:
            print("[ERROR] invalid brightness input - value not found")
        else:
            if tmp in range(0, 256):
                error = False
                input_flag = "br"
                formated_input += str(tmp) + " "
            else:
                print("[ERROR] brightness input is outside the accepted range (0-255)")

    elif "cl" in usr_input:
        error = True
        try:
            tmp = usr_input[usr_input.index("cl") + 1]
        except IndexError:
            print("[ERROR] invalid color input - color value not found")
        else:
            if tmp in PREDEFINED_COLORS.keys():
                error = False
                input_flag = "cl"
                formated_input += PREDEFINED_COLORS[tmp] + " "
            else:
                try:
                    int(tmp, 16)
                except ValueError:
                    print("[ERROR] invalid color input - not hex or predefined")
                else:
                    error = False
                    input_flag = "cl"
                    formated_input += tmp + " "

    elif "code" in usr_input:
        error = True
        try:
            id = int(usr_input[usr_input.index("code") + 1])
            dict_type = usr_input[usr_input.index("code") + 2]
        except IndexError:
            print("[ERROR] Invalid code input")
        else:
            if id in range(0, 1000) and dict_type in ARUCO_DICT.keys():
                error = False
                input_flag = "code"
                formated_input += fetch_aruco(id, ARUCO_DICT[dict_type])
            else:
                print("[ERROR] Invalid code input")

    return (error, input_flag, formated_input)


###################################################################################################
########################################### MAIN ##################################################


def main(arduino: serial.Serial):
    # main loop
    while True:
        loop_start = time()

        # waiting on arduino response
        print("\n[INFO] Waiting for arduino response")
        while True:
            serial_out = arduino_read()
            # remove attached noise and tags that serial out outputs
            serial_out = [string.strip("\\n\\rb'") for string in serial_out]

            if len(serial_out) != 0:
                # checking to see if code size is 0
                size_check = ["size" if string.count("size") else None for string in serial_out]
                if any(size_check) and serial_out[size_check.index("size")][-1] == "0":
                    # if code size is 0, continues de loop and waits for the value to update
                    continue
                break

            if time() - loop_start >= 3:  # times out so that the code doesn't get stuck waiting for a response
                break

        for element in serial_out:
            print(element)  # printing out arduino response
        print("-------------------------------------------------------\n")

        while True:
            try:
                text = input("User input: ").lower().strip("\n").strip().split()  # Taking input from user
            except EOFError:
                arduino.close()
                quit()

            if not len(text):
                print("\033[F\033[K", end="")  # cursor up one line and clear it to the end
                continue

            if any(string in text for string in ["q", "quit", "exit"]):
                print("\033[F\033[K", end="")
                print("[INFO] closing serial connection")
                arduino.close()
                quit()

            if any(string in text for string in ["-h", "--help", "man"]):
                man()
                continue

            print("", end="\033[K")
            error, input_flag, formated_input = input_parser(text)

            if len(input_flag):
                arduino_write(input_flag + " " + formated_input)
                break
            elif error:
                print("\033[3F", end="\033[K")  # cursor 2 lines up and clear line
                pass
            else:
                print("[INFO] Input carries no meaning, arduino will not be updated", end="\033[F\033[K")


###################################################################################################
######################################## DRIVER CODE ##############################################


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-p",
        "--port",
        default="/dev/ttyACM0",
        help="Serial port to attempt arduino connection - defaults to /dev/ttyACM0",
    )

    args = vars(ap.parse_args())

    try:
        arduino = serial.Serial(port=args["port"], baudrate=9600, timeout=0.1)
    except:
        print("\n[FATAL] Couldn't establish serial connection on port", args["port"])
        quit()
    else:
        print("\n[INFO] Arduino connection successfull")

    PREDEFINED_COLORS = {
        "r": "FF0000",
        "g": "00FF00",
        "b": "0000FF",
        "w": "FFFFFF",
    }

    ARUCO_DICT = {
        "dict4": cv2.aruco.DICT_4X4_1000,
        "dict5": cv2.aruco.DICT_5X5_1000,
        "dict6": cv2.aruco.DICT_6X6_1000,
        # "dict7": "7",
        "dict_or": cv2.aruco.DICT_ARUCO_ORIGINAL,
    }

    main(arduino)
