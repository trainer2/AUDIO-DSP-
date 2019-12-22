//  Generates sinewave with fixed freq with a simple exponencial decay envelope
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef PI
#define PI  3.141592654
#endif

enum {ARG_NAME,
	  ARG_OUTFILE,
	  ARG_DUR,
	 ARG_HZ,
	 ARG_SR,
	 ARG_SLOPE,
	 ARG_NARGS};

int main(int argc, char** argv){

	unsigned int i, sr,  nsamps;
	double samp, dur, freq, srate, k,a,x,slope;
	double start, end, fac;
	double twoPi = 2.0 * PI;
	double angleincr;
	FILE* fp=NULL;

	if (argc != ARG_NARGS) {
		printf ("Usage: tuningfork outfile.txt duration(s) frequency srate decayRate");
		return 1;
	}

	fp = fopen(argv[ARG_OUTFILE], "w");
	if (fp == NULL){
		fprintf(stderr, "ERROR CREATING OUTPUT FILE %s\n", argv[ARG_OUTFILE] );
		return 1;
	}

	dur = atof(argv[ARG_DUR]);
	freq = atof(argv[ARG_HZ]);
	srate= atof(argv[ARG_SR]);
	slope = atof(argv[ARG_SLOPE]);
	nsamps = (int) (dur * srate);
	angleincr = twoPi * freq/srate;
	k = dur/nsamps;
	a = exp(-k/slope);	//decay factor
	x = 1.0;

	for(i=0; i<nsamps; i++){
		x *= a;
		samp = x*sin(angleincr*i); //oscillator
		fprintf(fp, "%.81f\n", samp );
	}
	fclose(fp);
	printf("DONE!");
	return 0;
}