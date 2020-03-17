#include "QRCodeTools.h"
#include <iostream>


QRCodeTools::QRCodeTools(int v) :Ver(v), sz(4 * (v - 1) + 21), dbg(false), maskID(0) {}

QRCodeTools::~QRCodeTools(){}

void QRCodeTools::setDbg(bool flag)
{
	dbg = flag;
}

QRDecodeTools::QRDecodeTools(int v, float e) : QRCodeTools(v), Eps(e){
	setThreshold(e);
}

QRDecodeTools::~QRDecodeTools()
{
	if (dbg)
	std::cout << "QRDecodeTools destructed." << std::endl;
	img.~Mat();
}

int QRCodeTools::mask(const int x, const int y) const
{
	if (y < 6 && y > 2 && x == 10)
		return 0;
	switch (maskID) {
	case 0: return (x + y) & 1;
	case 1: return y & 1;
	case 2: return x % 3 == 0;
	case 3: return (x + y) % 3==0;
	case 4: return ((y & 1) + (x % 3)) &1;
	case 5: return (((x * y) & 1) + x * y % 3) % 3==0;
	case 6: return (((x * y) & 1) + x * y % 3) & 1;
	case 7: return ((x * y) % 3 + (x + y) & 1) & 1;
	}
	return 0;
}

int QRCodeTools::checkPos(const int x, const int y) const //return 1 if err
{
	int chk = 0;
	if (x < 0 || x >= sz || y < 0 || y >= sz)	chk = 1;
	else if (x == 6 || y == 6)	chk = 1;
	else if ((x < 8 && y < 8) || (x < 8 && sz - y < 9) || (sz - x < 9 && y < 8))	chk = 1;
	else if (y > 2 && y < 6 && x == 10)	chk = 1;
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
	if (!gotQR)
		return false;
	if (in.channels() == 3)
		cv::cvtColor(in, img, cv::COLOR_BGR2GRAY);
	else
		in.getMat().copyTo(img);
	maskID = ((img.at<uchar>(map(10, 3)) &1) << 2) + ((img.at<uchar>(map(10, 4)) & 1)<< 1) + (img.at<uchar>(map(10, 5)) & 1);
	if (dbg)
		std::cout << "maskID=" << maskID << std::endl;
	return true;
}

int QRDecodeTools::detected()
{
	if (dbg && gotQR)
		std::cout << Rect[0] << Rect[1] << Rect[2] << Rect[3] << std::endl;
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
	if (val <= Eps * UCHAR_MAX)	return mask(x, y);
	else if (val >= (1 - Eps) * UCHAR_MAX)	return !mask(x, y);
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
	maskID = 0;
	Rect.clear();
}

QREncodeTools::QREncodeTools(int v):QRCodeTools(v){
	src = cv::Mat(cv::Size(sz + 6, sz + 6), CV_8UC1, cv::Scalar(0));
	makeSrc();
	flush();
}

void QREncodeTools::flush()
{
	maskID = 0;
	src.copyTo(img);
}

int QREncodeTools::write(const int x, const int y, const int bit)
{
	if (checkPos(x, y)) {
		//std::cout << "Pos error:(" << x << "," << y << ") can't be written." << std::endl;
		return -1;
	}
	img.at<uchar>(cv::Point(x + 3, y + 3)) = (bit ^ mask(x, y)) ? UCHAR_MAX : 0;
	if (dbg)
		std::cout << "wrote " << bit << " at " << cv::Point(x, y) << std::endl;
	return 0;
}

void QREncodeTools::display()const
{
	std::cout << "Display QRCode:" << std::endl;
	for (int i = 0; i < sz; ++i) {
		for (int j = 0; j < sz; ++j)
			std::cout << (img.at<uchar>(cv::Point(i + 3, j + 3)) ? "■":"□" );
		std::cout << std::endl;
	}
}

