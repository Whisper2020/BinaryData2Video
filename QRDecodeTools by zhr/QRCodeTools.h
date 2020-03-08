#pragma once
#include "opencv2/opencv.hpp"
#include <vector>
class QRDecodeTools
{

public:	
	QRDecodeTools(int v = 16, float e = 0.2f); //v是二维码的版本，大小=4 * (v - 1) + 21，默认（无参）即可（sz=81*81，Ver=16），e是检测阈值，默认0.2
	int loadQRCode(cv::InputArray in); //加载并检测图片，如果检测到二维码返回1，否则返回0
	int detected(); //是否检测到二维码，检测到返回True，否则返回0
	int read(const int x, const int y)const; //获取对应坐标（x,y）的编码值，坐标越界则返回-1，正常读取（在阈值内）则返回0（代表黑色），或1（代表白色），不清楚则抛出uchar的异常代表灰度值。debug=1时输出像素点位置。
	void setThreshold(const float x); //设定检测阈值，范围[0,0.5)。默认为0.2 即：read(x,y)时，像素灰度[0,0.2*255]为白色，[0.8*255,255]为黑色，范围外则认为模糊，抛出异常
	void flush(); //!!加载新的帧之前都要刷新
	void setDbg(bool flag); //设置调试标志=1输出read的实际像素坐标和析构函数执行标志。默认=0
	~QRDecodeTools();
private:
	int Ver, sz, gotQR;
	bool dbg;
	cv::Point dx, dy, lx, ly;
	float Eps;
	cv::QRCodeDetector wheels;
	cv::Mat img;
	std::vector <cv::Point2f> Rect;
	int checkPos(const int x, const int y)const; //检查坐标是否合法
	cv::Point map(const int x, const int y) const; //read坐标到像素点坐标的映射
	cv::Point2f rate(const int p, const cv::Point2f& a, const cv::Point2f& b)const;
};

