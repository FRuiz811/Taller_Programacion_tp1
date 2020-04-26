#include "server.h"
#include <stdio.h>
#include <sys/socket.h>

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

int server_run(server_t* self, const char* argv[]) {
	if(server_connect_to(self, argv[1]) == -1)
		return -1;
	while(true){
		
	}
}

int server_recv_message(server_t* self, char* buffer, size_t lenght) {
	
}

int server_close(server_t* self) {
	socket_shutdown(&(self->socket_server), SHUT_RDWR);
	if(socket_destroy(&(self->socket_server)) == -1) 
		return -1;
	return 0;
}