cv::Size QREncodeTools::output(cv::OutputArray out, int rate)
{
	//img.copyTo(out);
	if (checkValid(rate)) {
		std::cerr << "this frame can't be encoded." << std::endl;
		//std::cerr << "-1";
		return cv::Size(0, 0);
	}
	//std::cout << "Here";
	cv::Size outSize(sz + 6, sz + 6);
	outSize *= rate;
	if (dbg) {
		std::cout << "MaskID=" << maskID << std::endl;
	}
	img.at<uchar>(cv::Point(10+3, 3+3)) = ((maskID >> 2) & 1) ? UCHAR_MAX : 0;
	img.at<uchar>(cv::Point(10+3, 4+3)) = ((maskID >> 1) & 1) ? UCHAR_MAX : 0;
	img.at<uchar>(cv::Point(10+3, 5+3)) = (maskID & 1) ? UCHAR_MAX : 0;
	cv::resize(img, out, outSize, rate, rate, cv::INTER_NEAREST);
	return outSize;
}

void QREncodeTools::makeSrc() //生成二维码模板
{
	drawLocator(6, 6); //画四个定位块
	drawLocator(6, sz - 1);
	drawLocator(sz - 1, 6);
	//drawLocator(sz - 1, sz - 1);
	for (int i = 9; i < sz - 3; ++i) { //画时间线
		src.at<uchar>(cv::Point(i, 9)) = (i & 1) ? 0 : 255;
		src.at<uchar>(cv::Point(9, i)) = (i & 1) ? 0 : 255;
	}
	for (int i = 0; i < sz + 6; ++i) //清空边框
		for (int j = 0; j < 3; ++j) {
			src.at<uchar>(cv::Point(i, j)) = 255;
			src.at<uchar>(cv::Point(j, i)) = 255;
			src.at<uchar>(cv::Point(i, sz - j + 5)) = 255;
			src.at<uchar>(cv::Point(sz - j + 5, i)) = 255;
		}
}

void QREncodeTools::drawLocator(const int x, const int y)
{
	int t;
	for (int i = x - 4; i < x + 5; ++i)
		for (int j = y - 4; j < y + 5; ++j) {
			t = std::max(labs(i - x), labs(j - y));
			src.at<uchar>(cv::Point(i, j)) = (t == 2 || t == 4) ? 255 : 0;
		}
}

int QREncodeTools::checkValid(const int rate)
{

	cv::resize(img, tmp, cv::Size(sz + 6, sz + 6)*rate, rate, rate, cv::INTER_NEAREST);
	if (maskID) {
		if (dbg)
			std::cout << "Change mask:" << maskID << std::endl;
		//cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);
		//imshow("Image", tmp);
		//cv::waitKey();
	}

	//imwrite("tmp.jpg", tmp);
	if (wheels.detect(tmp, Rect) && (epseq(Rect[0].y, Rect[1].y) && epseq(Rect[0].x, Rect[3].x) && epseq(Rect[1].x, Rect[2].x) && epseq(Rect[2].y, Rect[3].y)))
		return 0;
	return regenerate(rate);
}

int QREncodeTools::regenerate(const int rate)
{
	int bit;
	if (maskID == 7) {
		return -1;
	}
	for (int i = 0; i < sz; ++i)
		for (int j = 0; j < sz; ++j) 
		if (!checkPos(i, j)){
			bit = img.at<uchar>(cv::Point(i + 3, j + 3)) & 1;
			bit ^= mask(i, j);
			img.at<uchar>(cv::Point(i + 3, j + 3)) = bit;
		}
	maskID++;
	for (int i = 0; i < sz; ++i)
		for (int j = 0; j < sz; ++j) 
			if (!checkPos(i, j)) {
			bit = img.at<uchar>(cv::Point(i + 3, j + 3));
			bit ^= mask(i, j);
			img.at<uchar>(cv::Point(i + 3, j + 3)) = bit ? UCHAR_MAX : 0;
		}
	return checkValid(rate);
}

int QREncodeTools::epseq(float x, float y) const
{
	//std::cout << "Comparing" << x << ' ' << y << std::endl;
	return fabs(x - y) < 1e-1;
}
