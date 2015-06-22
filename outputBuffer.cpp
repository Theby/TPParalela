/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#include "outputBuffer.hpp"

void OutputBuffer::setLine(std::string line, bool resultado){ 

	if(capacidad_actual == limite)
		empty.wait();

	if(resultado){
		content.push_back(line+" sí");
	}else{
		content.push_back(line+" no");
	}
	capacidad_actual++;

	full.signal();
}

std::string OutputBuffer::getLine(){

	if(capacidad_actual == 0)
		full.wait();

	std::string content_line = content.front();
	content.pop_front();
	capacidad_actual--; 

	empty.signal();

	return content_line;
}