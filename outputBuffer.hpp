/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#ifndef __outputBuffer_HPP_INCLUDED__
#define __outputBuffer_HPP_INCLUDED__

#include <uC++.h>
#include <list>
#include <string>

_Monitor OutputBuffer {
	private:
		uCondition full, empty;
		std::list<std::string> content;
		int limite;
		int capacidad_actual;
	public:
		OutputBuffer() : limite(10), capacidad_actual(0) { }
		OutputBuffer(int init) : limite(init), capacidad_actual(0) { }
		~OutputBuffer(){ }

		void setLine(std::string line, bool resultado);
		std::string getLine();
};

#endif