/******** decode.cpp ********/
#include <iostream>
#include <cstdlib>
#include <cstdio>
//#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


void decode(char* str);//参数为图片的绝对位置
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
void decode(char* str)
{
	Mat image = imread(str);
	if (!image.data || image.channels() != 3) {
		cout << "Can't open the picture or the picture's channels aren't 3." << endl;
		return;
	}
	const int Rows = image.rows, Cols = image.cols;
	FILE* filePoint = NULL;
	const char* strs = "encoded.bin";
	fopen_s(&filePoint, strs, "wb");
	char* pData = (char*)image.data;
	for (int i = 0; i < Rows * Cols; i++)
		for (int j = 0; j < 3; j++)
			fwrite(&pData[i * 3 + j], sizeof(char), 1, filePoint);
	fclose(filePoint);
}
//End ldr
