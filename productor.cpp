/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#include "productor.hpp"
#include <fstream>
#include <string>


void Productor::main(){
	std::ifstream input_file(this->inputFileName.c_str());

	std::string line;
	while(getline(input_file, line)){
		this->inputBuffer.setLine( line );
	}

	this->inputBuffer.setLine("end");
}