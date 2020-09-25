#ifndef ASKA
#define ASKA

// I'm aware of how pointless and resource consuming it is to declare every sample as a double-type for *these. 
// It's just that I started with the wrong foot, and right now I care more about getting things to work than them being efficient

#include "Audiofile/AudioFile.h"
#include<iostream>
#include<vector>
#include <utility> 
#include <string>
#include <limits>
#include <cstdio>
#include <stdlib.h>     /* srand, rand */
#include <math.h>

using namespace std;
#define PARAMS ssignal& s, int fftsize = global_fftsize, int hop = global_hop,double (*window)(int i, int size) = Hann


const double PI    = 3.141592654;
const double TWOPI = 6.283185307179586476925286766559;
const double HALFPI = 1.5707963267948966192313216916398;
      double SAMPLING_RATE = 44100;
int global_fftsize = 2048*8;
int global_hop = 2048*8*4/5;
bool global_save = true;
struct complex;
typedef unsigned int uint;
typedef AudioFile<double>::AudioBuffer Buffer;
typedef std::vector<complex> csignal;//una ***, porque ssignal que es una señal real usa la "s" que se usa para la frecuencia cuando se habla de transformadas, pero bueno
typedef std::vector<double> ssignal;//signal already taken in std library
double sineCtCalc(double freq, double srate = SAMPLING_RATE){ return 2*PI* (double) freq / (double) srate;}
//#define ssignal std::vector<double>
//typedef std::vector<double> audio; //Cada 3 meses que retomo esto me dan ganas de ponerle otro nombre a las cosas! sera posible
int randSp(int max){return rand()%max;}
int randc(int _){return rand();}


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

void writeToFile(vector<breakpoint> bps, const char* path){
	FILE *fp;
    fp = fopen(path,"w");
	for(int i=0; i< bps.size(); i++) fprintf(fp,"%.3f\t%.81f\n", bps[i].x, bps[i].y);
    fclose(fp);
}

template<class T>
void truncate(vector<T>& s, int to){
	for (int i = to; i < s.size(); ++i) s.pop_back();
}

struct complex{
	double re;
	double im; 
	complex(double a=0.0, double b = 0.0){re=a; im=b;}
	complex operator*(complex b){return complex(re*b.re-im*b.im, im*b.re+re*b.im);}
	complex operator+(complex &b){return complex (re+b.re, im+b.im);}
	complex operator-(complex b){return complex (re-b.re, im-b.im);}
    complex& operator+=(complex b){re+=b.re; im+=b.im;}
    complex& operator-=(complex b){re-=b.re; im-=b.im;}

	complex operator/(complex &b){
		double modulo = pow(b.re,2) + pow(b.im,2);
		return complex((re*b.re+im*b.im) / modulo, (im*b.re-re*b.im / modulo));
	}
	complex operator/(double &b){
		return complex(re/b, im /b);
	}
};

double abs(complex c){
	return sqrt(c.re*c.re+c.im*c.im);
}
void conj(complex& c){ c.im *= -1;}
complex conjugado(complex& b){return complex(b.re, -1*b.im);}

/**********************************************************************************/
/********************            SPECTRUM   ***********************************/


//typedef pair<vector<double>, vector<double> > spectrum;
struct spectrum{
public:
	//cada lugar tiene densidad espectral de la respectiva frecuencia
	//El indice i tiene frec i*2pi/(sr/2) /N  con N el tamaño de la señal
	vector<double> cosines; 
	vector<double> sines;
	int sr = SAMPLING_RATE;
	int init_sample = 0; //para usarlo en dftblocks
	int block_size = 512;
	//vector<double>* original;

	spectrum(int size){
		cosines = vector<double>(size,0);
		sines = vector<double>(size,0);	
	}
	spectrum(){vector<double> cosines; vector<double> sines;}


