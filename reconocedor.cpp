/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#include "reconocedor.hpp"
#include <regex>
#include <string>

void Reconocedor::main(){
	std::string linea;
	
	while(true){
		linea = this->inputBuffer.getLine();
		
		if(linea.compare("hasEnded") == 0){
			break;
		}

		if(linea.compare("end") != 0){
			std::regex rx("(A|C|G|T)*GT|CT*(A|C|G|T)*");
			bool resultado = regex_match(linea.begin(), linea.end(), rx);
			this->outputBuffer.setLine(linea, resultado);
		}else{
			this->outputBuffer.setLine(linea, true);
			break;
		}
	}
}