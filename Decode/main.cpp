#include "QRCodeTools.h"

int main(int argc, char** argv)
{
	int k = Video_To_Image_Separate(argv[1]);
	readframes(argv[2], argv[3], k);
	/*写入文件接口*/
	return 0;
}