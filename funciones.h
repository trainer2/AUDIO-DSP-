#ifndef ASKA
#define ASKA

#include "Audiofile/AudioFile.h"
#include<iostream>
#include<vector>
#include <utility> 
#include <string>
#include <limits>

using namespace std;


const double PI    = 3.141592654;
const double TWOPI = 6.283185307179586476925286766559;
const double HALFPI = 1.5707963267948966192313216916398;
const double SAMPLING_RATE = 44100;

typedef int uint;
typedef AudioFile<double>::AudioBuffer Buffer;
float sineCtCalc(double freq, double srate){ return 2*PI* (double) freq / (double) srate;}





/**********************************************************************************/
/********************            BREAKPOINTS   ***********************************/

struct breakpoint{
	double x; //tiempo en segundos
	double y=0;  //valor en ese punto
	breakpoint(double t, double val) :x(t), y(val){}
	};

void append(vector<breakpoint>& b1, const vector<breakpoint>& b2){
	double lastTime = b1[b1.size()-1].x;
	for(int i=0; i<b2.size(); i++) b1.push_back(breakpoint(b2[i].x + lastTime, b2[i].y));
}

//Saca breakpoints muy cercanos para eliminar redundancia. 
//min_sep va en segundos
void deflate(vector<breakpoint>& b, double min_sep){
	for(int i = b.size()-1; i>0; i--)
		if(b[i].x - b[i-1].x <min_sep) b.erase(b.begin()+i);
}


/**********************************************************************************/
/**************************** AUDIO BUFFERS     ************************************/


void normalize(vector<double>& buffer){
	double maxsamp=0;
	for(int i =0; i< buffer.size(); i++) maxsamp = max(maxsamp, buffer[i]);
	for(int i =0; i<buffer.size(); i++) buffer[i] = min(1., abs(buffer[i] / maxsamp));
}

void writeSameSamples(Buffer& buffer, int channels,int pos, double value){
	for(int c= 0; c<channels; c++) buffer[c][pos] = value;
}

void writeSineSamples(uint from, uint to, vector<double>& res, double sineCt, double phase =0){
	
	for(int i = from +1; i< to; i++) 
	 res[i] = (sin(sineCt * (i-from) +phase));
	return;
}



/**********************************************************************************/
/**************************** IN / OUT WAVS    ************************************/


//Lee el primer canal de un wav
vector<double> readWav(string path){
	AudioFile<double> wav;
	assert(wav.load(path));
	return wav.samples[0];
}

AudioFile<double> leerWav(string path){
	AudioFile<double> wav;
	assert(wav.load(path));
	return wav;
}

//Genera un wav 
void guardarWav(Buffer& buffer, int canales, int bitdepth, double srate, string filename){

	AudioFile<double> wav;
	wav.setAudioBuffer(buffer);
	wav.setNumChannels (canales);
	wav.setBitDepth (bitdepth);
	wav.setSampleRate (srate);
	wav.save (string(filename) + ".wav");
	wav.printSummary();
}

void guardarWavMono(vector<double>& buffer, int bitdepth, double srate, string filename){
	Buffer bf;
	bf.push_back(buffer);
	guardarWav(bf, 1, bitdepth, srate, filename);
}


/**********************************************************************************/
/**************************** BUFFER SHUFFLE    ************************************/


//Mezcla los samples en el rango Desde, Hasta, aleatoriamente
void shuffleAudio(vector<double>& buffer, uint from, uint to){ random_shuffle(&buffer[from], &buffer[to]);
}

void shuffleAudioRandomSine(vector<double>& buffer, double freq, uint amp){
	double sineCt =sineCtCalc(freq, 44100);
	uint seno;
	for(uint i = 0; i< buffer.size()-amp; i++){
	seno = sin(sineCt*i);
	shuffleAudio(buffer, i, i+seno);
	i+=seno ;}

}

//Te invierte los samples en ese rango. EJ: 1,2,3,4 va a 4,3,2,1
void mirrorAudioInRange(vector<double>& buffer, uint from, uint to){
	for(uint i=from; i<(to+from)/2; i++) swap(buffer[i], buffer[to -1+ from - i]);
}

//Revierte todos los samples agarrandolos de a windows del mismo tamaño
void mirrorAudioConstWindow(vector<double>& buffer, uint window){
	for(uint i = 0; i< buffer.size() -window; i+= window) mirrorAudioInRange(buffer, i, i+window);
}

void mirrorAudioRandWindow(vector<double>& buffer){
	uint i =0;
	uint r =0;
	while (i+r< buffer.size()){
	r = rand()% 25000;
 	mirrorAudioInRange(buffer, i, i+r);
	i+=r;
	}
}


void mirrorAudioSineWindow(vector<double>& buffer, double freq, uint amp = 1000, double srate = 44100){
	double sineCt =sineCtCalc(freq, srate);
	uint seno; 
	for(uint i = 0; i< buffer.size()-amp; i++){
		seno = sin(sineCt*i);
		mirrorAudioInRange(buffer, i, i+seno);
		i+=seno ;}
}


#endif
