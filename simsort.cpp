#include <vector>
#include <iostream>
#include <smmintrin.h>
#include <fstream>
#include <ostream>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <algorithm>
#include <iterator>
#include <functional>
#include <stdlib.h> 

// g++ -msse3 -msse4 simsort.cpp -std=c++11
// Gprof: g++ -pg -msse3 -msse4 simsort.cpp -std=c++11
//        gprof -p a.out gmon.out > analysis.txt

using namespace std;

void printf_register(vector<float> R){
	cout << "(";
	for (std::vector<float>::iterator i = R.begin(); i != R.end(); ++i)
	{
		cout << " " << *i;
	}
	cout << ")\n";
}

void printf_debug(__m128 R){
	printf("(%5.3f, %5.3f, %5.3f, %5.3f)\n", R[0], R[1], R[2], R[3]);
}

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
__m128 middleMinMax(__m128 R){
	__m128 aux_r;

	aux_r = _mm_movehdup_ps(R);
	return _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(2, 0, 2, 0));
}

// Realiza una MinMax Network
__m128 minmaxNetwork(__m128 R){
	__m128 aux_r;

	aux_r = _mm_shuffle_ps(R,R, _MM_SHUFFLE(3, 2, 3, 2));
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(1, 0, 1, 0));

	R = middleMinMax(R);

	aux_r = swapMiddle(R);
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(3, 2, 1, 0));

	return R;
}

// Compara dos registros dejando los valores menores menores en R1 y los mayores en R2
// comparano por columna
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

