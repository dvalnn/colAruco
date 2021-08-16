#include <iostream>

#include "predefined_dictionaries/aruco_dict_4x4_1000.h"
#include "predefined_dictionaries/aruco_dict_5x5_1000.h"
#include "predefined_dictionaries/aruco_dict_6x6_1000.h"
#include "predefined_dictionaries/aruco_dict_7x7_1000.h"
#include "predefined_dictionaries/aruco_dict_original_1000.h"

#define BIT(n) (1 << n)

#define MAX_CODE_SIZE 6
#define MAX_CODE_SIZE_WITH_BORDER 10

using namespace std;

void byteTranslator(uint8_t *codedBytes, int size, int *translatedBytes);
void addZeroBorder(int translatedBytes[], int size, int withBorder[]);
int fillWith1(int size);
int addBorder(int translatedBytes[], int size, int withBorder[]);

int main(int argc, char *argv[])
{

    int id = atoi(argv[1]);
    int type = atoi(argv[2]);
    int size = 0;

    // int translatedCode[7] = {0};
    uint8_t *codeFromDict;
    int translatedCode[MAX_CODE_SIZE] = {0};
    // int arduinoInput[MAX_CODE_SIZE_WITH_BORDER] = {0};

    switch (type)
    {
    case 0:
        codeFromDict = DICT_ARUCO_BYTES[id][0];
        size = 5;
        break;
    case 4:
        codeFromDict = DICT_4X4_1000_BYTES[id][0];
        size = 4;
        break;
    case 5:
        codeFromDict = DICT_5X5_1000_BYTES[id][0];
        size = 5;
        break;
    case 6:
        codeFromDict = DICT_6X6_1000_BYTES[id][0];
        size = 6;
        break;
    // case 7:
    //     codeFromDict = DICT_7X7_1000_BYTES[id][0];
    //     size = 7;
    //     break;
    default:
        return 0;
    }

    byteTranslator(codeFromDict, size, translatedCode);

    cout << "code " << size << " ";
    for (short i = 0; i < size; i++)
        cout << translatedCode[i] << " ";

    cout << endl;

    return 0;
}

void byteTranslator(uint8_t codedBytes[], int size, int translatedBytes[])
{
    short bitsCount = size * size;
    short bitsLength = 0;

    short *bits;
    bits = new short[bitsCount];
    for (int i = 0; i < size; i++)
    {
        short start = 8;
        if (bitsCount - bitsLength < 8)
            start = bitsCount - bitsLength;

        for (short j = start - 1; j >= 0; j--)
        {
            bits[bitsLength] = (codedBytes[i] >> j) & 1;
            bitsLength++;
        }
    }

    for (short i = 0; i < bitsCount; i++)
        translatedBytes[i / size] += bits[i] << ((size - 1) - (i % size));
    delete[] bits;
}


/*
void createArucoMarkers() {
    cv::Mat outputMarker;

    cv::Ptr<cv::aruco::Dictionary> markerDictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    for (int i = 0; i < 50; i++) {
        cv::aruco::drawMarker(markerDictionary, i, 500, outputMarker);

        ostringstream fileName;
        string imageName = "4x4Marker_";
        string directoryName = "../resources/";
        fileName << directoryName << imageName << i << ".jpg";
        imwrite(fileName.str(), outputMarker);
    }
}
*/
