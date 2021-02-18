import serial
import subprocess
import argparse
from sys import exit
from time import time

ap = argparse.ArgumentParser()
ap.add_argument("-p", "--port",
                default="/dev/ttyACM0",
                help="Serial port to attempt arduino connection - defaults to /dev/ttyACM0")

args = vars(ap.parse_args())

try:
    arduino = serial.Serial(port=args["port"], baudrate=9600, timeout=.1)
except:
    print("\n[FATAL] Couldn't establish serial connection on port", args["port"])
    exit(0)
else:
    print("\n[INFO] Arduino connection successfull")

PREDEFINED_COLORS = {
    "r": "FF0000",
    "g": "00FF00",
    "b": "0000FF",
    "w": "FFFFFF",
}

ARUCO_DICT = {
    "dict_4x4": "4",
    "dict_5x5": "5",
    "dict_6x6": "6",
    "dict_7x7": "7",
    "dict_original": "0",
}


def arduino_write(serial_in: str):
    arduino.write(bytes(serial_in, 'utf-8'))
    arduino.flush()


def arduino_read():
    serial_out = arduino.readlines()
    return [str(value) for value in serial_out]


def fetch_aruco(id: str, size: str) -> str:

    completed_process = subprocess.run(
        ["../arucoCodeGen/arucoDictTranslator", id, size], capture_output=True, text=True)
    return completed_process.stdout.strip("\n").strip()


def input_parser(text: str) -> tuple:
    flag = ""
    usr_input = text.strip().split()
    formated_input = ""

    if "exit" in usr_input:
        exit(0)

    if "save" in usr_input:
        flag = "save"

    elif "load" in usr_input:
        flag = "load"

    elif "br" in usr_input:
        try:
            tmp = int(usr_input[usr_input.index("br") + 1])
        except ValueError:
            print("\n[WARNING] invalid brightness input - not integer value\n")
        except IndexError:
            print("\n[ERROR] invalid brightness input - value not found\n")
        else:
            if tmp in range(0, 256):
                flag = "br"
                formated_input += (str(tmp) + " ")
            else:
                print(
                    "\n[ERROR] brightness input is outside the accepted range (0-255)\n")

    elif "cl" in usr_input:
        try:
            tmp = usr_input[usr_input.index("cl") + 1]
        except IndexError:
            print("\n[ERROR] invalid color input - color value not found\n")
        else:
            if tmp in PREDEFINED_COLORS.keys():
                flag = "cl"
                formated_input += (PREDEFINED_COLORS[tmp] + " ")
            else:
                try:
                    int(tmp, 16)
                except ValueError:
                    print(
                        "\n[WARNING] invalid color input - not hex or predefined\n")
                else:
                    flag = "cl"
                    formated_input += (tmp + " ")

    elif "code" in usr_input:
        try:
            tmp1 = int(usr_input[usr_input.index("code") + 1])
            tmp2 = usr_input[usr_input.index("code") + 2]
        except IndexError:
            print("[ERROR] Invalid code input - values not found")
        else:
            if tmp1 in range(0, 1000) and tmp2 in ARUCO_DICT.keys():
                flag = "code"
                formated_input += fetch_aruco(str(tmp1), ARUCO_DICT[tmp2])
            else:
                print("\n[ERROR] Invalid code input\n")

    print("flag:", flag)
    print("formated arduino input:", formated_input)

    return (flag, formated_input)


# main loop
while True:
    loop_start = time()

    # waiting on arduino response
    print("\n[INFO] Waiting for arduino response\n")
    while True:
        serial_out = arduino_read()
        serial_out = [string.strip("\\n\\rb'") for string in serial_out]

        if len(serial_out) != 0 and serial_out[2][-1] != "0":
            break

        if time() - loop_start >= 3: #times out so that the code doesn't get stuck waiting for a response
            break

    for element in serial_out:
        print(element)  # printing out arduino response
    print("-------------------------------------------------------")
    print()

    while True:
        text = input("User input: ").lower()  # Taking input from user
        if not len(text):
            continue
        flag, formated_input = input_parser(text)

        if len(flag):
            arduino_write(flag + " " + formated_input)
            break
        else:
            print("[INFO] Input carries no meaning, arduino will not be updated\n")
