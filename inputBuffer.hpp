/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#ifndef __inputBuffer_HPP_INCLUDED__
#define __inputBuffer_HPP_INCLUDED__

#include <uC++.h>
#include <list>
#include <string>

_Monitor InputBuffer {
	private:
		uCondition full, empty;
		std::list<std::string> content;
		int limite;
		int capacidad_actual;
		bool hasEnded;
	public:
		InputBuffer()
		 : limite(10), capacidad_actual(0), hasEnded(false) { }
		InputBuffer(int init)
		 : limite(init), capacidad_actual(0), hasEnded(false) { }
		~InputBuffer(){ }

		void setLine(std::string line);
		std::string getLine();
};

#endif