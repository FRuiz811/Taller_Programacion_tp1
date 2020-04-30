#ifndef BUFFER_DINAMICO_H
#define BUFFER_DINAMICO_H
#include <stdlib.h>

typedef struct buffer {
	char* string;
	size_t length;
}buffer_t;


//Alloca en memoria un buffer de tamaño length y devuelve
//un puntero al struct buffer_t.
buffer_t* buffer_create(size_t length);

//Concatena al buffer self, el string ubicado en new_data de longitud length
//Devuelve 0 en caso de que se haya podido concatenar todo correctamente o 
//devuelve -1 en caso de error.
int buffer_concatenate(buffer_t* self, const char* new_data, size_t length);


void buffer_destroy(buffer_t* self);

#endif