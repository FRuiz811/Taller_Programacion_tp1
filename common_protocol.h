#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H
#include <stdint.h>
#include "common_dynamic_buffer.h"

typedef struct protocol {
	uint32_t id;
	buffer_t* message;
}protocol_t;

//Retorna 0 en caso de que se haya podido crear correctamente.
//Devuelve -1 en caso de error.
int protocol_create(protocol_t* self);

uint32_t protocol_id_message(protocol_t* self);

//Realiza el encoding del string recibido en message y de longitud length.
//A su vez, devuelve la longitud del mensaje encodeado y el mensaje en si
//a través del puntero encoded.
//Retorna -1 en caso de algún error.
int protocol_encode_message(protocol_t* self, char* message, uint32_t length,
														uint8_t** encoded);

//Encargada de decodificar los primeros 16 bytes del mensaje.
//Devuelve en el array info_message, los uint32_t correspondientes a 
//longitud del cuerpo, id, y longitud del array en ese orden.
void protocol_get_info_message(protocol_t* self,uint8_t* message, 
															 uint32_t length, uint32_t info_message[]);

//Encargada de decodificar el array del mensaje.
//Devuelve en el array info_message, los strings correspondientes a cada
//una de las partes del mensaje en orden (destino, ruta, interfaz y metodo)
void protocol_decode_header(protocol_t* self, uint8_t message[],
														uint32_t length, char* info_message[]);

//Encargada de decodificar el array del mensaje.
//Devuelve en el array info_message, los strings correspondientes a cada
//uno de los parámetros que se encuentren el mensaje en el orden que sean
//leídos.
void protocol_decode_body(protocol_t* self, uint8_t message[],
													uint32_t length, char* info_message[]);

void protocol_destroy(protocol_t* self);

#endif
