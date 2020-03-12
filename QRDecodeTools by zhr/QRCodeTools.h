#pragma once
#include "opencv2/opencv.hpp"
#include <vector>
class QRCodeTools { //两种工具的基类
public:
	QRCodeTools(int v = 16); //v是二维码的版本
	~QRCodeTools();
	void setDbg(bool flag);//设置调试标志=1每次输出read/write的实际像素坐标。默认=0
	virtual void flush()=0;
protected:
	int Ver, sz; //Ver 二维码的版本，sz 长宽所占像素
	bool dbg; //调试标志
	cv::Mat img;
	int checkPos(const int x, const int y)const; //检查坐标是否是可写入的合法坐标
};
class QREncodeTools: public QRCodeTools {
public:
	QREncodeTools(int v = 16);
	void flush(); //!!写完一张图片后要刷新
	int write(const int x, const int y, const int bit); //依次为x,y（从0开始）,bit=0写入黑色，bit=1写入白色
	void mask(); //上掩膜，待补充。
	void display()const; //在stdout临时查看编码结果，需要把控制台的行宽调大
	cv::Size output(cv::OutputArray, int rate = 7); //输出到OutputArray。默认rate=7,一个原像素=7*7，返回的Size是帧的宽高
private:
	cv::Mat src;
	void makeSrc();
	void drawLocator(const int x, const int y);
};
class QRDecodeTools: public QRCodeTools
{
public:	
	QRDecodeTools(int v = 16, float e = 0.2f); //v默认（无参）即可（sz=81*81，Ver=16），e是检测阈值，默认0.2
	int loadQRCode(cv::InputArray in); //加载并检测图片，如果检测到二维码返回1，否则返回0
	int detected(); //load之后用来判断是否检测到二维码，检测到返回True，否则返回0
	int read(const int x, const int y)const; //获取对应坐标（x,y）的编码值，坐标越界则返回-1，正常读取（在阈值内）则返回0（代表黑色），或1（代表白色），不清楚则抛出uchar的异常代表灰度值。debug=1时输出像素点位置。
	void setThreshold(const float x); //设定检测阈值，范围[0,0.5)。默认为0.2 即：read(x,y)时，像素灰度[0,0.2*255]为白色，[0.8*255,255]为黑色，范围外则认为模糊，抛出异常
	void flush(); //!!loadQRCode(新的帧)之前都要刷新
	~QRDecodeTools();
private:
	int gotQR;
	float Eps;
	cv::QRCodeDetector wheels;
	std::vector <cv::Point2f> Rect;
	cv::Point map(const int x, const int y) const; //read坐标到像素点坐标的映射
	cv::Point2f rate(const int p, const cv::Point2f& a, const cv::Point2f& b)const;
};

