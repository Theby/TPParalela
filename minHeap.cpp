#include "minHeap.hpp"

bool MinHeap::Compare(std::tuple<float, float> first, std::tuple<float, float> second){
	if(std::get<0>(first) < std::get<0>(second)){
		return true;
	}

	return false;
}

void MinHeap::Insert(std::tuple<float, float> newTuple){
	int myself;
	int father;
	std::tuple<float, float> aux;

	// Insertar al final del árbol
	this->minHeap.push_back(newTuple);

	// Obtiene los indices de la nueva tupla y su padre
	myself = (this->minHeap.size() - 1);
	father = (myself-1)/2;

	// Mientras no sea el primer elemento
	while(father >= 0){

		// Compara si es menor que su padre
		if(this->Compare(this->minHeap[myself], this->minHeap[father])){
			// Si no lo es

			// Swaping
			aux = this->minHeap[myself];
			this->minHeap[myself] = this->minHeap[father];
			this->minHeap[father] = aux;

			// Calculo de variables de posición
			myself = father;
			father = (myself-1)/2;
		}else{
			// Si no
			// El árbol está balanceado, no es necesario seguir revisando
			break;
		}
	}	
}

std::tuple<float, float> MinHeap::Delete(){
	int myself = 0;
	int izq = 2*myself + 1;
	int der = izq + 1;
	int swapingPos;
	int size;
	// Se guarda el menor para retornarlo al final
	std::tuple<float, float> menor = this->minHeap.front();
	std::tuple<float, float> aux;
	
	// Pone en la raíz el último elemento del vector y lo elimina de esa posición
	this->minHeap[0] = this->minHeap.back();
	this->minHeap.pop_back();

	// Tamaño tras sacar un elemento
	size = this->minHeap.size();

	// Si no hay hijo izquierdo entonces se llegó a una hoja
	while(izq < size){

		// Si el padre actual solo tiene hijo izquierdo
		if(der >= size){
			// El derecho será igual al izquierdo para poder realizar
			// las comparaciones
			der = izq;
		}

		// Compara si el hijo izquierdo es menor que el derecho
		if (this->Compare(this->minHeap[izq], this->minHeap[der]))
		{
			// Si lo es

			// Se compara él con su hijo izquierdo
			if (this->Compare(this->minHeap[izq], this->minHeap[myself])){
				// Si lo es

				// Guarda la posición del izquierdo para el swaping
				swapingPos = izq;
			}else{
				// Si no
				// El es menor que sus dos hijos, el árbol está balanceado
				break;
			}
		}else{
			// Si no
			// Se compara él con su hijo derecho
			if (this->Compare(this->minHeap[der], this->minHeap[myself])){
				// Si lo es

				// Guarda la posición del derecho para el swaping
				swapingPos = der;
			}else{
				// Si no
				// El es menor que sus dos hijos, el árbol está balanceado
				break;
			}
		}

		// Swaping
		aux = this->minHeap[myself];
		this->minHeap[myself] = this->minHeap[swapingPos];
		this->minHeap[swapingPos] = aux;

		// Calcula los nuevos hijos
		izq = 2*swapingPos + 1;
		der = izq + 1;
	}

	return menor;
}

bool MinHeap::empty(){	
	return this->minHeap.empty();
}