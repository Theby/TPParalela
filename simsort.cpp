/***********************************************
 * LAB2: OpenMP - SIMD + Shared-Memory
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include "minHeap.hpp"

#ifdef _OPENMP
	#include <omp.h>
#endif

using namespace std;

/* HELPERS */
// Intercambia los valores intermedios del arreglo
__m128 swapMiddle(__m128 R){
	return _mm_shuffle_ps(R, R, _MM_SHUFFLE(3, 1, 2, 0));
}

// Intercambia los valores intermedios entre dos arreglos
__m128 dobleSwapMiddle(__m128 R1, __m128 R2){
	return _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(3, 1, 2, 0));
}

// Invierte el orden de un registro
__m128 invertir(__m128 R){
	return _mm_shuffle_ps(R, R, _MM_SHUFFLE(0, 1, 2, 3));
}

// Entrega un registro que tiene el primer digito repetido en todas sus posiciones
__m128 primeroRepetido(__m128 R){
	return _mm_shuffle_ps(R, R, _MM_SHUFFLE(0, 0, 0, 0));
}

// Realiza el paso intermedio de una red minmax
// La razón de está función es que este paso es usado
// al final de la Bitonic Merge Network y para no repetir código
// se a exteriorizado
__m128 middleMinMax(__m128 R){
	__m128 aux_r;

	aux_r = _mm_movehdup_ps(R);
	return _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(2, 0, 2, 0));
}

// Compara dos registros dejando los valores menores menores en R1 y los mayores en R2
// La comparación se hace por columna
void compara2R(__m128& R1, __m128 &R2){
	__m128 aux1, aux2;
	__m128 comp;

	// Compara el primer registro con el segundo
	comp = _mm_cmplt_ps(R1, R2);
	// Obtiene el menor
	aux1 = _mm_blendv_ps(R2, R1, comp);
	// Obtiene el mayor
	aux2 = _mm_blendv_ps(R1, R2, comp);

	R1 = aux1;
	R2 = aux2;
}

// Pasa cuatro arreglos de 4 digitos a un vector, este guarda la secuencia
// ordenada de forma secuencial
vector<float> float16(float* a, float* b, float* c, float* d){
	vector<float> v;

	for(int i=0; i < 16 ; i++){
		if(i<4){
			v.push_back(a[i]);
		}else if(i>=4 && i<8){
			v.push_back(b[i%4]);
		}else if(i>=8 && i<12){
			v.push_back(c[i%4]);
		}else if(i>=12 && i<16){
			v.push_back(d[i%4]);
		}
	}

	return v;
}

/* Operaciones SIMD */

// Realiza una MinMax Network sobre un registro dejando ordenado
// de menor a mayor los números en él
__m128 minmaxNetwork(__m128 R){
	__m128 aux_r;

	aux_r = _mm_shuffle_ps(R,R, _MM_SHUFFLE(3, 2, 3, 2));
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(1, 0, 1, 0));

	R = middleMinMax(R);

	aux_r = swapMiddle(R);
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(3, 2, 1, 0));

	return R;
}

// Realiza una MinMax Network entre cuatro registros, esto deja ordenado
// Todos los registros comparando por columnas
void minmaxNetwork_R(__m128& Af, __m128& Bf, __m128& Cf, __m128& Df){

	// Compara el primero con el tercero
	compara2R(Af, Cf);
	// Compara el segundo con el cuarto
	compara2R(Bf, Df);
	// compara el primero con el segundo
	compara2R(Af, Bf);
	// compara el tercero con el cuarto
	compara2R(Cf, Df);
	// Compara el segundo con el tercer
	compara2R(Bf, Cf);
}

// Realiza el ordenamiento InRegister, ordenando cuatro registros en una matriz
// de 4x4
void ordenamientoInRegister(__m128& Af, __m128& Bf, __m128& Cf, __m128& Df){

	// MinMax entre los cuatro registros
	// Quedan ordenados por columa
	minmaxNetwork_R(Af, Bf, Cf, Df);

	// Redshuffle
	// La primera columna ahora es la primera fila y así con cada columna
	_MM_TRANSPOSE4_PS(Af, Bf, Cf, Df);
}

