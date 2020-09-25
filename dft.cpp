#ifndef DFT
#define DFT

//La famosa DFT
// Asume señales discretas periodicas
// De un set de N puntos, obtengo la energia de N/2 +1 frecuencias. Los otros N/2-1 datos son las fases. La frec 0 es el DC offset y no lleva fase. Tampoco la frec N/2+1. N a N datos
#include "funciones.h"
bool print = false;
//TODO: PAtear la horrible window function a la clase spectrum



//Como la señal la asumo real, te doy los coeficientes reales.
//N es hasta que sample voy a analizar
spectrum dft(ssignal& x, double (*window)(int i, int size) = id, int N=0){
	if(print) cout <<"ANALIZANDO SENIAL CON DFT CLASICO" << endl << endl;
	if (N==0) N= x.size();
	double waveCt = TWOPI/N; //Quiero que la componten k dé k vueltas en los N samples
	spectrum out(N/2+1);
	for(int i = 0; i<N; i++) x[i] *= window(i,N);

    for(int f=0;  f < N/2+1;++f){
  	  for (int i = 0; i < N; ++i){ //Covarianza de cada frecuencia. Despues en el syntethise, dividimos por la cant de samples para que sea mas una correlacion. O asi lo entiendo
		out.cosines[f] += x[i] * cos(i*f*waveCt);		
		out.sines[f]   -= x[i] * sin(i*f*waveCt);

		}
		if(print) cout << '\r'<< f<<" / " <<N;
	}
	cout << endl;
	return out;
}



//IGNORO EL SR ACA Y LO INCLUYO AL MOMENTO DE OUTEAR EL GRAFICO
ssignal syntethise(spectrum S, double (*window)(int i, int size) = id){
	ssignal& cosines = S.cosines; //renombre
	ssignal& sines = S.sines;
	int N = 2*cosines.size() - 2; //tamaño de la señal a reconstruir
	ssignal out(N, 0); 	//creamos la señal de salida
	double waveCt = TWOPI/N;

	//Rescale spectrum factors
	//Lo que yo tengo es densidad espectral. Distribuyendo esto en el tamaño de cada banda de espectro, obtengo la amplitud deseada
	//Cada banda mide 2/N *(SR/2) menos las de los bordes, que miden la mitad. sines[0] y sines[N/2] valen 0.
	for (int i = 1; i < cosines.size()-1; ++i){
		cosines[i] /= N/2;
		sines[i] /= -1*N/2;
	}
	cosines[0] /= N;
	cosines[cosines.size()-1] /= N;

	for (int i = 0; i < N; ++i){
	 	for (int k = 1; k < cosines.size() ; ++k) 
			out[i] += cosines[k]*cos(i*k*waveCt) + sines[k]*sin(i*k*waveCt);
		out[i] *= window(i, N);
		if(print) cout << '\r'<< i<<" / " <<N;
	}
		
	return out;

}






//for more accuracy, split into little regions and dft on them. Then, for resynthesis, if hop != block_size, it is summed on overlapping points.
// Todos los bloques tienen el mismo tamaño. Se recortan los ultimos de la señal
//ANalizo hasta el sample N
vector<spectrum> dftBlocks(const ssignal& x, int block_size=512, double (*window)(int i, int size) = id, int hop =0, int N =0){
	cout << "Calculando dft_blocks..." <<endl;

	if (hop == 0) hop = block_size;
	if (N==0) N = x.size();
	vector<spectrum> out;
	if(x.size()<block_size) {hop = x.size()-1; block_size=x.size()-1;}

	ssignal s = ssignal(block_size);
	for (int k = 0; k +block_size < N; k+=hop)
	{
		for (int i = 0; i < block_size; ++i) s[i] = x[i+k];
		spectrum S = dft(s, window);
		S.init_sample = k;
		S.block_size = block_size;
		out.push_back(S);
	}
	return out;
}



