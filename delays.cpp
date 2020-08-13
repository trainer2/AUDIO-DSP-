//C audio programming chapter 6
// Basicamente circular buffers y eso
// Increible lo que se puede hacer con esto


#include "funciones.h"

//Feedfoward, easy delay
//El size con que se lo crea es el tiempo * Srate
struct delayLine{
	vector<double> samps;
	double read();
	void write(double x);
	int currentRead;
	int currentWrite;

	delayLine(int size){samps = vector<double>(size, 0); currentRead = 0; currentWrite=size-1;}
};


double delayLine::read(){
	++currentRead = (currentRead<samps.size() ? currentRead : 0);	
	return samps[currentRead];	//Yeah this doesnt go like this but im just testing the concept and not gonna make the code too wordy
}
void delayLine::write(double x){
	samps[currentWrite] = x;
	++currentWrite = (currentWrite<samps.size() ? currentWrite : 0);
}



//OJO: NO ES LA SEÑAL X + delay(X). ES UN "FILTRO" RECURSIVO, UN DELAY RECURSIVO. Pero no se como llamar al otro. Los dos son IIR.
//El primero es un comb filter
//OJO: LAS DOS OPCIONES DE ESTA FUN NO HACEN LO MISMO. La primera, y la que dice IIRecho si.
ssignal singleEcho( ssignal& x, double gain, double time, bool buffer_version = false ) {
//In-place version
	int delay_samples =  (double) SAMPLING_RATE * time;
	if (not buffer_version){
		for (int i = delay_samples; i < x.size(); ++i)
    	 x[i] += gain * x[i-delay_samples] ;
    	return x;
	}

//circular delay buffer version
	delayLine D(delay_samples+1);
	for (int i = 0; i < x.size(); ++i)
	{
		D.write(x[i]);
		x[i] += gain * D.read();
	}
	return x;
}



//INCREIBLE!!
ssignal IIRecho(ssignal& x, double gain, double time){
   delayLine D(time*SAMPLING_RATE);
   for (int i = 0; i < x.size(); ++i)
   {// Igual que el otro, pero con las acciones al reves
   		x[i] += gain*D.read();
  	 	D.write(x[i]);
   }
   return x;
}


/*				DEL LIBRO			*/
// x + delay(x) ?
ssignal combFilter(ssignal& x, double gain, double time){
	delayLine D(time*SAMPLING_RATE);
	for (int i = 0; i < x.size(); ++i)
	{	
		double out = D.read();
		x[i] = out;
		D.write(x[i] + gain*out);
	}
	return x;
}

ssignal allPass(ssignal& x, double gain, double time){
	delayLine D(time*SAMPLING_RATE);
	for (int i = 0; i < x.size(); ++i)
	{	
		double out = D.read();
		D.write(x[i] + gain*out);
		x[i] = out - gain*x[i];
	}	
	return x;
}
/*************************   TEST DEFINITIONS     *****************************************/



void singleEchoTest();
void singleEchoTest2();
void singleEchodothesame();
void increasingEcho();
void constantEcho();
void IIRechoTest();
void IIRvsSimple();


int main(int argc, char const *argv[])
{
	IIRvsSimple();

	return 0;
}




/*	************************  TESTS ********************				*/

void IIRechoTest(){
	SAMPLING_RATE = 11025;
	ssignal s1 = readWav("carriers/speech.wav");//Srate = 11025
	ssignal s2 = s1;
	IIRecho(s1, 0.5, 0.15);
	IIRecho(s2, 0.2, 0.07);
	normalize(s1);
	normalize(s2);
	guardarWavMono(s1, 16, 11025,"IIR 150ms");
	guardarWavMono(s1, 16, 11025,"IIR 70ms");

}

void singleEchoTest(){
	SAMPLING_RATE = 11025;
	ssignal s1 = readWav("carriers/speech.wav");//Srate = 11025
	ssignal s2 = s1;
	singleEcho(s1, 1, 0.15, true);
	singleEcho(s2, 1, 0.15, true);
	guardarWavMono(s1, 16, 11025,"echoline");
	guardarWavMono(s2, 16, 11025,"echoinplace");
}


//Interesante para electroacustica
void singleEchoTest2(){
	SAMPLING_RATE = 11025;
	ssignal s1 = readWav("carriers/speech.wav");//Srate = 11025
	singleEcho(s1, 1, 0.7, false);
	guardarWavMono(s1, 16, 11025,"inplaceTest");
}

void singleEchodothesame(){
	SAMPLING_RATE = 11025;
	ssignal s1 = readWav("carriers/speech.wav");//Srate = 11025
	ssignal s2 = s1;
	singleEcho(s1, 0.5, 0.15, true);
	singleEcho(s2, 0.5, 0.15, false);
	s1-s2;
	double maxsamp =0;
	for (int i =0; i<s1.size();i++) maxsamp = max( abs(s1[i]), maxsamp);
	cout << maxsamp; //This should be 0 or very close? 
}


void increasingEcho(){
	SAMPLING_RATE = 11025;
	ssignal s1 = readWav("carriers/speech.wav");//Srate = 11025
	ssignal s2 = s1;
	ssignal s3 = s1;
	singleEcho(s1, 1.8, 0.7, false);
	singleEcho(s2, 1.8, 0.7, true );
	IIRecho(s3,    1.8, 0.7);

	guardarWavMono(s1, 16, 11025,"inplace increasing");
	guardarWavMono(s2, 16, 11025,"buffer increasing");
	guardarWavMono(s3, 16, 11025,"IIR increasing");

}


void constantEcho(){
	SAMPLING_RATE = 11025;
	ssignal s1 = readWav("carriers/speech.wav");//Srate = 11025
	ssignal s2 = s1;
	ssignal s3 = s1;

	singleEcho(s1, 1, 0.4, false);
	singleEcho(s2, 1, 0.4, true );
	IIRecho(s3,    1, 0.4);

	guardarWavMono(s1, 16, 11025,"inplace constant");
	guardarWavMono(s2, 16, 11025,"buffer constant");
	guardarWavMono(s3, 16, 11025,"IIR constant");

}

void IIRvsSimple(){
	SAMPLING_RATE = 11025;
	ssignal s1 = readWav("carriers/speech.wav");//Srate = 11025
	ssignal s2 = s1;
	ssignal s3 = s1;
	ssignal s4 = s1;
	ssignal s5 = s1;
	ssignal s6 = s1;

	//pairs of two
	singleEcho(s1, 0.1, 0.6, false);
	singleEcho(s2, 0.1, 0.4, true );
	singleEcho(s3, 0.75, 0.7, false);
	singleEcho(s4, 0.75, 0.7, true );
	IIRecho(s5,    0.1, 0.6);
	IIRecho(s6,    0.75, 0.7);

	guardarWavMono(s1, 16, 11025,"inplace 100ms");
	guardarWavMono(s2, 16, 11025,"buffer 100ms");
	guardarWavMono(s5, 16, 11025,"IIR 100ms");
	guardarWavMono(s3, 16, 11025,"inplace 750ms");
	guardarWavMono(s4, 16, 11025,"buffer 750ms");
	guardarWavMono(s6, 16, 11025,"IIR 750ms");
}