// Ejecuta una Bionic Merge Network
// Esto ordena dos registros dejando los números ordenados de menor a mayor
void BMN(__m128& Af, __m128& Bf){
	__m128 Af_bmn, Bf_bmn, Af_lmm, Bf_lmm;;

	// Crea arreglos iniciales con dos shuffle por arreglo, hace una relación 1:1
	// Para empezar el BMN
	Af_bmn = _mm_shuffle_ps(Af, Bf, _MM_SHUFFLE(2, 0, 2, 0));
	Bf_bmn = _mm_shuffle_ps(Af, Bf, _MM_SHUFFLE(3, 1, 3, 1));

	Af_bmn = swapMiddle(Af_bmn);
	Bf_bmn = swapMiddle(Bf_bmn);

	// Aplica una red minmax a cada arreglo por separado
	Af_bmn = minmaxNetwork(Af_bmn);
	Bf_bmn = minmaxNetwork(Bf_bmn);

	// Las reordena nuevamente para hacer un último minmax
	Af_lmm = _mm_shuffle_ps(Af_bmn, Bf_bmn, _MM_SHUFFLE(1, 0, 1, 0));
	Bf_lmm = _mm_shuffle_ps(Af_bmn, Bf_bmn, _MM_SHUFFLE(3, 2, 3, 2));

	Af_bmn = swapMiddle(Af_lmm);
	Bf_bmn = swapMiddle(Bf_lmm);

	//Hace el último minmax
	Af_bmn = middleMinMax(Af_bmn);
	Bf_bmn = middleMinMax(Bf_bmn);

	// Reordena el registro intermedio
	Af = swapMiddle(Af_bmn);
	Bf = swapMiddle(Bf_bmn);
}

// Ejecuta mergeSIMD sobre cuatro arreglos (16 números)
// Esto deja ordenado los 16 números de de menor a mayor entre los 4 registros
void mergeSIMD(__m128& Af, __m128& Bf, __m128& Cf, __m128& Df){
	__m128 O1, O2, O2_menor, O2_mayor;
	__m128 comp;

	// Se cargan Af y Cf en una BMN (invirtiendo Cf) para obtener dos registros O1 y O2
	O1 = Af;
	O2 = Cf;
	O2_menor = Bf;
	O2_mayor = Df;

	// Compara los dos arreglos restantes para obtener el menor y el mayor
	comp = _mm_cmplt_ps(primeroRepetido(Bf), primeroRepetido(Df));
	// Obtiene el menor
	O2_menor = _mm_blendv_ps(Df, Bf, comp);
	// Obtiene el mayor
	O2_mayor = _mm_blendv_ps(Bf, Df, comp);	

	O2 = invertir(Cf);
	BMN(O1, O2);

	// Se extrae O1 y O1 toma el valor de O2
	Af = O1;
	O1 = O2;

	// Se repite el proceso
	O2 = invertir(O2_menor);
	BMN(O1, O2);

	Bf = O1;
	O1 = O2;

	// BMN final
	O2 = invertir(O2_mayor);
	BMN(O1, O2);

	Cf = O1;
	Df = O2;
}


/* Conjunto de operaciones SIMD */
// Ejecuta todas las operaciones de SIMD con el fin de ordenar 16 digitos
// entre 4 registros
void SIMD_Part(float* a, float* b, float* c, float* d){
	__m128 Af, Bf, Cf, Df;

	// Guarda los arreglos en registros de punto flotante
	Af = _mm_load_ps(a);
	Bf = _mm_load_ps(b);
	Cf = _mm_load_ps(c);
	Df = _mm_load_ps(d);


	// IN-REGISTER
	// Se alteran los valores de Af, Bf, Cf y Df al pasar el valor como referencia
	ordenamientoInRegister(Af,Bf,Cf,Df);

	// Invierte Bf y Df para preparar la BMN
	Bf = invertir(Bf);
	Df = invertir(Df);

	// BMN
	// Se alteran los valores de Af y Bf, Cf y Df al pasar el valor como referencia
	BMN(Af, Bf);
	BMN(Cf, Df);

	// Merge SIMD
	// Se alteran los valores de Af, Bf, Cf y Df al pasar el valor como referencia
	mergeSIMD(Af,Bf,Cf,Df);

	// Guarda los registros
	_mm_store_ps(a, Af);
	_mm_store_ps(b, Bf);
	_mm_store_ps(c, Cf);
	_mm_store_ps(d, Df);
}

