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
	int val = 1;
	int status = getaddrinfo(host, port, hints, &results);
	if (status != 0)
		return -1;
	
	for (iter = results; iter != NULL; iter = iter->ai_next) {
		self->socket_fd = socket(iter->ai_family,iter->ai_socktype,0);
		if (self->socket_fd == -1)
			continue;
		if(host == NULL) {
			if (bind(self->socket_fd, iter->ai_addr, iter->ai_addrlen) == 0) {
				setsockopt(self->socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
				break; //se linkeo correctamenente
			}
		} else {
			if(connect(self->socket_fd, iter->ai_addr, iter->ai_addrlen) != 1) 
				break;
		} //Se conectó correctamente con el servidor
		socket_destroy(self);
	}
	if(iter == NULL) 
		return -1;

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

int socket_send(socket_t* self, const char* buffer, size_t lenght) {
	size_t sended_bytes = 0;
	int result_send;
	size_t remaining_bytes = lenght;
	while(sended_bytes < lenght) {
		result_send = send(self->socket_fd, &buffer[sended_bytes], remaining_bytes, MSG_NOSIGNAL);
		if(result_send == -1)
			return -1;
		else if (result_send == 0)
			return -1;	
		sended_bytes += result_send;
		remaining_bytes -= result_send;
	}
	return sended_bytes;
}

int socket_recv(socket_t* self, char* buffer, size_t lenght){
	size_t received_bytes = 0;
	int result_recv;
	size_t remaining_bytes = lenght;
	while(received_bytes < lenght) {
		received_bytes = recv(self->socket_fd, &buffer[received_bytes], remaining_bytes, 0);
		if(result_recv == -1)
			return -1;
		else if (result_recv == 0)
			return -1;	
		received_bytes += result_recv;
		remaining_bytes -= result_recv;
	}
	return received_bytes;
}

int socket_shutdown(socket_t* self, int mode){
	return shutdown(self->socket_fd, mode);
}
