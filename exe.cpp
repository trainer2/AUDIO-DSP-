#ifndef EXE
#define EXE 

#include <string>
#include <iostream>
#include <windows.h>
#include "funciones.h"
#include "convolution.cpp"
#include "generalFx.cpp"
#include "fft.cpp"
#include "dft.cpp"
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include "stft.cpp"
#include "phasevocoder.cpp"
#include "spectralfx.cpp"
typedef std::vector<std::string> stringvec;
void read_directory(const std::string& name, stringvec& v);

void convolucionar(){
	ssignal we = readWav("carriers/WEFrag.wav");
ssignal chord = readWav("carriers/chord.wav");
ssignal conv = convolution1(we, chord);
guardarWavRapido(conv, "we+chord");

}

//Hace convolucion de todas las files de d1 con todas las files de d2. Ojo. Tarda una reeeeeeeeeeeeeeeeeeee bocha
//Pensar que dos audios que juntos duran 30s pueden tardar 30'+
void convolucionarDosDirs(string dir1, string dir2){
	stringvec v1; stringvec v2;
	read_directory(dir1, v1); read_directory(dir2, v2);

    for (int i = 2; i < v1.size(); ++i)
    {
    	cout << v1[i]<<endl;

    }

	for (int i = 2; i < v1.size(); ++i)
		for (int j = 2; j < v2.size(); ++j)
		{
			string  nuevo = v1[i] + "  +  " +  v2[j];
			cout << "Haciendo ahora la convolucion entre " << nuevo <<endl;
			ssignal s1 = readWav(dir1 + v1[i]);
			ssignal s2 = readWav(dir2 + v2[j]);
	
			ssignal out = convolution1(s1, s2);
			guardarWavRapido(out, nuevo );
		}
}

void deconvolve(){
		ssignal dry = readWav("drywets/sweep.wav");
        ssignal wet = readWav("drywets/Piano Sweep Sustain.wav");
        ssignal out = Wiener_deconv(dry, wet, id);

        guardarWavRapido(out, "IR1"); 

        ssignal wet2 = readWav("drywets/Piano Sweep Sustain 2.wav");
        ssignal out2 = Wiener_deconv(dry, wet2, id);
        guardarWavRapido(out2, "IR2");

        ssignal wet3 = readWav("drywets/Yamaha DADF#AD SM58 Amp.wav");
        ssignal out3 = Wiener_deconv(dry, wet3, id);
        guardarWavRapido(out3, "IR3");
}
void read_directory(const std::string& name, stringvec& v)
{
    std::string pattern(name);
    pattern.append("\\*");
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
        do {
            v.push_back(data.cFileName);
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    } 
}

void experimentoImpulses(){
		ssignal imp = trueImpulse(440, 2);
	for (int i = 2; i < 5000; i+=i)
	{
		ssignal d = impulse(i, 440, 2, false);
		d = d- imp;
		guardarWavRapido(d, "impulse " + to_string(i)+" harmonics cosine");

	   
	    d = impulse(i, 440, 2, true);
		d = d- imp;
		guardarWavRapido(d, "impulse " + to_string(i)+" harmonics sine");
	}
	guardarWavRapido(imp, "posta");
}

void experimentoTodasFreq(){

	double from = 500.0;
	double to = 750.0;
	double step =100.0;
	ssignal s;
	double dstep = step*0.1;
	double width = to - from;
	while (from >30.0 && to < 17000.0){
		s=allFrequencies(step, 3, from, to);
		guardarWavRapido(s, "temps/todas freq - " + to_string(from) + "hz a " + to_string(to) + "hz - spacing " + to_string(step) );
		step -= dstep;
		if(step < width/500 or step <=0){
			to *= 2.5; 
			from *=0.6;
			width = to-from;
			step = (width)/20;
			dstep = step*0.1;
		}

	}
}

