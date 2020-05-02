#include "server.h"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

#define SOCKET_WAITING 20
#define MSG_RECV "OK\n"

int server_create(server_t* self) {
	socket_t socket;
	socket_create(&socket);
	self->socket_server = socket;

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

static void server_receive_info(server_t* self, uint32_t info_header[]){
	int buffer_size = 16;
	uint8_t info_message_received[buffer_size];
	server_recv_message(self,&info_message_received, buffer_size);
	protocol_get_info_message(&self->protocol, info_message_received, 16, info_header);
	printf("* Id: 0x%08x\n", info_header[1]);
	return;
}

static void _print_message(char* header_parts[]){
	printf("* Destino: %s\n", header_parts[0]);
	printf("* Path: %s\n", header_parts[1]);
	printf("* Interfaz: %s\n", header_parts[2]);
	printf("* Método: %s\n", header_parts[3]);
	if(strchr(header_parts[4],'s') != NULL)
		printf("* Parámetros:\n");
	return;
}

static int server_receive_header(server_t* self, int length) {
	uint8_t header[length];
	char* header_parts[5];
	server_recv_message(self,&header, length);
	protocol_decode_header(&self->protocol, header, length, header_parts);
	_print_message(header_parts);
	return strlen(header_parts[4]);
}

static void server_receive_body(server_t* self, int length, int cant_parmeters) {
	uint8_t body[length];
	char* body_parameters[cant_parmeters];
	server_recv_message(self,&body, length);
	protocol_decode_body(&self->protocol, body, length, body_parameters);
	int i = 0;
	while (i < cant_parmeters) {
		printf("\t * %s\n", body_parameters[i]);
		i++;
	}
	return;
}

int server_run(server_t* self, const char* argv[]) {
	if (server_connect_to(self, argv[1]) == -1)
		return -1;
	if (server_accept_connection(self) == -1)
		return -1;
	uint32_t info_header[3];
	server_receive_info(self, info_header);
	int cant_parmeters = server_receive_header(self,info_header[2]);
	if (info_header[0] != 0){
		int mod = (info_header[2] + 16) % 8;
		int padding = 8 - mod;
		char zeros[padding];
		server_recv_message(self,zeros, padding);
		server_receive_body(self,info_header[0], cant_parmeters);
	}
	return 0;
}

int server_send_message(server_t* self, const void* buffer, size_t length){
	return socket_send(&(self->socket_communicator), buffer, length);
}

int server_recv_message(server_t* self, void* buffer, size_t length) {
	return socket_recv(&(self->socket_communicator), buffer, length);
}

int server_close(server_t* self) {
	socket_shutdown(&(self->socket_server), SHUT_RDWR);
	protocol_destroy(&(self->protocol));
	if (socket_destroy(&(self->socket_server)) == -1) 
		return -1;
	return 0;
}