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

// Realiza el paso intermedio de una red minmax
__m128 middleMinMax(__m128 R){
	__m128 aux_r;

	aux_r = _mm_movehdup_ps(R);
	return _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(2, 0, 2, 0));
}

// Realiza una MinMax Network
__m128 minmax_network(__m128 R){
	__m128 aux_r;

	aux_r = _mm_shuffle_ps(R,R, _MM_SHUFFLE(3, 2, 3, 2));
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(1, 0, 1, 0));

	R = middleMinMax(R);

	aux_r = swapMiddle(R);
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(3, 2, 1, 0));

	return R;
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
	Af_bmn = minmax_network(Af_bmn);
	Bf_bmn = minmax_network(Bf_bmn);

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

int main(){

	__m128i A, B;
	__m128 Af, Bf;

	//INPUT
	// Arreglos de prueba alineados a 16 bytes
	int32_t a[4] __attribute__((aligned(16))) = { 5, 20, 25, 30};
	int32_t b[4] __attribute__((aligned(16))) = { 2, 7, 9, 18 };

	// Carga los registros
	A = _mm_set_epi32(a[3], a[2], a[1], a[0]);
	B = _mm_set_epi32(b[3], b[2], b[1], b[0]);

	// Los pasa a registros de punto flotante
	Af = _mm_castsi128_ps(A);
	Bf = _mm_castsi128_ps(B);
	
	// MINMAX NETOWRK
	// Ordena los registros usando una red minmax
	Af = minmax_network(Af);
	Bf = minmax_network(Bf);

	// Invierte B para preparar la BMN
	Bf = invertir(Bf);

	// BMN
	// Se alteran los valores de Af y Bf al pasar el valor como referencia
	BMN(Af, Bf);	

	// IN-REGISTER
	// OBJ: ordenar 16 digitos
	


	//OUTPUT
	// Pasa los registros a enteros
	A = _mm_castps_si128(Af);
	B = _mm_castps_si128(Bf);

	// Guarda los registros
	_mm_store_si128((__m128i*)&a, A);
	_mm_store_si128((__m128i*)&b, B);

	// Imprime los registros en consola
	printf_register(a);
	printf_register(b);
}

