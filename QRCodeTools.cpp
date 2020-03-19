#include "QRCodeTools.h"
#include <iostream>

QRCodeTools::QRCodeTools(int v) :Ver(v), sz(4 * (v - 1) + 21), dbg(false), maskID(0) {}

QRCodeTools::~QRCodeTools() {}

void QRCodeTools::setDbg(bool flag)
{
	dbg = flag;
}

QRDecodeTools::QRDecodeTools(int v, float e) : QRCodeTools(v), Eps(e) {
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
	//if (x < 6 && x > 2 && y == 10)
	//	return 0;
	//switch (maskID) {
	//case 0: return (x + y) & 1;
	//case 1: return y & 1;
	//case 2: return x % 3 == 0;
	//case 3: return (x + y) % 3 == 0;
	//case 4: return ((y & 1) + (x % 3)) & 1;
	//case 5: return (((x * y) & 1) + x * y % 3) % 3 == 0;
	//case 6: return (((x * y) & 1) + x * y % 3) & 1;
	//case 7: return ((x * y) % 3 + (x + y) & 1) & 1;
	//}
	return 0;
}

int QRCodeTools::checkPos(const int x, const int y) const //return 1 if err
{
	int chk = 0;
	if (x < 0 || x >= sz || y < 0 || y >= sz)	chk = 1;
	else if (x == 6 || y == 6)	chk = 1;
	else if ((x < 8 && y < 8) || (x < 8 && sz - y < 9) || (sz - x < 9 && y < 8))	chk = 1;
	else if (x > 2 && x < 6 && y == 10)	chk = 1;
	return chk;
}

cv::Point QRDecodeTools::map(const int a, const int b) const
{
	cv::Point2f x0 = rate(a, Rect[0], Rect[1]), x1 = rate(a, Rect[3], Rect[2]),
		y0 = rate(b, Rect[0], Rect[3]), y1 = rate(b, Rect[1], Rect[2]);
	//std::cout << x0 << ' ' << x1 << ' ' << y0 << ' ' << y1 << ' ' << std::endl;
	float A1 = x1.y - x0.y, B1 = x0.x - x1.x, C1 = x0.y * x1.x - x0.x * x1.y,
		A2 = y1.y - y0.y, B2 = y0.x - y1.x, C2 = y0.y * y1.x - y0.x * y1.y;
	return cv::Point((int)round((B1 * C2 - B2 * C1) / (A1 * B2 - A2 * B1)), (int)round((A1 * C2 - A2 * C1) / (B1 * A2 - B2 * A1)));
}

cv::Point2f QRDecodeTools::rate(const int p, const cv::Point2f& a, const cv::Point2f& b) const
{
	float f = (float)(p + 0.5) / sz;
	return cv::Point2f(f * b.x + (1 - f) * a.x, f * b.y + (1 - f) * a.y);
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
	maskID = ((img.at<uchar>(map(3, 10)) & 1) << 2) + ((img.at<uchar>(map(4, 10)) & 1) << 1) + (img.at<uchar>(map(5, 10)) & 1);
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
		std::cout << "Pos error:(" << x << "," << y << ") can't be read." << std::endl;
		return -1;
	}
	val = img.at<uchar>(map(x, y));
	if (dbg)
		std::cout << "got " << val << " at " << map(x, y) << std::endl;
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

