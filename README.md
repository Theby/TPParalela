# TPParalela
Repositorio para el taller de programación paralela de la USACH, impartido por el profesor Fernando Rannou

uC++ se ejecuta con:

 * ./exp -i in.txt -o out.txt -h número_de_tareas -L largo_buffer1 -l largo_buffer2

El archivo de entrada contendrá un string por línea. El número de tareas indicado es el número de tareas reconocedoras. Las opciones -L y -l son opcionales e indican los largos de los buffers.
Estos largos se refieren al número máximo de strings que pueden almacenar, no al número máximo de caracteres. Por defecto, los buffers tendrán largo 10. Ambos buffers deben ser implementados como monitores.