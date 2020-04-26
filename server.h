#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include "socket.h"

typedef struct {
	socket_t socket_server;
}server_t;

int server_create(server_t* self);

int server_connect_to(server_t* self, const char* port);

int server_is_already_connected(server_t* self);

int server_send_message(server_t* self, const char* buffer, size_t lenght);

int server_recieve_message(server_t* self, char* buffer, size_t lenght);
 
int server_close(server_t* self);
#endif