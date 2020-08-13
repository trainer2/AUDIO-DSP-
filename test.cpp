

//Idea: Si la transformada de Step(x) es sinc(x) y viceversa... Si le agrego -sinc(x) a ruido blanco, deberia cancelar una parte del espectro no?? Pruebo!!

#include "funciones.h"
double sinc(double x){ if(x==0) return 1; else return sin(x)/x;}

int main(int argc, char const *argv[])
{
	ssignal noise = readWav("carriers/wnoise.wav");
	/* RESUTLADO: NO ANDUVO

	//Quiero un espectro ancho para ver el espectro. Esto corresponde a comprimir el sinc(x). o sea, tomar sinc(x/a) con a >1.
	// Tambien quiero que no este centrado en 0... Lamentablemente no se como hacerlo, pero espero que si a es muy grande, voy a cubrir bastantes frecuencias audibles.
	ssignal Sinc = ssignal(noise.size(), 0);
	double a = 100;
	for (int x = 0; x < noise.size(); ++x)
		Sinc[x] = sinc(x/a);

	guardarWavRapido(Sinc, "sincTest");
	sum(Sinc, noise);
	guardarWavRapido(Sinc, "sincandnoiseTest");
	*/

	//Otra prueba: Con Sinc centrada en la señal, y noise mas corto, de 1/2 segundo.
	ssignal wnoise = ssignal(22050);	
	ssignal Sinc   = ssignal(22050);
	//Lleno las señales
	for (int i = 0; i < 22050; ++i)
		wnoise[i] = noise[i];
	for (int i = 22050/2; i < 22050 ; ++i)
	{	double x = i - 22050/2;
		Sinc[i] = sinc(x/100);
	}	
	for (int i = 22050/2; i > 0 ; --i)
	{	double x = 22050/2-i;
		Sinc[i] = sinc(x/100);
	}

	guardarWavRapido(Sinc, "sincTest2");
	sum(Sinc, wnoise);
	guardarWavRapido(Sinc, "sincandnoiseTest2");
	return 0;
}