void derivarDir(string path, string dirout){
	stringvec v; 
	read_directory(path, v);
	for (int i = 2; i < v.size(); ++i)
	{
		ssignal s = readWav(path + v[i]);
		ssignal out = derivar(s);
		guardarWavRapido(out, dirout + v[i] + "derivado");
		out =integrar(s);
		guardarWavRapido(out, dirout + v[i] + "integrado");
		out =integrarTrucho(s);
		guardarWavRapido(out, dirout + v[i] + " integrado cualquiera");
	}
}

//La idea: Tengo un sinc con pico cada 1/441 segundos. Voy sumando esos sincs. Si dura 10 segundos tengo 4400 sincs que sumar, que son sincs shifteados nada mas
void periodizeSinc(){
	int N = SAMPLING_RATE *10;
	ssignal s = ssignal(N, 0);
	double sineCt = sineCtCalc(441);
	for (int i = 0; i < N; ++i)
		for (int j = 0; j <4410 ; ++j) 
		//cuando j es 3, por ejemplo, quiero que ese sinc este centrado en el segundo 3/441s , el sample 3/441*44100 = 300. Cuando j es tres, entonces, i = 300 deberia hacer que i - 100j = 0
		s[i] += sinc((double) (i-100*j)*sineCt);
	csignal S = fft2(s);
	to_graph(S, "res dft/sinc periodica.txt");

	//guardarWavRapido(s, "periodized sinc");

}
//Esto equivale a periodizar con deltas / SHA /dirac comb

void FTtestSimple(){
	csignal f(4); f[0] =complex(0);f[1] =complex(1);f[2] =complex(2);f[3] =complex(3);
	csignal FF  = fft2(f);

	cout << f[0].re << " " <<f[1].re << " "<<f[2].re << " "<<f[3].re<<endl << f.size()<<endl;
	cout << f[0].im << " " <<f[1].im << " "<<f[2].im << " "<<f[3].im<<endl << f.size()<<endl;


	cout << FF[0].re << " " <<FF[1].re << " "<<FF[2].re << " "<<FF[3].re<<endl << FF.size()<<endl;
	cout << FF[0].im << " " <<FF[1].im << " "<<FF[2].im << " "<<FF[3].im<<endl << FF.size()<<endl;





}

void FTtest2(){

	ssignal S = readWav("carriers/cello.wav");
	csignal F = fft2(S);
	to_graph(F, "dft res/cello.txt");
	ssignal IF = ifft2(F);
	guardarWavRapido(IF, "dft res/cello");
}

void FTtest3(){
/*
	cout <<"\n\nORIGINAL\n\n\n";

	ssignal f; for (uint32_t i = 0; i < 4; ++i)
	{
		f.push_back((double)i);
		cout <<f[i]<< endl;
	}
	csignal c= reordered_copy(f, (uint8_t) 30, 8);

	cout <<"REORDENADA" << endl <<endl;
	for (uint32_t i = 0; i < 4; ++i)
	{
		printf("%e  +  %e i\n", c[i].re, c[i].im);
		//cout << i << " SE HACE " << (reverse(i) >>(29))<<endl;//<< reverse(i) << endl;
	}
*/

	ssignal S = readWav("carriers/chord.wav");
	csignal F = fft3(S);
	to_graph(F, "dft res/FFT3.txt");
	ssignal IF = ifft3(F);
	guardarWavRapido(IF, "dft res/IFFT3");
	csignal F2 = fft2(S);
	to_graph(F2, "dft res/FFT2.txt");
	ssignal IF2 = ifft2(F2);
	guardarWavRapido(IF2, "dft res/IFFT2");


}
void FTtest(){
	csignal f(4); f[0] =0;f[1] =1;f[2] =2;f[3] =3;
	//csignal g = freverse(f); 
	csignal g = f;
	cout << g[0].re << " " <<g[1].re << " "<<g[2].re << " "<<g[3].re<<endl;
	to_graph(g, "dft res/ test.txt");
	//SEÑALES
	int dur = 44100;
	ssignal s = sincx(440, dur);
	ssignal s2 = puretone(440, dur);
	s2  =s2+ puretone(880, dur);
	s2  =s2+ puretone(1000, dur);
	guardarWavRapido(s, "dft res/s");
	guardarWavRapido(s2, "dft res/s2");
	
	//ANALISIS
	spectrum S  = dft(s);
	spectrum S2 = dft(s2);

	csignal F  = dft2(s);
	csignal F2 = dft2(s2);

	csignal FF  = fft2(s);
	csignal FF2 = fft2(s2);

	//Grafico del espectro
	S.outPowerSpectrum("dft res/dft clasico.txt");
	S2.outPowerSpectrum("dft res/dft clasico2.txt");

	to_graph(F,  "dft nuevo.txt");
	to_graph(F2, "dft nuevo2.txt");

	to_graph(FF,  "dft res/fft.txt");
	to_graph(FF2, "dft res/fft2.txt");

	//Resintesis

	s = syntethise(S);
	guardarWavRapido(s, "dft res/dft clasico");
	s= syntethise(S2);
	guardarWavRapido(s, "dft res/dft clasico2");
	s= idft2(F);	
	guardarWavRapido(s, "dft res/dft nuevo");
	s= idft2(F2);
	guardarWavRapido(s, "dft res/dft nuevo 2");
	s= ifft2(FF);
	guardarWavRapido(s, "dft res/ifft");
	s= ifft2(FF2);
	guardarWavRapido(s, "dft res/ifft2");

	//periodizeSinc();
}


