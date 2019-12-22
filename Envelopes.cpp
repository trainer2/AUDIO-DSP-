#include "Interpoladoras.cpp"
#include "funciones.h"


//Sigue la onda trackeando los maximos locales y creando un breakpoint  por cada uno de ellos
//va por copia porque lo estoy modificando
//TODO: Que rastree bien los silencios y los momentos distorsionados. No lo esta haciendo de esta manera
vector<breakpoint> Maxs(vector<double> buffer,double srate, bool norm = false){

	vector<breakpoint> res;
	if(norm) normalize(buffer);
	else for(int i=0; i<buffer.size(); i++) buffer[i]=abs(buffer[i]);
	res.push_back(breakpoint(0, buffer[0]));
	
	for(int i=1; i<buffer.size()-1; i++)
		//Dado que puedo tener zonas distorsionadas o que valgan 0 constantemente, para no ser redundante
		//hago >= y >. 
		if(buffer[i] > buffer[i-1] and buffer[i] >= buffer[i+1]) 
			res.push_back(breakpoint((double) i/srate,buffer[i]));
	return res;
}


//Igual pero lo  hace por ventana. El window_size va en cantidad de samples.
vector<breakpoint> MaxsWindowed(vector<double> buffer, double srate, uint window_size, bool norm=false){

	vector<breakpoint> res;
	if (norm) normalize(buffer);
	else for(int i=0; i<buffer.size(); i++) buffer[i]=abs(buffer[i]);
	res.push_back(breakpoint(0, buffer[0]));

	for(int i=0; i<buffer.size(); i+=window_size){
		double maxsamp=0;
		double maxsampTime;
		for(int w=i; w<i+window_size; w++)
			if(buffer[w] > maxsamp){
			 	maxsamp = max(maxsamp, buffer[w]);
			 	maxsampTime = (double) w / srate;
			}
		res.push_back(breakpoint(maxsampTime, maxsamp));
	}

	return res;

}

//TODO
//Ademas de leer por ventanas, si la transiente en una ventana es muy pronunciada genera varios breakpoints mas
vector<breakpoint> MaxsWindowedVariable(vector<double> buffer, uint window_size, float sensibility){

	vector<breakpoint> res;

	return res;

}


//Aplica la envolvente. Esta puede volver a dispararse cuando se termina, silenciar,
// o leer de atras para adelte y ciclar eso.
//TODO: una interpolacion de la envolvente cuando el rate es menor a 1. No viene al caso ahora.
//TODO: HACER FUNCIONAL PARA ENV_RATES NO ENTEROS


//s = una leida de envolvente y silencio
//r = retriggerea la envolvente cuando se termina
//c = cyclea ir y venir de la envolvente
//(TODO) l = ajusta el largo de la envolvente para que matchee el audio
void Envolver(vector<double>& in, vector<double>& envelope, uint env_rate = 1, char type = 'c'){

	uint env_index=0;
	uint i=0;
	bool llendo = true;
	while (i< in.size()){
		if(env_index >= envelope.size() or env_index <0){
			if(type == 's') {in.resize(i);return;}
			if(type == 'r') env_index = 0;
			if(type == 'c') llendo =!llendo; 
		}

		in[i] *= envelope[env_index];
		env_index = llendo ? env_index + env_rate : env_index - env_rate;
		i+= 1;
	}

}




//Extrae la envolvente del wav que le pasas por el path como parametro y la guarda como envolvente en el mismo directorio con un nuevo nombre
enum {programName, wavPath, follow_type, interpolType, window_size, Ncycles, normlze};

#ifndef applyEnvelope
int main(int argc, char const *argv[])
#else 
int main2(int argc, char const* argv[])
#endif
{

	if(argc<interpolType) cout<< "\n \n USAGE: filename follow_type interpolation_type window_size (opt, #samples) Cycles (opt) Normalize (opt)\n"

						"*****************************************************************\n \n"
						"Envelope Follower Type: \n"
						"1- Maxs\n"
						"2-Max windowed\n"
						"3-Max windowed variable (mejor para transientes pronunciadas (TODO)\n \n"

						"********************************************************************\n \n"
						"interpolation_type\n\n"
						"1- Lineal\n"
						"2- Exponencial\n"
						"3- Senoidal (aca se usa el parametro Cycles. Real entre 0 y +inf)\n \n \n"

						"Poner cualquier cosa en Normalize para que sea true";

	string path = argv[wavPath];
	int follow = atoi(argv[follow_type]);
	uint interp = atoi(argv[interpolType]);
	uint winsize; if(argc > window_size) winsize = atoi(argv[window_size]); 
	double cycles; if(argc > Ncycles)    cycles = (double) atof(argv[Ncycles]);
	bool norm =false; if(argc >normlze) norm= true; 

	AudioFile<double> wav;
	assert(wav.load(path));
	double srate = (double) wav.getSampleRate();
	int bitdepth = wav.getBitDepth();
	vector<double>& samples = wav.samples[0];
	vector<breakpoint> B;

	if(follow == 1) B = Maxs(samples,  srate, norm);
	if(follow == 2 && argc > window_size) B = MaxsWindowed(samples, srate, winsize, norm);
	//TODO caso 3

	vector<double> envelope;
	if(interp == 1) envelope = interpolate(B, interpoladoraLineal, srate, 0);
	if(interp == 2) envelope = interpolate(B, interpoladoraExponencial, srate, 0);
	if(interp == 3) envelope = interpolate(B, interpoladoraSenoidal, srate, cycles);

	size_t lastindex = path.find_last_of("."); 
	string envName = path.substr(0, lastindex) + "_envelope" + to_string(interp); 
	guardarWavMono(envelope, bitdepth, srate,envName );

	return 0;
}
