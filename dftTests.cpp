#include "dft.cpp"

void outPowerSpectrumBlock(vector<spectrum>& S, string path){
	for (int i = 0; i < S.size(); ++i)
		S[i].outPowerSpectrum((path+to_string(i)+".txt").c_str());
}
void showHanning(){ //OK!!
	ssignal s(1024);
	for (int i = 0; i < 1024; ++i) s[i] =Hann(i,1024);
	guardarWavRapido(s, "hanning");
}

void seriesTest();

//RESULTADO: SIGUE FALLANDO. No se cual sera el error realmente
int main(int argc, char const *argv[])
{
	seriesTest();
	return 0;
}



void variousToneTests(){

	ssignal s440short = puretone(440, 100);
	ssignal s440long  = puretone(440, 4096);

	//BLOCK PROCESSING TEST SHORT
	vector<spectrum> blockidnohop  = dftBlocks(s440short,512, id,512);
	vector<spectrum> blockidsihop  = dftBlocks(s440short,512, id,128);
	vector<spectrum> blockhannohop = dftBlocks(s440short,512, Hann,512);
	vector<spectrum> blockhannsihop = dftBlocks(s440short,512, Hann,128);

	ssignal sin1 =syntethise(blockidnohop);
	ssignal sin2 =syntethise(blockidsihop);
	ssignal sin3 =syntethise(blockhannohop, Hann);
	ssignal sin4 =syntethise(blockhannsihop,Hann);

	guardarWavRapido(sin1, "analysis window results/sin1"); //Perfecto
	guardarWavRapido(sin2, "analysis window results/sin2"); //Perfecto
	guardarWavRapido(sin3, "analysis window results/sin3"); //algo paso, pero suena bien
	guardarWavRapido(sin4, "analysis window results/sin4"); //algopaso, pero suena bien




	//BLOCK PROCESSING TEST LONG
	vector<spectrum> longidnohop = dftBlocks(s440long,512, id,512);
	vector<spectrum> longidsihop = dftBlocks(s440long,512, id,128);
	vector<spectrum> longhannohop = dftBlocks(s440long,512, Hann,512);
	vector<spectrum> longhannsihop = dftBlocks(s440long,512, Hann,128);

	ssignal sin5 =syntethise(longidnohop);
	ssignal sin6 =syntethise(longidsihop);
	ssignal sin7 =syntethise(longhannohop, Hann);
	ssignal sin8 =syntethise(longhannsihop,Hann);

	guardarWavRapido(sin5, "analysis window results/sin5"); //Casi perfect
	guardarWavRapido(sin6, "analysis window results/sin6"); //Casi bien
	guardarWavRapido(sin7, "analysis window results/sin7"); //NO
	guardarWavRapido(sin8, "analysis window results/sin8"); //NO

	outPowerSpectrumBlock(longhannsihop, "analysis window results/analysis");
}

void celloTest(){
	//Masomenos safa... Pero no se escucha natural, sino roto. Pero el efecto macabro quedó
	ssignal cello 	  = readWav("carriers/cello.wav");
	vector<spectrum> celloblocks = dftBlocks(cello, 1024,Hann, 512);
	ssignal cellosin = syntethise(celloblocks);
	guardarWavRapido(cellosin, "analysis window results/cello");
}

void seriesTest(){
	//MODULAR
	ssignal modular = readWav("carriers/modular.wav");
	vector<spectrum> modblocks = dftBlocks(modular, 1024,Hann, 512);
	ssignal modsin  = syntethise(modblocks);
	guardarWavRapido(modsin, "analysis window results/modular");
	
	//ORGAN
	ssignal organ = readWav("carriers/organ.wav");
	vector<spectrum> organblocks = dftBlocks(organ, 1024,Hann, 512);
	ssignal organsin = syntethise(organblocks);
	guardarWavRapido(organsin, "analysis window results/organ");
	
	//PAD
	ssignal pad = readWav("carriers/pad.wav");
	vector<spectrum> padblocks = dftBlocks(pad, 1024,Hann, 512);
	ssignal padsin  = syntethise(padblocks);
	guardarWavRapido(padsin, "analysis window results/pad");

}


