#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include "socket.h"

typedef struct server {
	socket_t socket_server;
	socket_t socket_communicator;
}server_t;

int server_create(server_t* self);

int server_run(server_t* self, const char* argv[]);

int server_connect_to(server_t* self, const char* port);

int server_is_already_connected(server_t* self);

int server_accept_connection(server_t* self);

int server_send_message(server_t* self, const char* buffer, size_t length);

int server_recv_message(server_t* self, char* buffer, size_t length);
 
int server_close(server_t* self);
#endif