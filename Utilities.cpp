#include "funciones.h"

//CONVERT ALL STEREO FILES TO MONO FILES
//REQUIERES C++ 17

#include <string>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main()
{
    std::string path = "files/";
    for (const auto & entry : fs::directory_iterator(path))

    	Audiofile<double> wav = leerwav(entry.path()); 
    	if (wav.isStereo){
	    	//int sampleRate = wav.getSampleRate();
			//int bitDepth =   wav.getBitDepth();
			wav.setNumChannels(1);
			trimTail
			wav.save (entry.path(), AudioFileFormat::Wave);
    	}
	return 0;
}




//TRIM 0's tail
void trimTail(Audiofile<double>& s){ while ( (s.samples[0])[s.size()-1] == 0) (s.samples[0]).popback();}