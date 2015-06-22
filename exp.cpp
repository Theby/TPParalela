/***********************************************
 * LAB4: uC++
 * Desarrollado por: Esteban Gaete Flores
 * USACH - Ramo: Taller de Programación Paralela
 ***********************************************/

#include "productor.hpp"
#include "reconocedor.hpp"
#include "escritor.hpp"
#include <string>
#include <iostream>

using namespace std;

void uMain::main(){		
	
	// Lista de comandos disponibles en el sistema
	string command_i = "-i";
	string command_o = "-o";
	string command_h = "-h";
	string command_L = "-L";
	string command_l = "-l";
	string command_help = "-help";
	// Nombre del archivo de entrada
	string inputFile = "";
	// Nombre del archivo de salida
	string outputFile = "output.txt";
	// Número de tareas reconocedoras
	int numero_tareas = 1;
	// Largo del primero buffer
	int largo_buffer1 = 10;
	// Largo del segundo buffer
	int largo_buffer2 = 10;
	// Para verificar si el proceso de comandos salío bien
	bool command_success = true;

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
			cout << "  -o    |  nombre archivo de salida  |  output.txt         " << endl;
			cout << "  -h    |  número de tareas          |  1                  " << endl;
			cout << "  -L    |  largo del buffer 1        |  10                 " << endl;
			cout << "  -l    |  largo del buffer 2        |  10                 " << endl;
			cout << " -help  |                            |                     " << endl;
			cout << endl;
			cout << "Comando |  Descripción                                                          " << endl;
			cout << "  -i    |  Permite identificar el archivo de entrada.                           " << endl;
			cout << "        |  Debe ser un archivo *.txt                							 " << endl;
			cout << "  -o    |  Permite darle un nombre al archivo de salida                         " << endl;
			cout << "        |  Debe ser un archivo *.txt                                            " << endl;
			cout << "  -h    |  Determina el número de tareas que ejecutarán tareas reconocedoras    " << endl;
			cout << "  -L    |  Determina el largo del buffer 1                                      " << endl;
			cout << "  -l    |  Determinar el largo del buffer 2 									 " << endl;
			cout << " -help  |  Activa esta ventana de ayuda                                         " << endl;
			cout << endl;
			command_success = false;
		}
	}

	if(argc <= 2){
		cout << endl;
		cout << "Debe especificar almenos el archivo de entrada con '-i nombrearchivo.txt'" << endl;
		cout << "Para más ayuda use '-help'" << endl;
		cout << endl;
		command_success = false;
	}else if(argc%2 == 0){
		cout << endl;
		cout << "Todo comando debe tener un argumento, si el comando no es escrito se usarán los valores por defecto" << endl;
		cout << "Para más ayuda use '-help'" << endl;
		cout << endl;
		command_success = false;
	}else{
		for (int i = 1; i < argc; i++){
			if(command_i.compare(argv[i]) == 0){
				inputFile = argv[i+1];
				if(inputFile.find(".txt") == string::npos){
					cout << endl;
					cout << "El parametro -i debe contener un archivo de extensión *.txt" << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					command_success = false;
				}
			}else if(command_o.compare(argv[i]) == 0){
				outputFile = argv[i+1];
				if(outputFile.find(".txt") == string::npos){
					cout << endl;
					cout << "El parametro -i debe contener un nombre de extensión *.txt" << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					command_success = false;
				}
			}else if(command_h.compare(argv[i]) == 0){
				numero_tareas = atoi(argv[i+1]);
				if(numero_tareas < 1){
					cout << endl;
					cout << "El parametro '-h' debe ser mayor o igual a 1." << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					command_success = false;
				}
			}else if(command_L.compare(argv[i]) == 0){
				largo_buffer1 = atoi(argv[i+1]);
				if(largo_buffer1 < 1){
					cout << endl;
					cout << "El parametro '-L' debe ser mayor o igual a 1." << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					command_success = false;
				}
			}else if(command_l.compare(argv[i]) == 0){
				largo_buffer2 = atoi(argv[i+1]);
				if(largo_buffer2 < 1){
					cout << endl;
					cout << "El parametro '-l' debe ser mayor o igual a 1." << endl;
					cout << "Para más ayuda use '-help'" << endl;
					cout << endl;
					command_success = false;
				}
			}
			i++;
		}
	}

	if(command_success){
		InputBuffer inputBuffer(largo_buffer1);
		OutputBuffer outputBuffer(largo_buffer2);
		Reconocedor *reconocedores[numero_tareas];

		Productor *productor = new Productor(inputBuffer, inputFile);
		for (int i = 0; i < numero_tareas; i++)
		{
			reconocedores[i] = new Reconocedor(inputBuffer, outputBuffer, i);			
		}
		Escritor *escritor = new Escritor(outputBuffer, outputFile);

		delete productor;
		for (int i = 0; i < numero_tareas; i++)
		{
			delete reconocedores[i];
		}
		delete escritor;
	}
}