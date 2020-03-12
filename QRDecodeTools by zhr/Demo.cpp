#include <iostream>
#include "QRCodeTools.h"
using namespace std;
using namespace cv;

void TestRead() {
	QRDecodeTools i;
	Mat img = imread("test.png");
	i.flush();
	i.loadQRCode(img);
	cout << "Detect=" << i.detected() << endl;
	try {
		cout << "Read:" << i.read(10, 10) << endl;
		cout << "Read:" << i.read(7, 8) << endl;
		i.setDbg(true);
		cout << "Read:" << i.read(8, 8) << endl;
	}
	catch (int e) {
		cout << "Fuzz: val=" << e << endl;
	}
	return;
}
void TestWrite() {
	QREncodeTools i;
	Mat img;
	i.write(40, 40, 1);
	i.display(); //可以临时查看写图结果
	cout<<"Size:"<<i.output(img);
	i.flush();
	namedWindow("Image", WINDOW_AUTOSIZE);
	imshow("Image", img);
	waitKey();
}
int main()
{
	//TestRead();
	TestWrite();
	return 0;
}

