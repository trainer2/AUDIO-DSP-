#include "funciones.h"
#include "osciladores.h"
#include <vector>
#include <string>
#include <iostream>

using namespace std;


const double frec = 440;
const int harmonics = 10+1;
const int dur =3;

void sq(); void swt(); void tri(); void sinLookup(); void genpulse(int); void camIdea(int);

int main(int argc, char const *argv[])
{
	sinLookup();
	tri();
	return 0;
}


void camIdea(int n){
	vector<double>* sintable = init_sinTable();
	vector<lookupOscillator> oscs;
	for(int i =1 ; i< n+1; i++) oscs.push_back(lookupOscillator(frec *(i+n),1.0 /(double) i,sintable ));
	
	vector<double> buffer(dur*44100);
	for(int i =0; i< dur*44100; i++) buffer[i] =sum(oscs)/n *2;
	guardarWavMono(buffer, 16, 44100, "IDEA" +to_string(n) + ".wav");
}

void genpulse(int nHarm){

	vector<double>* sintable = init_sinTable();
	vector<lookupOscillator> oscs;

	for(int i =1 ; i< nHarm+1; i++) oscs.push_back(lookupOscillator(frec *i,1,sintable ));
	vector<double> buffer(dur*44100);
	for(int i =0; i< dur*44100; i++) buffer[i] =sum(oscs)/nHarm;

	guardarWavMono(buffer, 16, 44100, "PULSE" +to_string(nHarm) + ".wav");

}

void sinLookup(){
	vector<double>* sintable = init_sinTable();
	vector<lookupOscillator> oscs;

	for(int i =1 ; i< harmonics; i++) oscs.push_back(lookupOscillator(frec *i, (double) 1.0/ (double) (i+1),sintable ));
	vector<double> buffer(dur*44100);
	for(int i =0; i< dur*44100; i++) buffer[i] =sum(oscs);

	guardarWavMono(buffer, 16, 44100, "LOOKUP" +to_string(harmonics) + ".wav");

}



void writeSth(){
	vector<Oscillator> oscs;

	for(int i =0 ; i< harmonics; i++) oscs.push_back(Oscillator(frec *i, (double) 1.0/ (double) (i+1)));
	vector<double> buffer(dur*44100);
	for(int i =0; i< dur*44100; i++) buffer[i] =sum(oscs);

	guardarWavMono(buffer, 16, 44100, "OSC3" +to_string(harmonics) + ".wav");

}

void sq(){
	vector<Oscillator> oscs;

	double normFac =0;
	for(int i=1;   i<harmonics;  i+=2 ) normFac += 1.0 / i; 
	for(int i =1 ; i< harmonics; i+=2 ) if(i*frec >= srate/2) break; else oscs.push_back(Oscillator(frec *i, (double) 1.0/ (double) i  ));
	vector<double> buffer(dur*44100);
	for(int i =0; i< dur*44100; i++) buffer[i] =sum(oscs) /normFac;

	guardarWavMono(buffer, 16, 44100, "SQ" +to_string(harmonics) + ".wav");
	cout<<normFac;
}

void swt(){
	vector<Oscillator> oscs;
	double normFac =0;
	for(int i=1;   i<harmonics;  i++ ) normFac += 1.0 / i; 
	for(int i =1 ; i< harmonics; i++ ) if(i*frec >= srate/2) break; else oscs.push_back(Oscillator(frec *i, (double) 1.0/ (double) i ));
	vector<double> buffer(dur*44100);
	for(int i =0; i< dur*44100; i++) buffer[i] =sum(oscs) / normFac;

	guardarWavMono(buffer, 16, 44100, "SWT" +to_string(harmonics) + ".wav");
	cout <<normFac;
}


void tri(){
	vector<Oscillator> oscs;
	double normFac = 0;
	for(int i=1;   i<harmonics;  i+=2 ) normFac += 1.0 / (i*i); 
	for(int i =1 ; i< harmonics; i+=2 ) if(i*frec >= srate/2) break; else oscs.push_back(Oscillator(frec *i, (double) 1.0/ (double) (i*i)  ));
	vector<double> buffer(dur*44100);
	for(int i =0; i< dur*44100; i++) buffer[i] =sum(oscs) / normFac;

	//guardarWavMono(buffer, 16, 44100, "TRI" +to_string(harmonics) + ".wav");
	escuchar(buffer);
	cout << normFac;
}



