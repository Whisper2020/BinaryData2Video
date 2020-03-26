#include "QRCodeTools.h"

QRCodeTools::QRCodeTools(int v) :Ver(v), sz(4 * (v - 1) + 21), dbg(false), maskID(0) {}

QRCodeTools::~QRCodeTools() {}

void QRCodeTools::setDbg(bool flag)
{
	dbg = flag;
}

QRDecodeTools::QRDecodeTools(int v, float e, int r) : QRCodeTools(v), Eps(e), rounD(r) {
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
	if (x < 6 && x > 2 && y == 10)
		return 0;
	switch (maskID) {
	case 0: return (x + y) & 1;
	case 1: return y & 1;
	case 2: return x % 3 == 0;
	case 3: return (x + y) % 3 == 0;
	case 4: return ((y & 1) + (x % 3)) & 1;
	case 5: return (((x * y) & 1) + x * y % 3) % 3 == 0;
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
	else if (x > 2 && x < 6 && y == 10)	chk = 1;
	//else if (y > 2 && y < 6 && x == 10)	chk = 1;
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
	if (dbg == true)
		std::cout << Rect[0] << Rect[1] << Rect[2] << Rect[3] << std::endl;
	if (in.channels() == 3)
		cv::cvtColor(in, img, cv::COLOR_BGR2GRAY);
	else
		in.getMat().copyTo(img);
	double xx = 0.0f, nx = 9999.0f, xy = 0.0f, ny = 9999.0f;
	for (int i = 0; i < 3; ++i) {
		xx = Rect[i].x > xx ? Rect[i].x : xx;
		xy = Rect[i].y > xy ? Rect[i].y : xy;
		nx = Rect[i].x < nx ? Rect[i].x : nx;
		ny = Rect[i].y < ny ? Rect[i].y : ny;
	}
	if (xy > img.rows || xx > img.cols || min(nx, ny) < 0) {
		gotQR = false;
		return false;
	}

	maskID = (readM(3, 10) << 2) + (readM(4, 10) << 1) + (readM(5, 10) & 1);
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

int QRDecodeTools::read(const int x, const int y)
{
	if (checkPos(x, y)) {
		std::cout << "Pos error:(" << x << "," << y << ") can't be read." << std::endl;
		return -1;
	}
	int ret = readM(x, y);
	if (dbg && gotQR) {
		int t;
		Point p = map(x, y);
		if (ret == -1)	t = 127;
		if (ret == 0)	t = 0;
		if (ret == 1)	t = UCHAR_MAX;
		for (int i = 0; i < 9; ++i)
			img.at<uchar>(p + Point(i / 3 - 1, i % 3 - 1)) = t;
	}

	if (ret == -1)
		return -1;
	return ret ^ mask(x, y);
}

void QRDecodeTools::setThreshold(const float x)
{
	if (x < 0.0f || x >= 0.5f)
		std::cout << "Error. Eps can't be" << x << std::endl;
	Eps = x;
}

void QRDecodeTools::flush()
{
	static int num = 0;
	char image_name[50];
	if (gotQR == true && dbg) {
		sprintf_s(image_name, "%s%d%s", "readimage", ++num, ".jpg");
		imwrite(image_name, img);
	}
	gotQR = 0;
	maskID = 0;
	Rect.clear();
}

void QRDecodeTools::ReadFrame(const char* str, int datamatrix[][TSIZE])
{
	Mat img = imread(str);
	flush();
	loadQRCode(img);
	int len = 0;
	for (int i = 0; i < sz; ++i)
		for (int j = 0; j < sz; ++j)
			if (!checkPos(i, j))
				datamatrix[j][i] = read(i, j);
}

int QRDecodeTools::readM(const int x, const int y) const
{
	int val = 0, f = rounD * 2 + 1;
	if (!gotQR)	return -1;
	Point p = map(x, y);
	for (int i = 0; i < f * f; ++i)
		val += img.at<uchar>(p + Point(i / f - rounD, i % f - rounD));
	val /= f * f;
	if (dbg);
		//std::cout << "got " << val << " at " << map(x, y) << std::endl;
	if (val >= (1 - 1.5*Eps) * UCHAR_MAX)	return 1;
	else if (val <= 2 * Eps * UCHAR_MAX)	return 0;
	else return -1;
}

bool writeFile(int arr[][TSIZE], FILE* fp, FILE* fw) {
	const int inforlens = ((TSIZE - 6) * TSIZE - 153) / 8;
	char ch;
	static int cherk = 1;
	bool ends = false, frameNUM[14] = { false }, terminalLEN[12] = { false }, Byte[8] = { false }, Cherk[8] = { false };
	int i = 8, j = 0, k, framenum = 0, terminallen = 0, rows = 0, cols = 0, times, cnt = 0;

	/*读取帧编码*/
	for (k = 0; k < 14; k++) {
		frameNUM[k] = arr[i][j++];
		if (j >= 6) {
			i++;
			j = 0;
		}
	}
	framenum = BTD(frameNUM, 14);//二进制转化
	cout << "当前帧编码：" << framenum << "\t应得帧编码" << cherk << endl;
	if (cherk > framenum)
		return false;
	while (cherk < framenum) {
		if (cherk + 20 < framenum)
			return false;
		ch = 0;
		for (int temp = 0; temp < inforlens; temp++) {
			fwrite(&ch, sizeof(char), 1, fp);
			fwrite(&ch, sizeof(char), 1, fw);
		}
		cherk++;
	}
	cherk++;

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
			cout << rows << "     " << j << endl;
		}
	}*/

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
	}*/

	/*验证数据区3的信息*//*
	for (i = 8; i < TSIZE - 1; i++) {
		for (j = 8, times = 0; j < TSIZE - 1; j++)
			if (arr[i][j])
				times++;
		if ((times % 2) ^ arr[i][j])
			rows = i;

	for (j = 8; j < TSIZE; j++) {
		for (i = times = 8; i < TSIZE - 1; i++)
			if (arr[i][j])
				times++;
		if ((times % 2) ^ arr[i][j])
		arr[rows][j] = !arr[rows][j];
	}*/

	/*读取信息*/
	for (k = 0, i = 0, j = 8; k < terminallen; k++) {
		for (times = 0; times < 8; times++) {
			if (arr[i][j] == -1) {
				Cherk[times] = 0;
				arr[i][j] = 0;
				cnt++;
			}
			else
				Cherk[times] = 1;
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
		ch = (char)BTD(Byte, 8);
		fwrite(&ch, sizeof(char), 1, fp);
		ch = (char)BTD(Cherk, 8);
		fwrite(&ch, sizeof(char), 1, fw);
	}
	cout << "Fuzz:" << cnt << endl;
	return ends;
}

