// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include<portaudio.h>

#include <stdio.h>
#include <math.h>

using namespace std;

#define FRAME_BLOCK_LEN 256
#define SAMPLING_RATE 44100
#define TWOPI 6.283185307179586476925286766559
#define fmct TWOPI / SAMPLING_RATE

const char END = ' ';
const char STOP= 's';
const char RESUME = 'r';
const char CHANGE = 'c';
const char PM = 'p';
const char FM = 'f';
char curMode = PM;

float Car_frequency;
float Mod_frequency;
float Mod_index;
double carr_ct;
double mod_ct;
PaStream* audioStream;

void readFrequency() {
	printf("Modulator frequency in Hertz: \n");
	scanf_s("%f", &Mod_frequency);
	
	printf("Modulation Index in Hertz: \n");
	scanf_s("%f", &Mod_index);
	
	printf("Carrier frequency in Hertz: \n");
	scanf_s("%f", &Car_frequency);
	
	mod_ct = TWOPI * Mod_frequency / SAMPLING_RATE;
	carr_ct = TWOPI * Car_frequency / SAMPLING_RATE;
}

int audio_callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)

{
	float* out = (float*)outputBuffer;
	unsigned long sample_count;
	static unsigned int n = 0;
	static double fmPhase = 0;

	if (curMode == PM) 
		for (sample_count = 0; sample_count < framesPerBuffer; sample_count++) {
			double carrPhase = carr_ct * n;
			double modPhase = mod_ct * n;
			double outsample = cos(carrPhase + Mod_index * sin(modPhase));
			*out++ = outsample;	//L
			*out++ = outsample; //R
			n++;
		}
	

	else  /* FM*/
		for (sample_count = 0; sample_count < framesPerBuffer; sample_count++) {
			double outsample = cos(fmPhase);
			*out++ = outsample; //L
			*out++ = outsample; //R
			fmPhase += fmct * (Car_frequency + Mod_index * Mod_frequency * cos(fmct * n++ * Mod_frequency));
		}
	return paContinue;
}

void iniciar() {
	int i; int id;
	const PaDeviceInfo* info;
	const PaHostApiInfo* hostapi;
	PaStreamParameters outputParameters;
	PaStreamParameters inputParameters;

	readFrequency();
	printf("INICIALIZANDO PORTAUDIO \n");

	Pa_Initialize();

	/* 				OUPUT  				*/

	for (i = 0; i < Pa_GetDeviceCount(); i++) {
		info = Pa_GetDeviceInfo(i);
		hostapi = Pa_GetHostApiInfo(i);
		if (info->maxOutputChannels > 0 && hostapi != NULL) printf("%d: [%s] %s (output)\n", i, hostapi->name, info->name);
	}
	printf("Elegi el numero del audio device OUTPUT a usar\n");
	scanf_s("%d", &id);
	info = Pa_GetDeviceInfo(id);
	hostapi = Pa_GetHostApiInfo(info->hostApi);
	printf("ABRIENDO EL OUTPUT AUDIO DEVICE [%s] %s\n", hostapi->name, info->name);
	outputParameters.device = id;
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = info->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;


	//Hacemos lo mismo con el input ahora
	
	/* 				INPUT  				*/
	
	for (i = 0; i < Pa_GetDeviceCount(); i++) {
		info = Pa_GetDeviceInfo(i);
		hostapi = Pa_GetHostApiInfo(i);
		if (info->maxInputChannels > 0 && hostapi != NULL) printf("%d: [%s] %s (input)\n", i, hostapi->name, info->name);
	}
	printf("Elegi el numero del audio device INPUT a usar\n"); 
	scanf_s("%d", &id);
	info = Pa_GetDeviceInfo(id);
	hostapi = Pa_GetHostApiInfo(info->hostApi);
	printf("ABRIENDO EL INPUT AUDIO DEVICE [%s] %s\n", hostapi->name, info->name);
	inputParameters.device = id;
	inputParameters.channelCount = 2;
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = info->defaultLowOutputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;
	
	

	Pa_OpenStream(
		&audioStream,
		&inputParameters,
		&outputParameters,
		SAMPLING_RATE,
		FRAME_BLOCK_LEN,
		paClipOff,
		audio_callback,
		NULL);
	Pa_StartStream(audioStream);
	printf("Listo. Envia un espacio en blanco para salir!\n");

}

void terminar() {
	Pa_StopStream(audioStream);
	Pa_CloseStream(audioStream);

	Pa_Terminate();

}

int main() {

	iniciar();
	char r='r';
	printf("'p' para PM \n'f' para FM\n");
	while (r != END) {
		scanf_s("%c", &r); 
		if (r == STOP) Pa_StopStream(audioStream);
		else if (r == RESUME) Pa_StartStream(audioStream);
		else if (r == CHANGE) readFrequency();
		else if (r == FM)	curMode = FM;
		else if (r == PM) curMode = PM;
		else Pa_Sleep(100);

	}
	terminar();
	return 0;
}

//Los device que me funcionan son:
// 3 - Microfono integrado
// 5 - Parlante out
// 6 - OUT MTRACK