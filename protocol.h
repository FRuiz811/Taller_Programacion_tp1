#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdio.h>
#include <stdint.h>
#include "buffer_dinamico.h"

typedef struct protocol {
	size_t id;
	buffer_t* buffer;
}protocol_t;

int protocol_create(protocol_t* self);

int protocol_encode_message(protocol_t* self, char* message, size_t length);

int protocol_decode_message(protocol_t* self, char* message, size_t length);

uint32_t protocol_id_message(protocol_t* self);

void protocol_destory(protocol_t* self);



#endif