void simpleToneHanningTest(){

	/*Resultado: Hanning Windoe OK para tonos simples */

	//HANNING WINDOW TEST
	ssignal s440short = puretone(440, 100);
	ssignal s440long  = puretone(440, 4096);

	spectrum shortid  = dft(s440short);
	spectrum shorthan = dft(s440short, Hann);
	spectrum longid   = dft(s440short);
	spectrum longhan  = dft(s440long);

	shortid.outPowerSpectrum("analysis window results/shortid");
	shorthan.outPowerSpectrum("analysis window results/shorthan");
	longid.outPowerSpectrum("analysis window results/longid");
	longhan.outPowerSpectrum("analysis window results/longhan");




}








/**************************************************************

				 TESTS		-- ANDUVIERON!

***************************************************************/
void analysisTest();
void analysisTest2();
void synthesisTest();
void identityTest();

//TODO: HACER TESTS SOBRE EL ZERO PADDING. Los de analysis2 sobre todo
//TODO: HACER TESTS DONDE PRECALCULO LAS FRECUENCIAS RESULTANTES Y LEMANDO UNA SEÑAL ARMONICA SOBRE ESAS FRECUENCIASs


void synthesisTest(){
	//PRUEBA DE SYNTEHSIS:
	spectrum d1(17);
	d1.cosines = vector<double>(17,1);
	d1.sines = vector<double>(17,0);
	ssignal d = syntethise(d1);
	guardarWavRapido(d, "impulse test" );
}

void analysisTest(){
	//RESULTADO: ANDA!! Ya para 50 samples, analiza SUPER BIEN!.
	//Sin embargo, para Super Long las dos frecuencias siguen estando muy juntas. El ancho de banda es de 44hz, por lo que las dos caen en la misma banda
	//Con HIRES, tomando un MONTON de samples... ahi si anda! Muestra dos picos. Aun estan muy cercanos, pero se alcanza a distsinguir.


	//Declaro las señales que voy a usaar
	ssignal Sin4410short= ssignal(5);
	ssignal Sin4410med  = ssignal(15);
	ssignal Sin4410long = ssignal(50);
	ssignal Sin4410VeryLong = ssignal(1000);
	ssignal Sin4410EXTREME  = ssignal(4096);

	ssignal Sin4420short= ssignal(5);
	ssignal Sin4420med  = ssignal(15);
	ssignal Sin4420long = ssignal(50);
	ssignal Sin4420VeryLong = ssignal(1000);
	ssignal Sin4420EXTREME  = ssignal(4096);


	double sineCt4410 = sineCtCalc(4410, 44100);
	double sineCt4420 = sineCtCalc(4420, 44100);


	//Les cargo los datoos
	for(int i = 0; i< Sin4410short.size(); i++) Sin4410short[i] = sin(i*sineCt4410);
	for(int i = 0; i< Sin4410med.size(); i++)   Sin4410med[i]   = sin(i*sineCt4410);
	for(int i = 0; i< Sin4410long.size(); i++)  Sin4410long[i]  = sin(i*sineCt4410);		
	for(int i = 0; i< Sin4410VeryLong.size(); i++) Sin4410VeryLong[i] = sin(i*sineCt4410);
	for(int i = 0; i< Sin4410EXTREME.size(); i++) Sin4410EXTREME[i] = sin(i*sineCt4410);

	for(int i = 0; i< Sin4420short.size(); i++) Sin4420short[i] = sin(i*sineCt4420);
	for(int i = 0; i< Sin4420med.size(); i++)   Sin4420med[i]   = sin(i*sineCt4420);
	for(int i = 0; i< Sin4420long.size(); i++)  Sin4420long[i]  = sin(i*sineCt4420);		
	for(int i = 0; i< Sin4420VeryLong.size(); i++) Sin4420VeryLong[i] = sin(i*sineCt4420);
	for(int i = 0; i< Sin4420EXTREME.size(); i++) Sin4420EXTREME[i] = sin(i*sineCt4420);


	//Analisis de algo basico
	spectrum dft1 = dft(Sin4410short);
	spectrum dft2 = dft(Sin4410med);
	spectrum dft3 = dft(Sin4410long);
	spectrum dft4 = dft(Sin4410VeryLong);

	//Mando a archivos para graficar
	dft1.outPowerSpectrum("analysis results/sineShort.txt");
	dft2.outPowerSpectrum("analysis results/sineMed.txt");
	dft3.outPowerSpectrum("analysis results/sineLong.txt");
	dft4.outPowerSpectrum("analysis results/sineVeryLong.txt");

	//Analisis de resolucion
	spectrum dftshort = dft(Sin4410short + Sin4420short);
	spectrum dftmed = dft(Sin4410med + Sin4420med);
	spectrum dftlong =dft(Sin4410long + Sin4420long);
	spectrum dftverylong= dft(Sin4410VeryLong + Sin4420VeryLong);
	spectrum dftHIRES = dft(Sin4410EXTREME + Sin4420EXTREME);

	//Espectro de la suma de las señales
	dft1.outPowerSpectrum("analysis results/sumsineShort.txt");
	dft2.outPowerSpectrum("analysis results/sumsineMed.txt");
	dft3.outPowerSpectrum("analysis results/sumsineLong.txt");
	dft4.outPowerSpectrum("analysis results/sumsineVeryLong.txt");
	dftHIRES.outPowerSpectrum("analysis results/HIRES.txt");

/*
	//Para verificar lo que estoy generando
	guardarWavRapido(Sin4410short, "short");
	guardarWavRapido(Sin4410med, "med");
	guardarWavRapido(Sin4410long, "long");
	guardarWavRapido(Sin4410VeryLong, "vLong");*/
}

