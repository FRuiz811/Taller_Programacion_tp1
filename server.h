#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include "common_socket.h"
#include "common_protocol.h"

typedef struct server {
	socket_t socket_server;
	socket_t socket_communicator;
	protocol_t protocol;
}server_t;

//Retorna -1 en caso de error, 0 en caso contrario.
int server_create(server_t* self);

int server_run(server_t* self, const char* argv[]);

//Devuelve -1 en caso de no poderse conectar y 0 en caso de éxito
int server_connect_to(server_t* self, const char* port);

//Setea el socket_communicator con el socket correspondiente
//que permitirá la comunicación con el cliente.
//Retorna -1 en caso de error, 0 en caso contrario.
int server_accept_connection(server_t* self);

int server_send_message(server_t* self, const void* buffer, uint32_t length);

//Retorna -1 en caso de error, 0 en caso contrario.
int server_recv_message(server_t* self, void* buffer, uint32_t length);

//Realiza el shutdown de los 2 canales de comunicación con el cliente
//Libera la memoria utilizada.
void server_close(server_t* self);

#endif
