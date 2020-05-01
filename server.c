#include "server.h"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

#define SOCKET_WAITING 20
#define BUFFER_SIZE 71	
#define MSG_RECV "OK\n"

int server_create(server_t* self) {
	socket_t socket;
	socket_create(&socket);
	self->socket_server = socket;
	self->socket_communicator = NULL;
	
	protocol_t protocol;
	if (protocol_create(&protocol) == -1)
		return -1;
	self->protocol = protocol;
	return 0;
}

int server_is_already_connected(server_t* self) {
	return socket_is_connected(&(self->socket_server));
}

int server_connect_to(server_t* self, const char* port) {
	if (server_is_already_connected(self))
		return -1;
	int connection_success = socket_bind_and_listen(&(self->socket_server), port, SOCKET_WAITING);
	return connection_success;
}

int server_accept_connection(server_t* self) {
	socket_t communicator;
	socket_create(&communicator);
	if (socket_accept(&(self->socket_server),&communicator) == -1) {
		fprintf(stderr, "No se pudo aceptar ninguna nueva conexión\n");
		return -1;
	}
	self->socket_communicator = communicator;
	return 0;
}


int server_run(server_t* self, const char* argv[]) {
	if (server_connect_to(self, argv[1]) == -1)
		return -1;
	if (server_accept_connection(self) == -1)
		return -1;
	char buffer[BUFFER_SIZE];
	while (server_recv_message(self, buffer, BUFFER_SIZE) > 0) {
		printf("Buffer recibido: %s\n", buffer);
		memset(buffer,0,BUFFER_SIZE);
		server_send_message(self, MSG_RECV, 3);
	}

	return 0;
}

int server_send_message(server_t* self, const char* buffer, size_t length){
	return socket_send(&(self->socket_communicator), buffer, length);
}

int server_recv_message(server_t* self, char* buffer, size_t length) {
	return socket_recv(&(self->socket_communicator), buffer, length);
}

int server_close(server_t* self) {
	socket_shutdown(&(self->socket_server), SHUT_RDWR);
	if (socket_destroy(&(self->socket_server)) == -1) 
		return -1;
	return 0;
}