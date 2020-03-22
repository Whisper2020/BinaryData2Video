#include "QRCodeTools.h"

int main(int argc, char** argv)
{
	int Num = readFile(argv[1]);
	string str = argv[3];
	Image_to_Video_Generate(Num, Rows, Cols, atof(str.c_str()) / 1000, argv[2]);
	cout << argv[2] << endl;

	return 0;
}