#include "funciones.h"

/* 		INPUTS: Center Frequency / Cuttof , Bandwitdh		*/
/*		Working filters: At main definition					*/

vector<double> buffer;
const char LOWPASS = 'L';
const char HIGHPASS ='H';


//Con parametros por default: LOWPASS
void First_Order_Filter_FIR(double a=0.5, double b=0.5, unsigned int nsamples=1){
	for(int i=nsamples; i<buffer.size(); i++) buffer[i]= a*buffer[i] + b*buffer[i-nsamples];
}


void First_Order_Filter_IIR(char TYPE = LOWPASS, double fc =1000 ){
	double b = sqrt( pow(2.0- cos(TWOPI*fc/SAMPLING_RATE), 2) -1.0) - 2.0 + cos(TWOPI*fc /SAMPLING_RATE);
	if(TYPE == LOWPASS) b *= -1;
	double a = 1.0+b;

	for(int i=1; i<buffer.size(); i++) 
		buffer[i] = a*buffer[i] + b*buffer[i-1];
}

void Resonator(double fc  = 1000., double BW = 1.){
	
	/*		Filter coefficients 				*/
	double R =  1.0- PI*BW/SAMPLING_RATE;
	double R2 = 2*R;
	double b = R*R;		//b2
	double costh = R2 /(1.+b)*cos(TWOPI*fc/SAMPLING_RATE);
	double scalingFactor = (1-b) * sin(acos(costh));
	//double a = cos(TWOPI*fc/SAMPLING_RATE) * (-1.0) * 4.0 *b /(1.0+b); //b1

	/*Scales so that a sinusoid of frequency fc and amplitude 1 will be outputed with amplitude 1*/
	//double scalingFactor = (1.0-b)*sin(TWOPI*fc/SAMPLING_RATE);

	//TODO: OPTIMIZAR ESTA PARTE
	vector<double> orig = buffer;
	double delay[2]; 

	for (int i=2; i<buffer.size(); i++){
		buffer[i] = buffer[i]*scalingFactor +R2*costh*buffer[i-1] - b*buffer[i-2];
		//buffer[i] -= a*buffer[i-1] +b*buffer[i-2] + c* orig[i-2];
		//buffer[i] /= scalingFactor;
	}
	return;
}



void Resonator2(double fc  = 1000., double BW = 1.){
	
	/*		Filter coefficients 				*/
	double R =  1.0- PI*BW/SAMPLING_RATE;
	double R2 = 2*R;
	double b = R*R;		//b2
	double costh = R2 /(1.+b)*cos(TWOPI*fc/SAMPLING_RATE);
	double scalingFactor = 1-R;

	/*Scales so that a sinusoid of frequency fc and amplitude 1 will be outputed with amplitude 1*/
	//double scalingFactor = (1.0-b)*sin(TWOPI*fc/SAMPLING_RATE);


	vector<double> orig = buffer;
	double delay[2] = {buffer[1], buffer[0]};
	for (int i=2; i<buffer.size(); i++){
		double w = scalingFactor*buffer[i] + R2*costh*delay[0] - b*delay[1];
		buffer[i] = w-R*delay[1];
		delay[1] = delay[0];
		delay[0] = w;
	}
	return;
}



/*  		Butterworth Design				*/


void Butterworth(const vector<double>& bufferIn, int i,
										   double a0, double a1, double a2,
													  double b1, double b2)
{
	buffer[i]= a0*bufferIn[i] + a1*bufferIn[i-1] + a2*bufferIn[i-2] -
					b1*buffer[i-1] + b2*buffer[i-2];
}

void LowPass(double fc =1000., double BW=1.){
	double pifsr = PI*fc / SAMPLING_RATE;
	double lambda = 1.0/tan(pifsr);

	double a0 = 1.0 / (1.0+2.0*lambda+lambda*lambda);
	double a1 = 2.*a0;
	double a2 =	a0;
	double b1 = 2.0*a0*(1-lambda*lambda);
	double b2 =a0*(1-2*lambda+lambda*lambda);

	vector<double> bufferIn = buffer;
	for(int i=2; i<buffer.size(); i++)
		Butterworth(bufferIn, i, a0,a1,a2,b1,b2);
	return;
}


void HighPass(double fc=1000., double BW=1.){
	double pifsr = PI*fc / SAMPLING_RATE;
	double lambda = tan(pifsr);

	double a0 = 1.0 / (1.+2.*lambda+lambda*lambda);
	double a1 = 2.*a0;
	double a2 =	a0;
	double b1 = 2.*a0*(lambda*lambda-1.);
	double b2 =a0*(1.-2.*lambda+lambda*lambda);
	vector<double> bufferIn = buffer;
	for(int i=2.; i<buffer.size(); i++)
		Butterworth(bufferIn, i, a0,a1,a2,b1,b2);
	return;
}


void BandPass(double fc=1000., double BW=1.){
	double pibwsr = PI*BW/SAMPLING_RATE;
	double twopifsr = TWOPI*fc/SAMPLING_RATE;
	double lambda = 1./tan(pibwsr);
	double phi  = 2.*cos(twopifsr);

	double a0 = 1.0/(1.+lambda);
	double a1 = 0.;
	double a2 = -1.*a0;
	double b1 = -1.*lambda*a0*phi;
	double b2 = a0*(lambda-1.);
	
	vector<double> bufferIn = buffer;
	for(int i=2; i<buffer.size(); i++)
		Butterworth(bufferIn, i, a0,a1,a2,b1,b2);
	return;
}


void BandReject(double fc=1000., double BW=1.){
 	double pifsr  = PI*fc / SAMPLING_RATE;
	double twopifsr = TWOPI*fc/SAMPLING_RATE;
	double lambda = tan(pifsr);
	double phi  = 2.*cos(twopifsr);

	double a0 = 1.0/(1.+lambda);
	double a1 = -1.*phi*a0;
	double a2 = a0;
	double b1 = -1.*a0*phi;
	double b2 = a0*(lambda-1.);

	vector<double> bufferIn = buffer;
	for(int i=2; i<buffer.size(); i++)
		Butterworth(bufferIn, i, a0,a1,a2,b1,b2);
	return;
}


int main(int argc, char const *argv[])
{
	for (int i = 0; i <=8; ++i)
	{
		buffer = readWav("carriers/wnoise.wav");
		
		if(i==0) First_Order_Filter_FIR();			//SUCCESS!
		if(i==1) First_Order_Filter_IIR(LOWPASS);	//SUCCESS!
		if(i==2) First_Order_Filter_FIR(HIGHPASS);	//SUCCESS!
		if(i==3) Resonator();						//ES UN BAND PASS DE FRECUENCIA CENTRAL UNICA: QUe pasara si cambio BW?
		if(i==4) Resonator2();						//Lo mismo pero mas preciso
		if(i==5) LowPass();							//Recontra efectivo, pero hay que normalizar esa verga
		if(i==6) HighPass();						//NO ANDA: Funciona igual que el lowpass
		if(i==7) BandPass();						//NO ANDA: Funciona igual que el lowpass. Ambos dan el MISMO output sample a sample
		if(i==8) BandReject();						//SUCCESS!!

		guardarWavMono(buffer, 16, SAMPLING_RATE, "wnoise" + to_string(i));

	}

	return 0;
}

