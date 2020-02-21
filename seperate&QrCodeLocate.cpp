#define _CRT_SECURE_NO_WARNING
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <Windows.h>

using namespace std;
using namespace cv;

Mat src; Mat src_gray;

void seperate(const char* pathMp4, const char* pathPng)//seperate("test.mp4", "x%d.png");//示例
{
	string strFfmpegPath = "E:\\ffmpeg-4.2.2-win64-static\\bin\\ffmpeg";//ffmpeg.exe所在的位置
	string strPathMp4 = pathMp4;//待转化的mp4
	string strPathPng = pathPng;//生成的png
	string strCmdContent = "/c" + strFfmpegPath + " -i " + strPathMp4 + " " + strPathPng;
	ShellExecute(NULL, "open", "cmd", strCmdContent.c_str(), NULL, SW_HIDE);
}

RNG rng(12345);
//Scalar colorful = CV_RGB(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));

//获取轮廓的中心点
Point Center_cal(vector<vector<Point> > contours, int i)
{
	int centerx = 0, centery = 0, n = contours[i].size();
	//在提取的小正方形的边界上每隔周长个像素提取一个点的坐标，
	//求所提取四个点的平均坐标（即为小正方形的大致中心）
	centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
	centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
	Point point1 = Point(centerx, centery);
	return point1;
}

void cvtColor_src(Mat& src, Mat& src_gray)
{
	//  转换单通道
	if (src.channels() == 4)
	{
		cv::cvtColor(src, src_gray, COLOR_BGRA2GRAY);
	}
	else if (src.channels() == 3)
	{
		cv::cvtColor(src, src_gray, COLOR_BGR2GRAY);
	}
	else if (src.channels() == 2)
	{
		cv::cvtColor(src, src_gray, COLOR_BGR5652GRAY);
	}
	else if (src.channels() == 1)
	{
		// 单通道的图片直接就不需要处理
		src_gray = src;
	}
	else
	{ // 负数,说明图有问题 直接返回   
		src_gray = src;
	}

}

int main(int argc, char** argv[])
{
	src = imread("core.png", 1);
	Mat src_all = src.clone();
	cout << src.channels()<<"  ";

	//彩色图转灰度图
	cvtColor_src(src, src_gray);
	//对图像进行平滑处理
	blur(src_gray, src_gray, Size(1,1));
	//使灰度图象直方图均衡化
	equalizeHist(src_gray, src_gray);
	namedWindow("src_gray");
	imshow("src_gray", src_gray);


	Scalar color = Scalar(1, 1, 255);
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	Mat drawing = Mat::zeros(src.size(), CV_8UC3);
	Mat drawing2 = Mat::zeros(src.size(), CV_8UC3);
	Mat drawingAllContours = Mat::zeros(src.size(), CV_8UC3);

	//指定112阀值进行二值化
	threshold(src_gray, threshold_output, 112, 255, THRESH_BINARY);

	namedWindow("Threshold_output");
	imshow("Threshold_output", threshold_output);

	findContours(threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	int c = 0, ic = 0, k = 0, area = 0;

	//通过黑色定位角作为父轮廓，有两个子轮廓的特点，筛选出三个定位角
	int parentIdx = -1;
	for (int i = 0; i < contours.size(); i++)
	{
		//画出所有轮廓图
		drawContours(drawingAllContours, contours, parentIdx, CV_RGB(255, 255, 255), 1, 8);
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}

		//有两个子轮廓
		if (ic >= 2)
		{
			//保存找到的三个黑色定位角
			contours2.push_back(contours[parentIdx]);
			//画出三个黑色定位角的轮廓
			drawContours(drawing, contours, parentIdx, CV_RGB(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 1, 8);
			ic = 0;
			parentIdx = -1;
		}
	}

	//填充的方式画出三个黑色定位角的轮廓
	for (int i = 0; i < contours2.size(); i++)
		drawContours(drawing2, contours2, i, CV_RGB(rng.uniform(100, 255), rng.uniform(100, 255), rng.uniform(100, 255)), -1, 4, hierarchy[k][2], 0, Point());

	//获取三个定位角的中心坐标
	Point point[3];
	for (int i = 0; i < contours2.size(); i++)
	{
		point[i] = Center_cal(contours2, i);
	}

	//计算轮廓的面积，计算定位角的面积，从而计算出边长
	area = contourArea(contours2[1]);
	int area_side = cvRound(sqrt(double(area)));
	for (int i = 0; i < contours2.size(); i++)
	{
		//画出三个定位角的中心连线  
		line(drawing2, point[i % contours2.size()], point[(i + 1) % contours2.size()], color, area_side / 8);
	}

	namedWindow("DrawingAllContours");
	imshow("DrawingAllContours", drawingAllContours);

	namedWindow("Drawing2");
	imshow("Drawing2", drawing2);

	namedWindow("Drawing");
	imshow("Drawing", drawing);
	
	//接下来要框出这整个二维码
	Mat gray_all, threshold_output_all;
	vector<vector<Point> > contours_all;
	vector<Vec4i> hierarchy_all;
	cvtColor(drawing2, gray_all, COLOR_BGR2GRAY);


	threshold(gray_all, threshold_output_all, 45, 255, THRESH_BINARY);
	findContours(threshold_output_all, contours_all, hierarchy_all, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));//RETR_EXTERNAL表示只寻找最外层轮廓

	Point2f fourPoint2f[4];
	//求最小包围矩形
	RotatedRect rectPoint = minAreaRect(contours_all[0]);

	//将rectPoint变量中存储的坐标值放到 fourPoint的数组中
	rectPoint.points(fourPoint2f);


	for (int i = 0; i < 4; i++)
	{
		line(src_all, fourPoint2f[i % 4], fourPoint2f[(i + 1) % 4], Scalar(20, 21, 237), 3);
	}

	namedWindow("Src_all");
	imshow("Src_all", src_all);

	//框出二维码后，就可以提取出二维码，然后使用解码库zxing，解出码的信息。
	//或者研究二维码的排布规则，自己写解码部分

	waitKey(0);
	return(0);
}

