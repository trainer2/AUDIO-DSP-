#ifndef PANCPP
#define PANCPP

#include "Interpoladoras.cpp"
#include "funciones.h"
#include <math.h>

const double centerPan = 0.5;
const double LeftPan = 0;
const double rightPan = 1;


double sinp(double x){ return sin(HALFPI*x);}
double cosp(double x){ return cos(HALFPI*x);}

//Se rige por
void panSamplesLinear(double& L, double& R, double pos){
	if(pos<0)  pos=0;
	if(pos >1) pos=1;
	R *= pos;
	L *= (1-pos); 
}

void panSamplesConst(double& L, double& R, double pos){
	if(pos<0) pos=0;
	if(pos >1)pos=1;
	R *= sinp(pos);
	L *= cosp(pos);
}



//Input: Buffer (definido en funciones.h) con dos canales (Estereo) o mono. Idealmente mono
//Type: l o L es paneo lineal, c = constant power
//InterpolType debiera seguir la misma convencion que la de envelopes pero es 'l' lineal 'e' exponencial 's' senoidal
void pan(AudioFile<double>& estereo, vector<breakpoint>& breakpoints, char pantype = 'l', char interpolType = 'l'){

	const double srate = estereo.getSampleRate();

	if(!estereo.isStereo()) {
		vector<double> channelR = estereo.samples[0];
		estereo.samples.push_back( channelR);
	}

	vector<double> panning;

	switch(interpolType){

	case('l'): panning=	interpolate(breakpoints, interpoladoraLineal,srate,1);

	case('e'): panning=	interpolate(breakpoints, interpoladoraExponencial,srate,1);

	case('s'): panning=	interpolate(breakpoints, interpoladoraSenoidal,srate,1);
	}

	vector<double>& L = estereo.samples[0];
	vector<double>& R = estereo.samples[1];

	if(pantype=='l' or pantype=='L')
		for(int i =0; i<min(panning.size(),L.size()) ; i++) panSamplesLinear(L[i], R[i], panning[i]);

	if(pantype=='c' or pantype=='C')
		for(int i =0; i<min(panning.size(),L.size()) ;i++) panSamplesConst(L[i], R[i], panning[i]);

	estereo.save("panned3.wav");
	return;
}






int main(int argc, char const *argv[])
{
	AudioFile<double> wav = leerWav("carriers/tri.wav");
	vector<breakpoint> B;
	B.push_back(breakpoint(0,0));
	B.push_back(breakpoint(3,2));

	pan(wav,B, 'c');
	return 0;
}











#endif