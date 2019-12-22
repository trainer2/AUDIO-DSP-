//  Generates sinewave with fixed freq and exponencial decay and writes it to a raw binary data. 
// 
//'isfloat9 parameter must be 0 or 1 and indicates the data byte-length
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef PI
#define PI = 3.141592654
#endif


//Generates an integer sequence, where ARG_NAME = 0, ARG_OUTFILE = 1, ... ARG_HZ = 3., etc.
//The prefix ARG arises because we will use these numbers as indexes to the program arguments, e.g., 
// 	argv[ARG_HZ]		This work as numbers: ARG_NAME+1 = ARG_OUTFILE, etc.

//The 0th argument is always reserved for the program name. 
//We should use this method to organize code and keep track of program arguments.

enum {ARG_NAME,
	  ARG_OUTFILE,
	  ARG_DUR,
	 ARG_HZ,
	 ARG_SR,
	 ARG_AMP,
	 ARG_TYPE,
	 ARG_NARGS};

// Generataes an enumerated secuence. We will use it so as to know the bit-length of each sample
enum samptype {RAWSAMP_SHORT, RAWSAMP_FLOAT};

//Machine Endian-ness printer. 0 For big endian
//As an int consists of four bytes, the Hex representation of 1 is either 01 or 10 (big or little endian)
//A pointer declared this way will adress the leftmost byte's adress. Therefore, by looking at the leftmost value of that byte, 
//we can know the endianness of the machine we are in. 
int machine_endianness(){
	int one = 1;
	char * endptr = (char *) &one;
	return *endptr;
}

const char* endianness[2] = {"big endian", "little endian"};



int main(int argc, char** argv){

	unsigned int i, nsamps;
	unsigned int samptype, endian, bitreverse;
	double samp, dur, freq, srate, amp;
	double start, end, fac, maxsamp; //amplitude controls
	unsigned int maxframe=0; //where the wave reaches it's peak
	double twoPi = 2.0 * PI;
	double angleincr;
	FILE* fp = NULL; 	//Data type for managing files included in studio.h
	float fsamp;
	short ssamp;

	//Input parameters read
	if (argc != ARG_NARGS) {
		printf ("Usage: tuningfork outfile.raw duration frequency srate maxAmplitud isfloat");
		return 1;
	}

	dur = atof(argv[ARG_DUR]);
    freq = atof(argv[ARG_HZ]);
    srate = atof(argv[ARG_SR]);
    amp = atof(argv[ARG_AMP]);
    samptype= (uint atoi) (argv[ARG_TYPE]);

    if (samptype > 1 || samptype < 0){
    	printf("sampleType ('isfloat' parameter) must be 0 or 1" );
    	return 1;
    }


	fp = fopen(argv[ARG_OUTFILE], "wb"); //open in "write binary" mode

	if (fp == NULL){
		fprintf(stderr, "Error creating output file %s\n", argv[ARG_OUTFILE] );
		return 1;
	}


	//Relevant variales preparation
	nsamps = dur*srate;
	angleincr = twoPi * freq / srate;
	start = 1.0;
	end = 1.0e-4;
	maxsamp=0.0;
	fac = pow(end/start, 1.0/nsamps);		//after 'nsamp' iterations, start will be exactly 'end'
	endian = machine_endianness();
	printf("Writing %d %s samples \n", nsamps, endianness[endian]);


	//Main loop start!
	if (samptype == RAWSAMP_SHORT){

		for (i=0; i <nsamps; i++){
			samp = amp*sin(angleincr*i) * start;
			start *= fac;

			ssamp = (short) (samp * 32767.0);
			if (fwrite (&ssamp, sizeof(short),1,fp) != 1){
				printf("Error writing data to file \n");
				return 1;
			}


			if (fabs(samp) > maxsamp){//Update maximum
				maxsamp = fabs(samp);
				maxframe = i;
			}

		}

	}
	else {

		for (i=0; i< nsamps; i++){
			samp = amp*sin(angleincr*i) * start;
			start *= fac;
			fsamp = (float) samp;
			if (fwrite (&fsamp, sizeof(float), 1, fp) != 1) {
				printf("Error while writing data to file\n");
				return 1;
			}

			if (fabs(samp) > maxsamp){
				maxsamp = fab(samp);
				maxframe = i;
			}
		}

	}

	fclose(fp);
	printf("done. Maximmum sample value = %.81f at frame %d\n", maxsamp, maxframe);

	return 0;
}