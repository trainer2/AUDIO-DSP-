#pragma once
#include "funciones.h"

//Hace convolucion así directo con FFT de las dos señales, sin hacerlo por frames
//Como quiero que haya el leakage y quiero usar FFT para no estar mil millones de años, no voy a usar zero pad.
//Trunca. Si s tiene 513 samples o 1023 samples, lo trunca a 512
ssignal circularConv(ssignal s, ssignal h){
	//Zero pad, tipicamente h, para poder hacer la multiplicacion espectral. Si padeo el IR sigue ocurriendo la circular
	while(s.size()<h.size()) s.push_back(.0);
	while(h.size()<s.size()) h.push_back(.0);

	csignal C = fft3(s);
	csignal H = fft3(h);

	for (int i = 0; i <C.size() ; ++i) C[i] = C[i]*H[i];

	return ifft3(C);
}


//Toma dos señales. Devuelve una cuyo espectro tiene las magnitudes espectrales de la primera y las fases de la segunda. 
//Tip: Poner En la de fases la que tenga eventos sonoros interesantes: Transientes, picos. Y en la de frecuencias, sonidos armonicos

ssignal XSynth(ssignal freqs, ssignal phas){

	//1-Igualo la cantidad de samples entre los dos.
	//Zero pad la mas chica hasta igualar a la otra
	while (freqs.size() < phas.size()) freqs.push_back(.0);
	while (freqs.size() > phas.size()) phas.push_back(.0);

	//2-fft
	csignal F = fft3(freqs);
	csignal P = fft3(phas);
	csignal out(F.size());

	//3-Este coso
	for (int i = 0; i < F.size(); ++i)
	{
		double mag = F[i].re*F[i].re+F[i].im*F[i].im;
		mag = sqrt(mag);
		double phase = atan2(P[i].im, P[i].re);
		out[i].re = cos(phase)*mag;
		out[i].im = sin(phase)*mag;
	}
	return ifft3(out);
}

//Hace exactamente un reverse de la señal! Exactamente eso, más correr todo un sample a la derecha. O sea: Deja el sample 0 fijo, y hace reverse de todo lo que queda. Que loco.
//Es equivalente a "mirror audio in range(1, size(s))"
void spectralReverse(ssignal &s){
	csignal C = fft3(s);
	for (int i = 0; i < C.size(); ++i) C[i].im *=-1;
	s = ifft3(C);
}



ssignal igualarReIm(ssignal& s){

	csignal C = fft3(s);
	for (int i = 0; i < C.size(); ++i) C[i].im=C[i].re;
	return ifft3(C);
}