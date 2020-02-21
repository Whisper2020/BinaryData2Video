/******** encode.cpp ********/
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//函数声明
void encode(char *str1, char *str2);//参数1为二进制文件的绝对位置，参数2为要保存为图片的名字

using namespace std;
//全局变量定义
int Rows, Cols;//二进制文件中转为图片的像素的行列信息

int main(int argc, char** argv) {
	cout<<"Encoding.."<<endl;
	return 0;
}

/* -----zhr-----*/

ZHR's code here.

//END zhr.

/*----ldr----*/
void encode(char *str1, char *str2) {
    FILE* filePoint = NULL;
    fopen_s(&filePoint, str1, "rb");
    if (filePoint == NULL) {
        cout << "Failed to open file!" << endl;
        return;
    }
    Mat image(Rows, Cols, CV_8UC3, Scalar::all(0));
    char* pData = (char*)image.data;
    for (int i = 0; i < Rows * Cols; i++)
        for (int j = 0; j < 3; j++)
            fread(&pData[i * 3 + j], sizeof(char), 1, filePoint);
    fclose(filePoint);
    imwrite(str2, image);
}

//End ldr
