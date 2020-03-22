#include "QRCodeTools.h"

QRCodeTools::QRCodeTools(int v) :Ver(v), sz(4 * (v - 1) + 21), dbg(false), maskID(0) {}

QRCodeTools::~QRCodeTools() {}

void QRCodeTools::setDbg(bool flag)
{
	dbg = flag;
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

void QREncodeTools::WriteFrame(bool datamatrix[][TSIZE], int flames)
{
	Mat img;
	char name[20];
	sprintf_s(name, "%s%d%s", "image", flames, ".jpg");
	for (int i = 0; i < sz; ++i)
		for (int j = 0; j < sz; ++j)
			if (!checkPos(i, j))
				write(i, j, datamatrix[j][i]);
	output(img);
	flush();
	imwrite(name, img);
}

int readFile(const char str[]) {
	QREncodeTools Frame;
	FILE* filePoint = NULL;
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
					arr[i][j++] = ch >> k & 1;
					if (j >= TSIZE - 1)
					{
						j = 8;
						i++;
					}
				}
			}
		}
		if (index == inforlens) {
			if (feof(filePoint))
				process(arr, flame, inforlens);
			else
				process(arr, flame, -1);
		}
		else
			process(arr, flame, index - 1);
		Frame.WriteFrame(arr, flame++);
	}
	return flame - 1;
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

void Image_to_Video_Generate(int num, int frame_width, int frame_height, float video_time, string video_name)
{
	//frame_height——图片高度
	//frame_width——图片宽度
	//frame_fps——图片帧率
	//num——图片帧数
	//video_time——视频播放时间，秒
	//图像预先存储命名为：imageXX.jpg(eg:image1,image2...)
	char image_name[20];
	string s_image_name;
	VideoWriter writer;
	int Video_Num = 0;
	int isColor = 1;//非0为彩色
	float time = 0;//记录播放时间
	writer = VideoWriter(video_name, writer.fourcc('X', 'V', 'I', 'D'), FPS, Size(frame_width, frame_height), isColor);
	/*cout << "frame_width is " << frame_width << endl;
	cout << "frame_height is " << frame_height << endl;
	cout << "frame_fps is " << FPS << endl;*/
	int i = 0;
	Mat img;
	Video_Num = (int)(FPS * video_time);
	if (Video_Num > num)
	{
		cout << "视频长度不足,按原长度播放" << endl;
		Video_Num = num;
	}
	/*插入过渡图片 过渡时间2s*/
	sprintf_s(image_name, "%s%d%s", "front_image", 1, ".jpg");
	s_image_name = image_name;
	img = imread(s_image_name);
	while (i++ < 20)
	{
		writer.write(img);
	}
	i = 0;

	while (i <= Video_Num)
	{
		sprintf_s(image_name, "%s%d%s", "image", ++i, ".jpg");//将所存图片名字写入
		s_image_name = image_name;
		img = imread(s_image_name);//读入图片
		if (!img.data)//判断图片调入是否成功
		{
			cout << "Could not load image file...\n" << endl;
		}
		writer.write(img);
		if (waitKey(30) == 27)//27 ESC键ascll码
		{
			cout << "按下ESC键" << endl;
			break;
		}

	}
	/*插入过渡图片 过渡时间2s*/
	i = 0;
	sprintf_s(image_name, "%s%d%s", "front_image", 1, ".jpg");
	s_image_name = image_name;
	img = imread(s_image_name);
	while (i++ < 20)
	{
		writer.write(img);
	}
	cout << "视频已生成。有效图片：" << Video_Num << " 帧" << endl;
}
