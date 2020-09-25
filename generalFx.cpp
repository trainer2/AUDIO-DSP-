#include "funciones.h"
#include "fft.cpp"
#include "phasevocoder.cpp"
#include "dft.cpp"


/****************************************************** TIME STRETCHING  ************************************/
//RESULTADO: El que mas me gusta es FFT, porque tengo que resolver ese tema de PV con las envolventes o lo que lo haga tan raro.
//UN SIGUIENTE PASO SERIA NO COPIAR FRAME A FRAME SINO INTERPOLAR. Esto aflojaria los artifacts en las transientes (escuchas dos ataques seguidos? dale)

ssignal stretchPV(PARAMS){
	vector<csignal> S = frequencyEstimate2(s,fftsize, hop, window);
	vector<csignal> Stretched;;
	for (int i = 0; i < S.size()-1; ++i)
	{
		Stretched.push_back(S[i]);
		Stretched.push_back(S[i]);
	}
	csignal temp; temp.push_back(complex((double) s.size()*2)); //le paso el tamaño para la resintesis por aca
	Stretched.push_back(temp);
	return pvs(Stretched, fftsize,hop);
}


ssignal stretchFFT(PARAMS){

	stftSpectrum S = stft(s,fftsize, hop, window);
	vector<csignal> Stretched;
	for (int i = 0; i < S.cs.size(); ++i)
	{
		Stretched.push_back(S.cs[i]);
		Stretched.push_back(S.cs[i]);
	}
	S.cs=Stretched;
	S.size*=2;
	return istft(S);
}

ssignal shrinkFFT(PARAMS){
	stftSpectrum S = stft(s,fftsize, hop, window);
	vector<csignal> Shrinked;
	for (int i = 0; i < S.cs.size(); i+=2)
		Shrinked.push_back(S.cs[i]);
	//Shrinked.push_back(S.cs[S.cs.size()-1]);
	S.cs=Shrinked;
	S.size/=2;
	return istft(S);
}


ssignal shrinkPV(PARAMS){
	vector<csignal> S = frequencyEstimate2(s,fftsize, hop, window);
	vector<csignal> Shrinked;;
	for (int i = 0; i < (S.size()-1); i+=2)
		Shrinked.push_back(S[i]);
	csignal temp; temp.push_back(complex((double) s.size()/2)); //le paso el tamaño para la resintesis por aca
	Shrinked.push_back(temp);
	return pvs(Shrinked, fftsize,hop);
}


ssignal freezeFFT(PARAMS){return s;}

ssignal freezePV(PARAMS){return s;}

/******************************************************************************************************************************************/
/*****************************************************************PITCH SHIFTING ****************************************************/
//RESULTADOS: SUPER INTERESANTE!! Son sonidos medio rotos pero atractivos igualmente (esos artifacts).


ssignal pitchUpFFT(PARAMS){
	stftSpectrum S = stft(s,fftsize, hop, window);
	for (int frame = 0; frame < S.cs.size(); ++frame)
	{
		for (int bin = S.fftsize/2-1; bin >0; bin--)
		{
			//PRIMERA MITAD DE COORDENADAS
			S.cs[frame][bin] =  S.cs[frame][bin/2];
			S.cs[frame][bin/2]=complex(0.,0.);

			//SEGUDA MITAD DE COORDENADAS
			S.cs[frame][bin+fftsize/2] = conjugado(S.cs[frame][bin]);
		}
	}
	return istft(S);
}



ssignal pitchDownFFT(PARAMS){
	stftSpectrum S = stft(s,fftsize, hop, window);
	for (int frame = 0; frame < S.cs.size(); ++frame)
	{
		for (int bin =0 ; bin < fftsize/4; bin++)
		{
			//PRIMERA MITAD DE COORDENADAS
			S.cs[frame][bin] =  S.cs[frame][bin*2];
			S.cs[frame][bin*2]=complex(0.,0.);

			//SEGUDA MITAD DE COORDENADAS
			S.cs[frame][bin+fftsize/2] = conjugado(S.cs[frame][bin]);
		}
	}
	return istft(S);
}

