#include "QRCodeTools.h"
#include <iostream>
QRDecodeTools::QRDecodeTools(int v, float e) :Ver(v), sz(4 * (v - 1) + 21), dbg(false) {
	setThreshold(e);
}
QRDecodeTools::~QRDecodeTools()
{
	if (dbg)
	std::cout << "QRDecodeTools destructed." << std::endl;
	img.~Mat();
}

int QRDecodeTools::checkPos(const int x, const int y) const //return 1 if err
{
	int chk = 0;
	if (x < 0 || x >= sz || y < 0 || y >= sz)	chk = 1;
	else if (x == 6 || y == 6)	chk = 1;
	else if ((x < 8 && y < 8) || (x < 8 && sz - y < 9) || (sz - x < 9 && sz - y < 9))	chk = 1;
	return chk;
}

cv::Point QRDecodeTools::map(const int a, const int b) const
{
	cv::Point2f x0 = rate(a, Rect[0], Rect[1]), x1 = rate(a, Rect[3], Rect[2]),
		y0 = rate(b, Rect[0], Rect[3]), y1 = rate(b, Rect[1], Rect[2]);
	//std::cout << x0 << ' ' << x1 << ' ' << y0 << ' ' << y1 << ' ' << std::endl;
	float A1 = x1.y - x0.y, B1 = x0.x - x1.x, C1 = x0.y*x1.x - x0.x*x1.y,
		A2 = y1.y - y0.y, B2 = y0.x - y1.x, C2 = y0.y*y1.x - y0.x*y1.y;
	return cv::Point((int)round((B1*C2-B2*C1)/(A1*B2-A2*B1)), (int)round((A1*C2-A2*C1)/(B1*A2-B2*A1)));
}

cv::Point2f QRDecodeTools::rate(const int p, const cv::Point2f& a, const cv::Point2f& b) const
{
	float f = (float)(p + 0.5) / sz;
	return cv::Point2f(f*b.x + (1-f)*a.x, f*b.y + (1-f)*a.y);
}


int QRDecodeTools::loadQRCode(cv::InputArray in)
{
	gotQR = wheels.detect(in, Rect);
	if (in.channels() == 3)
		cv::cvtColor(in, img, cv::COLOR_BGR2GRAY);
	else
		in.getMat().copyTo(img);
	if (gotQR) {
		lx = Rect[1] - Rect[0];
		ly = Rect[2] - Rect[1];

	}
	return gotQR;
}

int QRDecodeTools::detected()
{
	return gotQR;
}

int QRDecodeTools::read(const int x, const int y) const
{
	int val;
	if (!gotQR)	return -1;
	if (checkPos(x, y)) {
		std::cout << "Pos error:(" << x << "," << y <<") can't be read."<< std::endl;
		return -1;
	}
	val = img.at<uchar>(map(x, y));
	if (dbg)
		std::cout << "got "<< val<<" at "<< map(x, y) <<std::endl;
	if (val <= Eps * UCHAR_MAX)	return 0;
	else if (val >= (1 - Eps) * UCHAR_MAX)	return 1;
	else throw val;
}

void QRDecodeTools::setThreshold(const float x)
{
	if (x < 0.0f || x >= 0.5f)
		std::cout << "Error. Eps can't be" << x << std::endl;
	Eps = x;
}


void QRDecodeTools::flush()
{
	gotQR = 0;
	Rect.clear();
}

void QRDecodeTools::setDbg(bool flag)
{
	dbg = flag;
}
