#include <stdio.h>
//#include <emmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <cstdint>

void printf_register(int32_t* R){
	printf("(%i, %i, %i, %i)\n", R[0], R[1], R[2], R[3]);
}

__m128 minmax_network(__m128 R){
	__m128 aux_r;

	aux_r = _mm_shuffle_ps(R,R, _MM_SHUFFLE(3, 2, 3, 2));
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(1, 0, 1, 0));

	aux_r = _mm_movehdup_ps(R);	
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(2, 0, 2, 0));

	aux_r = _mm_shuffle_ps(R, R, _MM_SHUFFLE(3, 1, 2, 0));
	R = _mm_shuffle_ps(_mm_min_ps(R,aux_r), _mm_max_ps(R,aux_r), _MM_SHUFFLE(3, 2, 1, 0));

	return R;
}

__m128 invertir(__m128 R){
	return _mm_shuffle_ps(R, R, _MM_SHUFFLE(0, 1, 2, 3));
}

int main(){

	__m128i A, B;
	__m128 Af, Bf, Cf;

	//INPUT
	// Arreglos de prueba alineados a 16 bytes
	int32_t a[4] __attribute__((aligned(16))) = { 8, 9, 9, 10};
	int32_t b[4] __attribute__((aligned(16))) = { 3, 5, 12, 7 };

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

	// Invierte B
	Bf = invertir(Bf);


	// BMN
	// OBJ: Ordenar 8 digitos
	// TODO: crear dos arreglos con la mescla(1:1) y cada uno meterlo a minmax network
	// Crear dos arreglos con el resultado del minmax tal que sea (1:1)
	// Aplicar un min max a cada arreglo
	// Reordenar los dos arreglos para que queden ordenados



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

