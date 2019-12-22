#define applyEnvelope
#include "funciones.h"
#include "Envelopes.cpp"

enum {zero, carrier, modulator, envtype, envrate, nargs};


int main(int argc, char const *argv[])
{
	      
	if (argc < nargs ) {cout<< "USO: \n \n"

		"carrierPath.wav 	Envelope.wav	envtype(stop o retrigger o cycle)  envelopeReadRate(int) \n \n"; return 1;}

	string in  = argv[carrier];
	string mod = argv[modulator];
	uint rate = atoi(argv[envrate]);
	char type = argv[envtype][0];
	size_t lastindex = in.find_last_of("."); 
	string out = in.substr(0, lastindex) + "_enveloped_" + to_string(type);
	vector<double> buffer = readWav(in);
	vector<double> envolvente = readWav(mod);
	Envolver( buffer, envolvente, rate, type) ;
	guardarWavMono( buffer, 16, 44100, out);

	return 0;
}