void stftest(){
	ssignal S = readWav("carriers/chord.wav");
	stftSpectrum Sp = stft(S);
	S = istft(Sp);
	guardarWavRapido(S, "dft res/stft");
}


void PVtest(){

	//1-Analisis corto de una señal para ver cuales son los bin de frecuencias
	ssignal s = puretone(440, 44100);
	csignal F = fft3(s);
//	to_graph(F, "dft res/bins de fft 1024");
	//RESULTADO: El bin está justo en 430.664Hz	, tiene un ancho de 43.1hz

	ssignal s430 = puretone(430.664, 44100); //Cae justelli
	ssignal s425 = puretone(425, 44100);
	ssignal s420 = puretone(420, 44100);
	ssignal s400 = puretone(400, 44100);

	frequencyEstimate(s,1024,  512, Hann, "440");
	frequencyEstimate(s430,1024,  512, Hann, "430");	
	frequencyEstimate(s425,1024,  512, Hann, "425");	
	frequencyEstimate(s420,1024,  512, Hann, "420");
	frequencyEstimate(s400,1024,  512, Hann, "400");

}


void PVtest2(){
	ssignal s = puretone(1.34583, 44100);
	csignal F = fft3(s);
	to_graph(F, "NADA.txt");
	frequencyEstimate(s, 32768, 32768,id, "NADA");

	ssignal s2 = puretone(2., 44100);
	csignal F2 = fft3(s2);
	to_graph(F2, "NADA2.txt");
	frequencyEstimate(s2, 32768, 32768,id, "NADA2");
}

void PVtest3(){
	ssignal s = puretone(1.34583, 44100);
	csignal F = fft3(s);
	//to_graph(F, "LIBRO.txt");
	frequencyEstimate2(s, 32768, 32768,id, "LIBRO");

	ssignal s2 = puretone(2., 44100);
	csignal F2 = fft3(s2);
	//to_graph(F2, "LIBO2.txt");
	frequencyEstimate2(s2, 32768, 32768,id, "LIBRO2");
}

void PVtest4(){
	ssignal s = puretone(1.34583, 44100);
	csignal F = fft3(s);
	//to_graph(F, "LIBRO.txt");
	frequencyEstimate2(s, 32768, 32768,id, "LIBRO");

	ssignal s2 = puretone(2., 44100);
	csignal F2 = fft3(s2);
	//to_graph(F2, "LIBO2.txt");
	frequencyEstimate2(s2, 32768, 32768,id, "LIBRO2");
}

