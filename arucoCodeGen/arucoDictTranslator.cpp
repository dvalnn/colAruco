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
    // size = addBorder(translatedCode, size, arduinoInput);

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

int addBorder(int translatedBytes[], int size, int withBorder[])
{
    size += 4;

    //primeira e última filas preenchidas com 1 em todas as posições
    withBorder[0] = fillWith1(size);
    withBorder[size - 1] = fillWith1(size);
    //segunda e penúltima filas preenchidas com 10..(0)..01
    withBorder[1] = BIT(size - 1) + 1;
    withBorder[size - 2] = BIT(size - 1) + 1;

    //restantes filas preenchidas com o código com borda de 1 e 0
    for (short i = 2; i < size - 2; i++)
        withBorder[i] += (BIT(size - 1) + (translatedBytes[i - 2] << 2) + 1);
        
    return size;
}

int fillWith1(int size)
{
    int value = 0;
    for (int i = 0; i < size; i++)
        value += BIT(i);
    return value;
}