#include <iostream>
#include "QRCodeTools.h"
using namespace std;
using namespace cv;

void Demo() {
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
int main()
{
	Demo();
	return 0;
}

