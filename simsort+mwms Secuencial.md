	/* OpenMP */
	num_hebras = (int)pow(2, nivel_recursividad);

	omp_set_num_threads(num_hebras);
	#pragma omp parallel firstprivate(veces, num_hebras) private(a , b, c, d, mytid, n, k) shared(memblock)
	{
		/* Variables de control */		
		mytid = omp_get_thread_num();
		n = veces/num_hebras;
		k = mytid * n;

		/* Ordenamiento */
		// Se realiza según las veces especificadas
		for (int i = k; i < n + k; i++)
		{
			// Se asignan desde memblock de a 16 números en los registros
			// los cuatro primeros van a 'a', los cuatro siguientes a 'b'
			// y así hasta d, luego se repite el siglo según las veces
			for (int j = 0; j < 4; j++)
			{
				a[j] = memblock[j+0 + i*(16)];
				b[j] = memblock[j+4 + i*(16)];
				c[j] = memblock[j+8 + i*(16)];
				d[j] = memblock[j+12 + i*(16)];
			}

			/* Ordenamiento SIMD */
			// Se ejecuta el ordenamiento en los registros
			SIMD_Part(a, b, c, d);

			// Se añade el resultado ordenado al vector de secuencias
			// float 16 pasa todos los registros a un único vector
			secuencias.push_back(float16(a, b, c, d));
		}	
	}

	/* MultiWay Merge Sort	*/
	// Hace un merge de todos los vectores de 16 ordenados
	output = mwms(secuencias);