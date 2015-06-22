/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#ifndef __escritor_HPP_INCLUDED__
#define __escritor_HPP_INCLUDED__

#include <uC++.h>
#include <string>
#include "outputBuffer.hpp"

_Task Escritor {
	OutputBuffer &outputBuffer;
	std::string outputFileName;

	public:
		Escritor( OutputBuffer &buf, std::string output_name ) : outputBuffer( buf ), outputFileName(output_name) { }
	private:
		void main();
};

#endif