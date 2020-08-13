#ifndef CONV
#define CONV
//FUENTE: CAP 6 y 7 de DSP  Steven W. Smith, Ph.D.
//Falta: Mas detalle sobre linearidad de fase, causalidad, dft... que viene en los cap siguientes
//No me quedo NADA claro porque convolucionar una señal consigo misma muchas veces tiende a una gaussiana. EDIT: Ahora si. Viene del hecho de que podes ver a la señal como una funcion de distribucion de masa/prob. La convolucion es la suma de esas funciones de distribucion/densidad. La suma, TCL.

//So: Convolucion es esto: tengo una señal S y le hago convolucion con T.
// si T es (1,0,0,0) se comporta como la identidad. Si es (2,0,0,0) se comporta como 2*S. Si es (7,0,0) se comporta como 7*S
// si T es (0,1,0,0) se comporta como (0,S) (delay 1 sample). Si es (0,0,1) se comporta como (0,0,S) (delay de 2 Samples)
// si T es (0,1,2,0) se comporta como (0,S) + (0,0, 2*S). And so on.
// O sea: Es la suma de delays escalados de la señal

#include "funciones.h"
#include "dft.cpp"
ssignal convolution1(const ssignal& s1, const ssignal& s2){
	cout << "Empezando convolucion de las dos señales"<<endl;
	cout <<s1.size()<<'\n';
	ssignal s3 = ssignal(s1.size()+s2.size()-1,0);
	for (int i = 0; i < s1.size(); ++i)	{

	  for (int j = 0; j <s2.size() ; ++j)
	  		s3[i+j] += s1[i]*s2[j];

			//cout << '\r'<< i<<" / " <<s1.size();
	}
	 return s3;
}

//TODO: PADDING DE S1 (ponerle M ceros a izq y der)
ssignal convolution2(ssignal s1, const ssignal& s2){

	ssignal s3 = ssignal(s1.size()+s2.size()-1,0);
	int M = min(s1.size(), s2.size());
	for (int k = 0; k < s3.size(); ++k)
		for(int m=0; m<M; ++m) 
			s3[k] += s2[m]*s1[k-m];
	return s3;
}


/* 												DECONVOLUCION 										*/


//NO ANDA

//La mas basica posible. Mono
// Pienso asi: Tengo mi input 1, que tipicamente es un sine-sweep digital
// Tengo mi input 2, que es el mismo sweep pero "wet" con el room
// El out es idealmente el IR del room
ssignal deconvolution( const ssignal& dry, const ssignal& wet, int block_size=2048,double (*window)(int i, int size) = id ){
		cout << "Empezando deconvolution" <<endl;
		int N = min(dry.size(), wet.size());
		ssignal Y = wet;
		ssignal X = dry;
		truncate(Y, N);
		truncate(X, N);
		//Asumo    wet  = dry (conv) IR_room
	 	//			    = dft(dry) * dft(IR)

	    // 	   dft(wet)/dft(dry) = dft(IR)
	    // 	  synth( dft / dft ) = IR
		cout << "Calculando dft de las señales de entrada..." <<endl;
	    vector<spectrum> dry_dft = dftBlocks(X, block_size, window,0,N); 
	    vector<spectrum> wet_dft = dftBlocks(Y, block_size, window,0,N); 
		cout << "Sintetizando la señal de salida..." <<endl;
		ssignal out = syntethise(wet_dft/dry_dft); 
		cout << "Deconvolution listo" <<endl;
	    return out;
}

/*In the specific case of estimating the impulse response of a room with sine
sweeps, Y will be the recording of the room, X will be the excitation sweep
and W the estimated impulse response in frequency domain. The effect of the
parameter λ is as follows: if it is too small, the regularization will be not enough
and the impulse response will be noisy; on the other hand, if λ is too big the
deconvolution will not be effective and the impulse response will still contain
components of Y:*/

ssignal Wiener_deconv( const ssignal& X2, const ssignal& Y2,double (*window)(int i, int size) = id ){

			cout << "Empezando deconvolution" <<endl;

	int N = min(X2.size(), Y2.size());
	ssignal Y = Y2;
	ssignal X = X2;
	truncate(Y, N);
	truncate(X, N);
	double lambda = 0.005;

			cout << "Calculando dft de las señales de entrada..." <<endl;

	spectrum X_dft = dft(X, window,N);
	spectrum Y_dft = dft(Y,window,N);
	
	vector<complex> dry;
	vector<complex> wet;
	for (int i = 0; i < N/2; ++i)
	{
		dry.push_back(complex(X_dft.cosines[i], X_dft.sines[i]));
		wet.push_back(complex(Y_dft.cosines[i], Y_dft.sines[i]));
	}


	double max = 0;  
	  for (int j = 0; j < N/2; ++j)
			if(max < abs(dry[j])) max = abs(dry[j]);

	double sigma = max*lambda;
	vector<complex> YxCX;
	for (int i = 0; i < N/2; ++i)
	{
		conj(dry[i]);
		YxCX.push_back(wet[i]*dry[i]);
	}
	sigma *= sigma;
	for (int i = 0; i < N/2; ++i)
	{
		double d =  ( pow(abs(dry[i]), 2) + sigma);
		YxCX[i] = YxCX[i] / d;
	}

	spectrum W(N/2);
	for (int i = 0; i < N/2; ++i){
	 W.cosines[i]=(YxCX[i].re);
	 W.sines[i]=YxCX[i].im;
	}
		cout << "Sintetizando la señal de salida..." <<endl;

	ssignal syn = syntethise(W);
	//truncate(syn, ) esta linea no la entiendo bien para que seria
	return syn;
}

