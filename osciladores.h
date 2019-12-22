#ifndef OSCPP
#define OSCPP

#include <math.h>
#include<vector>
#include <iostream>
using namespace std;

const double srate = 44100;

#ifndef ASKA
const double PI    = 3.141592654;
const double TWOPI = 6.283185307179586476925286766559;
#endif


//*		TODO: UPDATE TO ALLOW FM, PD and PM *
/*  			TABLE LOOKUP OSCILLATORS  */
/*				ALL TABLES HAVE 1HZ FULL AMPLITUDE (digital 0dB) WAVES AT 44100 SRATE */
vector<double> SINTABLE;
vector<double>* init_sinTable(){
	if(SINTABLE.size()==0)
		for(int i=0; i<44100; i++) SINTABLE.push_back(sin(i*TWOPI /44100));
	return &SINTABLE;
}




class lookupOscillator{
public:

	double freq;
	int curIndex;
	int indexIncr;
	double amp;
	vector<double>* table;

	lookupOscillator(double f, double a, vector<double>* t) : freq(min(f, 44099.0/2.0)), amp(a), table(t), curIndex(0), indexIncr((int)freq)
	{};

	void next();
	double tick();

};

void lookupOscillator::next(){
 curIndex+= indexIncr;
 if (curIndex > 44100) curIndex -= 44100;
}

double lookupOscillator::tick(){
	next();
	return (*table)[curIndex]*amp;
}



/*		SIMPLE OSCILLATORS */
//TODO: AMPLITUDES
class Oscillator{
public:

	Oscillator(double frec, double amp, double ip, double opt);//FALTA: TIPO DE OSCILLADOR
	double freq;
	double curPhase;
	double phaseIncr;
	double amp;
	
	void next() 		{curPhase+=phaseIncr;}
	void prev() 		{curPhase-=phaseIncr;}
	void next(int nsamp){curPhase+=nsamp*phaseIncr;}
	void prev(int nsamp){curPhase-=nsamp*phaseIncr;}
	
	//next and fold over2Pi
	void nextF();
	void prevF();


	/*       Opcionales - Por si se usan para hacer otras ondas    	*/
	double cycleLen;
	double upLen;
	double upSlope;
	double downSlope; 
	double curAmp;
	//double (*osc) (double t) = NULL;
	
};



// S es un oscilador propiamente dicho. Tiene que cumplir que sea periodica con periodo per
//Esto lo hago por si despues se me canta meter una cuadrada como oscilador
Oscillator::Oscillator(double frec, double ampl=1, double initialPhase=0, double opt=0.5){

	freq = frec;
	curPhase=initialPhase;
	phaseIncr = TWOPI/srate * freq;
	amp = ampl;
	cycleLen = TWOPI *srate/freq;
	upLen = cycleLen*opt;
	if (opt ==0) upSlope = 2/cycleLen;
	else {upSlope = 2 /upLen;
	   	  downSlope= 2 / (cycleLen-upLen);} 
	curAmp =0;
	//osc = new double (*S)(double, double);
}

/*
void Oscillator::next(){
	curPhase+=phaseIncr;
	//return *osc(curPhase);
}
void Oscillator::prev(){
	curPhase -= phaseIncr;
	//return *osc()
}*/

void Oscillator::nextF(){
	curPhase+=phaseIncr;
	while(curPhase>=TWOPI) curPhase -= TWOPI;
}

void Oscillator::prevF(){
	curPhase-=phaseIncr;
	while(curPhase<TWOPI) curPhase += TWOPI;
}


//Seno normalizado: sen(0)
double sinOsc(Oscillator& osc){
	osc.nextF();
	return osc.amp*sin(osc.curPhase);
}


double pulse(Oscillator& osc){
	double ret;
	if(osc.curPhase < osc.upLen ) ret=1;
	else ret =-1;
	osc.nextF();
	return ret*osc.amp;
}

double tri(Oscillator& osc){
	if (osc.curPhase <= osc.upLen) osc.curAmp += osc.upSlope;
	else osc.curAmp -= osc.downSlope;

	osc.nextF();
	return osc.curAmp * osc.amp;;
}


double sum(vector<Oscillator>& oscs){
	double sum=0;
	for(int i=0; i<oscs.size(); i++) sum+=sinOsc(oscs[i]);
	return sum;
}


double sum(vector<lookupOscillator>& oscs){
	double sum=0;
	for(int i=0; i<oscs.size(); i++) sum+=oscs[i].tick();
	//cout <<sum;
	return sum;
}
#endif