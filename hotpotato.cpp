/***********************************************
 * LAB3: MPI 1
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#include <stdlib.h>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <time.h>
#include <mpi.h>

using namespace std;
using namespace MPI;

// Retorna un vector con el vecino izquierdo y derecho de un nodo
vector<int> getVecinos(int my_rank, int world_size){
	int vecino_izq;
	int vecino_der;

	vector<int> v;

	if(my_rank == 1){
		vecino_izq = world_size;
		vecino_der = my_rank + 1;
	}else if(my_rank == world_size){
		vecino_izq = my_rank - 1;
		vecino_der = 1;
	}else{
		vecino_izq = my_rank - 1;
		vecino_der = my_rank + 1;
	}

	v.push_back(vecino_izq);
	v.push_back(vecino_der);

	return v;
}

// Permite tener una lista con todos los jugadores iniciales
vector<int> getListaJugadores(int world_size){
	vector<int> v;

	for (int i = 1; i <= world_size; i++)
	{
		v.push_back(i);
	}

	return v;
}

int main (int argc , char * argv [])
{
	int my_rank, world_size;
	int token, initial_token;

	const string command_help = "-h";
	const string command_t = "-t";

	/* Analisis de los parametros de entrada 
		Se comprueba la validez de cada parametro */
	for (int i = 1; i < argc; i++){
		if(command_help.compare(argv[i]) == 0){
			cout << endl;
			cout << "Este programa tiene 5 parametros de entrada, de los cuales" << endl;
			cout << "uno es obligatorio, si el resto no se especifica se usarán valores por defecto." << endl;
			cout << endl;
			cout << "Comando |  Argumento                 |  Valor por defecto  " << endl;
			cout << "  -t    |  Valor inicial de la papa  |  10                 " << endl;
			cout << " -help  |                            |                     " << endl;
			cout << endl;
			cout << "Comando |  Descripción                                                          " << endl;
			cout << "  -t    |  Permite asignar el valor inicial de la papa. Este debe ser positivo. " << endl;
			cout << " -help  |  Activa esta ventana de ayuda                                         " << endl;
			cout << endl;
			return -2;
		}
	}

	if(argc == 1){
		// Valores por defecto
		token = 10;
	}else if(argc%2 == 0){
		cout << endl;
		cout << "Todo comando debe tener un argumento, si el comando no es escrito se usarán los valores por defecto" << endl;
		cout << "Para más ayuda use '-help'" << endl;
		cout << endl;
		return -1;
	}else{
		for (int i = 1; i < argc; i++){
			if(command_t.compare(argv[i]) == 0){
				token = atoi(argv[i+1]);
				if(token <= 0){
					cout << endl;
					cout << "El parametro '-t' debe ser positivo." << endl;
					cout << "Para más ayuda use '-h'" << endl;
					cout << endl;
					return -1;
				}
			}
			i++;
		}		
	}

	// Guarda el valor inicial del token
	initial_token = token;

	// Inicia MPI
	Init(argc, argv);

	// Cada proceso obtiene su rank y el tamaño maximo de procesos en operación
	my_rank = COMM_WORLD.Get_rank() + 1;
 	world_size = COMM_WORLD.Get_size();

 	// Cada proceso guarda el valor de su vecino derecho e izquierdo
 	vector<int> vecino = getVecinos(my_rank, world_size);

 	// Obtiene una lista con
 	vector<int> jugadores = getListaJugadores(world_size);

 	// Barrera para sincronización
 	COMM_WORLD.Barrier();

 	// Proceso 0 envía la primera papa
 	if (my_rank == 1) {
	 	// Se envía al derecho
	 	COMM_WORLD.Send(&token, 		// Información a enviar 
	 	       	             1, 		// Tamaño de la info
	 	       		   MPI_INT, 		// Tipo de info
	 	         vecino[1] - 1,    	  	// A quién se envía la info
	 	       	             0      	// Tag usado para identificar la info
		);
 	}

	srand(time(NULL));

 	// Comienza el juego
 	while(true){
		// Recibo del izquierdo
	    COMM_WORLD.Recv(&token, 		// Información a recibir
	    			  		 1, 		// Tamaño de la info
	    			   MPI_INT, 		// Tipo de info
	    		 vecino[0] - 1,    	  	// De quién se espera recibir la info
	    			         0       	// Tag usado para identificar la info
		);

	    // Si el valor recibido es positivo entonces se ha recibido una papa
	    // Si no, se ha recibido un rank de un nodo que ha perdido
	    if(token >= 0){
		    token = token - ((rand() % initial_token) + 1);

		    // Si es mayor que 0 sigo en juego
		    // si no, he perdido
		    if(token >= 0){
		    	cout << "Proceso " << my_rank - 1 << " tiene la papa con valor " << token << endl;		    

			 	// Se envía al derecho
			 	COMM_WORLD.Send(&token, 		// Información a enviar 
			 	       	             1, 		// Tamaño de la info
			 	       		   MPI_INT, 		// Tipo de info
			 	       	 vecino[1] - 1,    	  	// A quién se envía la info
			 	       	             0      	// Tag usado para identificar la info
				);

			}else{
				cout << "Proceso " << my_rank - 1 << " tiene la papa con valor " << token << " (Proceso " << my_rank - 1 << " sale del juego)" << endl;

				// Vuelve negativo el rank para enviarlo al vecino y que se trasmita el mensaje uno a uno
				my_rank *= -1;

				// Se envía al derecho
			 	COMM_WORLD.Send(&my_rank, 		// Información a enviar 
			 	       	               1, 		// Tamaño de la info
			 	       		     MPI_INT, 		// Tipo de info
			 	       	   vecino[1] - 1,    	// A quién se envía la info
			 	       	               0      	// Tag usado para identificar la info
				);

				break;
			}
		}else{
			// Se vuelve positivo el valor obtenido para obtener el rank recibido
			token *= -1;

			int pos = std::find(jugadores.begin(), jugadores.end(), token) - jugadores.begin();
			bool starter = false;
			bool notLast = true;

			// reviso si soy el nodo izquierdo del nodo a borrar
			if((pos - 1) < 0){
				if(jugadores[jugadores.size() - 1] == my_rank){
					notLast = false;
					vecino[1] = jugadores[(pos + 1) % jugadores.size()];
				}
			}else{
				if(jugadores[(pos - 1)] == my_rank){
					notLast = false;
					vecino[1] = jugadores[(pos + 1) % jugadores.size()];
				}
			}

			// reviso si soy el nodo derecho del nodo a borrar
			if(jugadores[(pos + 1) % jugadores.size()] == my_rank){
				starter = true;
				if((pos - 1) < 0){
					vecino[0] = jugadores[jugadores.size() - 1];
				}else{
					vecino[0] = jugadores[pos - 1];
				}
			}

			// Borra el elemento
			jugadores.erase(jugadores.begin()+pos);

			// Verifica si soy el único que queda en juego
			if(jugadores.size() == 1){
				cout << "Proceso " << my_rank - 1 << " es el ganador" << endl;

				break;
			}

			// Verifica si no soy el último en la cadena broadcast
			if(notLast){
				token *= -1;

				// Se envía a la derecha
			 	COMM_WORLD.Send(&token, 		// Información a enviar 
			 	       	             1, 		// Tamaño de la info
			 	       		   MPI_INT, 		// Tipo de info
			 	         vecino[1] - 1,    		// A quién se envía la info
			 	       	             0      	// Tag usado para identificar la info
				);
			}

			// Verifica si soy el que debe comenzar la siguiente ronda
			if(starter){

				token = initial_token;

				// Se envía al derecho
			 	COMM_WORLD.Send(&token, 		// Información a enviar 
			 	       	             1, 		// Tamaño de la info
			 	       		   MPI_INT, 		// Tipo de info
			 	         vecino[1] - 1,    	  	// A quién se envía la info
			 	       	             0      	// Tag usado para identificar la info
				);

			}
		}
	}

 	Finalize();

 	return 0;
 }