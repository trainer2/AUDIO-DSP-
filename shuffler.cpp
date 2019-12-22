#include "funciones.h"

enum shuffle {programName, in, shuffletype, winsize, sineAmp, nargs};

int main(int argc, char const *argv[])
{
		if (argc < winsize ) {cout<< "USO: \n \n"

		"In.wav  	Shuffletype      Winsize/Freq 		sineAmp(opt)   \n \n"

	"********************************************************************\n \n"
	"Shuffle Type ---- Window va en cantidad de samples\n\n"
	"1- Mirror window constante\n"
	"2- Mirror window variable random\n"
	"3- Mirror window variable senoidal\n"
	"4- Random con window cte\n" 
	"5- Random con window senoidal\n "; return 1;}

	string path = argv[in];
	uint type = atoi(argv[shuffletype]);
	uint window = atoi(argv[winsize]);
	uint amp = atoi(argv[sineAmp]);
	vector<double> buffer = readWav(path);
	double sineCt = sineCtCalc(window, 44100);


	if (type == 1) mirrorAudioConstWindow(buffer, window);
	if (type == 2) mirrorAudioRandWindow(buffer);
	if (type == 3) mirrorAudioSineWindow(buffer, window, amp, 44100);
	if (type == 4) for(int i =0; i<=buffer.size()-window; i+= window) shuffleAudio(buffer, i, i+window);
	if (type == 5) shuffleAudioRandomSine(buffer, window, amp);

	guardarWavMono(buffer, 16, 44100, "Shuffled");
	return 0;
}