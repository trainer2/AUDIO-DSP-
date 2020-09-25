#ifndef FFT
#define FFT
#include <math.h>
#include "funciones.h"
#include "dft.cpp"
//Acá la fase la voy a medir en radianes normalizados, que creo que va a ser lo mejor para las implementaciones que quiero hacer.
//  r equivale a r'*pi radianes. Asi 0.5 es i, 1.5 es -i, 1 es -1, -1 es -1, 2 es 0, -0.5 es -i... las reglas algebraicas se mantienen todas.
/*struct PolarComplex{
	double mag;
	double phase;
	void girar(double p) {phase += p;}
	PolarComplex(double& x){ mag = abs(x); phase = x>0 ? 0 :-1;}
	PolarComplex(double& m, double& p){mag = m; phase = p;}
	PolarComplex& operator*(PolarComplex &c){c.mag*mag; c.phase+phase; return *this;}
	PolarComplex& operator+(double& x){assert((int) phase %2 ==0); mag +=x; return *this;}
};


typedef std::vector<PolarComplex> pcsignal;*/

/*
pcsignal to_complex(ssignal& s){
	pcsignal cs;
	for (int i = 0; i < s.size(); ++i) cs.push_back(PolarComplex(s[i]));
	return cs;
}*/

csignal dft2(ssignal& s){
	if (print) cout <<" DFT2" << endl<<endl;
	int N = s.size();
	if (N%2 !=0) {s.pop_back();N--;}
	csignal F = csignal(N, complex(0,0));

	for (int i = 0; i < N; ++i) F[0].re += s[i];
	for (int i = 0; i < N; i+=2) 
		F[N/2].re += s[i];
		for (int i = 1; i < N; i+=2)
		F[N/2].re -= s[i];

	double ct = -1*TWOPI/N;
	double ct_k= ct;
	for (int k = 1; k < N/2; ++k)
	{
		double ct_i = ct_k;
		 for (int i = 0; i < N; ++i)
		{
			F[k].re += cos(ct_i) * s[i];
			F[k].im += sin(ct_i) * s[i];
			ct_i += ct_k;
		}
		ct_k += ct;
   	if(print) cout << '\r'<< k<<" / " <<N;

	}
	for (int k = N/2+1; k < N; ++k) {F[k].re = F[k-N/2].re; F[k].im = -1*F[k-N/2].im;}

	cout << "Listo DFT2" << endl;
	return F;
}

csignal dft2(csignal& s ){
	if (print) cout <<"DFT2" << endl<<endl;
	int N = s.size();
	if (N%2 !=0) {s.pop_back();N--;}
	csignal F = csignal(N, complex(0,0));

	for (int i = 0; i < N; ++i) {F[0] += s[i];}
	for (int i = 0; i < N; i+=2) 
		{F[N/2] += s[i];
         F[N/2] -= s[i+1];}

	double ct = -1*TWOPI/N;
	double ct_k= ct;
	for (int k = 1; k < N; ++k)
	{
		if(k==N/2) continue;
		double ct_i = ct_k;
		 for (int i = 0; i < N; ++i)
		{
			complex c= complex(cos(ct_i), sin(ct_i)) * s[i];
			F[k] += c;
			ct_i += ct_k;
		}
		ct_k += ct;
	   	if(print) cout << '\r'<< k<<" / " <<N;
	}
	cout << "listo IDFT2" <<endl;
	return F;
}



//Prepara F para graficarla. LLeva a los modulos y pone el DC offset en el sample del medio para tener un espectro even
//Queda [-ultimo bin, - anteultimo bin .... -bin2, -bin1, 0, bin1, bin2, ..., ultimo bin,  Nyquist]
//Tengo N samples espaciados 1/SR. el lugar 0 es para la frecuencia SR/2, el N/2 para 0. Ponele 40k de sr y 10 samples. Entonces en 5 samples tengo que ir de 0 a 20k. Cada sample subo 4khz.
void to_graph(csignal& F, const char* path){
	int N = F.size();
	ssignal s = ssignal(N);
	for (int i = 0; i < N; ++i)
		s[i] = F[i].re*F[i].re + F[i].im*F[i].im;
	for (int i = 0; i < N/2; ++i)
		swap(s[i], s[N/2+i]);


	vector<breakpoint> graph;
	for (int i = 0; i < N; ++i)
	{	double freq = (i-N/2)*SAMPLING_RATE/N;
		graph.push_back(breakpoint(freq,s[i]));
	}
	///guardarWavRapido(s, to_string(char(rand()*255)) + to_string(char(rand()*255)));
	writeToFile(graph, path);
}

