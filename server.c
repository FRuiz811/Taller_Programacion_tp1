#include "server.h"
#include <stdio.h>

#define SOCKET_WAITING 20

int server_create(server_t* self) {
	socket_t socket;
	socket_create(&socket);
	self->socket_server = socket;
	return 0;
}

int server_is_already_connected(server_t* self) {
	return socket_is_connected(&(self->socket_server));
}

int server_connect_to(server_t* self, const char* port) {
	if(server_is_already_connected(self))
		return -1;
	int connection_success = socket_bind_and_listen(&(self->socket_server), port, SOCKET_WAITING);
	return connection_success;
}


int server_send_message(server_t* self, const char* buffer, size_t lenght) {
	
}

int server_close(server_t* self) {
	socket_shutdown(&(self->socket_server), SHUT_RDWR);
	if(socket_destroy(&(self->socket_server)) == -1) 
		return -1;
	return 0;
}