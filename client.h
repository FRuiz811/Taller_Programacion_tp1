#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include "socket.h"
#include "protocol.h"

typedef struct client {
	socket_t socket_client;
	FILE* input;
	protocol_t protocol;
}client_t;

int client_create(client_t* self);

int client_run(client_t* self, int argc, const char* argv[]);

int client_connect_to(client_t* self, const char* host, const char* port);

int client_is_already_connected(client_t* self); //Ver si es necesaria

int client_define_input(client_t* self,int argc, const char* argv[]);

int client_process_file(client_t* self);

int client_send_message(client_t* self, const char* buffer, size_t length);

int client_recv_message(client_t* self);
 
int client_close(client_t* self);
#endif