//ESE bug esta bueno (mal recorrido el espectro, ver indices del segunod for)
ssignal pitchDownFFTALT(PARAMS){
	stftSpectrum S = stft(s,fftsize, hop, window);
	for (int frame = 0; frame < S.cs.size(); ++frame)
	{
		for (int bin =0 ; bin < fftsize/2-1; bin++)
		{
			//PRIMERA MITAD DE COORDENADAS
			S.cs[frame][bin] =  S.cs[frame][bin*2];
			S.cs[frame][bin*2]=complex(0.,0.);

			//SEGUDA MITAD DE COORDENADAS
			S.cs[frame][bin+fftsize/2] = conjugado(S.cs[frame][bin]);
		}
	}
	return istft(S);
}


ssignal pitchUpPV(PARAMS){
	vector<csignal> S = frequencyEstimate2(s,fftsize, hop, window);

	printf("FFT SIZE ES %i Frame SIZE ES %i \n", fftsize, S[0].size());
	for (int frame = 0; frame < S.size()-1; ++frame)
	{
		for (int bin = fftsize/2-1; bin >0; bin--)
		{
			S[frame][bin] =  S[frame][bin/2];
			S[frame][bin/2]=complex(0.,0.);
			S[frame][bin+fftsize/2] = conjugado(S[frame][bin]);
		}		
	}
	return pvs(S, fftsize,hop);
}



ssignal pitchDownPV(PARAMS){

	vector<csignal> S = frequencyEstimate2(s,fftsize, hop, window);

	for (int frame = 0; frame < S.size()-1; ++frame)
	{
		for (int bin = 0; bin < fftsize/4; ++bin)
		{
			S[frame][bin] =  S[frame][bin*2];
			S[frame][bin*2]=complex(0.,0.);
			S[frame][bin+fftsize/2] = conjugado(S[frame][bin]);
		}		
	}
	return pvs(S, fftsize,hop);
}






/******************************************************************************************************************************************/




/*****************				SCRAMBlE			**************************/

/*
//Recibe un fft y te intercambia las bandas de 1khz segun el orden que le llega en ese array (40 elementos. Se asume que es simetrico para que la señal de real, pero puede no serlo)
csignal scramble(csignal& ff, int* perm, int fftsize=global_fftsize){

	csignal ff2=ff;
	float binfreq;
	vector<int> indices;
	float current = -21000.;

	for (int bin = 0;bin  < ff.size(); ++bin)
	{
		binfreq = (float) bin*SAMPLING_RATE/(float)fftsize;
		if(bin>=current) 
			{indices.push_back(bin); //indices[i] va a guardar cual es el bin que tiene la ultima frecuencia de la banda 1000*i
			current+=1000.;
			printf("BIN %i FREQ %i\n",bin, current );
		}
	}

	//Por ejemplo:Si me llega [3,2,1,0] en perm (y simplifico diciendo que tengo 3 bandas), entonces la banda de [-20khz : -10khz] va a los ultimos indices
	for (int i = 0; i < i.size()-1; ++i)
	{
		for (int freqindex = indices[i]; freqindex <indices[i+1] ; ++i)
		{
			ff2[freqindex]
			
		}
	}

	return ff2;

}*/






/******************************************************************************************************************************************/



//Basic. Takes a signal in, a starting index, a sample lenght and an out duration, and makes a new audio with that fragment looped a loot all it can fit
//envelope applies an envelope to the sampled fragment. You could also define two envelopes,
// one for fin and one for fout, in case you so desire, or have fin already include a fout. In case you use fout,
// You can specify the start sample with half, or leave it 0 for actual half of sampLen
ssignal repeatFrag(const ssignal& in, int from, int sampLen, double outDuration, 
			double (envelope (int, int)) = id, bool dofout = false, 
			double fout(int, int) = id, int half =0)
{
	int outLen = (int) (outDuration * SAMPLING_RATE);
	ssignal out = ssignal(outLen);	
	//int insize = in.size();
	//from = min(from, insize - sampLen-1); //security check
	ssignal repeat = ssignal(sampLen);
	for (int i = 0; i < sampLen; ++i) repeat[i] = in[i+from];	
	
	//ENVELOPE APPLY
	if (half==0) half=sampLen/2;
	if(dofout) {for (int i = 0; i < half;++i) repeat[i]*=envelope(i,half);
			    for (int i = half; i < sampLen; ++i) repeat[i]*=fout(i, half);}
	else for (int i = 0; i < sampLen;++i) repeat[i]*=envelope(i,sampLen);

	//Cycle
	int j=0;
	for (int i = 0; i < outLen; ++i)
	{
		out[i]=repeat[j++];
		if(j>=sampLen) j=0;	
	}
	return out;
}
//This is able to do, say, take 100 samples, apply fade in to the first 50, apply fade in to the later 50, and loop. Or take 30-70