void PVSYNTHESISTEST(){
	int fftlong = 512*16; int hoplong=256*16;
	int fftshort = 1024; int hopshort=512;
	
	ssignal s = readWav("carriers/chord.wav");
	vector<csignal> PLON = frequencyEstimate2(s, fftlong, hoplong,Hann);
	s=pvs(PLON, fftlong,hoplong);
	guardarWavRapido(s, "PV RESINTESIS LONG ANALYSIS");

	s = readWav("carriers/chord.wav");
	vector<csignal> P = frequencyEstimate2(s, fftshort, hopshort,Hann);
	s=pvs(P, fftshort,hopshort);
	guardarWavRapido(s, "PV RESINTESIS SHORT ANALYSIS");

}

void PVMORPHTEST(){
	int fftsize = 4096*16; int hop = fftsize/8;
	ssignal s1 = readWav("carriers/chord.wav");
	ssignal s2 = readWav("carriers/beautiful2.wav");
	ssignal out=PVmorph(s1, s2, true, true, fftsize, hop, id);
	guardarWavRapido(out, "morph both");
	out=PVmorph(s1, s2, false, true, fftsize, hop, id);
	guardarWavRapido(out, "morph Amp");
	out=PVmorph(s1, s2, true, false, fftsize, hop, id);
	guardarWavRapido(out, "morph Freq");
	//frequencyEstimate2(s2, 32768, 32768,id, "F ESTIMATE", true);

	//out=PVmorph(s1, s2, false, false);
	//guardarWavRapido(out, "morph None");
}

void SHRINKTEST(){
	ssignal s1 = readWav("carriers/chord.wav");
	ssignal s2 = readWav("carriers/beautiful2.wav");
	ssignal out;

	out=stretchFFT(s1);
	guardarWavRapido(out,"FFT STRETCHED");
	out=stretchPV(s1);
	guardarWavRapido(out,"PV STRETCHED");
	out=shrinkPV(s1);
	guardarWavRapido(out,"PV shrinked");
	out=shrinkFFT(s1);
	guardarWavRapido(out,"FFT shrinked");

}
void PITCHTEST(){
	ssignal s1 = readWav("carriers/chord.wav");
	ssignal out;
	out=pitchUpFFT(s1);
	guardarWavRapido(out,"Pitched UP FFT");
	out=pitchDownFFT(s1);
	guardarWavRapido(out,"Pitched DOWN FFT");
	out=pitchUpPV(s1);
	guardarWavRapido(out,"Pitched UP PV");
	out=pitchDownPV(s1);
	guardarWavRapido(out,"Pitched DOWN PV");

}

void IDFTEST(){
	int fftsize = 4096*2; int hop = fftsize/2;

	ssignal s1 = readWav("carriers/cello.wav");
	vector<csignal> PV = IFD(s1, fftsize, hop, Hann);
	ssignal out;
	 out = pvs(PV, fftsize, hop);
	guardarWavRapido(out, "cello IDF ANALYSIS + PVS SINTESIS 8k");
	out = addsyn(PV, fftsize, hop);
	guardarWavRapido(out, "cello IDF ANALYSIS + OSC SINTESIS 8k");
	PV = frequencyEstimate2(s1, fftsize, hop);
	out = addsyn(PV, fftsize, hop);
	guardarWavRapido(out, "cello PV ANALYSIS + OSC SINTESIS 8k");
	out = pvs(PV, fftsize, hop);
	guardarWavRapido(out, "cello PV ANALYSIS + PV SINTESIS 8k");

}


void scrambleDir(string dir1){

	stringvec v1;
	read_directory(dir1, v1);
	for (int i = 2; i < v1.size(); ++i)
		{
			string  nuevo = v1[i];
			cout << "Haciendo ahora el unscramble de " << nuevo <<endl;
			unscramble(dir1+"/"+v1[i],0 , "d=5000");
			unscramble(dir1+"/"+v1[i], 1, "d=FULL");
		}
}

void processSignals(string dir1){

	stringvec v1;
	read_directory(dir1, v1);
	ssignal s;
	ssignal s2;
	for (int i = 2; i < v1.size(); ++i)
		{
			string  nuevo = v1[i];
			cout << "Haciendo ahora el H Transform y Analytical de " << nuevo <<endl;
			s=readWav(dir1+"/"+v1[i]);
			s2=HTransform(s);
			s2 = s2+s;
			guardarWavRapido(s2,  "Transforms2/"+v1[i] + " Hilbert ");
			s2=to_analytical(s);
			s2 = s2+s;
			guardarWavRapido(s2, "Transforms2/"+v1[i] + " analytical ");

		}
}


