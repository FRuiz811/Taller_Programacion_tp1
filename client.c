#include "client.h"
#include <stdio.h>

int client_create(client_t* self) {
	socket_t socket;
	socket_create(&socket);
	self->socket_client = socket;
	return 0;
}

int client_is_already_connected(client_t* self) {
	return socket_is_connected(&(self->socket_client));
}

int client_connect_to(client_t* self, const char* host, const char* port) {
	if(client_is_already_connected(self))
		return -1;
	int connection_success = socket_connect(&(self->socket_client), host, port);
	return connection_success;
}


int client_send_message(client_t* self, const char* buffer, size_t lenght) {

}

int client_close(client_t* self) {
	socket_shutdown(&(self->socket_client), SHUT_RDWR);
	if(socket_destroy(&(self->socket_client)) == -1) 
		return -1;
	return 0;
}