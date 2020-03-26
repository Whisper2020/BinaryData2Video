#include <iostream>
using namespace std;

int main(){
	FILE *fp1 = fopen("e3.bin", "rb");
	FILE *fp2 = fopen("out.bin", "rb");
	FILE *fp3 = fopen("err.bin", "rb");
	
	unsigned char ch1, ch2, ch3;
	int num = 0;
	while(!feof(fp2)){
		fread(&ch1, sizeof(unsigned char), 1, fp1);
		fread(&ch2, sizeof(unsigned char), 1, fp2);
		fread(&ch3, sizeof(unsigned char), 1, fp3);
		for(int i = 0; i < 8; i++){
			if(ch3 >> i&1){
				if((ch1 >> i&1) != (ch2 >>i&1)){
					cout << num << endl;
					return 0;
				}
			}
			num++;
		}
	}
	cout << "Cherk!!" << endl;
	return 0;
} 