ssignal syntethise(vector<spectrum> S, double (*window)(int i, int size) = id){

	//Creo la señal de salida, para lo cual averiguo su tamaño. Todos los espectros de entrada miden lo mismo
	int N = S[S.size()-1].init_sample + S[S.size()-1].block_size; //es eso lol
	ssignal out = ssignal(N, 0);

	//Voy sintetizando y promediando cuando hay solapamiento
	int block_size = S[0].block_size;

	for (int i = 0; i < S.size(); ++i)
	{
		ssignal synth = syntethise(S[i], window);
		for(int k=S[i].init_sample; k<min(S[i].init_sample+block_size, N); k++) out[k] += synth[k-S[i].init_sample];
	}

	return out;
}

//Divide amplitud a amplitud. Asumo que ambos dft fueron hechos con los mismos parámetros
spectrum operator/(const spectrum& s1, const spectrum& s2){
	cout << "Dividiendo las señales..."<<endl;
	spectrum out = s1;
	int N = s1.cosines.size();
	for (int i = 1; i < N-1; ++i)
	{	
		if(s2.cosines[i]!= 0) out.cosines[i] /= s2.cosines[i];
		if(s2.sines[i]  != 0) out.sines[i] /= s2.sines[i];
	}
	out.cosines[0] /= s2.cosines[0]; out.cosines[N-1] /= s2.cosines[N-1]; //Evito dos divisiones por cero seguras
	return out;
}
//Divide amplitud a amplitud. Asumo que ambos dft fueron hechos con los mismos parámetros
vector<spectrum> operator/(const vector<spectrum>& s1, const vector<spectrum>& s2){
	vector<spectrum> out;
	assert(s1.size()==s2.size());
	for (int i = 0; i < s1.size(); ++i){ cout<<"Dividiendo el bloque " <<i <<endl; out.push_back(s1[i]/s2[i]);}
	return out;
}



//Audio Programming 8.2.1 - Freq & Amp Cross Synth
// s1 me da los modulos y s2 las fases y eso se multiplica
// NO SALIO MUY BIEN
ssignal FAxsynth(ssignal& s1, ssignal& s2, int block_size = 2048, int hop = 2048){


	cout <<"X synthing...\n";
	int N = min(s1.size(), s2.size());
	cout <<"Analizing first...\n";
	vector<spectrum> S1 = dftBlocks(s1,block_size,Hann, hop,N); //Tomo de a 2048 samples = 50ms
	cout << "Analizing second...\n";
	vector<spectrum> S2 = dftBlocks(s2,block_size,Hann, hop,N);
	vector<spectrum> New;
	cout <<"Doing...\n";
	//Reemplazo cada bloque
	for (int i = 0; i < S1.size(); ++i)
	{
		//La cant de frecuencias de cada analisis va a ser 2048/2 + 1
		int analisis_size = block_size/2+1;
		spectrum modified = spectrum(analisis_size);		
		for (int k = 0; k < analisis_size; ++k)
			{
			double Magnitude  = sqrt(pow(S1[i].cosines[k],2) + pow(S1[i].sines[k],2));
			Magnitude =sqrt(Magnitude);
			double phase;
			if(S2[i].cosines[k] ==0) phase = PI/2;
			else phase =atan2(S2[i].sines[k], S2[i].cosines[k]);

			modified.cosines[k]  = Magnitude*cos(phase);
			modified.sines[k]    = Magnitude*sin(phase);
			modified.init_sample = i*hop;
			modified.block_size  = block_size;
		}
		New.push_back(modified);
	}
	cout << "synthehising...\n";
	ssignal out = syntethise(New);
	return out;
}






int main2(){
	ssignal s1 = readWav("carriers/speech.wav");
	ssignal s2 = readWav("carriers/wnoise.wav");
	truncate(s1, 44100*2.5);
	truncate(s2, 44100*2.5);

	ssignal out1 = FAxsynth(s1, s2);
	
	ssignal out2 = FAxsynth(s2, s1);

	guardarWavRapido(out1, "xsynth1");
	guardarWavRapido(out2, "xsynth2");
	return 0;
}

#endif