/* Parte no SIMD: Multiway Merge Sort */
// Implementa un MWMS sobre varios vectores usando una implementación
// de minHeap
vector<float> mwms(vector<vector<float>> secuencias){
	vector<float> output;
	std::tuple<float, int> elemento;
	MinHeap minHeap;
	int num_lista;
	
	// Recorre todas las listas para sacar los primeros elementos
	// eliminandolo de su arreglo original
	for (unsigned int j = 0; j < secuencias.size(); j++){
		minHeap.Insert(std::make_tuple(secuencias[j][0], (int)j));
		// Borra el elemento de la lista original
		secuencias[j].erase(secuencias[j].begin());	
	}

	// Mientras el minHeap no esté vacio
	while(!minHeap.empty()){
		// Saca el menor, lo borra y lo guarda en output
		elemento = minHeap.Delete();
		output.push_back(std::get<0>(elemento));

		// Búsca otro número de la misma lista de donde salio
		// y lo ingresa al minHeap
		num_lista = std::get<1>(elemento);

		// Si la lista no está vacia
		if(!secuencias[num_lista].empty()){
			// Agrea un nuevo elemento de la lista de donde salió
			minHeap.Insert(
				std::make_tuple(
					secuencias[num_lista][0],
					num_lista
				)
			);
			// Borra el número de la lista de listas
			secuencias[num_lista].erase(secuencias[num_lista].begin());			
		}
	}	
	return output;
}

/* Merge 2-way */
// Toma un elemento de un vector y lo compara con el otro
// Si es menor lo guarda si no guarda el del otro
// se avanza iterativamente por cada uno mientras se guardan
// sus valores
vector<float> mergeTW(vector<float> first, vector<float> second){
	vector<float> output;
	int firstCounter = 0;
	int secondCounter = 0;
	int size = first.size();

	while(firstCounter < size || secondCounter < size){
		if(firstCounter == size){
			output.push_back(second[secondCounter]);
			secondCounter++;
		}else if(secondCounter == size){
			output.push_back(first[firstCounter]);
			firstCounter++;
		}else{
			if(first[firstCounter] < second[secondCounter]){
				output.push_back(first[firstCounter]);
				firstCounter++;
			}else{
				output.push_back(second[secondCounter]);
				secondCounter++;
			}
		}
	}

	return output;

}

/* Creación del arbol de threads */
// Permite hacer todo el algoritmo general
vector<float> arbolDeHebras(int nivel_actual, 
							int nivel_recursividad_maximo, 
							float * memblock,
							int inicio,
							int largo
							)
{
	vector<vector<float>> output;
	vector<float> mergedOutput;
	output.resize(2);

	#pragma omp parallel num_threads(2) firstprivate(nivel_actual, nivel_recursividad_maximo, inicio, largo) shared(output, memblock)
	{
		// Guardo mi ID del nivel, puede ser 0 o 1
		int mytid = omp_get_thread_num();
		// Dentro del pragma entramos a un nuevo nivel de paralelismo
		nivel_actual++;
		// Se divide en dos el largo
		largo = largo/2;
		// Se asigna el nuevo inicio del arreglo
		inicio += largo * mytid;

		// Reviso si he llegado al nivel de recursividad más bajo
		if(nivel_actual != nivel_recursividad_maximo){
			// Si no

			// Creo dos hebras más en el siguiente nivel
			// Capturo el resultado de mis dos hebras en mi espacio asignado
			output[mytid] = arbolDeHebras(nivel_actual,nivel_recursividad_maximo, memblock, inicio, largo);
		}else{
			// Si he llegado entonces realizo el SIMD Sort y el MWMS de mis listas
			// asignadas guardando el resultado en en mi espacio asignado
			vector<vector<float>> secuencias;
			// Número de veces que se cargaran 16 flotantes y que se ejecutará
			// toda la secuencia SIMD completa
			int veces = largo/16;

			/* Declaración de registros */
			// Se usan de a cuatro registros alineados a 16 bytes
			float a[4] __attribute__((aligned(16)));
			float b[4] __attribute__((aligned(16)));
			float c[4] __attribute__((aligned(16)));
			float d[4] __attribute__((aligned(16)));

			/* Ordenamiento */
			// Se realiza según las veces correspondientes al largo
			for (int i = 0; i < veces; i++)
			{
				// Se asignan desde memblock de a 16 números en los registros
				// los cuatro primeros van a 'a', los cuatro siguientes a 'b'
				// y así hasta d, luego se repite el siglo según las veces
				for (int j = 0; j < 4; j++)
				{
					a[j] = memblock[inicio + j+0 + i*(16)];
					b[j] = memblock[inicio + j+4 + i*(16)];
					c[j] = memblock[inicio + j+8 + i*(16)];
					d[j] = memblock[inicio + j+12 + i*(16)];
				}
				/* Ordenamiento SIMD */
				// Se ejecuta el ordenamiento en los registros
				SIMD_Part(a, b, c, d);
				// Se añade el resultado ordenado al vector de secuencias
				// float 16 pasa todos los registros a un único vector
				secuencias.push_back(float16(a, b, c, d));
			}

			/* MultiWay Merge Sort	*/
			// Hace un merge de todos los vectores de 16 ordenados			
			output[mytid] = mwms(secuencias);			
		}
	}

	// Hago Merge two-way para pasar ambas listas del output a una sola
	mergedOutput = mergeTW(output[0],output[1]);

	return mergedOutput;
}




















