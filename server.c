#include "server.h"
#include <sys/socket.h>
#include <string.h>

#define SOCKET_WAITING 20
#define MSG_RECV "OK\n"
#define INFO_HEADER_SIZE 16
#define MAX_PADDING 8

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

//Retorna 0 en caso de que no esté conectado y 1 en caso de que si
static int server_is_already_connected(server_t* self) {
	return socket_is_connected(&(self->socket_server));
}

int server_connect_to(server_t* self, const char* port) {
  if (server_is_already_connected(self))
		return -1;
	int connection_success = socket_bind_and_listen(&(self->socket_server),
													port, SOCKET_WAITING);
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

int server_send_message(server_t* self, const void* buffer, uint32_t length){
	return socket_send(&(self->socket_communicator), buffer, length);
}

int server_recv_message(server_t* self, void* buffer, uint32_t length) {
	return socket_recv(&(self->socket_communicator), buffer, length);
}

//Se reciben los primeros 16 bytes del mensaje. Imprime el ID del msj
//Además setea en info_header la longitud del body, el id, y la longitud
//del array.
//Devuelve la cantidad de bytes recibidos o -1 en caso de que se cierre
//la comunicación
static int server_receive_info(server_t* self, uint32_t info_header[]){
	uint8_t info_message_received[INFO_HEADER_SIZE];
	int bytes_received = server_recv_message(self,&info_message_received, 
											 INFO_HEADER_SIZE);
	if (bytes_received > 0) {
		protocol_get_info_message(&self->protocol, info_message_received,
								  INFO_HEADER_SIZE, info_header);
		printf("* Id: 0x%08x\n", info_header[1]);
	}
	return bytes_received;
}

static void _print_message(char* header_parts[]){
	printf("* Destino: %s\n", header_parts[0]);
	printf("* Ruta: %s\n", header_parts[1]);
	printf("* Interfaz: %s\n", header_parts[2]);
	printf("* Metodo: %s\n", header_parts[3]);
	return;
}

//Se recibe la cantidad de bytes indicada en length, correspondientes al
//header. Se setean cada una de las partes del mensaje y se imprimen en
//pantalla. Devuelve -1 en caso de error y la cantidad de
//parámetros que hay en caso contrario.
static int server_receive_header(server_t* self, int length) {
	uint8_t* header = malloc(sizeof(uint8_t) * length);
	if (header == NULL)
		return -1;
	char* header_parts[5];
	header_parts[4] = NULL;
	int cant_parmeters = 0;
	if (server_recv_message(self,header, length) == -1)
		return -1;
	protocol_decode_header(&self->protocol, header, length, header_parts);
	_print_message(header_parts);
	if(header_parts[4] != NULL)
		cant_parmeters = strlen(header_parts[4]);
	free(header);
	return cant_parmeters;
}

//Se recibe la cantidad de bytes indicada en length, correspondientes al
//body. En cant_parameters se indica la cantidad de parámetros a recibir.
//Retorna -1 en caso de error, 0 en caso contrario.
static int server_receive_body(server_t* self, int length, 
								int cant_parmeters) {
	uint8_t* body = malloc(sizeof(uint8_t) * length + 1);
	if (body == NULL)
		return -1;
	char** body_parameters = malloc(sizeof(char*) * cant_parmeters);
	if (body_parameters == NULL)
		return -1;
	if (server_recv_message(self,body, length) == -1)
		return -1;
	protocol_decode_body(&self->protocol, body, length, body_parameters);
	int i = 0;
	printf("* Parametros:\n");
	while (i < cant_parmeters) {
		printf("    * %s\n", body_parameters[i]);
		i++;
	}
	free(body_parameters);
	free(body);
	return 0;
}

int server_run(server_t* self, const char* argv[]) {
	if (server_connect_to(self, argv[1]) == -1)
		return -1;
	if (server_accept_connection(self) == -1)
		return -1;
	uint32_t info_header[3];
	while (server_receive_info(self, info_header) > 0) {
		int cant_parmeters = server_receive_header(self,info_header[2]);
		int mod = (info_header[2] + INFO_HEADER_SIZE) % MAX_PADDING;
		if (mod != 0){
			int padding = MAX_PADDING - mod;
			char zeros[MAX_PADDING];
			if (server_recv_message(self,zeros, padding) == -1)
				return -1;
		}
		if (info_header[0] != 0){
			if (server_receive_body(self,info_header[0], cant_parmeters) == -1)
				return -1;
		}
		printf("\n");
		if (server_send_message(self, MSG_RECV, strlen(MSG_RECV)) == -1)
			return -1;
	}
	return 0;
}

void server_close(server_t* self) {
	socket_shutdown(&(self->socket_server), SHUT_RDWR);
	socket_shutdown(&(self->socket_communicator), SHUT_RDWR);
	protocol_destroy(&(self->protocol));
	socket_destroy(&(self->socket_server));
	socket_destroy(&(self->socket_communicator));
	return;
}
