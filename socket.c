#define _POSIX_C_SOURCE 201112L	
#include "socket.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <netdb.h>
#include <sys/socket.h>

//Se inicializa el socket en un estado inválido 
//Esto obliga a que luego del create obligue al programador
//hacer un bind_and_listen o un connect
int socket_create(socket_t* self) {
	self->socket_fd = -1;
	return 0;
}

//Cierra el file descriptor utilizado por el socket
int socket_destroy(socket_t* self) {
	int closed = close(self->socket_fd);
	return closed;
}

int resolve_address(socket_t* self, struct addrinfo* hints, const char* host, const char* port){
	struct addrinfo *results, *iter;
	int status = getaddrinfo(host, port, hints, &results);
	if (status != 0){
		printf("Error getaddrinfo\n");
		return -1;
	}

	for (iter = results; iter != NULL; iter = iter->ai_next) {
		self->socket_fd = socket(iter->ai_family,iter->ai_socktype,iter->ai_protocol);
		printf("%d\n", self->socket_fd);
		if (self->socket_fd == -1){
			printf("Error\n");
			continue;
		}
		if(host == NULL) {
			if (bind(self->socket_fd, iter->ai_addr, iter->ai_addrlen) == 0){
				printf("Sever\n");
				break; //se linkeo correctamenente
			}
		} else {
			if(connect(self->socket_fd, iter->ai_addr, iter->ai_addrlen) != -1) {
				printf("Conectado\n");
				break;
			}
			else{
				printf("Connect fail\n");
			}
		} //Se conectó correctamente con el servidor
		close(self->socket_fd);
	}
	if(iter == NULL){
		printf("Iter NULL\n");
		return -1;
	}

	return 0;
}

int socket_bind_and_listen(socket_t* self, const char* port, size_t max_waiting) {
	struct addrinfo hints;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (resolve_address(self, &hints, NULL, port) == -1)
		return -1;
	
	if (listen(self->socket_fd, max_waiting) == -1)
		return -1;
	
	return 0;
}

int socket_connect(socket_t* self, const char* host, const char* port) {
	struct addrinfo hints;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; //IPv4	
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	printf("Connect to resolve_address\n");
	if (resolve_address(self, &hints, host, port) == -1)
		return -1;
	return 0;
}

int socket_accept(socket_t* self, socket_t* accepted_socket){
	int new_fd = accept(self->socket_fd, NULL, NULL);
	if (new_fd == -1)
		return -1;
	accepted_socket->socket_fd = new_fd;
	return 0;
}