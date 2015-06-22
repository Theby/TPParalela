/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#ifndef __productor_HPP_INCLUDED__
#define __productor_HPP_INCLUDED__

#include <uC++.h>
#include <string>
#include "inputBuffer.hpp"

_Task Productor {
	InputBuffer &inputBuffer;
	std::string inputFileName;

	public:
		Productor( InputBuffer &buf, std::string input_name ) : inputBuffer( buf ), inputFileName(input_name) { }
	private:
		void main();
};

#endif