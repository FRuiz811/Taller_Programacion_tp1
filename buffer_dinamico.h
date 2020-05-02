#ifndef BUFFER_DINAMICO_H
#define BUFFER_DINAMICO_H
#include <stdlib.h>

typedef struct buffer {
	char* data;
	size_t length;
}buffer_t;


//Alloca en memoria un buffer de tamaño length y devuelve
//un puntero al struct buffer_t.
buffer_t* buffer_create(size_t length);

//Concatena al buffer self, los bytes ubicado en data_to_add de longitud length
//Devuelve 0 en caso de que se haya podido concatenar todo correctamente o 
//devuelve -1 en caso de error.
int buffer_concatenate(buffer_t* self, const void* data_to_add, size_t length);

size_t buffer_get_length(buffer_t* self);

void* buffer_get_data(buffer_t* self);

void buffer_destroy(buffer_t* self);

#endif