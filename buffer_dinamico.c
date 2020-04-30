#include "buffer_dinamico.h"
#include <string.h>
#include <stdio.h>


buffer_t* buffer_create(size_t length) {
	buffer_t* buffer = malloc(sizeof(buffer_t));

	if (buffer == NULL)
		return NULL;

	buffer->string = malloc(sizeof(char) * length);

	if (length > 0 && buffer->string == NULL) {
		free(buffer);
		return NULL;
	}

	buffer->length = length;
	return buffer;
}

int buffer_concatenate(buffer_t* self, const char* new_data, size_t length) {
	size_t previous_length = self->length;
	size_t new_lenght = sizeof(char) * (length + previous_length);
	char* new_string = realloc(self->string, new_lenght);	
	
	if (new_string == NULL)
		return -1;

	memcpy(&new_string[previous_length], new_data, length);
	self->string = new_string;
	self->length = length + previous_length;

	return 0;
}

void buffer_destroy(buffer_t* self) {
	free(self->string);
	free(self);
	return;
}