// Realiza una MinMax Network entre cuatro registros en ves de cuatro números de un solo registro
__m128 minmaxNetwork_R(__m128& Af, __m128& Bf, __m128& Cf, __m128& Df){

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

void ordenamientoInRegister(__m128& Af, __m128& Bf, __m128& Cf, __m128& Df){

	// MinMax entre los cuatro registros
	// Quedan ordenados por columa
	minmaxNetwork_R(Af, Bf, Cf, Df);

	// Redshuffle
	// La primera columna ahora es la primera fila y así con cada columna
	_MM_TRANSPOSE4_PS(Af, Bf, Cf, Df);
}

// Ejecuta una Bionic Merge Network
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

void SIMD_Part(float* a, float* b, float* c, float* d){
	__m128 Af, Bf, Cf, Df;

	// Los pasa a registros de punto flotante
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

void Push(vector<float>& heap, float val) {
    heap.push_back(val);
    push_heap(heap.begin(), heap.end(), greater<float>());
}

float Pop(vector<float>& heap) {
    float val = heap.front();
     
    //This operation will move the smallest element to the end of the vector
    pop_heap(heap.begin(), heap.end(), greater<float>());
 
    //Remove the last element from vector, which is the smallest element
    heap.pop_back();


    return val;
}

vector<float> mwms(vector<vector<float>> secuencias){
	vector<float> output;
	/*float menor = 0;
	int vector_id = 0;

	while(!secuencias.empty()){	
		
		menor = secuencias[0][0];
		
		for (int i = 0; i < secuencias.size(); i++)
		{

			if(!secuencias[i].empty()){
				vector<float> aux_v = secuencias[i];

				if (menor >= aux_v[0])
				{
					menor = aux_v[0];
					vector_id = i;
				}
			}else{				
				secuencias.erase(secuencias.begin()+i);
			}
		}

		output.push_back(menor);
		secuencias[vector_id].erase(secuencias[vector_id].begin());
		if(secuencias[vector_id].empty()){
			secuencias.erase(secuencias.begin()+vector_id);
		}
	}*/

	vector<float> minHeap;
	for (int j = 0; j < secuencias.size(); j++){
		for (int i = 0; i < secuencias[j].size(); i++)
		{
			Push(minHeap, secuencias[j][i]);
		}
	}

	while(!minHeap.empty()){
		output.push_back(Pop(minHeap));
	}


	return output;
}











int main(int argc, char *argv[]){
	/* Arreglo de números */
	// Vector que guarda cada secuencia ordenada por separado
	vector<vector<float>> secuencias;
	// Guarda la secuencia completa de los números ordenados
	vector<float> output;
	/* File I/O */
	// Guarda el tamaño total del archivo leído
	streampos size;
	// largo: el largo total de números flotantes del archivo
	// veces: número de veces que se cargaran 16 flotantes y que se ejecutará
	// toda la secuencia SIMD completa
	int largo,veces;
	// Arreglo que guarda en memoria el archivo leído en completo
	// Esto disminuye la cantidad de I/O necesarios para ordenar el archivo de entrada
	float * memblock;
	// Referencia al archivo de salida
	int file_output;
	/* Command lines */
	string input_name;
	string output_name = "output.raw";
	string command_i = "-i";
	string command_o = "-o";
	string command_N = "-N";
	string command_d = "-d";
	string command_h = "-h";
	int num_elementos = 0;
	int debug = 0;

	/* Analisis de los parametros de entrada */
	for (int i = 1; i < argc; i++){
		if(command_h.compare(argv[i]) == 0){
			cout << endl;
			cout << "Este programa tiene 5 parametros de entrada, de los cuales" << endl;
			cout << "uno es obligatorio, si el resto no se especifica se usarán valores por defecto." << endl;
			cout << endl;
			cout << "Comando |  Argumento                 |  Valor por defecto  " << endl;
			cout << "  -i    |  nombre archivo de entrada |                     " << endl;
			cout << "  -o    |  nombre archivo de salida  |  output.raw         " << endl;
			cout << "  -N    |  largo de la lista         |  largo total de -i  " << endl;
			cout << "  -d    |  1 o 0                     |  0                  " << endl;
			cout << "  -h    |                            |                     " << endl;
			cout << endl;
			cout << "Comando |  Descripción                                                          " << endl;
			cout << "  -i    |  Permite identificar el archivo de entrada.                           " << endl;
			cout << "        |  Debe ser un archivo binario de extension *.raw                       " << endl;
			cout << "  -o    |  Permite darle un nombre al archivo de salida                         " << endl;
			cout << "        |  Debe ser de extension *.raw                                          " << endl;
			cout << "  -N    |  Determina cuantos números hay en el archivo de entrada               " << endl;
			cout << "  -d    |  Activa o desactiva el modo debug.                                    " << endl;
			cout << "        |  0: No existira ningún feedback del comportamiento de la aplicación   " << endl;
			cout << "        |  1: Se imprimirá la secuencia final ordenada, un elemento por línea.  " << endl;
			cout << "  -h    |  Activa esta ventana de ayuda                                         " << endl;
			cout << endl;
			return -2;
		}
	}

	if(argc <= 2){
		cout << endl;
		cout << "Debe especificar almenos el archivo de entrada con '-i nombrearchivo.raw'" << endl;
		cout << "Para más ayuda use '-h'" << endl;
		cout << endl;
		return -1;
	}else if(argc%2 == 0){
		cout << endl;
		cout << "Todo comando debe tener un argumento, si el comando no es escrito se usarán los valores por defecto" << endl;
		cout << "Para más ayuda use '-h'" << endl;
		cout << endl;
		return -1;
	}else{
		for (int i = 1; i < argc; i++)
		{
			if(command_i.compare(argv[i]) == 0){
				input_name = argv[i+1];
				if(input_name.find(".raw") == string::npos){
					cout << endl;
					cout << "El parametro -i debe contener un archivo de extensión *.raw" << endl;
					cout << "Para más ayuda use '-h'" << endl;
					cout << endl;
					return -1;
				}
			}else if(command_o.compare(argv[i]) == 0){
				output_name = argv[i+1];
				if(output_name.find(".raw") == string::npos){
					cout << endl;
					cout << "El parametro -i debe contener un nombre de extensión *.raw" << endl;
					cout << "Para más ayuda use '-h'" << endl;
					cout << endl;
					return -1;
				}
			}else if(command_N.compare(argv[i]) == 0){
				num_elementos = atoi(argv[i+1]);
				if(num_elementos <= 0){
					cout << endl;
					cout << "El parametro '-n' debe ser mayor o igual a 1." << endl;
					cout << "Para más ayuda use '-h'" << endl;
					cout << endl;
					return -1;
				}else if(num_elementos != 1 && num_elementos != 2 && num_elementos != 4 && num_elementos != 8){
					if(num_elementos%16 != 0){
						cout << endl;
						cout << "El parametro '-n' debe ser potencia de 2." << endl;
						cout << "Para más ayuda use '-h'" << endl;
						cout << endl;
						return -1;
					}
				}
			}else if(command_d.compare(argv[i]) == 0){
				debug = atoi(argv[i+1]);
				if(debug > 1 || debug < 0){
					cout << endl;
					cout << "El parametro de '-d' debe ser 0 o 1." << endl;
					cout << "Para más ayuda use '-h'" << endl;
					cout << endl;
					return -1;
				}
			}
			i++;
		}		
	}

	/* Reading */
	ifstream input(input_name.c_str(), ios::in|ios::binary|ios::ate);
	if(input.is_open()){
		size = input.tellg();
	    memblock = new float[size];
	    input.seekg (0, ios::beg);
	    input.read ((char *)memblock, size);
	    input.close();
	}else{
		cout << endl;
		cout << "No ha sido posible abrir el archivo" << endl;
		cout << "Para más ayuda use '-h'" << endl;
		cout << endl;
		return -1;
	}

	if(num_elementos == 0){
		largo = (int)size/4;
	}else{
		largo = num_elementos;
	}
	veces = largo/16;	

	float a[4] __attribute__((aligned(16)));
	float b[4] __attribute__((aligned(16)));
	float c[4] __attribute__((aligned(16)));
	float d[4] __attribute__((aligned(16)));

	for (int i = 0; i < veces; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			a[j] = memblock[j+0 + i*(16)];
			b[j] = memblock[j+4 + i*(16)];
			c[j] = memblock[j+8 + i*(16)];
			d[j] = memblock[j+12 + i*(16)];
		}

		// Ordena a nivel de procesador
		SIMD_Part(a, b, c, d);

		// Se añade el resultado
		secuencias.push_back(float16(a, b, c, d));
	}	

	// MultiWay Merge Sort	
	output = mwms(secuencias);	

	//OUTPUT FILE
	file_output = open(output_name.c_str(), O_WRONLY | O_CREAT, S_IRWXO | S_IRWXG | S_IRWXU);
	write(file_output, &output[0], size);
	close(file_output);

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