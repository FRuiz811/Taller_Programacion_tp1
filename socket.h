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

//Se encarga de conectar el socket aceptador (del lado del servidor)
//con el puerto indicado como parámetro e indicarle la cantidad máxima
//de conexiones en espera que puede haber a la vez.
//Retorna 0 en caso de que el bind and listen haya sido correcto.
//Devuelve -1 en caso de error.
int socket_bind_and_listen(socket_t* self, const char* port, size_t max_waiting);

//Se encarga de conectar el cliente con el (host, port) indicando como
//parámetro. En el socket self se le asignará el file descriptor.
//Retorna 0 en caso de que la conexión sea exitosa, -1 en caso de error
int socket_connect(socket_t* self, const char* host, const char* port);

//Retorna en accepted_socket, el socket que permitirá la
//comunicación con el cliente.
//Devuelve -1 en caso de error, y 0 si se ha podido aceptar la conexión
int socket_accept(socket_t* self, socket_t* accepted_socket);


//Envía un stream que comienza en buffer de longitud lenght.
//En caso de error o que el socket esté cerrado, devuelve -1.
//Caso contrario, devuelve la cantidad de bytes enviados.
int socket_send(socket_t* self, const char* buffer, size_t lenght);

//Almacena en un buffer de longitud lenght todos los bytes
//recibidos en el socket self.
//En caso de error o que el socket esté cerrado, devuelve -1.
//Caso contrario, devuelve la cantidad de bytes recibidos.
int socket_recv(socket_t* self, char* buffer, size_t lenght);


//Se realiza el shutdown del socket self, con el mode indicado.
//mode: SHUT_WR, SHUT_RD, SHUT_RDWR
//Devuelve -1 en caso de error, 0 en caso de éxito.
int socket_shutdown(socket_t* self, int mode);


//Verifica que el socket esté conectado 
//Si está conectado retorna true, en caso contrario false.
int socket_is_connected(socket_t* self);

#endif