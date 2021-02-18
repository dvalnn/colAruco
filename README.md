ColAruco

Source code for the controler of a rgb led matrix that displays Aruco codes. Upload the main.cpp to the arduino controler and then run the python script.

When running the python script pass the serial port an argument (if not passed defaults to /dev/ttyACM0) The python code calls for a c++ subprocess on runtime (arucoDictTranslator.cpp) which might need to be manually compiled with the correct name after clonning the rep (due to ubuntu permissions).

The brightness and color of the leds can be adjusted at will. The brigthnes range goes from 0 (leds off) to 255 (maximum brightness). (Example input: br 23) There are 4 predifined colors (red, green, blue, and white) and may be acessed inputing cl r/g/b/w in the python script. To display other colors, the hex value must be manually inputed (Ex: cl ff00ff -> value for pink)

The code arduino source code is 100% cpp/arduino and can be run by an Arduino Uno board, or better. (An Arduino Nano board might be able to run the code as well, but i haven't tested it yet).

The Arduino code needs the standard Arduino libraries as well as the Adafruit_NeoPixel library to run properly (if you open the folder with the PlatformIO ide for vscode it should handle that without problems).

This software was developed on ubunto 20.04 lts and should work fine on other Linux distributions. Should also work fine on Windows given all the python and c++ requirements are met and the correct serial port for connection is specified.
