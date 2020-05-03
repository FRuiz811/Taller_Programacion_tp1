#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H
#include <stdio.h>
#include <stdint.h>
#include "common_dynamic_buffer.h"

typedef struct protocol {
	uint32_t id;
	buffer_t* message;
}protocol_t;

int protocol_create(protocol_t* self);

int protocol_encode_message(protocol_t* self, char* message, uint32_t length);

void protocol_get_info_message(protocol_t* self,uint8_t* message, 
															 uint32_t length, uint32_t info_message[]);

void protocol_decode_header(protocol_t* self, uint8_t message[],
														uint32_t length, char* info_message[]);

void protocol_decode_body(protocol_t* self, uint8_t message[],
													uint32_t length, char* info_message[]);

uint32_t protocol_id_message(protocol_t* self);

int protocol_get_message_encoded(protocol_t* self, uint8_t** message);

void protocol_free_message(protocol_t* self, uint8_t* message);

void protocol_destroy(protocol_t* self);

#endif
