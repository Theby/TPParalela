/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#include "escritor.hpp"
#include <fstream>

void Escritor::main(){
	std::ofstream output_file(this->outputFileName.c_str());
	std::string line;
	if(output_file.is_open()){
		while(true){
			line =  this->outputBuffer.getLine();
			if(line.compare("end sí") != 0){
				output_file << line + "\n";
			}else{
				break;
			}
		}
		output_file.close();
	}
}