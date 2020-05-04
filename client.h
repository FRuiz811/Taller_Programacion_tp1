#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include "common_socket.h"
#include "common_protocol.h"

typedef struct client {
	socket_t socket_client;
	FILE* input;
	protocol_t protocol;
}client_t;

//Retorna -1 en caso de error, 0 en caso contrario.
int client_create(client_t* self);

int client_run(client_t* self, int argc, const char* argv[]);

//Devuelve -1 en caso de no poderse conectar y 0 en caso de éxito
int client_connect_to(client_t* self, const char* host, const char* port);

//Dependiendo de si se ingresó como parámetro o no, selecciona ese archivo 
//o la entrada estándar como archivo de entrada.
//Devuelve -1 en caso de error, 0 en caso contrario.
int client_define_input(client_t* self,int argc, const char* argv[]);

//Función encargada de realizar el procesamiento del archivo de entrada
//y de enviarlo luego de que se procesa una línea y esta haya sido codificada
//Retorna -1 en caso de error, 0 en caso contrario.
int client_process_file(client_t* self);

int client_send_message(client_t* self, const uint8_t* buffer, uint32_t length);

//Recibe la confirmación del servidor y la imprime en pantalla.
//Retorna -1 en caso de error, 0 en caso contrario.
int client_recv_message(client_t* self);
 
//Realiza el shutdown de los 2 canales de comunicación con el server
//Libera la memoria utilizada.
void client_close(client_t* self);

#endif