/* 														EXPLICACION 	DE CONVOLUCION								*/ 


//Esa es la forma de ver la convolucion desde los inputs. Para pensarlo desde el output (o sea, "que valor va a tener el 5to sample del output?") lo hago asi:
// Doy vuelta T (T'[i]=T[T.size()-i]), y hago producto interno de T' con alguna porcion de S. Ponele que S es mas largo que T. Entonces si quiero el 8vo output, tengo que alinear
// T' para que quede el 8vo output de S como ultimo sample. Para el sample 0 ,por ejemplo, va a quedar todo T' afuera (multiplicado por 0) salvo el ultimo sample, que se multiplica con el S[0]

//Ya que estamos inventando T.size() puntos para S tanto a derecha y a izquierda, podemos mirar con desconfianza los primeros y ultimos T.size() puntos de OUT.
//Out[0] y Out[Last] es cualqueir cosa. Out[1] y Out[Prelast] es cualquier cosa, pero apeniitas mas confiable. Recien Out[T] ya es fiable de verdad.

//Y aca va la posta: Cuando tenes un sistema lineal, el output es una funcion lineal del input. Y en general, se asume "Shift Invariance"
//Claro, solo con linearidad, ya podria alcanzar con calcular f en los canonicos, para conocer f en cualquier input.
//Pero con el shift invariance... Solo necesito conocer f en el primer canonico! A este primer canonico se lo llama "funcion delta".
//Ya esta: f((1,0,0,0,0)) me da z ponele. f(0,1,0,0,0,0) me da (0,z) y asi. Entonces f((3,2,-5)) me da 3z+ 2(0,z) - 5(0,0,z). 
//Si ya tenemos z = f(delta) y queremos hallar cuanto vale f(S), podemos usar eeeeeell... algoritmo de arriba!! La convolucion S x z es EXACTAMENTE esto: hallar f(S) conociendo Z, y que f es lineal y shift invariante

//Es por eso que, al segundo operando de la convolucion, se lo suele llamar "impulse response"... o sea la imagen del sistema en el impulso unitario.
//Y ahi esta. Esa es la posta de la convolucion

//Algo que hay que notar es que en algebra lineal necesito n2 datos (una matriz) para clcular la imagen de cualquier transf. Si pido la propiedad de shift invariance, solo necesito n. Claro que estoy siendo super laxo con las dimensiones, porque nunca se termina de aclarar la longitud especifica de la señal digital.


//Cosas que no son tan obvias: La inversa de (1,-1,0,0,0,0) (diferencias consecutivas) es (1,1,1,1,1,1.... infinitos) (running sum)
//Pero el running sum es mas efectivo cuando esta hecho recursivo


//Los filtros son sistemas lineales, en general, so todo esto sirve. A la señal respuesta de impulso de un filtro se lo llama kernel.

//The cutoff frequency of the filter is changed by making filter kernel wider or narrower.
//EL HIGH pass es la identidad (delta) - Low Pass









/***********************************  TESTS AS FILTER *********************/
						

int main2(int argc, char const *argv[])
{	
	return 0;
}

//Beautiful! pero tarda UNA ETERNIDAD
void CAE1(){

	ssignal elephant = readWav("carriers/elephant.wav");
	ssignal whale = readWav("carriers/Whale Call.wav");
	ssignal out = convolution1(elephant, whale);
	guardarWavRapido(out, "elephant whale conv");

}


/*				ANDA!!    */
void test1(){

	//Probando que tan bien anda cada filtro. En teoria LPEXP es generico, SQ mantiene los bordes puntiagudos y reduce ruido (?) y sinc separa una banda de frecuencias
	//Impulse response / kernel de algunos filtros
	ssignal LPEXP = ssignal(15,0.2); for (int i = 1; i < 15; ++i) LPEXP[i]*=0.793; 		//Funciona porque la transformada de una exponencial decreciente es otra exponencial decreciente
	ssignal LPSQ = ssignal(7,0.1);  
	ssignal LPSINC = ssignal(20,0); for (int i = 0; i < 20; ++i) LPSINC[i] = sin(i*TWOPI/44100)/i*TWOPI/44100;

	//HP: TODO. Es la delta - los de arriba
	//TODO: Indices negativos para convolution2
	ssignal noise = readWav("carriers/wnoise.wav");
	ssignal sq    = readWav("carriers/sq3.wav");

	ssignal outexpnoise = convolution1(noise, LPEXP);
	ssignal outexpsq = convolution1(sq, LPEXP);
	guardarWavMono(outexpnoise, 16, SAMPLING_RATE, "LPEXPNOISE");
	guardarWavMono(outexpsq, 16, SAMPLING_RATE, "LPEXPSQ");

	ssignal outnoisesq = convolution1(noise, LPSQ);
	ssignal outsqsq = convolution1(sq, LPSQ);
	guardarWavMono(outnoisesq, 16, SAMPLING_RATE, "LPSQNOISE");
	guardarWavMono(outsqsq, 16, SAMPLING_RATE, "LPSQSQ");

	ssignal outsincnoise = convolution1(noise, LPSINC);
	ssignal outsincsq = convolution1(sq, LPSINC);
	guardarWavMono(outsincnoise, 16, SAMPLING_RATE, "LPSINCNOISE");
	guardarWavMono(outsincsq, 16, SAMPLING_RATE, "LPSINCSQ");

}
#endif