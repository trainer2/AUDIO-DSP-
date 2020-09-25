#ifndef PVF
#define PVF

#include "stft.cpp"
#include "funciones.h"
#include <math.h>       /* atan2 */


//Los vectores de csignals devuelven normalmente los frames y un vector de pares (a+bi). En este caso, a es la amplitud y b la frequencia calculada para cada bin
//La resintesis se basa en eso, asi que cualquier metodo de analisis de aca puede juntarse con cualquier metodo de resintesis (salvo este primero "PV") que solo convierte a polares, malisimo

//Es igual que un stft pero me guarda las coordenadas polares en vez de las reales. Voy a abusar de la notacion y guardar para cada coeficiente cn "a+bi", "Mag + Phase i", simplemente para que todo sea compatible entre si
//En lineas generales funciona excelente en notas musicales, con frecuencias bien discretizadas y que no caigan varias en un mismo bin. Si esto no se cumple los efectos son muy raros.

// Frequency estimate (el metodo clasico del Phase Vocoder) y su resintesis con fft pvs andan muy bien para estos sonidos. OSC sintesis te hace mierda todo. todo.
//IDF no anda tan bien, le debo estar pifiando en algo al transcribirlo.
//OSC Te destroza todo, pero bien. Suena ree interesante


stftSpectrum PV(ssignal& s, int fftsize=1024, int hop=512, double (*window)(int i, int size) = Hann){

	//Analisis
	stftSpectrum pv = stft(s, fftsize, hop, window);


	//Convertir a polares
	for (int i = 0; i < pv.cs.size(); ++i)
	{
		for (int j = 0; j < pv.cs[i].size(); ++j)
		{
			complex& c = pv.cs[i][j];
			double mag2 = c.re*c.re+c.im+c.im;
			double phase = atan2(c.im,c.re); //argumento del numero complejo
			c.re =mag2; c.im = phase;
			//Faltaria unwrap de la fase
		}
	}
	return pv;
}


//ANDA!!! Realmente te va tirando la frecuencia. Increible
//Copiado del C audio programming
//En teoria, me da un vector de frames donde cada uno guarda el modulo y la frecuencia (tengo fftsize frecuencias distintas)
//Solo el ultimo ultimo es un unico item que guarda la cant de samples de S, para luego resintetizarlo mas facil
vector<csignal> frequencyEstimate2(ssignal& s, int fftsize=1024, int hop = 512,double (*window)(int i, int size) = Hann, const char* path = "", bool printCSV =false){

	ssignal windowed(fftsize);
	vector<double> lastphase(fftsize/2, 0.);
	vector<csignal> out;
	double fac = (double) SAMPLING_RATE/hop / TWOPI;
	double scal = TWOPI * hop / fftsize;
	int posout=0;
	for (int posin=posout = 0; posin < s.size(); posin+=hop)
	{
	//	printf("TODO PIOLA %i \n", posin);
		int mod = posin%fftsize;
		for (int i = 0; i < fftsize; ++i)
			if(posin+i<s.size()) windowed[(i+mod)%fftsize]=s[posin+i]*window(i, fftsize);
			else windowed[(i+mod)%fftsize] =0.;
		csignal F = fft3(windowed);
		csignal FPV(fftsize);
		for (int i = 1; i < fftsize/2; i++)
		{
			//if(i==fftsize/2) continue;
			complex& c = F[i];
			double mag =c.re*c.re+c.im*c.im;
			double phi;
			mag = sqrt(mag);
			if(c.re==0.0)  phi = PI/2*sign(c.im);
			else  phi = atan2(c.im, c.re);
			double dphi = phi-lastphase[i];
			lastphase[i] = phi;
			FPV[i].re=mag; FPV[i].im= (dphi+i*scal)*fac;
			FPV[i+fftsize/2].re=mag; FPV[i+fftsize/2].im= -1*(dphi+i*scal)*fac;
		}
		out.push_back(FPV);
	}

	if(printCSV){
		FILE *fp;
		string file = string("FREQUENCY ESTIMATIONS " );
		file += path;
		file +=string(".csv");
	    fp = fopen(file.c_str() ,"w");
		for (int frame = 0; frame < out.size(); ++frame)
		{
			for (int bin = 0; bin < fftsize/2; ++bin){
				fprintf(fp,"%.2f, \t", out[frame][bin].im);
			//	if(out[frame][bin].im >400 and out[frame][bin].im<450) printf("PARA EL BIN CLAVE LA FREC CALCULADA ES %.3f",out[frame][bin].im);

			}
			fprintf(fp, "%s", "\n" );
		}
	}
	csignal temp; temp.push_back(complex((double) s.size())); out.push_back(temp);

	return out;
}




