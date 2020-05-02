#define _POSIX_C_SOURCE 201112L	
#include "common_socket.h"
#include <unistd.h>
#include <string.h>
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

//Resuelve la conexión con el host (si es indicado) y el puerto al cual el
//soquet se conecta. El file descriptor que es utilizado queda almacenado
//en el soquet self.
static int _resolve_address(socket_t* self, struct addrinfo* hints, const char* host, const char* port) {
	struct addrinfo *results, *iter;
	int val = 1;
	if (getaddrinfo(host, port, hints, &results) != 0) {
		freeaddrinfo(results);
		return -1;
	}
	
	for (iter = results; iter != NULL; iter = iter->ai_next) {
		self->socket_fd = socket(iter->ai_family,iter->ai_socktype,0);
		if (self->socket_fd == -1)
			continue;
		if(host == NULL) {
			if (bind(self->socket_fd, iter->ai_addr, iter->ai_addrlen) == 0) {
				setsockopt(self->socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
				break;
			}
		} else {
			if(connect(self->socket_fd, iter->ai_addr, iter->ai_addrlen) != -1) 
				break;
		}
		socket_destroy(self);
	}
	freeaddrinfo(results);
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

	if (_resolve_address(self, &hints, NULL, port) == -1)
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
	if (_resolve_address(self, &hints, host, port) == -1)
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

int socket_send(socket_t* self, const void* buffer, size_t length) {
	size_t sended_bytes = 0;
	int result_send;
	size_t remaining_bytes = length;
	const char* char_buffer = buffer;
	while(sended_bytes < length) {
		result_send = send(self->socket_fd, &char_buffer[sended_bytes], remaining_bytes, MSG_NOSIGNAL);
		if(result_send == -1 || result_send == 0)
			return result_send;
		sended_bytes += result_send;
		remaining_bytes -= result_send;
	}
	return sended_bytes;
}

int socket_recv(socket_t* self, void* buffer, size_t length) {
	size_t received_bytes = 0;
	int result_recv;
	size_t remaining_bytes = length;
	char* char_buffer = buffer;
	while(received_bytes < length) {
		result_recv = recv(self->socket_fd, &char_buffer[received_bytes], remaining_bytes, 0);
		if(result_recv == -1 || result_recv == 0)
			return result_recv;
		received_bytes += result_recv;
		remaining_bytes -= result_recv;
	}
	return received_bytes;
}

int socket_shutdown(socket_t* self, int channel) {
	return shutdown(self->socket_fd, channel); 
}

int socket_is_connected(socket_t* self) {
	if (self->socket_fd == -1)
		return 0;
	return 1;
}