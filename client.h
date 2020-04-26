#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include "socket.h"

typedef struct {
	socket_t socket_client;
}client_t;

int client_create(client_t* self);

int client_connect_to(client_t* self, const char* host, const char* port);

int client_is_already_connected(client_t* self);

int client_send_message(client_t* self, const char* buffer, size_t lenght);

int client_recieve_message(client_t* self, char* buffer, size_t lenght);
 
int client_close(client_t* self);
#endif