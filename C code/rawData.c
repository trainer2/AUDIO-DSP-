#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const float PI = 3.1415;

int endian() {
	int one = 1; // 0x0001
	char* endptr = (char *) &one; //puntero a un byte
	return *endptr;
}

int main(int argc, char** argv){

	FILE* fp1;
	FILE* fp2;
	FILE* fp3;
	float sineCT = 440*PI/44100;
	float k;


	fp1 = fopen("senoBin", "wb");
	fp2 = fopen("sqBin", "wb");
	fp3 = fopen("nonsenseBin", "wb");

	for(int i=0; i<44100*2;i++)	{k=sin(i*sineCT); fwrite(&k, sizeof(float), 1, fp1);}
	for(int i=0; i<44100*2; i++){k= i%44100 < 22100 ? 1 : 0; fwrite(&k, sizeof(float), 1, fp2);};
	for(int i=0; i<44100*2; i++){k= (i-1)/i*k> 1 ? 0.5 : (i-1)/i*k; fwrite(&k, sizeof(float), 1, fp3);};
	return 0;
}