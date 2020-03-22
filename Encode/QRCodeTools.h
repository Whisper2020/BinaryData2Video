#pragma once
#include "const.h"
#include <vector>
#include <iostream>
#include "opencv2\opencv.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace cv;
using namespace std;

class QRCodeTools { //两种工具的基类
public:
	QRCodeTools(int v = VERSION); //v是二维码的版本
	~QRCodeTools();
	void setDbg(bool flag);//设置调试标志=1每次输出read/write的实际像素坐标。默认=0
	virtual void flush() = 0;
protected:
	int Ver, sz, maskID; //Ver 二维码的版本，sz 长宽所占像素
	bool dbg; //调试标志
	int mask(const int x, const int y)const; //上掩膜。
	cv::Mat img;
	cv::QRCodeDetector wheels;
	std::vector <cv::Point2f> Rect;
	int checkPos(const int x, const int y)const; //检查坐标是否是可写入的合法坐标
	const int AreaCount = 11;
	const int QrLocaterSize = 8;
	const int QrSize = 4 * (Ver - 1) + 21;
	const int areapos[11][2][2] =//[2][2]第一维度为宽高，第二维度为左上角坐标
	{
	 {{QrLocaterSize - 2,3} ,{0,QrLocaterSize}},//帧编码与结束帧标记
	 {{QrLocaterSize - 2,2},{0,QrLocaterSize + 3}},//结束帧信息长度

	 {{QrLocaterSize - 2,QrSize - 5 - 2 * QrLocaterSize},{0,QrLocaterSize + 5}},//左侧数据区
	 {{QrSize - 2 * QrLocaterSize,QrLocaterSize - 2},{QrLocaterSize,0}},//上侧数据区
	 {{QrSize - 1 - QrLocaterSize,QrSize - 1 - QrLocaterSize},{QrLocaterSize,QrLocaterSize}},//中央数据区

	 {{1,QrSize - 5 - 2 * QrLocaterSize},{QrLocaterSize - 1,QrLocaterSize}},//左侧行校验码
	 {{QrLocaterSize - 2,1},{0,QrSize - 1 - QrLocaterSize}},//左侧列校验码
	 {{1,QrLocaterSize - 2},{QrSize - 1 - QrLocaterSize,0}},//上侧行校验码
	 {{QrSize - 2 * QrLocaterSize,1},{QrLocaterSize,QrLocaterSize - 1}},//上侧列校验码
	 {{1,QrSize - QrLocaterSize},{QrSize - 1,QrLocaterSize}},//中央行校验码
	 {{QrSize - QrLocaterSize,1},{QrLocaterSize,QrSize - 1}}//中央列校验码
	};
};
class QREncodeTools : public QRCodeTools {
public:
	QREncodeTools(int v = VERSION);
	void flush(); //!!写完一张图片后要刷新
	int write(const int x, const int y, const int bit); //依次为x,y（从0开始）,bit=0写入黑色，bit=1写入白色

	void display()const; //在stdout临时查看编码结果，需要把控制台的行宽调大
	cv::Size output(cv::OutputArray, int rate = 7); //输出到OutputArray。默认rate=7,一个原像素=7*7，返回的Size是帧的宽高
	void WriteFrame(bool datamatrix[][TSIZE], int flames);
private:
	cv::Mat src, tmp;
	void makeSrc();
	void drawLocator(const int x, const int y);
	int checkValid(const int rate);
	int regenerate(const int rate);
	inline int epseq(float x, float y)const;
};

int readFile(const char str[]);
void process(bool arr[][TSIZE], int flame, int len);

void Image_to_Video_Generate(int num, int frame_width, int frame_height, float video_time, string video_name);