//inversa de la de arriba (frequency estimation)
//Copiado del C Audio Programming. 

//RESULTADO: Audivitamente es una inversa. Pero tiene un comportamiento rarisimo. Rarisimo. Como que el resultado de una llamada depende de la anterior. No se. Habria que testearlo mas en detalle
//La window es de suavizado, para el caso en que los frames salen de un morphing, y no para el caso de un analisis directo
ssignal pvs(vector<csignal>& pvc, int fftsize=1024, int hop = 512, double (*window)(int i, int size) = id){ 

	int N = (int) pvc[pvc.size()-1][0].re;
	pvc.pop_back();
	ssignal windowed(fftsize);
	vector<double> lastphase(fftsize/2,0.);
	double fac = hop*TWOPI/SAMPLING_RATE;
	double scal=SAMPLING_RATE/fftsize;
	ssignal s(N);
	ssignal frameifft(fftsize);
	int frame=0;
	for (int posout= 0; posout < N; posout+=hop, frame++)
	{
		for (int i = 1; i < fftsize/2; ++i)
		{

			double dphase = (pvc[frame][i].im-i*scal)*fac;
			double phi =lastphase[i]+dphase;
			lastphase[i]=phi;
			double mag = pvc[frame][i].re;

			pvc[frame][i].re = (mag*cos(phi));
			pvc[frame][i].im = (mag*sin(phi));
			pvc[frame][i+fftsize/2]=complex(mag*cos(phi), -1*mag*sin(phi));
		}

		frameifft = ifft3(pvc[frame]);
		int mod =posout%fftsize;
		for (int i = 0; i < fftsize; ++i)
		{
			if(posout+i<N)s[posout+i]+=frameifft[(i+mod)%fftsize]*window(i, fftsize);
		}
	}
	printf("%i FRAMES DE %i HECHOS\n", frame, pvc.size());
	csignal size; size.push_back(complex((double)N));pvc.push_back(size);

	return s;
}

//Intento de resintetizar una señal a partir de la data de un PV con osciladores. Lo abandone porque veo que es lento como un DFT
//ssignal ipv(){}


//Basado en el Phase Vocoder de arriba. Te devuelve un vector de frames donde cada frame es un vector de pares <magnitud, frecuencia>.
//Recibis dos señales, analizas. Partis igual  ala primera, y vas acercandote poco a poco en frecuencias y amplitudes a la segunda.
//Idealmente los espectros de s1 y s2 son parecidos, y s1 y s2 de duracion parecida
// Como te vas a basar en la resintesis basada en fft, hay bins de frecuencia, asi que no es tan llamativo como un aproach con osciladores y bancos de filtros
//La interpolacion seria lineal en amplitud y exponencial en frecuencia segun el libro, pero por lo dicho de los bins, no tiene sentido interpolar exponencialmente para cosas que esten tan cerca, creo

//RESULTADO: SUENA MUY EXTRAÑO. Algo estoy poniendo mal. Suena mejor para fftsize mas grandes de 2000 (4k para arriba me gusta) y para hops de la mitad, por ejemplo
//Lo principal es un pumping que no se de  donde nace al usar Hanning window (debo estar windoweando mal en algun lado?) y un clipping que se escucha en cada cambio de frame. Por mas que el hop sea corto se escucha este clipping
//Trate de solucionar el clipping con un window en la resintesis, pero hay que ser sutil con la ventana que elegis. No estoy logrando buenos resultados para hop bajos por el clipping. 