//Precioso... Bas drop y +Lluvia... precioso...

void spectralProcess(string dir1, string dirout){
	stringvec v1;
	read_directory(dir1, v1); 

	for (int i = 2; i < v1.size(); ++i)
		for (int j = i+1; j < v1.size(); ++j)
		{
			ssignal s1 = readWav(dir1 + v1[i]);
			ssignal s2 = readWav(dir1 + v1[j]);

			// XSYNTH
			string  nuevo = dirout + v1[i] + "  +  " +  v1[j] + "X SYNTH" ;
			ssignal out = XSynth(s1, s2);
			cout << "Haciendo ahora el XSYNTH de " << nuevo <<endl;
			guardarWavRapido(out, nuevo );


			//   CIRCULAR CONV
			nuevo = dirout + v1[i] + "  +  " +  v1[j] + "CIRCULAR CONV" ;
			cout << "Haciendo ahora el CIRCULAR CONV DE de " << nuevo <<endl;
			out = circularConv(s1, s2);
			guardarWavRapido(out, nuevo );


			//Spectral Reverse

			nuevo = dirout + v1[i] + "Phase Reversed" ;
			cout << "Haciendo ahora el PHASE REVERSE de " << nuevo <<endl;
			spectralReverse(s1);
			guardarWavRapido(s1, nuevo );

		}
}


//A ver la diferencia entre la circular y la por definicion
void Experimento(){

	//DIFERENCIA ENTRE CONVOLUION CIRCULAR Y DIRECTA
	ssignal s = randomss(1024);
	ssignal h = randomss(256);
	ssignal test(1024, 0.);
	test[0]=1;
	test[1]=.5;
	test[2]=.3;


	ssignal directa = convolution1(s, h);
	ssignal circular = circularConv(s, h);

	//Prediccion: Los primeros 256 samples de "circular" van a ser los primeros 256 de "directa" mas los ultimos 256 de "directa"
	ssignal diff(1024);
	cout << directa.size()<<endl;
	for (int i = 0; i < 1024; ++i) diff[i] = circular[i]-directa[i];
	for (int i = 0; i < 255; ++i)  diff[i] -= directa[i+1024]; 


	guardarWavRapido(directa, "directa", false);
	guardarWavRapido(circular, "circular", false);
	guardarWavRapido(diff, "diferencia", true);
	guardarWavRapido(s, "test", false);
	csignal T = fft3(test);
	test = ifft3(T);
	guardarWavRapido(test, "test i");

	//DIFERENCIA ENTRE CONVOLUCION CIRCULAR Y CON ZERO PAD

	//Differencia entre Reverse y Phase Invert
	ssignal ss2= s;
	spectralReverse(s);
	mirrorAudioInRange(ss2,0,1024);
	guardarWavRapido(s, "rev fft", false);
	guardarWavRapido(ss2, "rev definicion", false);

	ss2 =ss2- s;
	guardarWavRapido(ss2, "diferencia fase", false);




}

/*********************************************************************************************/


int main(){
	//convolucionarDosDirs("animals/", "IRS/");
	//derivarDir("carriers/", "calculus/");
	//experimentoTodasFreq();
	/*ssignal s = PM(5);
	guardarWavRapido(s, "Phase Mod Test 1");

	s  = PM(5, 3);
	guardarWavRapido(s, "Phase Mod Test 2");
	s = PM(5, 10);
	guardarWavRapido(s, "Phase Mod Test 3");*/
	//ssignal s = descramble("PS-4-scramble.wav");
	//unscramble("PS-4-scramble.wav");
	//scrambleDir("carriers/");
	//processSignals("carriers/");
	

	//spectralProcess("carriers/", "spectralFX/");
	
	Experimento();
	printf("OK!!\n");
	return 0;
}










#endif