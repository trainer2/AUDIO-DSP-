// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Header.h"

void outSamples(float* out) {
	static int i;
	for (i = 0; i < FRAME_BLOCK_LEN; i++) {
		*out++ = buffer[i];
		*out++ = buffer[i];
	}
}

int audio_callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)
{	
	outSamples((float*) &outputBuffer);
	return paContinue;
}

void iniciar() {
	int i; int id;
	const PaDeviceInfo* info;
	const PaHostApiInfo* hostapi;
	PaStreamParameters outputParameters;
	PaStreamParameters inputParameters;

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
	while (r != END) {
		scanf_s("%c", &r); 
		if (r == PAUSE) Pa_StopStream(audioStream);
		else if (r == RESUME) Pa_StartStream(audioStream);
		else Pa_Sleep(100);

	}
	terminar();
	return 0;
}

//Los device que me funcionan son:
// 3 - Microfono integrado
// 5 - Parlante out
// 6 - OUT MTRACK