	//La frecuencia que representa el indice i, y su inversa
	double frequency(int i) {return (double) i /(double) cosines.size()/2 *(double) sr;}
	int index(double freq){ return int(freq * cosines.size()*2/sr);}

	void outPowerSpectrum(const char* path){
		vector<breakpoint> graph;
		for (int i = 0; i < cosines.size(); ++i)
		{	double power = cosines[i]*cosines[i] + sines[i]*sines[i];
			power /= cosines.size();
			double ifrequency = frequency(i);
			graph.push_back(breakpoint(ifrequency,power));
		}
		writeToFile(graph, path);
	}
};

struct stftSpectrum
{
	vector<csignal> cs;
	//Estos parametros aseguran una resintesis igual. Cumplen la propiedad COLA. 
	//El problema de estas window es que casi ningun sample aparece con su amplitud original!! Ej: El sample 0 todo bien. El sample 512 todo bien. Todos los del medio, van a tener su amplitud reducida!
	int fftsize = 1024; 
	int hop = 512;
	double(*window)(int, int);
	int size;
	
};

//Deduzcamos la frecuencia de cada indice. 
// Una DFT de 512 puntos de sampling produce 257 puntos de coseno y seno.
// Supongamos que entre sample y sample en el tiempo hay un segundo. DFT asume que la onda es periodica con periodo 512.
// El primer indice guarda lo de la frecuencia 1/512, el segundo 2/512, y asi.
// Entonces frequencia = i/2size
// Pero, en realidad, entre sample y sample no hay un segundo sino 1/44100. O sea, el periodo es mucho mas corto, es 512/44100
// Entonces la frecuencia es 1/512*44100, 2/512*44100
// Que es i*sr/2size;

/**********************************************************************************/
/**************************** AUDIO BUFFERS     ************************************/

int sign(double n){if(n<0) return -1; else return 1;}
void normalize(vector<double>& buffer){
	double maxsamp=0;
	for(int i =0; i< buffer.size(); i++) maxsamp  = max(maxsamp, abs(buffer[i]));
	for(int i =0; i<buffer.size(); i++) buffer[i] = min(1., abs(buffer[i] / maxsamp)) * sign(buffer[i]);
}

void writeSameSamples(Buffer& buffer, int channels,int pos, double value){
	for(int c= 0; c<channels; c++) buffer[c][pos] = value;
}

void writeSineSamples(uint from, uint to, vector<double>& res, double sineCt, double phase =0){
	
	for(int i = from +1; i< to; i++) 
	 res[i] = (sin(sineCt * (i-from) +phase));
	return;
}

//Esto deberia ser una clase con operator ya, pero bueno
/*void sum(ssignal& s1, const ssignal& s2){
	sum(s1,s2);
}*/
//Igual que la otra pero para acortar nuevas funciones y no actualizar UNA linea de codigo lol. Estoy apurado para pelear con C++
ssignal sum(ssignal& s1, const ssignal& s2){
	for (int i = 0; i < min(s1.size(), s2.size()); ++i)
		s1[i] += s2[i];
	if(s2.size() > s1.size())
		for (int i = s1.size(); i <s2.size() ; ++i)
			s1.push_back(s2[i]);
	return s1;
}
ssignal diff(ssignal& s1, const ssignal& s2){
	for (int i = 0; i < min(s1.size(), s2.size()); ++i)
		s1[i] -= s2[i];
	if(s2.size() > s1.size())
		for (int i = s1.size(); i <s2.size() ; ++i)
			s1.push_back(-1*s2[i]);
	return s1;
}


ssignal operator+( ssignal& s1, const ssignal& s2 ){return sum(s1,s2);}
//ssignal operator+=( ssignal& s1, const ssignal& s2 ){for (int i = 0; i < min(s1.size(), s2.size()); ++i)s1[i]+=s2[i]; }
//ssignal::operator+=(const ssignal& s2 ){for (int i = 0; i < size(); ++i){*this[i]+=s2[i];}
ssignal operator-( ssignal& s1, const ssignal& s2 ){return diff(s1,s2);}


