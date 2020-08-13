#include "Osciladores.h"

#include <stdio.h>
#include <math.h>


#include "portaudio/include/portaudio.h"

using namespace std;

#define FRAME_BLOCK_LEN 256
#define SAMPLING_RATE 44100
#define TWOPI 6.283185307179586476925286766559

const char END    = ' ';
const char PAUSE  = 'p';
const char RESUME = 'r';

PaStream* audioStream;
double si=0;
void cacona(){printf("holi\n");}

int audio_callback( const void* inputBuffer, void* outputBuffer,
					unsigned long framesPerBuffer, 
					const PaStreamCallbackTimeInfo* timeInfo,
					PaStreamCallbackFlags statusFlags, 
					void* userData)

{

cacona();
	float* in = (float*) inputBuffer;
	float* out= (float*) outputBuffer;
	static double phase =0;
	unsigned long i;
	for(i=0; i< framesPerBuffer; i++){
		float sine = sin(phase);
		*out++ = *in++ * sine; //L
		*out++ = *in++ * sine; //R
		phase += si;
	}
	return paContinue;
}

void iniciar() {
	float frequency;
	int i; int id;
	const PaDeviceInfo* info;
	const PaHostApiInfo* hostapi;
	PaStreamParameters outputParameters;
	PaStreamParameters inputParameters;

	printf("Modulator frequency in Hertz: \n" );
	scanf("%f", &frequency);
	si = TWOPI * frequency / SAMPLING_RATE;
	printf("INICIALIZANDO PORTAUDIO \n");

	Pa_Initialize();

	/* 				OUPUT  				*/

	for(i=0; i<Pa_GetDeviceCount();i++){
		info=Pa_GetDeviceInfo(i);
		hostapi=Pa_GetHostApiInfo(i);
		if(info->maxOutputChannels >0) printf("%d: [%s] %s (output)\n",i,hostapi->name, info->name );
	}
	printf("Elegi el numero del audio device OUTPUT a usar\n");
	scanf("%d", &id);
	info = Pa_GetDeviceInfo(id);
	hostapi= Pa_GetHostApiInfo(info->hostApi);
	printf("ABRIENDO EL OUTPUT AUDIO DEVICE [%s] %s\n", hostapi->name, info->name );
	outputParameters.device=id;
	outputParameters.channelCount = 2; 
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = info->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;


	//Hacemos lo mismo con el input ahora

	/* 				INPUT  				*/

	for(i=0; i<Pa_GetDeviceCount();i++){
		info=Pa_GetDeviceInfo(i);
		hostapi=Pa_GetHostApiInfo(i);
		if(info->maxInputChannels >0) printf("%d: [%s] %s (input)\n",i,hostapi->name, info->name );
	}
	printf("Elegi el numero del audio device INPUT a usar\n");
	scanf("%d", &id);
	info = Pa_GetDeviceInfo(id);
	hostapi= Pa_GetHostApiInfo(info->hostApi);
	printf("ABRIENDO EL INPUT AUDIO DEVICE [%s] %s\n", hostapi->name, info->name );
	inputParameters.device=id;
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

int main(){

	iniciar();
	while(getchar() != END) Pa_Sleep(100);
	terminar();
	return 0;
}