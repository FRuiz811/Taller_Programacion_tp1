#ifndef SOCKET_H
#define SOCKET_H
#include <stdlib.h>

// TDA que representará al socket
typedef struct{
	int socket_fd;
}socket_t;

//Constructor del socket
int socket_create(socket_t* self);

//Destructor del socket
int socket_destroy(socket_t* self);

int socket_bind_and_listen(socket_t* self, const char* port, size_t max_waiting);

int socket_accept(socket_t* self, socket_t* accepted_socket);

int socket_connect(socket_t* self, const char* host, const char* port);

int socket_send(socket_t* self, const char* buffer, size_t lenght);

int socket_recv(socket_t* self, char* buffer, size_t lenght);

int socket_shutdown(socket_t* self);

#endif