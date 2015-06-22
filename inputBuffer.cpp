/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#include "inputBuffer.hpp"

void InputBuffer::setLine(std::string line){ 

	if(capacidad_actual == limite)
		empty.wait();

	content.push_back(line);
	capacidad_actual++;

	full.signal();
}

std::string InputBuffer::getLine(){

	if(!this->hasEnded){
		if(capacidad_actual == 0)
			full.wait();

		if(!this->hasEnded){
			std::string content_line = content.front();
			content.pop_front();
			capacidad_actual--;

			if(content_line.compare("end") == 0){			
				this->hasEnded = true;
				full.signal();			
			}

			empty.signal();

			return content_line;
		}else{
			full.signal();
			return "hasEnded";
		}
	}else{
		return "hasEnded";
	}
}