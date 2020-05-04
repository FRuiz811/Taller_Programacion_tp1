#ifndef COMMON_DYNAMIC_BUFFER_H
#define COMMON_DYNAMIC_BUFFER_H
#include <stdlib.h>
#include <stdint.h>

typedef struct buffer {
	char* data;
	uint32_t length;
	uint32_t pos;
}buffer_t;


//Alloca en memoria un buffer de tamaño length y devuelve
//un puntero al struct buffer_t.
buffer_t* buffer_create(uint32_t length);

//Concatena al buffer self, los bytes ubicados a partir de data_to_add
//de longitud length.
//Devuelve 0 en caso de que se haya podido concatenar todo correctamente o
//devuelve -1 en caso de error.
int buffer_concatenate(buffer_t* self, const void* data_to_add,
					   uint32_t length);

uint32_t buffer_get_length(buffer_t* self);

void* buffer_get_data(buffer_t* self);

void buffer_destroy(buffer_t* self);

#endif