void analysisTest2(){
	//En este experimento, voy a probar si es importante el espaciado en la escala lineal o logaritmica para los picos.
	//Si me preguntan antes, estoy muy seguro de que es en la escala lineal. Pero vamos a verificarlo. Porque si agrego zero padding, cambia la cosa
	// La idea es que en un segundo pasaron muchisimas diferencias de 4410hz con 4420hz (una hizo 10 ciclos mas), la misma que 0.1hz con 0.11hz en 100 segundos
	// Pero esa idea es erronea en este experimento, porque el espaciado no es el mismo. Pero si la propocion entre blabla

	//ALTA FRECUENCIAS CERCANAS
	ssignal Sin4410EXTREME  = ssignal(6000);
	ssignal Sin4420EXTREME  = ssignal(6000);
	double sineCt4410 = sineCtCalc(4410, 44100);
	double sineCt4420 = sineCtCalc(4420, 44100);
	for(int i = 0; i< Sin4410EXTREME.size(); i++) Sin4410EXTREME[i] = sin(i*sineCt4410);
	for(int i = 0; i< Sin4420EXTREME.size(); i++) Sin4420EXTREME[i] = sin(i*sineCt4420);

	//BAJAS FRECUENCIAS CERCANAS
	ssignal Sin4EXTREME  = ssignal(6000);
	ssignal Sin14EXTREME  = ssignal(6000);
	double sineCt4 = sineCtCalc(4, 44100);
	double sineCt14 = sineCtCalc(14, 44100);
	for(int i = 0; i< Sin4EXTREME.size() ; i++) Sin4EXTREME[i]  = sin(i*sineCt4);
	for(int i = 0; i< Sin14EXTREME.size(); i++) Sin14EXTREME[i] = sin(i*sineCt14);

	//suma
	ssignal highs = Sin4410EXTREME + Sin4420EXTREME;
	ssignal lows  =  Sin4EXTREME + Sin14EXTREME;

	//Analisis y grafico
	spectrum dftHIRES = dft( highs + lows);
	dftHIRES.outPowerSpectrum("analysis results/LETSSEE.txt");
}

//Ve si la sintetisis del analisis es igual a la señal de salida
void identityTest(){


	//Creo y lleno Sinusoide y cuadrada
	ssignal Sin440 = ssignal(512);
	ssignal Sq     = ssignal(512);

	double sinCt = sineCtCalc(440, 44100);
	for(int i=0; i<512; i++) Sin440[i] = sin(sinCt*i) + sin(sinCt*i + 0.4);
	int i =0;
	double sig =1;
	while(i<512){
		Sq[i] = sig ;
		if(i++%5 ==0) sig=-1*sig;
	} 

	//Analizo
	spectrum sindft = dft(Sin440);
	spectrum sqdft  = dft(Sq);

	//Resintesis y comparo
	ssignal sinsint = syntethise(sindft);
	ssignal sqsint  = syntethise(sqdft);

	Sin440 - sinsint;
	Sq - sqsint;
	guardarWavRapido(Sin440, "sinresint");
	guardarWavRapido(Sq, "sqresint");

}