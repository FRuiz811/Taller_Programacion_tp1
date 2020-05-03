#include "common_dynamic_buffer.h"
#include <string.h>
#include <stdio.h>


buffer_t* buffer_create(uint32_t length) {
	buffer_t* buffer = malloc(sizeof(buffer_t));

	if (buffer == NULL)
		return NULL;

	buffer->data = malloc(sizeof(char) * length);

	if (length > 0 && buffer->data == NULL) {
		free(buffer);
		return NULL;
	}

	buffer->length = length;
	return buffer;
}

int buffer_concatenate(buffer_t* self, const void* data_to_add, 
					   uint32_t length) {
	uint32_t previous_length = self->length;
	uint32_t new_lenght = sizeof(char) * (length + previous_length);
	char* new_data = realloc(self->data, new_lenght);	
	
	if (new_data == NULL)
		return -1;

	memcpy(&new_data[previous_length], data_to_add, length);
	self->data = new_data;
	self->length = length + previous_length;

	return 0;
}

void* buffer_get_data(buffer_t* self){
	return self->data;
}

uint32_t buffer_get_length(buffer_t* self) {
	return self->length;
}

void buffer_destroy(buffer_t* self) {
	free(self->data);
	free(self);
	return;
}
