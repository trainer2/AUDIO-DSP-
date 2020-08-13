#include "funciones.h"
#include "dft.cpp"
#include "convolution.cpp"

// 1- Voy a crear una señal que solo tenga una ventana como espectro. Tengo que ver como le genero el espectro en las frecuencias que yo quiero, entonces
// Esto va a cambiar la fase de la señal convolucionada seguramente
spectrum Square(int size, double fromfreq, double tofreq, double sizeFactor = 100.0){

	spectrum out = spectrum(size);
	int ifrom = out.index(fromfreq);
	int ito   = out.index(tofreq);
	double height = (double) 1/ (tofreq-fromfreq) * sizeFactor * size;

	for (int i = 0; i < out.cosines.size()/2; ++i)
		if(i> ifrom and i<ito) out.cosines[i] = height;

	return out;
}

ssignal bandpass1(ssignal& s, double fromfreq, double tofreq){
	spectrum BP = Square(1024, fromfreq, tofreq);
	cout << "Synthesis ok \n";
	ssignal BPsig = syntethise(BP);
	return convolution1(s, BPsig);
}

//2- Modifico el espectro directamente
ssignal bandpass2(ssignal& s, double fromfreq, double tofreq, double (*g)(double x) =id){

	

}


//Resultado: Filtraron, pero no un bandpass como dicho. 
int main(int argc, char const *argv[])
{
	spectrum S = Square(4096, 1000, 2000);
	ssignal s  = syntethise(S);
	guardarWavRapido(s,"serasinc");
	return 0;
}

void bandpassTest(){

	ssignal sq3    = readWav("carriers/sq3.wav");
	ssignal wnoise = readWav("carriers/wnoise.wav");
	ssignal synth  = readWav("carriers/synth.wav");

	ssignal bpsq     = bandpass1(sq3,    600, 6000);
	ssignal bpwnoise = bandpass1(wnoise, 100, 1000);
	ssignal bpsynth  = bandpass1(synth,  1000, 3000);	

	guardarWavRapido(bpsq, "filter results/ sq3");
	guardarWavRapido(bpwnoise, "filter results/ wnoise");
	guardarWavRapido(bpsynth, "filter results/ synth");

}