#ifndef STFTF
#define STFTF

#include "funciones.h"
#include "fft.cpp"
struct stftSpectrum;

stftSpectrum stft(ssignal &s, int fftsize=1024, int hop=512, double (*window)(int i, int size) = Hann){

	stftSpectrum Sp;
	Sp.fftsize = fftsize;
	Sp.hop = hop;
	Sp.window = window;
	Sp.size = s.size();
	int s_k =0;
	ssignal windowed(fftsize);
	while(s_k+fftsize <= s.size())
	{
		//window(i, size) vale 0 en i=0 y 1 en i=size/2
		for (int i = 0; i < fftsize; ++i, ++s_k)
		{windowed[i] = s[s_k] * window(i, fftsize);}
		csignal anal = fft3(windowed);
		Sp.cs.push_back(anal);
		s_k -= (fftsize-hop);
	}
	return Sp;
}

//Anda perfecto si los parametros cumplen COLA, habria que ajustarlo si no lo cumplen. El problema actual es que casi ningun sample tiene su amplitud real en cada analisis (un remedio para esto son los "FLAT-TOP WINDOWS")

ssignal istft(stftSpectrum& Sp){
	printf("STFT DE %i FRAMES\n",Sp.cs.size() );
	ssignal s(Sp.size, 0.);
	ssignal ss;
	int s_k=0;
	for (int i = 0; i < Sp.cs.size(); ++i)
	{
		ss = ifft3(Sp.cs[i]);
		for (int k = 0; k < Sp.fftsize; ++k, ++s_k) s[s_k] += ss[k];
		s_k-=(Sp.fftsize-Sp.hop);
	}
	return s;
}


#endif