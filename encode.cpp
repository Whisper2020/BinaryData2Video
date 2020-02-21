/******** encode.cpp ********/
#include <iostream>
#include <cstdlib>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//函数声明

using namespace std;
int Rows, Cols;//用于解码时读取图片的行列像素数
//全局变量定义

int main(int argc, char** argv) {
	cout<<"Encoding.."<<endl;
	return 0;
}

/* -----ldr-----*/
void encode(char* str)
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