//Una implementacion basada en que todas las multiplicaciones que se hagan en el dominio de la frecuencia son con funciones pares puede hacerse mucho más rápido, usando las coordenadas polares de arriba y guardando en un solo item el resultado de la suma de la frecuencia positiva y negativa (las multiplicaciones con los fasores son simples sumas, y todas las sumas entre complejos se reducen a sumas reales). Esta asumcion no está justificada en el caso general (convolucion y otros efectos) pero si en el caso de filtros y puro analisis
//un dft de filtro, por ejemplo, aprovecha esto. Esto lo hago abajo. 
//Bueno, no se si "mucho" mas rapido. Una idea que no resultó
/*
//guarda las frequencias y las fases nada mas
csignal dft3(ssignal s){
	int N = s.size();
	csignal F = csignal(N, PolarComplex(0.0,0.0));

	for (int i = 0; i < N; ++i) F[0] += s[i];
	for (int i = 0; i < N; i+=2) F[N/2] += s[i] - s[i+1];

	return F;

}
*/

csignal freverse(csignal& s){ csignal S = s; for (int i = 1; i < S.size()/2; ++i) swap(S[i], S[S.size()-i]);return S;}
ssignal idft2(csignal S){
	csignal S2= freverse(S);
	csignal C = dft2(S2);
	ssignal s(S.size());
	for (int i = 0; i < S.size(); ++i) s.push_back(C[i].re/S.size());
	return s;
}


int fft_size;
csignal fft1_helper(csignal& s, int from, int step){
	if(step ==fft_size) {csignal cs; cs.push_back(s[from]); return cs;}
	int N = fft_size/step;
	csignal F1 = fft1_helper(s, from, step*2);
	csignal F2 = fft1_helper(s, from+1, step*2);
	csignal F = csignal(N, complex(0.0,0.0));

	csignal f2(N/2,complex(0.0,0.0));
	for (int i = 0; i < N/2; ++i) f2[i] = F2[i]*complex(cos(TWOPI*i/N), sin(TWOPI*i/N));
	for (int i = 0; i < N/2; ++i)
	{
									F[i] = F1[i] +f2[i];
									F[i+N/2] = F1[i] -f2[i];
	}
	return F;
	
}
csignal fft1(ssignal& s){
	if (print) cout <<"FFT" << endl;
	int N = 1;
	while (N<s.size()) N*=2;
	if(N!= s.size()) N>>1;
	csignal out = csignal(N);
	fft_size = N;
	csignal in; for(int i =0; i<N; i++) in.push_back(complex(s[i]));
	cout <<endl;
	return fft1_helper(in, 0, 1);
}
csignal fft1(csignal& s){
	int N = 1;
	while (N<s.size()) N*=2;
	if(N!= s.size()) N>>1;
	csignal out = csignal(N);
	fft_size = N;
	return fft1_helper(s, 0, 1);
}


csignal fft2(csignal& s){
	if (s.size() == 1) return s;
	int N = s.size();
	//cout << N << '\r';
	csignal s1;
	csignal s2;
	//cout << N;
	for (int i = 0; i < N; i++)
	{s1.push_back(s[i]);
	 s2.push_back(s[++i]);
	}
	 csignal F1 = fft2(s1);
	 csignal F2 = fft2(s2);
	 csignal F(N);
	 double ct = TWOPI/N;
	 for (int i = 0; i < N/2; ++i)
	 {
	 	complex c = F2[i]*complex(cos(ct*i), sin(ct*i));
	 	F[i] = F1[i] + c;
	 	F[i+N/2] = F1[i] -c;
	// 	cout << "Tamanio actual "  << N<<  "   "<<'\r';
	 }
	// cout << endl << "TAMAÑO DEL FFT " << F.size();
	 return F;
}
csignal fft2(ssignal &s){
	int N = 1;
	while (N<s.size()) N*=2;
	if(N!= s.size()) N/=2;
	cout <<"FFT TAMANIO " << N << endl;
	cout << "SEÑAL TAMANIO" << s.size();
	csignal c(N); for (int i = 0; i < N ; ++i) c[i] = complex(s[i]);
	cout << "GO";
	return fft2(c);
}

ssignal ifft2(csignal& s){
	csignal S = freverse(s);
	csignal F = fft2(S);
	ssignal out;
	for (int i = 0; i < S.size(); ++i) out.push_back(F[i].re/S.size());
	return out;
}

ssignal ifft1(csignal S){
	csignal s = freverse(S);
	ssignal out(S.size());
	csignal F = fft1(s);
	for (int i = 0; i < S.size(); ++i) out.push_back(F[i].re/S.size());
	return out;
}

/*
//fast fft (?) usa mi función "bit-reversal" en assembler
csignal ffft(float* s, int N ){
	pcsignal cs = to_complex(s);
	bit_reversal(cs,N);
	return cs;

}*/


/********************* POSTA  -EL QUE VOY A USAR PARA TESTEAR EL RESTO*****************************/


