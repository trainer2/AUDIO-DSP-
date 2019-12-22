#ifndef INTERPOL
#define INTERPOL

#include "funciones.h"

using namespace std; 


//La interpoladora parametro debiera tomar los puntos extremos reescalados en tiempo a indices, un vector,
// y llenar el vector con los valores entre esos indices 
vector<double> interpolate(vector<breakpoint> brpts, void f(breakpoint, breakpoint, double, vector<double>&, double, double), double srate = 44100, double cycles = 1){

	//1- Creamos el vector que vamos a devolver
	double lastPointTime = brpts[ brpts.size()-1].x;
	vector<double> res( lastPointTime *srate +1 , 0);

	//2-Llenamos los puntos que ya conocemos
	for(int t =0; t<brpts.size(); t++) res[brpts[t].x*srate] = brpts[t].y;

	//3- Interpolamos entre cada par de muestras 
	for(int x1 =0; x1<brpts.size()-1; x1++){

		breakpoint inicial = brpts[x1];
		breakpoint final   = brpts[x1+1];
		double dx = final.x - inicial.x;
		dx *= srate; //indico en cantidad de samples
		f(inicial, final, dx, res, srate, cycles);
	}

	return res;

}

void interpoladoraLineal(breakpoint p1, breakpoint p2, double dx, vector<double>& res, double srate, double dummy){

	double pendiente = (p2.y - p1.y)/dx;
	uint posInicial = floor(p1.x * srate);
	/*b1 = p1.y -p1.x *pendiente;
	b2 = p2.y -p2.x *pendiente;
	b= (b1+b2)/2; //Saco el promedio de los dos valores analiticamente iguales*/

	for(int i =posInicial; i<p2.x*srate; i++) res[i] = p1.y + (i-posInicial)*pendiente;
	return;
}

void interpoladoraExponencial(breakpoint p1, breakpoint p2, double dx, vector<double>& res, double srate, double dummy){
	double x1 = p1.x;
	double x2 = p2.x;
	double y1 = p1.y;
	double y2 = p2.y;
	x1 *= srate;
	x2*= srate;
	/*VIEJO : En los pequeños rangos numericos que tengo, es indisinguible de una lineal
	//La funcion es e**(k(x-x1) + y1 -1
	double k = log(y2-y1 +1) / dx;
	for(int i = x1+1; i< x2; i++) 
		res[i] = exp(k* (i-x1)) + y1 -1 ;*/

	//TODO: Calcularlo eficiente haciendo que en cada paso solo se multiplique el sample anterior
	if (y1==0) y1 += numeric_limits<double>::epsilon();
	if (y2==0) y2 += numeric_limits<double>::epsilon();
	double base= y2/y1;
	for(int i = x1+1; i< x2; i++) res[i] = y1 * pow(base, (i-x1)/dx);

}


//TODO: Por ahora ignora completamente la altura final, que la deja fija en la misma altura inicial
//Para que ande como envolvente, la cant de cyclos debiera ser menor a 0.5
void interpoladoraSenoidal(breakpoint p1, breakpoint p2, double dx, vector<double>& res, double srate, double cycles = 1){

	//Quiero que de x1 a x2 entren cycles cyclos de senoidal
	//sineCt
	double dt = p2.x - p1.x;
	double freq = cycles/dt;
	double phase = asin(p1.y);
	double sineCt = sineCtCalc(freq, srate);
	uint x1 = (uint) floor(p1.x*srate);
	uint x2 = (uint) floor(p2.x*srate);
	writeSineSamples(x1, x2, res, sineCt, phase);
}

//TODO: INTERPOLACION SPLINES Y PARABOLAS PARAMETRIZABLES


//ADSR Pero el tiempo de sustain está fijo
vector<breakpoint> ADSRTPoints(double att, double dec, double sustime, double sustain, double rel ){

	vector<breakpoint> adsrt;
	adsrt.push_back(breakpoint(att, 1));
	adsrt.push_back(breakpoint(att+dec, sustain));
	adsrt.push_back(breakpoint(att+dec+sustime, sustain));
	adsrt.push_back(breakpoint(att+dec+sustime+rel,0));
	return adsrt;
}





int tempMain(int argc, char const *argv[])
{
	/* EJEMPLO*/
	vector<breakpoint> B;
	B.push_back(breakpoint(0,0));
	B.push_back(breakpoint(1,0.5));
	B.push_back(breakpoint(3,0));

	vector<double> TEST= interpolate(B, interpoladoraSenoidal,44100,2); //el utlimo parametro es "cantidad de ciclos" usado para interpolacion senoidal
	vector<vector<double>> buffer; 
	buffer.push_back(TEST);
	
	guardarWav(buffer, 1,16,44100, string("Senoidal2"));

	return 0;
}

#endif
