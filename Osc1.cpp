#include "Audiofile/AudioFile.h"
#include "funciones.h"
#include <math.h>
#include <iostream>

//TODO: PASAR TODO A DOUBLE

//ESCRIBE EL WAV NORMALIZADO ENTRE 0 y 1 EN ESTE MISMO DIRECTORIO SEGUN LOS SIGUIENTES PARAMETROS
//Se puedee elegir el slope bias de subida/bajada de la triangular o el pw de la cuadrada. Representa tiempo que tarda en subir / tiempo que tarda en bajar

enum {ThisProgramName, frequency, wave, duration, PW, outname, SR, BitDepth, CHN, nargs};
enum {zero, SINE, SQUARE, TRI, SWT, PULSE};
int main(int argc, char const *argv[])
{
	if (argc < 6){
		printf("(%s)\n", "\n \n"

		"**************************************************************************************" 

			"USAGE: FREQUENCY WAVE DURATION(s) PW/Slope OUT_NAME.xxx  (opt)SR (opt)BitDepth (opt) Channels" );
		std::cout<<
					  "WAVES: -1 Sine \n"
							  "-2 Square\n"
							  "-3 Triangle\n"
							  "-4 sawtooth\n"
							  "-5 pulse \n"
							  "-PW & slope between 0 and 1\n"
							  ;
		exit(1);
	}

	float freq = atof(argv[frequency]);
	int dur= atoi(argv[duration]) ;
	int wavetype = atoi(argv[wave]);
	float srate = 44100;
	int channels = 1;
	int bitd = 16;
	float pw = 0.5;
	float slope = 0.5;
	const char* filename = argv[outname];

	if(argc > SR) srate = 	atof(argv[SR]);
	if(argc > BitDepth) bitd =	atoi(argv[BitDepth]);
	if(argc > PW) pw = 		atof(argv[PW]); 
	if(argc > CHN) channels =atoi(argv[CHN]);

	int nSamples = dur * srate;
	AudioFile<double> wav;
	AudioFile<double>::AudioBuffer buffer;
	buffer.resize(channels);
	int pulsecounter = 0;
	int slopeCounter = 0;
	if(wavetype==SQUARE) {pw=0.5; wavetype = PULSE;}
	if(wavetype== TRI)   slope = pw;
	if(wavetype==SWT) {slope = 0; wavetype = TRI;}
	for (int c = 0; c< channels; c++) buffer[c].resize(nSamples);

	float cyclelen = (float) srate / (float) freq;			 //Duracion de un ciclo en cantidad de samples de una onda cualquiera
	float pulseLen = cyclelen*pw;
	float sineCt = sineCtCalc((float) freq,  (float) srate);	//Cte multiplicativa en la formula sin(sineCt * t)
	float upSlopeDur = cyclelen *slope; //en cantidad de samples
	float upSlope, downSlope;
	if (slope==0) downSlope = 2/cyclelen;
	else {upSlope = 2 /upSlopeDur;   //delta y / delta x 
	   	  downSlope= 2 / (cyclelen-upSlopeDur);} 
	float lastSample = -1;

	for(int i = 0; i< nSamples; i++){ //BASTANTE mas ineficiente pero concentra el codigo con un solo for. Se cambia en 5'.
	
		if (wavetype == SINE){
			writeSameSamples(buffer, channels, i, sin(sineCt * i));	
		}

		if (wavetype== PULSE){	
			if (pulsecounter <pulseLen) writeSameSamples(buffer, channels, i, 0);
			else writeSameSamples(buffer, channels, i, 1);

			pulsecounter++;
			if (pulsecounter>=cyclelen) pulsecounter=0;
		}

		if (wavetype == TRI){
			//El Slope counter arranca en 0 y se incrementa hasta que llega al valor de "upslopeDur". Mientras no llega, la onda baja. Una vez que llega, la onda empiez a subir hasta que slopecounter valga cyclelen

			if(slopeCounter < upSlopeDur){
				if (lastSample<=-1) lastSample=-1;
				writeSameSamples(buffer, channels, i,lastSample);
				lastSample +=upSlope;
			}
			else {
				if(lastSample>=1) lastSample=1;
				writeSameSamples(buffer, channels, i, lastSample);
				lastSample-=downSlope;}

			slopeCounter++;
			if(slopeCounter >= cyclelen) {
				slopeCounter=0;
				if(slope == 0) lastSample=0;
			}
		}

	
	}




	wav.setAudioBuffer(buffer);
	wav.setNumChannels (channels);
	wav.setBitDepth (bitd);
	wav.setSampleRate (srate);

	wav.save (filename);
	wav.printSummary();
	return 0;
}