//Va recorriendo de a 100 samples, salta 50 para atras y mete 100, salta 50 para atras y mete 100
ssignal backloopDirect(ssignal& in, int chunk, int backjump){
	ssignal out; int j=0;
	for (int i = 0; i < in.size(); ++i)
	{
		out.push_back(in[i]);
		if(++j >= chunk) {i-=backjump; j=0;}
	}
	return out;
}

//Igual, pero el salto ahora no es constante. fun va a ser random en general
// con funciones lineales puede tener un eff interesante
// chunk tambien podria cambiar luego de cada salto 
ssignal backloopDirect(const ssignal& in, int chunk, int fun(int) = randSp, int maxJump=22000){
	ssignal out;
	int j=0;
	for (int i = 0; i < in.size(); ++i)
	{  
		out.push_back(in[i]);
		if(++j >= chunk) {i-=fun(i)%i; j=0;}
	}   return out;
}
//funciones posibles: f(x) = x/2, random en un rango



//Genera una señal que es la suma de un montonazo de frecuencias, como para acercarse al ruido blanco
ssignal allFrequencies(double spacing, float len, double from = 20.0, double to = 20000.0){

	int samps = len*SAMPLING_RATE;
	ssignal out(samps, 0);
	for (double f = from; f < to; f+=spacing)
		out = out + puretone(f, samps,rand());
	
	//normalize(out);
	return out;
}

ssignal dc(int samps){return ssignal(samps, 1);}
ssignal peak(int samps){ssignal out(samps,0); out[0]=1; return out; }

//Estas tres salen de una con IR's pero queria ver que onda asi
ssignal derivar(ssignal& s){
	ssignal der = ssignal (s.size());
	for (int i = 0; i < s.size()-1; ++i)
	 der[i] = (s[i+1] - s[i])/ (double) SAMPLING_RATE;
	return der;
}

ssignal integrar(ssignal& s){
	ssignal out(s.size());
	out[0]=s[0];
	for (int i = 0; i < s.size(); ++i) out[i] = s[i] + out[i-1];
	return out;
}

ssignal integrarTrucho(ssignal& s){
	ssignal out(s.size());
	out[0]=s[0];
	for (int i = 0; i < s.size(); ++i) out[i] = s[i] + s[i-1];
	return out;
}

//Prueba de Phase Modulation con una onda cuadrada
//Interesante sonido. El phasing entre dos asi renderizadas a rate casi 0 y sumadas tambien esta bueno
//step te dice cuantos ciclos de la onda pasaron antes de que aumentes la fase de nuevo
ssignal PM(int nharmonics,double rate = 1, double step= 2, double freq = 440,  double dur=3){
	int N = SAMPLING_RATE*dur;
	ssignal s = ssignal (N,0);
	double sineCt = sineCtCalc(freq);
	double phase =0;
	double dphase = rate;
	for (int i = 0; i < N; ++i){
		for (int h = 1; h <= nharmonics; h+=2){
			s[i] += sin(sineCt*h*i +h*phase )/h;
		}	if (i% (int) ((SAMPLING_RATE/freq) * step) == 0) phase += dphase;
		}
	return s;
}


//BUENISIMO
void unscramble(string pathWav, int d1000 = 0, string tail=""){

	ssignal s = readWav(pathWav);
	csignal F = fft3(s);
	csignal F2 =F;

	float ds = SAMPLING_RATE/F.size();//diferencia de freq entre dos bins consecutivos
	if(d1000 ==0){
	 d1000 = (int) (5000./ds); 
	}
	else d1000 = F.size()/8;

	int bandaAfrom = 0; //0-5khz
	int bandaAto = d1000 +bandaAfrom;

	int bandaBfrom = bandaAto+1; //5khz-10khz
	int bandaBto = d1000+bandaBfrom;

	int bandaCfrom = bandaBto+1; //10khz-15khz
	int bandaCto = d1000+bandaCfrom;

	int bandaDfrom = bandaCto+1; //15khz-20khz

	//REORDENAR F
	//ENTRA CBDA para las primeras 4 bandas de 1khz
	//A
	for (int i = 0; i < d1000; ++i)
	{
		//Freq positivas
		F2[i+bandaAfrom] = F[i+bandaDfrom];
		F2[i+bandaBfrom] = F[i+bandaBfrom];
		F2[i+bandaCfrom] = F[i+bandaAfrom];
		F2[i+bandaDfrom] = F[i+bandaCfrom];

		//Freq negativas
		F2[i+bandaAfrom+F.size()/2] = F[i+bandaDfrom+F.size()/2];
		F2[i+bandaCfrom+F.size()/2] = F[i+bandaAfrom+F.size()/2];
		F2[i+bandaBfrom+F.size()/2] = F[i+bandaBfrom+F.size()/2];
		F2[i+bandaDfrom+F.size()/2] = F[i+bandaCfrom+F.size()/2];
	}

	s=ifft3(F2);
	//printf("ds = %.2f\n",ds );
	//printf("1000hz son  %i indices\n",d1000 );
	guardarWavRapido(s, pathWav + "USNCRAMBLED" + tail);

}