ssignal puretone(double freq, int samples, double phase =0){
	ssignal out = ssignal(samples);
	double sineCt = sineCtCalc(freq);
	for(int i=0; i<samples; i++) out[i] =sin(sineCt*i+phase);
	return out;
}

ssignal sincx(double freq, int samples){
	ssignal out = ssignal(samples);
	double sineCt = sineCtCalc(freq);
	for (int i = 0; i < samples; ++i)
		out[i] = sin(sineCt*i)/(double) i;
	return out;
}
double sinc(double x) {
	if (x == 0) return 1;
	return sin(x)/x;
}

csignal randomcs(int len){
	csignal C(len); for (int i = 0; i < len; ++i) {C[i].re =  (double) rand()/RAND_MAX*2.-1.;;
												   C[i].im =  (double) rand()/RAND_MAX*2.-1.;;}
	return C;
}
ssignal randomss(int len){
	ssignal C(len); for (int i = 0; i < len; ++i) C[i] =  (double) rand()/RAND_MAX*2.-1.;;
	return C;
}




/**********************************************************************************/
/**************************** IN / OUT WAVS    ************************************/


//Lee el primer canal de un wav
vector<double> readWav(string path){
	AudioFile<double> wav;
	assert(wav.load(path));
	SAMPLING_RATE =  (double) wav.getSampleRate();
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

void guardarWavMono(const vector<double>& buffer, int bitdepth, double srate, string filename){
	Buffer bf;
	bf.push_back(buffer);
	guardarWav(bf, 1, bitdepth, srate, filename);
}
void guardarWavRapido(vector<double>& buffer, string filename, bool testeando=false) {
	if(testeando) buffer.push_back(1.); //para acelerar mis experimentos particulares. En particular, me evita normalizar
	if(not testeando) normalize(buffer); 
	guardarWavMono(buffer, 16,44100,filename);}


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


void printProgress(int i, int total){
	cout << '\r'<< i<<" / " <<total;

}



/***********************************************************

					WINDOW FUNCTIONS

*************************************************************/


//TODO: EL ANCHO DEBE DEPENDER DE HOP (TERCER PARAMETRO) PARA CUMPLIR COLA


//default window function
double id(int x, int dummy) {return 1.0;} //RECT

double Hann(int x, int size){

	static double cosCt = TWOPI/(size-1);
	return 0.5 - 0.5*cos(cosCt*x);
}

double tri(int x, int size){
	static int half = size/2; 
	if(x>half) return 2-2*x/size;
	else return 2*x/size;
}	

//TIENE TREMENDO PICO
//De 0 a 0.2 sube al cuadrado, y de a 0.8 a 1 baja como cuadrado
double almostFlat(int x, int size){
	if(x <0.2*size) return 25* x/size*x/size;
	if(x> 0.8*size) return 1- 25*pow((x/size-0.8),2.);
	if(x<0 or x>size) return 0;
	else return 1;
}





/***********************************************************

						ENVELOPES

*************************************************************/
//you can also use Window Functions as envelopes! Format is : Envelope(currentSample, Total Sample Lenght())


	/************* 			FADE INS 			*//////////////////////////////
	//They all work as fade outs if you pass total-current as the first variable

//For efficiency you could have an *double slope parameter and only calculate it one if NULL. but I dont care about that now
double lin(int current, int total){
	//double slope = 1.0 / (double) total;
	return (double) current / (double) total;
}

double x2(int current, int total){
	// f(x) = x2,   f(0) = 0, f(1) = 1
	// f(k*x)=k2x2, f(0) = 0, f(total)= 1 --> k=1/total. Stretch the graph!
	return (double) (current*current) / (double)(total*total);
}

/*
double gaussian(int current, int total){

	return
}*/

#endif
