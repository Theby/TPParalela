/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#ifndef __reconocedor_HPP_INCLUDED__
#define __reconocedor_HPP_INCLUDED__

#include <uC++.h>
#include "inputBuffer.hpp"
#include "outputBuffer.hpp"

_Task Reconocedor {
	InputBuffer &inputBuffer;
	OutputBuffer &outputBuffer;
	int id;

	public:
		Reconocedor( InputBuffer &buf1, OutputBuffer &buf2, int num )
		 : inputBuffer( buf1 ), outputBuffer( buf2 ), id(num) { }
	private:
		void main();
};

#endif