/*
//Ej 4.5 del curso de la transformada de Fourier (amo)
ssignal descramble(string pathWav){

	ssignal s = readWav(pathWav);
	spectrum S = dft(s);
	spectrum S2 = S;

	// RECIBO CBDA. QUIERO ABCD
	int bandaA, bandaB, bandaC, bandaD;
	bandaA =0;
	int i =0;
	while(S.frequency(i) <= 1000) {i++; cout<<i;}
	bandaB=i;
	while(S.frequency(i) <= 2000) i++;
	bandaC=i;
	while(S.frequency(i) <= 3000) i++;
	bandaD=i;
	while(S.frequency(i) <= 4000) i++;

	//BANDA A - 0 - 1khz
	int k=0; int j =bandaD;
	while(j < i) {
		S2.cosines[k] = S.cosines[j];	
		S2.sines[k++] = S.sines[j++];
	}

	//BANDA B - 1khz- 2khz
	j = bandaB;
	while(j < bandaC) {
		S2.cosines[k] = S.cosines[j];	
		S2.sines[k++] = S.sines[j++];
	}	
	
	//BANDA C - 2khz- 3khz
	j = bandaA;
	while(j < bandaB) {
		S2.cosines[k] = S.cosines[j];	
		S2.sines[k++] = S.sines[j++];
	}
	
	//BANDA D - 3khz- 4khz
	j = bandaC;
	while(j < bandaD) {
		S2.cosines[k] = S.cosines[j];	
		S2.sines[k++] = S.sines[j++];
	}
	ssignal s2 = syntethise(S2);
	return s2;
}*/
/*********************************************************
						A usar los fx 

***********************************************************/

