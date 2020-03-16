/******** decode.cpp ********/
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

void Video_To_Image_Separate(string filename);
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
/*视频划分为图像*/
void Video_To_Image_Separate(string filename)
{
	VideoCapture capture(filename);
	if (!capture.isOpened())
	{
		cout << "open video error";
	}
	/*CV_CAP_PROP_POS_MSEC – 视频的当前位置（毫秒）
	CV_CAP_PROP_POS_FRAMES – 视频的当前位置（帧）
	CV_CAP_PROP_FRAME_WIDTH – 视频流的宽度
	CV_CAP_PROP_FRAME_HEIGHT – 视频流的高度
	CV_CAP_PROP_FPS – 帧速率（帧 / 秒）*/
	int frame_width = (int)capture.get(CAP_PROP_FRAME_WIDTH);
	int frame_height = (int)capture.get(CAP_PROP_FRAME_HEIGHT);
	float frame_fps = capture.get(CAP_PROP_FPS);
	int frame_number = capture.get(CAP_PROP_FRAME_COUNT);//总帧数
	cout << "frame_width is " << frame_width << endl;
	cout << "frame_height is " << frame_height << endl;
	cout << "frame_fps is " << frame_fps << endl;
	int num = 0;//统计帧数
	Mat img;
	string img_name;
	char image_name[20];
	int kernel_size = 3;
	/*一个简单的滤波处理，防止雪花*/
	Mat kernel = Mat::ones(kernel_size, kernel_size, CV_32F) / (float)(kernel_size * kernel_size);

	while (true)
	{
		Mat frame;
		//从视频中读取一个帧
		bool bSuccess = capture.read(frame);
		if (!bSuccess)
		{
			cout << "不能从视频文件读取帧" << endl;
			break;
		}
		filter2D(frame, frame, -1, kernel);
		sprintf_s(image_name, "%s%d%s", "image", ++num, ".jpg");//保存的图片名
		img_name = image_name;
		/*以10fps生成的视频一般以30fps划分，重复三张，取中间一张最好*/
		if (num%3==1)
			imwrite(img_name, frame);//保存一帧图片
		if (waitKey(30) == 27 || num == frame_number)
		{
			cout << "按下ESC键" << endl;
			break;
		}
	}
}