/* MAIN */
int main(int argc, char *argv[])
{	/* Arreglo de números */
	// Vector que guarda cada secuencia ordenada por separado
	vector<vector<float>> secuencias;
	// Guarda la secuencia completa de los números ordenados
	vector<float> output;
	/* File I/O */
	// Guarda el tamaño total del archivo leído
	streampos size;
	// largo: el largo total de números flotantes del archivo
	int largo;
	// Arreglo que guarda en memoria el archivo leído en completo
	// Esto disminuye la cantidad de I/O necesarios para ordenar el archivo de entrada
	float *memblock;
	// Referencia al archivo de salida
	int file_output;
	/* Command lines */
	// Nombre del archivo de entrada
	string input_name;
	// Nombre del archivo de salida
	string output_name = "output.raw";
	// Lista de comandos disponibles en el sistema
	string command_i = "-i";
	string command_o = "-o";
	string command_N = "-N";
	string command_d = "-d";
	string command_L = "-L";
	string command_help = "-help";
	// Numero de elementos a leer del archivo de entrada
	int num_elementos = 0;	
	// Permite tener el nivel de recursividad del programa
	int nivel_recursividad = 1;	
	// Permite saber si está activado el modo debug o no
	int debug = 0;

	/* Analisis de los parametros de entrada 
		Se comprueba la validez de cada parametro */
	for (int i = 1; i < argc; i++){
		if(command_help.compare(argv[i]) == 0){
			cout << endl;
			cout << "Este programa tiene 5 parametros de entrada, de los cuales" << endl;
			cout << "uno es obligatorio, si el resto no se especifica se usarán valores por defecto." << endl;
			cout << endl;
			cout << "Comando |  Argumento                 |  Valor por defecto  " << endl;
			cout << "  -i    |  nombre archivo de entrada |                     " << endl;
			cout << "  -o    |  nombre archivo de salida  |  output.raw         " << endl;
			cout << "  -N    |  largo de la lista         |  largo total de -i  " << endl;
			cout << "  -L    |  número de recursividad    |  1                  " << endl;
			cout << "  -d    |  1 o 0                     |  0                  " << endl;
			cout << " -help  |                            |                     " << endl;
			cout << endl;
			cout << "Comando |  Descripción                                                          " << endl;
			cout << "  -i    |  Permite identificar el archivo de entrada.                           " << endl;
			cout << "        |  Debe ser un archivo binario de extension *.raw                       " << endl;
			cout << "  -o    |  Permite darle un nombre al archivo de salida                         " << endl;
			cout << "        |  Debe ser de extension *.raw                                          " << endl;
			cout << "  -N    |  Determina cuantos números hay en el archivo de entrada               " << endl;
			cout << "  -d    |  Activa o desactiva el modo debug.                                    " << endl;
			cout << "  -L    |  Define el nivel de recursividad que usará el programa para resolver  " << endl;
			cout << "        |  el problema, tal que el número de hebras será 2^nivel de recursividad" << endl;
			cout << "        |  0: No existira ningún feedback del comportamiento de la aplicación   " << endl;
			cout << "        |  1: Se imprimirá la secuencia final ordenada, un elemento por línea.  " << endl;
			cout << " -help  |  Activa esta ventana de ayuda                                         " << endl;
			cout << endl;
			return -2;
		}
	}

	if(argc <= 2){
		cout << endl;
		cout << "Debe especificar almenos el archivo de entrada con '-i nombrearchivo.raw'" << endl;
		cout << "Para más ayuda use '-help'" << endl;
		cout << endl;
		return -1;
	}else if(argc%2 == 0){
		cout << endl;
		cout << "Todo comando debe tener un argumento, si el comando no es escrito se usarán los valores por defecto" << endl;
		cout << "Para más ayuda use '-help'" << endl;
		cout << endl;
		return -1;
	}else{
		for (int i = 1; i < argc; i++){
			if(command_i.compare(argv[i]) == 0){
				input_name = argv[i+1];
				if(input_name.find(".raw") == string::npos){
					cout << endl;
					cout << "El parametro -i debe contener un archivo de extensión *.raw" << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					return -1;
				}
			}else if(command_o.compare(argv[i]) == 0){
				output_name = argv[i+1];
				if(output_name.find(".raw") == string::npos){
					cout << endl;
					cout << "El parametro -i debe contener un nombre de extensión *.raw" << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					return -1;
				}
			}else if(command_N.compare(argv[i]) == 0){
				num_elementos = atoi(argv[i+1]);
				if(num_elementos < 16){
					cout << endl;
					cout << "El parametro '-N' debe ser mayor o igual a 16." << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					return -1;
				}else if(num_elementos%16 != 0){
					cout << endl;
					cout << "El parametro '-N' debe ser multiplo de 16." << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					return -1;
				}
			}else if(command_L.compare(argv[i]) == 0){
				nivel_recursividad = atoi(argv[i+1]);
				if(nivel_recursividad <= 0){
					cout << endl;
					cout << "El parametro '-L' debe ser mayor o igual a 1." << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					return -1;
				}
			}else if(command_d.compare(argv[i]) == 0){
				debug = atoi(argv[i+1]);
				if(debug > 1 || debug < 0){
					cout << endl;
					cout << "El parametro de '-d' debe ser 0 o 1." << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					return -1;
				}
			}
			i++;
		}		
	}

	/* Reading */
	// Carga el fichero de entrada como solo lectura, modo binario y con el puntero
	// al final del archivo
	// Este algoritmo fue obtenido de: http://www.cplusplus.com/doc/tutorial/files/
	ifstream input(input_name.c_str(), ios::in|ios::binary|ios::ate);
	if(input.is_open()){
		// Como el puntero está al final del archivo es posible obtener
		// el tamaño total del archivo con este metodo
		size = input.tellg();
		// Se crea el arreglo de flotantes para guardar todos los datos
	    memblock = new float[size];
	    // Pone el puntero de lectura al comienzo del fichero
	    input.seekg (0, ios::beg);
	    // Lee todo el fichero y guarda la información en el arreglo memblock
	    input.read ((char *)memblock, size);
	    // Cierra el puntero al archivo
	    input.close();
	}else{
		cout << endl;
		cout << "No ha sido posible abrir el archivo" << endl;
		cout << "Para más ayuda use '-help'" << endl;
		cout << endl;
		return -1;
	}

	// Se determina el largo real de números flotantes
	// que tiene el fichero, esto es calculado a partir del
	// size o del parametro -N según corresponda
	if(num_elementos == 0){
		largo = (int)size/4;

		//Verifica que el largo del documento sea potencia de 16
		if(largo%16 != 0){
			cout << endl;
			cout << "El parametro '-N' debe ser multiplo de 16." << endl;
			cout << "Para más ayuda use '-help'" << endl;
			cout << endl;
			return -1;
		}
	}else{
		largo = num_elementos;
	}

	omp_set_dynamic(0);
	omp_set_nested(1);

	output = arbolDeHebras(0,nivel_recursividad, memblock, 0, largo);

	/* Output y Write output */
	// Se abre un archivo solo para escribir en él, además se le da acceso a todos
	// para leerlo o modificarlo
	file_output = open(output_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
	// Se escribe todo el output en el archivo de salida
	write(file_output, &output[0], size);
	// Se cierra el archivo
	close(file_output);

	/* Debug */
	// Si debug está activo se escribe toda la secuencia ordenada
	// con un número por línea
	if(debug == 1){
		cout << endl;
		cout << "Modo debug: secuencia final ordenada." << endl;
		for (std::vector<float>::iterator it = output.begin(); it != output.end(); ++it)
		{
			cout << *it << endl;
		}
		cout << endl;
	}

	return 0;
}