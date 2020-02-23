/******** decode.cpp ********/
#include <iostream>
#include <cstdlib>
#include <cstdio>
//#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


void decode(char* str1, char* str2);//参数1为图片的绝对位置，参数2为保存图片信息的二进制文件
using namespace std;
using namespace cv;

int main(int argc, char** argv) {
	cout << "Video file:" << argv[1] << endl;
	cout << "Decoding.." << endl;
	decode(argv[1]);
	cout << "Finish." << endl;
	cout << "Output file:" << argv[2] << endl;

	return 0;
}

/* -----zhr-----*/

//END zhr.
/*------ldr------*/
void decode(char* str1, char* str2)
{
    Mat image = imread(str1);
    if (!image.data || image.channels() != 3) {
        cout << "Can't open the picture or the picture's channels aren't 3." << endl;
        return;
    }
    int Rows = image.rows, Cols = image.cols;
    FILE* filePoint = NULL;
    fopen_s(&filePoint, str2, "wb");
    fwrite(&Rows, sizeof(int), 1, filePoint);
    fwrite(&Cols, sizeof(int), 1, filePoint);
    char* pData = (char*)image.data;
    for (int i = 0; i < Rows * Cols; i++)
        for (int j = 0; j < 3; j++)
            fwrite(&pData[i * 3 + j], sizeof(char), 1, filePoint);
    fclose(filePoint);
}
//End ldr
void seperate(const char* pathMp4, const char* pathPng)//seperate("test.mp4", "x%d.png");//示例
{
	string strFfmpegPath = "E:\\ffmpeg-4.2.2-win64-static\\bin\\ffmpeg";//ffmpeg.exe所在的位置
	string strPathMp4 = pathMp4;//待转化的mp4
	string strPathPng = pathPng;//生成的png
	string strCmdContent = "/c" + strFfmpegPath + " -i " + strPathMp4 + " " + strPathPng;
	ShellExecute(NULL, "open", "cmd", strCmdContent.c_str(), NULL, SW_HIDE);
}