ssignal PVmorph(ssignal& s1, ssignal& s2, bool morphFreq = true, bool morphAmp = true, int fftsize=1024, int hop=512, double (*window)(int i, int size) = Hann){

	vector<csignal> S1 = frequencyEstimate2(s1,fftsize, hop, window);
	vector<csignal> S2 = frequencyEstimate2(s2,fftsize, hop, window);

	int Nframes = min(  S1.size(), S2.size()); //Despues resto 1 porque el ultimo frame es trucho, solo es el tamaño de la señal
	printf("HACIENDO %i FRAMES EN EL MORPH\n", Nframes-1 );
	double Nframesd = (double) Nframes;
	//frec -> frec1 * pow(frec2/frec1, frame/Nframes), pero lo rompe :/
	//amp -> amp1 + frame/Nframes(amp2-amp1)
	for (int frame = 1; frame< Nframes-1; ++frame)
	{
		for (int bin = 0; bin < S1[frame].size(); ++bin)
		{
			double& f1=S1[frame][bin].im;
			double& f2=S2[frame][bin].im;
			double& mag1=S1[frame][bin].re;
			double& mag2=S2[frame][bin].re;
		
			//if(bin == S1[0].size()/4) printf("FREQ ANTES %f \n", f1);
			if(morphFreq) f1  += frame/Nframesd * (f2-f1); //*= pow(f2/f1, (double) frame/ Nframesd); //Esto es peligroso en el caso general, porque podria darte una frecuencia que no caiga en el bin correspondiente de fft. En este caso no sucede, pero multiplicando por ctes arbitrarias si (la base del time stretch es esto)
			if(morphAmp) mag1 += frame/Nframesd * (mag2-mag1);

		//	if(bin == S1[0].size()/4) printf("FREQ DESPUES %f \n", f1);
		}
	}
	return pvs(S1, fftsize,hop, almostFlat);
}




//C Audio Programming pg 571
//Otra forma de estimar la frecuencia, con otra derivacion de la formula
//REsultado: Analiza perfecto
vector<csignal> IFD(PARAMS){

	vector<csignal> out;
	ssignal w(fftsize);
	ssignal dwindow(fftsize);
	for (int i = 0; i < fftsize; ++i)
		w[i]=window(i,fftsize);
	for (int i = 1; i < fftsize; ++i)
		dwindow[i] = w[i]-w[i-1];
		dwindow[0] = w[0]*-1;
	double fac = SAMPLING_RATE/TWOPI; double fund = SAMPLING_RATE/fftsize;
	int posout=0;
	ssignal windowed(fftsize);
	printf("A CALCULAR LAS FRECUENCIAAS\n");
	for (int posin = 0; posin < s.size(); posin+=hop)
	{
		for (int i = 0; i < fftsize; ++i)
			if  (posin+i<s.size()) windowed[i]=s[i+posin]*dwindow[i];
			else windowed[i]=0.;
		csignal F1 = fft3(windowed);

		for (int i = 0; i < fftsize; ++i)
			if  (posin+i<s.size()) windowed[i]=s[i+posin]*w[i];
			else windowed[i]=0.;
		csignal F2 = fft3(windowed);

		csignal out_i(fftsize);
		for (int i = 0; i < fftsize; ++i)
		{
			double& a = F1[i].re;
			double& b = F1[i].im;
			double& c = F2[i].re;
			double& d = F2[i].re;
			double power = c*c+d*d;
			out_i[i].re = sqrt(power);
			if(power >0) out_i[i].im = ((b*c-a*d)/power)*fac+i*fund;
			else out_i[i].im=i*fund;
		}
		out.push_back(out_i);
	}
	csignal size; size.push_back(complex((double)s.size()));out.push_back(size);
	return out;
}


//RESULTADO: NOOOO, TE LA DESTRUYE. ME ENCANTA. 

int tablelen=10000;
double tresh = 0.5;
//Sintesis aditiva con osciladores lookup a partir de pares frecuencia-amplitud
ssignal addsyn(vector<csignal>& cs, int fftsize = global_fftsize, int hop = global_hop){


	int bins = fftsize;
	int size = (int) cs[cs.size()-1][0].re; cs.pop_back();
	double ratio = tablelen/SAMPLING_RATE;
	ssignal s(size);
	vector<double> outsum(hop);
	vector<double> amps(fftsize,0.);
	vector<double> phis(fftsize,0.);
	vector<double> fftfreqs(fftsize);
	vector<double> table(tablelen); 

	for (int i = 0; i < fftsize/2; ++i) {fftfreqs[i]=i*SAMPLING_RATE/fftsize; fftfreqs[i+fftsize/2]=fftfreqs[i];}
	for (int n = 0; n < tablelen; ++n)	table[n]=sin(n*TWOPI/tablelen);
	

	printf("A RESINTETIZAR, TAMANIO FINAL %i\n", size);
	int frame =0;
	for (int posout = 0; posout+hop < size; posout+=hop)
	{
		printf("VA EL FRAME %i de %i \r",frame , cs.size());
		for (int bin = 0; bin < cs[0].size(); ++bin)
		{
			double ampnext =cs[frame][bin].re;
			double freqnext = cs[frame][bin].im;
			double incra = (ampnext-amps[bin])/hop;
			double incrf = (freqnext-fftfreqs[bin])/hop;	
			if(ampnext>tresh) 
				for (int n =0; n<hop; n++){
					phis[bin]+=fftfreqs[bin]*ratio;
					while(phis[bin]<0) phis[bin]+=tablelen;
					while(phis[bin]>=tablelen) phis[bin]-=tablelen;
					outsum[n] +=amps[bin]*table[(int) phis[bin]];
					amps[bin]+= incra;
					fftfreqs[bin]+=incrf;
				}
			else amps[bin]=0.;
		}
		for (int n = 0; n < hop; ++n) s[n+posout] =outsum[n];
		frame++;
	}

	csignal s_size; s_size.push_back(complex((double)size));cs.push_back(s_size);//Restauro lo que me dieron al ppio

	return s;

}










