#ifndef COMMON_SOCKET_H
#define COMMON_SOCKET_H
#include <stdlib.h>
#include <stdint.h>

// TDA que representa al socket
typedef struct socket {
	int socket_fd;
}socket_t;

//Constructor del socket
int socket_create(socket_t* self);

//Destructor del socket
int socket_destroy(socket_t* self);

//Se encarga de conectar el socket aceptador (del lado del servidor)
//con el puerto indicado como parámetro e indicarle la cantidad máxima
//de conexiones en espera que puede haber a la vez.
//Retorna 0 en caso de que el bind and listen haya sido correcto.
//Devuelve -1 en caso de error.
int socket_bind_and_listen(socket_t* self, const char* port,
						   uint32_t max_waiting);

//Se encarga de conectar el cliente con el (host, port) indicando como
//parámetro. En el socket self se le asignará el file descriptor.
//Retorna 0 en caso de que la conexión sea exitosa, -1 en caso de error
int socket_connect(socket_t* self, const char* host, const char* port);

//Retorna en accepted_socket, un socket inicializado que permitirá la
//comunicación con el cliente.
//Devuelve -1 en caso de error, y 0 si se ha podido aceptar la conexión.
int socket_accept(socket_t* self, socket_t* accepted_socket);

//Envía un stream que comienza en buffer de longitud length.
//En caso de error devuelve -1. Si el socket está cerrado devuelve 0.
//Caso contrario, devuelve la cantidad de bytes enviados.
int socket_send(socket_t* self, const void* buffer, uint32_t length);

//Almacena en un buffer de longitud length todos los bytes
//recibidos en el socket self.
//En caso de error devuelve -1. Si el socket está cerrado devuelve 0.
//Caso contrario, devuelve la cantidad de bytes recibidos.
int socket_recv(socket_t* self, void* buffer, uint32_t length);


//channel: SHUT_WR, SHUT_RD, SHUT_RDWR
//Devuelve -1 en caso de error, 0 en caso de éxito.
int socket_shutdown(socket_t* self, int channel);


//Si está conectado retorna 1, en caso contrario 0.
int socket_is_connected(socket_t* self);

#endif