QREncodeTools::QREncodeTools(int v) :QRCodeTools(v) {
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
			std::cout << (img.at<uchar>(cv::Point(i + 3, j + 3)) ? "■" : "□");
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
	img.at<uchar>(cv::Point(3 + 3, 10 + 3)) = ((maskID >> 2) & 1) ? UCHAR_MAX : 0;
	img.at<uchar>(cv::Point(4 + 3, 10 + 3)) = ((maskID >> 1) & 1) ? UCHAR_MAX : 0;
	img.at<uchar>(cv::Point(5 + 3, 10 + 3)) = (maskID & 1) ? UCHAR_MAX : 0;
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

	cv::resize(img, tmp, cv::Size(sz + 6, sz + 6) * rate, rate, rate, cv::INTER_NEAREST);
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
			if (!checkPos(i, j)) {
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

void Demo(char str[]) {
	QRDecodeTools index;
	cv::Mat img = cv::imread(str);/*
    cv::imshow("0", img);
	cv::waitKey(50);*/
	if (img.empty())return;
	if (!img.data)return;
	index.flush();
	index.loadQRCode(img);
	if (!index.detected())return;
	int i, j, size = 7, arr[Rows][Cols];
	for (i = 0; i < size; i++)
		arr[0][i] = arr[0][Cols - size + i] = arr[size - 1][i] = arr[size - 1][Cols - size + i] = arr[Rows - size][i] = arr[Rows - 1][i] = 0;
	for (i = 1; i < size - 1; i++)
		arr[i][0] = arr[i][size - 1] = arr[i][Cols - size] = arr[i][Cols - 1] = arr[Rows - size + i][0] = arr[Rows - size + i][size - 1] = 0;
	for (i = 1; i < size - 1; i++)
		arr[1][i] = arr[1][Cols - size + i] = arr[size - 2][i] = arr[size - 2][Cols - size + i] = arr[Rows - size + 1][i] = arr[Rows - 2][i] = 1;
	for (i = 2; i < size - 2; i++)
		arr[i][1] = arr[i][size - 2] = arr[i][Cols - size + 1] = arr[i][Cols - 2] = arr[Rows - size + i][1] = arr[Rows - size + i][size - 2] = 1;
	for (i = 0; i < size - 4; i++)
		for (j = 0; j < size - 4; j++)
			arr[2 + i][2 + j] = arr[2 + i][Cols - size + 2 + j] = arr[Rows - size + 2 + i][2 + j] = 0;
	for (i = 0; i < size + 1; i++)
		arr[size][i] = arr[size][Cols - 1 - i] = arr[Rows - size - 1][i] = 1;
	for (i = 0; i < size; i++)
		arr[i][size] = arr[i][Cols - size - 1] = arr[Rows - 1 - i][size] = 1;
	bool blo = false;
	for (i = 0; i < Rows - 2 * size - 2; i++, blo = !blo)
		arr[size - 1][size + 1 + i] = arr[size + 1 + i][size - 1] = blo;
	
	FILE* filePoint = NULL;
	fopen_s(&filePoint, "test022802.bin", "wb");
	try {
		for (i = 0; i < size; i++) {
			if (i == size - 1)i++;
			for (j = 8; j < Cols - size - 1; j++)
				arr[j][i] = index.read(j, i);
		}
		for (i = 0; i < size; i++) {
			if (i == size - 1)i++;
			for (j = 8; j < Rows - size - 1; j++)
				arr[i][j] = index.read(i, j);
		}
		for (i = 8; i < Cols; i++)
			for (j = 8; j < Rows; j++)
				arr[i][j] = index.read(i, j);
	}
	catch (int e) {
		std::cout << "Fuzz: val=" << e << std::ends << i << std::ends << j << std::endl;
	}
	for (int i = 0; i < Cols; i++)
		for (int j = 0; j < Rows; j++) {
			char temp;
			if (arr[i][j])
				temp = 255;
			else
				temp = 0;
			fwrite(&temp, sizeof(char), 1, filePoint);
		}
	fclose(filePoint);
}

void QREncodeTools::WriteFrame(bool datamatrix[][TSIZE], int flames)
{
	Mat img;
	char name[20];
	sprintf_s(name, "%s%d%s", "image", flames, ".jpg");
	for (int i = 0; i < AreaCount; i++)
		for (int k = areapos[i][1][1]; k < areapos[i][0][1] + areapos[i][1][1]; k++)
			for (int j = areapos[i][1][0]; j < areapos[i][0][0] +areapos[i][1][0]; j++)
				write(j, k,datamatrix[j][k]);
	output(img);
	flush();
	imwrite(name, img);
}

void QRDecodeTools::ReadFrame(const char* str, bool datamatrix[][TSIZE])
{
	Mat img = imread(str);
	flush();
	loadQRCode(img);
	int len = 0;
	for (int i = 0; i < AreaCount; i++)
		for (int k = areapos[i][1][1]; k < areapos[i][0][1] + areapos[i][1][1]; k++)
			for (int j = areapos[i][1][0]; j < areapos[i][0][0] + areapos[i][1][0]; j++)
			{
				if(!((j==3||j==4||j==5)&&k==10))
				datamatrix[j][k] = read(j, k);
			}
}

bool readFile(const char str[]) {
	QREncodeTools Frame;
	FILE* filePoint = NULL;// fopen(str, "rb");
	fopen_s(&filePoint, str, "rb");
	char ch;
	int i, j, index, flame = 1;
	bool arr[TSIZE][TSIZE];
	const int inforlens = ((TSIZE - 6) * TSIZE - 153) / 8;
	for (i = 0; i < TSIZE; i++)
		for (j = 0; j < TSIZE; j++)
			arr[i][j] = true;
	while (!feof(filePoint)) 
	{
		for (index = i = 0, j = 8; !feof(filePoint) && index < inforlens; index++) {
			fread(&ch, sizeof(char), 1, filePoint);
			if (i >= 0 && i <= 5 && j >= 8) {
				for (int k = 0; k < 8; k++) {
					arr[i][j++] = ch >> k & 1;
					if (j >= TSIZE - 9) {
						j = 8;
						i++;
						if (i > 5) {
							i = 13;
							j = 0;
						}
					}
				}
			}
			else if (i > 6 && j >= 0 && j <= 7) {
				for (int k = 0; k < 8; k++) {
					arr[i][j++] = ch >> k & 1;
					if (j >= 6 && j < 8) {
						j = 0;
						i++;
						if (i >= TSIZE - 9) {
							i = 8;
							j = 8;
						}
					}
				}
			}
			else if (i >= 8 && j >= 8) {
				for (int k = 0; k < 8; k++) {
					//cout << index << ends <<i << ends << j << endl;
					arr[i][j++] = ch >> k & 1;
					if (j >= TSIZE - 1) {
						j = 8;
						i++;
					}
				}
			}
		}
		if (index == inforlens)
			process(arr, flame, -1);
		else
			process(arr, flame, index - 1);/*
		for (i = 0; i < TSIZE; i++) {
			for (j = 0; j < TSIZE; j++) {
				if (arr[i][j])
					cout << 1;// << ends;
				else
					cout << 0;// << ends;
			}
			cout << endl;
		}
		cout << endl;*/
		Frame.WriteFrame(arr, flame++);
		//writeFile(arr);
	}
	return true;
}

void process(bool arr[][TSIZE], int flame, int len) {
	int i = 8, j = 0, times = 0;

	/*填充帧编号*/
	for (int k = 0; k < 14; k++) {
		arr[i][j++] = flame >> k & 1;
		if (j >= 6) {
			i++;
			j = 0;
		}
	}

	/*如果len = -1，说明该帧为完整帧，结束帧标记置0，帧信息长度也置0*/
	if (len == -1) {
		arr[i][j] = 1;
		i++, j = 0;
		for (int k = 11; k >= 0; k--) {
			arr[i][j++] = 1;
			if (j >= 6) {
				i++;
				j = 0;
			}
		}
	}

	/*如果len!=0，说明已经是最后一帧，帧结束标记置1，帧信息长度为len*/
	else {
		arr[i][j] = 0;
		i++, j = 0;
		for (int k = 0; k < 12; k++) {
			arr[i][j++] = len >> k & 1;
			if (j >= 6) {
				i++;
				j = 0;
			}
		}
	}

	/*数据区1的行校验码*/
	for (i = 0; i < 6; i++) {
		for (j = 8, times = 0; j < TSIZE - 9; j++)
			if (arr[i][j])
				times++;
		if (times % 2)
			arr[i][j] = 1;
		else
			arr[i][j] = 0;
	}

	/*数据区1的列校验码*/
	for (j = 8; j < TSIZE - 8; j++) {
		for (i = times = 0; i < 6; i++)
			if (arr[i][j])
				times++;
		if (times % 2)
			arr[i + 1][j] = 1;
		else
			arr[i + 1][j] = 0;
	}

	/*数据区2的列校验码*/
	for (j = 0; j < 6; j++) {
		for (i = 8, times = 0; i < TSIZE - 9; i++)
			if (arr[i][j])
				times++;
		if (times % 2)
			arr[i][j] = 1;
		else
			arr[i][j] = 0;
	}

	/*数据区2的行校验码*/
	for (i = 8; i < TSIZE - 8; i++) {
		for (j = times = 0; j < 6; j++)
			if (arr[i][j])
				times++;
		if (times % 2)
			arr[i][j + 1] = 1;
		else
			arr[i][j + 1] = 0;
	}

	/*数据区3的行校验码*/
	for (i = 8; i < TSIZE - 1; i++) {
		for (j = 8, times = 0; j < TSIZE - 1; j++)
			if (arr[i][j])
				times++;
		if (times % 2)
			arr[i][j] = 1;
		else
			arr[i][j] = 0;
	}

	/*数据区3的列校验码*/
	for (j = 8; j < TSIZE; j++) {
		for (i = 8, times = 0; i < TSIZE - 1; i++)
			if (arr[i][j])
				times++;
		if (times % 2)
			arr[i][j] = 1;
		else
			arr[i][j] = 0;
	}
}

bool writeFile(bool arr[][TSIZE]) {
	const int inforlens = ((TSIZE - 6) * TSIZE - 153) / 8;
	char str[inforlens + 10];
	bool ends = false, frameNUM[14] = { false }, terminalLEN[12] = { false }, Byte[8] = { false };
	int i = 8, j = 0, k, framenum = 0, terminallen = 0, rows = 0, cols = 0, times;
	
	/*for (i = 0; i < TSIZE; i++) {
		for (j = 0; j < TSIZE; j++) {
			if (arr[i][j])
				cout << 1;// << ends;
			else
				cout << 0;// << ends;
		}
		cout << endl;
	}
	cout << endl;*/

	/*读取帧编码*/
	for (k = 0; k < 14; k++) {
		frameNUM[k] = arr[i][j++];
		if (j >= 6) {
			i++;
			j = 0;
		}
	}
	framenum = BTD(frameNUM, 14);//二进制转化

	/*读取是否为最后一帧，最后一帧提取结束帧信息长度*/
	if (arr[i][j])
		terminallen = inforlens;
	else {
		int k = 0;
		for (i = 11; i < 13; i++)
			for (j = 0; j < 6; j++)
				terminalLEN[k++] = arr[i][j];
		terminallen = BTD(terminalLEN, k);
		ends = true;
	}

	/*验证数据区1的信息*//*
	for (i = 0; i < 6; i++) {
		for (j = 8, times = 0; j < TSIZE - 9; j++)
			if (arr[i][j])
				times++;
		if ((times % 2) ^ arr[i][j])
			rows = i;
	}
	for (j = 8; j < TSIZE - 8; j++) {
		for (i = times = 0; i < 6; i++)
			if (arr[i][j])
				times++;
		if ((times % 2) ^ arr[i + 1][j]){
			arr[rows][j] = !arr[rows][j];
			//cout << rows << "     " << j << endl;
		}
	}

	/*验证数据区2的信息*//*
	for (j = 0; j < 6; j++) {
		for (i = 8, times = 0; i < TSIZE - 9; i++)
			if (arr[i][j])
				times++;
		if ((times % 2) ^ arr[i][j])
			cols = i;
	}
	for (i = 8; i < TSIZE - 8; i++) {
		for (j = times = 0; j < 6; j++)
			if (arr[i][j])
				times++;
		if ((times % 2) ^ arr[i][j + 1])
			arr[i][cols] = !arr[i][cols];
	}

	/*验证数据区3的信息*//*
	for (i = 8; i < TSIZE - 1; i++) {
		for (j = 8, times = 0; j < TSIZE - 1; j++)
			if (arr[i][j])
				times++;
		if ((times % 2) ^ arr[i][j])
			rows = i;
	}
	for (j = 8; j < TSIZE; j++) {
		for (i = times = 8; i < TSIZE - 1; i++)
			if (arr[i][j])
				times++;
		if ((times % 2) ^ arr[i][j])
			arr[rows][j] = !arr[rows][j];
	}

	/*读取信息*/
	for (k = 0, i = 0, j = 8; k < terminallen; k++) {
		for (times = 0; times < 8; times++) {
			Byte[times] = arr[i][j++];
			if (i >= 0 && i <= 5 && j >= 8) {
				if (j >= TSIZE - 9) {
					j = 8;
					i++;
					if (i > 5) {
						i = 13;
						j = 0;
					}
				}
			}
			else if (i > 6 && j >= 0 && j <= 7) {
				if (j >= 6 && j < 8) {
					j = 0;
					i++;
					if (i >= TSIZE - 9) {
						i = 8;
						j = 8;
					}
				}
			}
			else if (i >= 8 && j >= 8) {
				if (j >= TSIZE - 1) {
					j = 8;
					i++;
				}
			}
		}
		str[k] = (char)BTD(Byte, 8);
	}
	str[k] = '\0';
	std::cout << str;
	return ends;
}

int BTD(bool arr[], int lens) {
	int sums = 0, j = 1;
	for (int i = 0; i < lens; i++, j = j << 1)
		sums += arr[i] * j;
	return sums;
}

void readframes()
{
	QRDecodeTools Frame;
	bool arr[TSIZE][TSIZE] = { 1 };
	int i = 1;
	char name[20];
	while (1)
	{
		sprintf_s(name, "%s%d%s", "image", i++, ".jpg");
		Frame.ReadFrame(name, arr);
		if (writeFile(arr))
			break;
	}
}

