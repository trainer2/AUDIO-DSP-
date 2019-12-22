#ifndef TRANSFER
#define TRANSFER
//CURVAS DE TRANSFERENCIA - MUY Facilmente trasladable a PD
#include "funciones.h"

//Hay que normalizar el dominio e imagen de las funciones a (-1,1)


void transfer(vector<double>& buffer, double f(double, double, double), double freq =0.5, double phase =0, uint from =0, uint to =0){
	if(to ==0) to = buffer.size();
	for(uint i =from; i< to; i++) buffer[i] = f(buffer[i], freq, phase);

}


double seno(double x, double freq, double phase){ return sin(x * 2*PI * freq + phase);}
double cube(double x, double dummy1, double dummy2){return x*x*x;}
double centeredSquare(double x, double dummy1, double dummy2){return 2*x*x - 0.5;}

//TODO: TRIANGLE


int main(int argc, char const *argv[])
{
	vector<double> buffer1 = readWav("envelopeSamples/vln.wav");
	vector<double> buffer2 = readWav("envelopeSamples/vln.wav");
	vector<double> buffer3= readWav("envelopeSamples/vln.wav");
	vector<double> buffer4 = readWav("envelopeSamples/vln.wav");
	vector<double> buffer5 = readWav("envelopeSamples/vln.wav");
	transfer(buffer1, seno);
	transfer(buffer2, cube);
	transfer(buffer3, centeredSquare);

	transfer(buffer4, cube);
	transfer(buffer4, centeredSquare);

	transfer(buffer5, centeredSquare);
	transfer(buffer5, cube);

	guardarWavMono(buffer1 ,16, 44100, "sinT" );
	guardarWavMono(buffer2 ,16, 44100, "cubeT");
	guardarWavMono(buffer3 ,16, 44100, "sqT");
	guardarWavMono(buffer4 ,16, 44100, "cubeSqT");
	guardarWavMono(buffer5 ,16, 44100, "SqCubeT");

	return 0;
}


#endif