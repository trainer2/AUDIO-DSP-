#include "funciones.h"


//Un tono de risset es una sinusoide base y dos octavas más. El truco está en mantener constante la amplitud de la del medio e ir haciendo fade in-uoot de los otros.
//Al reiniciar el ciclo, tenes 4 tonos en realidad
ssignal SheperdTone(double freq){




}


/*, each tone consists of two sine waves with frequencies separated by octaves; the intensity of each is e.g. a raised cosine function 
of its separation in semitones from a peak frequency, which in the above example would be B4. According to Shepard,
 "(...) almost any smooth distribution that tapers off to subthreshold levels at low and high frequencies would have done as well
 as the cosine curve actually employed.*/
//Esta version usa dos tonos
ssignal SheperdTone2(){

	ssignal s(12*SAMPLING_RATE, 0);
	for (int i = 0; i < 12; ++i)
	{

	}


}



//TODO: Ondas cuyo upper cycle sea sinusuide y lower sea triangular ( o tri y sq, o etc y etc. fun genericas! con un stretch random en cada ciclo)