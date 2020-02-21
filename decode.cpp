/******** decode.cpp ********/
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	cout<<"Decoding.."<<endl;
	return 0;
}

/* -----ldr-----*/

int Rows, Cols;
void decode(char* str)
{
    Mat image = imread(str);
    if (!image.data || image.channels() != 3) {
        cout << "Can't open the picture or the picture's channels aren't 3." << endl;
        return;
    }
    Rows = image.rows, Cols = image.cols;
    FILE* filePoint = NULL;
    const char* strs = "encoded.bin";
    fopen_s(&filePoint, strs, "wb");
    char* pData = (char*)image.data;
    for (int i = 0; i < Rows * Cols; i++)
        for (int j = 0; j < 3; j++)
            fwrite(&pData[i * 3 + j], sizeof(char), 1, filePoint);
    fclose(filePoint);
}

//END ldr.