//Da vuelta los bits de x
uint32_t reverse(uint32_t x) 
{//https://stackoverflow.com/questions/9144800/c-reverse-bits-in-unsigned-integer
    x = ((x >> 1) & 0x55555555u) | ((x & 0x55555555u) << 1);
    x = ((x >> 2) & 0x33333333u) | ((x & 0x33333333u) << 2);
    x = ((x >> 4) & 0x0f0f0f0fu) | ((x & 0x0f0f0f0fu) << 4);
    x = ((x >> 8) & 0x00ff00ffu) | ((x & 0x00ff00ffu) << 8);
    x = ((x >> 16) & 0xffffu) | ((x & 0xffffu) << 16);
    return x;
}

template<class T>
csignal reordered_copy(vector<T>& s, uint8_t shift, int N){
	csignal c(N); for (uint32_t i = 0; i < N ; ++i) c[i] = complex(s[reverse(i) >> shift]);
	return c;
}

//Algo mas eficiente tendria un array para los reales por un lado y otro para los imaginarios por otro.
//Para señales reales, el tiempo de ejecucion se puede reducir a la mitad ahorrando calcular los coeficientes de la segunda mitad
csignal fft3(csignal& s, uint8_t log2N){

	double m =2;
	int minthalf = 1;
	int mint = 2;
	complex wm;
	complex w;
	int max =0;
	for (int count = 1; count <= log2N; ++count)
	{
		wm.re = cos(-1*TWOPI/m);
		wm.im = sin(-1*TWOPI/m);
		for (int k = 0; k < s.size(); k+=mint)
		{
		 	w.re=1;w.im=0;
		 	for (int j = 0; j < minthalf; ++j)
		 	{	
		 		int kj = k+j;
		 		complex t = w*s[kj+minthalf];
		 		complex u = s[kj];
		 		s[kj] = u+t;
		 		s[kj+minthalf] = u-t;
		 		w=w*wm;
		 	}
		}

		m*=2;
		mint*=2;
		minthalf*=2;
	}
	return s;
}


//Devuelve [DC offset, bin1, bin2.... Nyquist, -bin1, -bin2....]
uint8_t global_shift;
csignal fft3(ssignal& s){

	//CALCULO PARA EL BIT SHIFT
	int N = 1;
	uint8_t MSB = 0;
	while (N<s.size()) {N*=2; MSB++;}
	if(N>s.size()) {N/=2; MSB--;}
	uint8_t shift =0;
	if(MSB<=32) shift = 32-MSB;
	else {cout << "INPUT IS TOO BIG - ABORTING FFT" << endl; throw(1);}
	global_shift = shift;
	//printf("FFT TAMANIO %i, EXPONENTE %u\n",N, MSB);
	//printf("SENIAL TAMANIO %i \n", s.size());

	//BIT SHIFT
	//Estoy usando exp bits. Ponele que tengo que N = 8. Entonces el indice 3 (00000011) va al (11000000) 192. Como me manejo con 
	// 32 bits, recibo el 00...0011 y lo tendria que mandar al 00....11000000.  El bit reverse me da 1100..000. Ahora tengo que shiftear a derecha 32-8 bits
	csignal c=reordered_copy(s, shift, N);
	
	//PRODUCTOS INTERNOS
	//cout << "GO FFT"<<endl;
	return fft3(c, MSB);
}

uint8_t log2(int N){
	uint8_t exp =0;
	int n = N;
	while (n !=1) {exp++;n=n>>1;}
	return exp;
}
ssignal ifft3(csignal& s){
	//REVERSE SIGNAL
	csignal S = freverse(s);
	//BIT REVERSE
	S = reordered_copy(S,global_shift, S.size());
	uint8_t log2N = log2(S.size());
	//BASE Y COORDENADAS
	csignal F = fft3(S,log2N );
	ssignal out;
	for (int i = 0; i < S.size(); ++i) out.push_back(F[i].re/S.size());
	return out;
}







/**************************  	Algunas funciones *************/

//phase shift +-90°
ssignal HTransform(ssignal &s){

	csignal S = fft3(s);

	//Positivas: +90°. Multiplico por i

	for (int i = 0; i < S.size()/2; ++i)
	{
		swap(S[i].re, S[i].im);
		S[i].re*=-1;
	}

	//Negativas: -90°. Multiplico por -i
	for (int i = S.size()/2+1; i < S.size(); ++i)
	{
		swap(S[i].re, S[i].im);
		S[i].im*=-1;
	}
	
	return ifft3(S);
}

//Zeroes negative frequencies
ssignal to_analytical(ssignal& s){

	csignal S = fft3(s);
	for (int i = 0; i < S.size()/2; ++i)
	{
		S[i].re=.0;
		S[i].im=.0;
	}

	return ifft3(S);

}


#endif