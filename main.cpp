#define _CRT_SECURE_NO_WARNINGS
#include "QRCodeTools.h"

using namespace std;
using namespace cv;


int main()
{
	FILE* stream1;
	freopen_s(&stream1, "out.bin", "wb", stdout);
	/*char str1[] = "black01.bin";
	encode(str1);/*
	MapToVideo();

	char name[] = "T11.png";
	MapToBinary(name);
	cherk();*/
	char str[] = "test01.jpg", str2[] = "1.bin";
	/*Demo(str);
	cherk(str2);*/
	readFile(str2);
	readframes();
	return 0;
}