/*
int main(){
	ssignal out = peak(100);
	guardarWavRapido(out, "peak");

	return 0;

}

/*CAE 1 TP3*/
void tp3CAE1(){


	ssignal Anana = readWav("MATERIALES/Anana.wav");
	ssignal arpa = readWav("MATERIALES/arpa de boca.wav");
	ssignal corcho = readWav("MATERIALES/CORCHO 2.wav");
	ssignal galv = readWav("MATERIALES/Galvanizada.wav");
	ssignal gliss = readWav("MATERIALES/glissando.wav");
	ssignal laser = readWav("MATERIALES/LASER-001.wav");
	ssignal melodica = readWav("MATERIALES/MELODICA-007.wav");
	ssignal mid = readWav("MATERIALES/Mid tension.wav");
	ssignal redo = readWav("MATERIALES/redo.wav");
	ssignal Resorte = readWav("MATERIALES/Resorte.wav");
	ssignal birome = readWav("MATERIALES/STRINGS BIROME 3.wav");


	ssignal out1 = repeatFrag(Anana, 50, 10500, 2);
	ssignal	out2 = repeatFrag(Anana, 500, 10500, 2, lin);
	ssignal out3 = repeatFrag(arpa, 9000, 20500, 4);
	ssignal out4 = repeatFrag(arpa, 9000, 20500, 4, lin, true, x2, 600);
	ssignal out5 = repeatFrag(corcho, 50, 3500, 2);
	ssignal out6 = repeatFrag(galv, 50, 16000, 5);
	ssignal out7 = repeatFrag(galv, 94000, 7000, 5, lin);
	ssignal out8 = repeatFrag(gliss, 2500, 16000, 4);
	ssignal out9 = repeatFrag(gliss, 5500, 10500, 4, x2);
	ssignal out10 = repeatFrag(laser, 50, 10500, 2);
	ssignal out11 = repeatFrag(melodica, 44100, 10500, 5);
	ssignal out12 = repeatFrag(melodica, 84000, 900, 5);
	ssignal out13 = repeatFrag(melodica, 160000, 1500, 4);
	ssignal out14 = repeatFrag(melodica, 160000, 3000, 4);
	ssignal out15 = repeatFrag(melodica, 12000,  6000, 4);
	ssignal out16 = repeatFrag(melodica, 90000, 10500, 4);
	ssignal out17 = repeatFrag(melodica, 100000, 1500, 4);
	ssignal out18 = repeatFrag(melodica, 76000, 1500, 4);
	ssignal out19 = repeatFrag(mid, 150000, 10500, 2, lin, true, lin);
	ssignal out20 = repeatFrag(mid, 50, 10500, 2);
	ssignal out21 = repeatFrag(redo, 800, 4500, 3);
	ssignal out22 = repeatFrag(redo, 800, 4500, 3, x2);
	ssignal out23 = repeatFrag(redo, 800, 4500, 3, x2, true, x2);

	ssignal infalible = repeatFrag(birome, 0, birome.size(), birome.size()/SAMPLING_RATE+1);

	guardarWavRapido(out1, "out1");
	guardarWavRapido(out2, "out2");
	guardarWavRapido(out3, "out3");
	guardarWavRapido(out4, "out4");
	guardarWavRapido(out5, "out5");
	guardarWavRapido(out6, "out6");
	guardarWavRapido(out7, "out7");
	guardarWavRapido(out8, "out8");
	guardarWavRapido(out9, "out9");
	guardarWavRapido(out10, "out10");
	guardarWavRapido(out11, "out11");
	guardarWavRapido(out12, "out12");
	guardarWavRapido(out13, "out13");
	guardarWavRapido(out14, "out14");
	guardarWavRapido(out15, "out15");
	guardarWavRapido(out16, "out16");
	guardarWavRapido(out17, "out17");
	guardarWavRapido(out18, "out18");
	guardarWavRapido(out19, "out19");
	guardarWavRapido(out20, "out20");
	guardarWavRapido(out21, "out21");
	guardarWavRapido(out22, "out22");
	guardarWavRapido(out23, "out23");

	guardarWavRapido(infalible, "infalible");
}

void testBackloop(){
		//Problema: Los clicks al saltar.
	ssignal b = readWav("carriers/beautiful1.wav");
	ssignal out;
	for (int i = 0; i < 100; ++i)
	{
		out = backloopDirect(b,rand()%30000, randc, rand()%30000 );
		guardarWavRapido(out, "out" + std::to_string(i));
	}


}

void armonizarRuidoBlanco(){
		//Quiero ver si unos samples de ruido blanco hechos periodicos se perciben como sonido armonico
	//Para percibir un sonido armonico, este se tiene que repetir al menos 20veces en un segundo
	//Voy a probar repitiendolo 441veces por segundo
	//44100 samples x segundo / 441= 100 samples por ciclo

	ssignal wn = readWav("carriers/wnoise.wav");
	for (int i = 0; i < 15; ++i)
	{

		ssignal out = repeatFrag(wn, 100+rand()%5000,100 + rand()%30-15, 3, lin, true, lin);
		guardarWavRapido(out, "out" + std::to_string(i));

	}

}

void sumarTodasLasFreq(){

	ssignal n = allFrequencies(5.0, 2);
	guardarWavRapido(n, "n");

}







ssignal impulse(int nharmonics, double freq, double duration=1, bool sine = true){
	int N = duration*SAMPLING_RATE;
	ssignal d = ssignal(N,0);
	double sineCt = sineCtCalc(freq, SAMPLING_RATE);
	if(sine)
	for (int i = 0; i < N; ++i)
		for (int k = 1; k <= nharmonics; ++k)
			d[i] += sin(sineCt*k*i);
	else 
	for (int i = 0; i < N; ++i)
		for (int k = 1; k <= nharmonics; ++k)
			d[i] += cos(sineCt*k*i);		
	return d;
}

ssignal trueImpulse(double freq, double dur=1){
	int N = dur*SAMPLING_RATE;
	ssignal d = ssignal (N, 0);
	for (int i = 0; i < N; i+= (int) ((double) SAMPLING_RATE/freq))	
		d[i] = 1;
	return d;
}