/********************************


//NO ANDA D:
//copio tal cual el codigo de stft salvo que agrego la rotacion delos samples justo antes del analisis. Despues hago la cuenta de diferencia de fase
//Imprime una lista con las frecuencias que estima para cada bin en cada windowed snapshot en un tipo csv*/
void frequencyEstimate(ssignal& s, int fftsize=1024, int hop=512, double (*window)(int i, int size) = Hann, const char* path = ""){
	int fold =0;
	vector<double> lastphase(fftsize, 0.0);
	//IGUAL
		stftSpectrum Sp;
		Sp.fftsize = fftsize;
		Sp.hop = hop;
		Sp.window = window;
		Sp.size = s.size();
		int s_k =0;
		ssignal windowed(fftsize);
		while(s_k+fftsize <= s.size())
		{
		//window(i, size) vale 0 en i=0 y 1 en i=size/2
	//CAMBIO- ACA ROTO
		//por ejemplo: si hop es 256 y fftsize es 1024, la primer vuelta leo del (indice relativo) 0, la segunda del 256, la tercera del 512, la cuarta del 7xx y vuelvo a empezar en la siguiente
	int s_f=s_k;
	for (int i = 0; i < fold; ++i,++s_f)
		windowed[i] = s[s_f+fftsize-fold]*window(i-fftsize/2, fftsize);
	for (int i = fold; i < fftsize; ++i, ++s_k)
		windowed[i] = s[s_k]*window(i-fftsize/2, fftsize);
	fold = (fold+hop)%fftsize;
	
	//IGUAL
			csignal anal = fft3(windowed);
			Sp.cs.push_back(anal);
			s_k -= (fftsize-hop);
			fftsize = anal.size();

	//CONVIERTO A DIFERENCIAS DE FASEY POLARES	(casi igual que en PV)	
		for (int j = 1; j < fftsize; ++j)
		{
			complex& c = anal[j];
			double mag2 = c.re*c.re+c.im+c.im;
			double phase = c.re!=0. ? atan2(c.im,c.re) : 0.; //argumento del numero complejo
			c.re =mag2; 
			c.im = phase-lastphase[j];
			while(c.im>PI) c.im -= TWOPI;
			while(c.im<-1*PI) c.im+=TWOPI;
			lastphase[j]=phase; 
		}
	}

	//NUEVO: ESTIMO LA FRECUENCIA A PARTIR DELAS DIFERENCIAS DEFASE

	FILE *fp;
	string file = string("FREQUENCY ESTIMATIONS " );
	file += path;
	file +=string(".csv");
    fp = fopen(file.c_str() ,"w");
    cout<<file<<endl;
	double freqestimate;
	double binstepct =SAMPLING_RATE;	
	for (int frame = 0; frame < Sp.cs.size(); ++frame)
	{
		for (int bin = fftsize/2; bin < fftsize; ++bin){


			//Veamos como extraer la desviacion de frecuencia a partir de dphase
			//Si mis bin estan espaciados a 2, y me entra una frecuencia de 3hz, el bin de 2hz deberia tener una diferencia de fase de medio ciclo positivo. Es decir, dphase seria Pi. Y dfrec seria dphase/2Pi*binstep = 1 en este caso
			double fcentral = (bin-fftsize/2)*SAMPLING_RATE/fftsize;
			double dphase = Sp.cs[frame][bin].im; //+TWOPI*hop/fftsize*bin;  //Recordar que éste guarda la diferencia de faseee. Lo que se le suma es para recuperar la frecuencia
			if(fcentral >1 and fcentral<2) printf("LA DIFERENCIA DE FASE PARA EL BIN CLAVE ES %.8f\n LA FREC CENTRAL ES %.3f",dphase , fcentral);
			freqestimate = fcentral+dphase*binstepct;
			fprintf(fp,"%.2f, \t", freqestimate);
		}
			fprintf(fp, "%s", "\n" );
	}
    fclose(fp);
   	return;
}


//////////////

#endif
