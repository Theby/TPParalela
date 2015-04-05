#include <stdio.h>
//#include <emmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <cstdint>

// gcc -msse3 -msse4 simsort.cpp -std=c++11

void printf_register(int32_t* R){
	printf("(%i, %i, %i, %i)\n", R[0], R[1], R[2], R[3]);
}

// Intercambia los valores intermedios del arreglo
__m128 swapMiddle(__m128 R){
	return _mm_shuffle_ps(R, R, _MM_SHUFFLE(3, 1, 2, 0));
}

// Intercambia los valores intermedios entre dos arreglos
__m128 dobleSwapMiddle(__m128 R1, __m128 R2){
	return _mm_shuffle_ps(R1, R2, _MM_SHUFFLE(3, 1, 2, 0));
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

void ordenamientoInRegister(__m128& Af, __m128& Bf, __m128& Cf, __m128& Df){
	__m128 Pista1, Pista1_aux1, Pista1_aux2;
	__m128 Pista2, Pista2_aux1, Pista2_aux2;
	__m128 Pista3, Pista3_aux1, Pista3_aux2;
	__m128 Pista4, Pista4_aux1, Pista4_aux2;

	// Generar pistas
	Pista1_aux1 = _mm_shuffle_ps(Af, Cf, _MM_SHUFFLE(1, 0, 1, 0));
	Pista1_aux2 = _mm_shuffle_ps(Bf, Df, _MM_SHUFFLE(0, 1, 0, 1));
	Pista1 = dobleSwapMiddle(Pista1_aux1, Pista1_aux2);
	Pista1 = swapMiddle(Pista1);

	Pista2_aux1 = _mm_shuffle_ps(Af, Cf, _MM_SHUFFLE(1, 1, 1, 1));
	Pista2_aux2 = _mm_shuffle_ps(Bf, Df, _MM_SHUFFLE(1, 1, 1, 1));
	Pista2 = dobleSwapMiddle(Pista2_aux1, Pista2_aux2);
	Pista2 = swapMiddle(Pista2);

	Pista3_aux1 = _mm_shuffle_ps(Af, Cf, _MM_SHUFFLE(1, 2, 1, 2));
	Pista3_aux2 = _mm_shuffle_ps(Bf, Df, _MM_SHUFFLE(2, 1, 2, 1));
	Pista3 = dobleSwapMiddle(Pista3_aux1, Pista3_aux2);
	Pista3 = swapMiddle(Pista3);

	Pista4_aux1 = _mm_shuffle_ps(Af, Cf, _MM_SHUFFLE(1, 3, 1, 3));
	Pista4_aux2 = _mm_shuffle_ps(Bf, Df, _MM_SHUFFLE(3, 1, 3, 1));
	Pista4 = dobleSwapMiddle(Pista4_aux1, Pista4_aux2);
	Pista4 = swapMiddle(Pista4);

	// MINMAX NETOWRK
	// Ordena los registros usando una red minmax
	Af = minmaxNetwork(Pista1);
	Bf = minmaxNetwork(Pista2);
	Cf = minmaxNetwork(Pista3);
	Df = minmaxNetwork(Pista4);
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

// Invierte el orden de un registro
__m128 invertir(__m128 R){
	return _mm_shuffle_ps(R, R, _MM_SHUFFLE(0, 1, 2, 3));
}

// Entrega un registro que tiene el primer digito repetido en todas sus posiciones
__m128 primeroRepetido(__m128 R){
	return _mm_shuffle_ps(R, R, _MM_SHUFFLE(0, 0, 0, 0));
}

// Ejecuta mergeSIMD sobre cuatro arreglos (16 números)
void mergeSIMD(__m128& Af, __m128& Bf, __m128& Cf, __m128& Df){
	__m128 O1, O2, O2_menor, O2_mayor;
	__m128 comp;

	// Se cargan Af y Cf en una BMN (invirtiendo Cf) para obtener dos registros O1 y O2
	O1 = Af;
	O2 = Cf;

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

int main(){
	__m128i A, B, C, D;
	__m128 Af, Bf, Cf, Df;

	//INPUT
	// TODO: leer de un archivo binario
	// Arreglos de prueba alineados a 16 bytes
	int32_t a[4] __attribute__((aligned(16))) = { 12, 21, 4, 13} ;
	int32_t b[4] __attribute__((aligned(16))) = { 9, 8, 6, 7 };
	int32_t c[4] __attribute__((aligned(16))) = { 1, 14, 3, 0 };
	int32_t d[4] __attribute__((aligned(16))) = { 5, 11, 15, 10 };

	// Carga los registros
	A = _mm_set_epi32(a[3], a[2], a[1], a[0]);
	B = _mm_set_epi32(b[3], b[2], b[1], b[0]);
	C = _mm_set_epi32(c[3], c[2], c[1], c[0]);
	D = _mm_set_epi32(d[3], d[2], d[1], d[0]);


	// Los pasa a registros de punto flotante
	Af = _mm_castsi128_ps(A);
	Bf = _mm_castsi128_ps(B);
	Cf = _mm_castsi128_ps(C);
	Df = _mm_castsi128_ps(D);

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


	// MultiWay Merge Sort
	// No usa SIMD
	// Buscar el menor de todas las listas (de 16) comparando el primer número
	// Se saca este valor final y se agrega a la lista final (output)
	// Repetir hasta vaciar todas las listas


	//OUTPUT
	// Pasa los registros a enteros
	A = _mm_castps_si128(Af);
	B = _mm_castps_si128(Bf);
	C = _mm_castps_si128(Cf);
	D = _mm_castps_si128(Df);

	// Guarda los registros
	_mm_store_si128((__m128i*)&a, A);
	_mm_store_si128((__m128i*)&b, B);
	_mm_store_si128((__m128i*)&c, C);
	_mm_store_si128((__m128i*)&d, D);

	// Imprime los registros en consola
	printf_register(a);
	printf_register(b);
	printf_register(c);
	printf_register(d);
}

