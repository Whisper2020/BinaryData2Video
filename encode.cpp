/******** encode.cpp ********/
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include <opencv.hpp>

//函数声明
int encode(char* str1);//参数1为二进制文件的绝对位置,如果出错返回-1，正常运行返回图片的张数
void Image_to_Video_Generate(int num, int frame_width, int frame_height, float frame_fps, float video_time);//解释在函数实现部分

using namespace cv;
using namespace std;
//全局变量定义
const int Rows = 100, Cols = 100;//二进制文件中转为图片的像素的行列信息

int main(int argc, char** argv) {
	return work(argc, argv);
}
/* -----zhr----- */
int work(int argc, char** argv) {
	int retCode = 0;
	cout << "Binary file name: " << argv[1] << endl;
	cout << "Encoding.." << endl;
	retCode = encode(argv[1], "img");
	cout << "Output video file name: " << argv[2] << endl;
	cout << "Length of video: " << argv[3] << "ms" << endl; //注意argv[3]是char格式的，如果要当作数值应用atoi(argv[3])函数转换
	return retCode;
}
//END zhr.
/* -----ldr----- */
int encode(char* str1) {
    FILE* filePoint = NULL;
    fopen_s(&filePoint, str1, "rb");
    if (filePoint == NULL) {
        cout << "Failed to open file!" << endl;
        return -1;
    }
    int num;//图片的张数
    char name[20];
    fread(&num, sizeof(int), 1, filePoint);//二进制文件先给出有多少张图片
    Mat image(Rows, Cols, CV_8UC3, Scalar::all(0));
    for (int i = 1; i <= num; i++) {
        sprintf_s(name, "%s%d%s", "image", i, ".jpg");//输出图片的格式
        char* pData = (char*)image.data;
        for (int j = 0; j < Rows * Cols; j++)
            for (int k = 0; k < 3; k++)
                fread(&pData[j * 3 + k], sizeof(char), 1, filePoint);
        imwrite(name, image);
    }
    fclose(filePoint);
    return num;
}
//End ldr
/* -----y g----- */
void Image_to_Video_Generate(int num,int frame_width,int frame_height,float frame_fps,float video_time)
{
//frame_height——图片高度
//frame_width——图片宽度
//frame_fps——图片帧率
//num——图片帧数
//video_time——视频播放时间，毫秒
//图像预先存储命名为：imageXX.jpg(eg:image1,image2...)
	char image_name[20];
	string s_image_name;
	VideoWriter writer;
	int isColor = 1;
	string video_name = "out.avi";
	float time = 0;//记录播放时间
	writer = VideoWriter(video_name,writer.fourcc('X', 'V', 'I', 'D'), frame_fps, Size(frame_width, frame_height), isColor);
	cout << "frame_width is " << frame_width << endl;
	cout << "frame_height is " << frame_height << endl;
	cout << "frame_fps is " << frame_fps << endl;
	int i = 0;
	Mat img;
	while (i <= num)
	{
		sprintf_s(image_name, "%s%d%s", "image", ++i, ".jpg");//将所存图片名字写入
		s_image_name = image_name;
		img = imread(s_image_name);//读入图片
		if (!img.data)//判断图片调入是否成功
		{
			cout << "Could not load image file...\n" << endl;
		}
		writer.write(img);
		time += frame_fps;
		if (waitKey(frame_fps) == 27 || i == num)//27 ESC键ascll码
		{
			cout << "按下ESC键" << endl;
			break;
		}
		if (time >= video_time)
			break;
	}	
}
//end yg