int BTD(bool arr[], int lens) {
	int sums = 0, j = 1;
	for (int i = 0; i < lens; i++, j = j << 1)
		sums += arr[i] * j;
	return sums;
}

void readframes(char* str, char* err, int k)
{
	QRDecodeTools Frame;
	int arr[TSIZE][TSIZE] = { 1 };
	int i = 1;
	char name[20];
	FILE* fp2 = NULL;//
	fopen_s(&fp2, err, "wb");
	FILE* fp = NULL;//
	fopen_s(&fp, str, "wb");
	if (fp == NULL || fp2 == NULL)
		return;
	Frame.setDbg(true);
	while (i <= k)
	{
		sprintf_s(name, "%s%d%s", "image", i++, ".jpg");
		cout << "正在识别" << name << endl;
		Frame.ReadFrame(name, arr);
		if (!Frame.detected())
			cout << "识别二维码失败！" << endl;
		else if (writeFile(arr, fp, fp2))
			break;
		Frame.flush();
	}
	fclose(fp);
	fclose(fp2);
}

int Video_To_Image_Separate(string filename)
{
	QRCodeDetector q;
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
	int flag = 0;
	int QRCount = 1;
	int Video_Num = 0;
	int imageCount = 0;
	string img_name;
	char image_name[20];
	int kernel_size = 3;
	/*一个简单的滤波处理，防止雪花*/
	Mat kernel = Mat::ones(kernel_size, kernel_size, CV_32F) / (float)(kernel_size * kernel_size);
	vector <Point2f> Rect;
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
		++num;
		cout << "当前处理第" << num << "帧" << endl;
		if (flag == 1 || q.detect(frame, Rect))
		{
			flag = 1;
			if (QRCount % 3 == 2) {
				sprintf_s(image_name, "%s%d%s", "image", ++imageCount, ".jpg");//保存的图片名
				img_name = image_name;

				/*以10fps生成的视频一般以30fps划分，重复三张，取中间一张最好*/
				imwrite(img_name, frame);//保存一帧图片		
			}
			if (waitKey(30) == 27 || num == frame_number)
			{
				cout << "按下ESC键" << endl;
				break;
			}

			++QRCount;
		}
	}
	Video_Num = imageCount;
	int i;
	for (i = Video_Num; i > 0; i--)
	{
		sprintf_s(image_name, "%s%d%s", "image", i, ".jpg");
		//namedWindow("out", WINDOW_AUTOSIZE);

		img = imread(image_name);
		//imshow("out", img);
		//waitKey(0);
		if (q.detect(img, Rect))
			break;
	}
	cout << i << endl;
	return i;
}
