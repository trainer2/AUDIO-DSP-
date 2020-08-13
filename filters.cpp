#include "funciones.h"

/* 		INPUTS: Center Frequency / Cuttof , Bandwitdh		*/

void First_Order_Filter(vector<double>& buffer, double a=0.5, double b=0.5){
	for(int i=1; i<buffer.size(); i++) buffer[i]= a*buffer[i] + b*buffer[i-1];
}


void Resonator(vector<double>& buffer, double fc  = 1000, double BW = 1, double c =0){
	
	/*		Filter coefficients 				*/
	double R =  1- PI*BW/SAMPLING_RATE;
	double b = R*R;		//b2
	double a = cos(TWOPI*fc/SAMPLING_RATE) * (-1) * 4 *b /(1+b); //b1

	/*Scales so that a sinusoid of frequency fc and amplitude 1 will be outputed with amplitude 1*/
	double scalingFactor = (1-b)*sin(TWOPI*fc/SAMPLING_RATE);

	/* c should be 0, 1 or R*/

	//TODO: OPTIMIZAR ESTA PARTE
	vector<double> orig = buffer;

	for (int i=2; i<buffer.size(); i++){
		buffer[i] -= a*buffer[i-1] +b*buffer[i-2] + c* orig[i-2];
	}
	return;
}

/*  		Butterworth Design				*/


void Butterworth(vector<double>& bufferOut, const vector<double> bufferIn&, int i,
										   double a0, double a1, double a2,
													  double b1, double b2)
{
	bufferOut[i]= a0*bufferIn[i] + a1*bufferIn[i-1] + a2*bufferIn[i-2] -
					b1*bufferOut[i-1] + b2*bufferOut[i-2];
}

void LowPass(vector<double>& buffer, double fc, double BW){
	double pifsr = PI*fc / SAMPLING_RATE;
	double lambda = 1/tan/pifsr;

	double a0 = 1.0 / (1+2*lambda+lambda*lambda);
	double a1 = 2*a0;
	double a2 =	a0;
	double b1 = 2*a0*(1-lambda*lambda);
	double b2 =a0*(1-2*lambda+lambda*lambda);

	vector<double> bufferIn = buffer;
	for(int i=2; i<buffer.size(); i++)
		Butterworth(buffer, bufferIn, i, a0,a1,a2,b1,b2);
	return;
}


void HighPass(vector<double>& buffer, double fc, double BW){
	double pifsr = PI*fc / SAMPLING_RATE;
	double lambda = tan(pifsr);

	double a0 = 1.0 / (1+2*lambda+lambda*lambda);
	double a1 = 2*a0;
	double a2 =	a0;
	double b1 = 2*a0*(lambda*lambda-1);
	double b2 =a0*(1-2*lambda+lambda*lambda);
	vector<double> bufferIn = buffer;
	for(int i=2; i<buffer.size(); i++)
		Butterworth(buffer, bufferIn, i, a0,a1,a2,b1,b2);
	return;
}


void BandPass(vector<double>& buffer, double fc, double BW){
	double pibwsr = PI*BW/SAMPLING_RATE;
	double twopifsr = TWOPI*fc/SAMPLING_RATE;
	double lambda = 1/tan(pibwsr);
	double phi  = 2*cos(twopifsr);

	double a0 = 1.0/(1+lambda);
	double a1 = 0;
	double a2 = -1*a0;
	double b1 = -1*lambda*a0*phi;
	double b2 = a0*(lambda-1);
	
	vector<double> bufferIn = buffer;
	for(int i=2; i<buffer.size(); i++)
		Butterworth(buffer, bufferIn, i, a0,a1,a2,b1,b2);
	return;
}


void BandReject(vector<double>& buffer, double fc, double BW){
 	double pifsr  = PI*fc / SAMPLING_RATE;
	double twopifsr = TWOPI*fc/SAMPLING_RATE;
	double lambda = tan(pifsr);
	double phi  = 2*cos(twopifsr);

	double a0 = 1.0/(1+lambda);
	double a1 = -1*phi*a0;
	double a2 = a0;
	double b1 = -1*a0*phi;
	double b2 = a0*(lambda-1);

	vector<double> bufferIn = buffer;
	for(int i=2; i<buffer.size(); i++)
		Butterworth(buffer, bufferIn, i, a0,a1,a2,b1,b2);
	return;
}


int main(int argc, char const *argv[])
{
	for (int i = 0; i < 7; ++i)
	{
		vector<double> buffer = readWav("carriers/wnoise.wav");
		
		if(i==0) First_Order_Filter(buffer);
		if(i==1) Resonator()

	}



	return 0;
}

