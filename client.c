#include "client.h"
#include <sys/socket.h>
#include <string.h>
#include <stdint.h>
#include "common_dynamic_buffer.h"

#define BUFFER_SIZE 32

int client_create(client_t* self) {
	protocol_t protocol;
	if (protocol_create(&protocol) == -1)
		return -1;
	self->protocol = protocol;

	socket_t socket;
	socket_create(&socket);
	self->socket_client = socket;

	self->input = NULL;
	return 0;
}

//Retorna 0 en caso de que no esté conectado y 1 en caso de que si
static int client_is_already_connected(client_t* self) {
	return socket_is_connected(&(self->socket_client));
}

int client_connect_to(client_t* self, const char* host, const char* port) {
	if (client_is_already_connected(self)) {
		fprintf(stderr, "El cliente ya está conectado a un sevicio.\n");
		return -1;
	}
	int connection_success = socket_connect(&(self->socket_client), host, port);
	return connection_success;
}

int client_define_input(client_t* self,int argc, const char* argv[]) {
	if (argc == 3)
		self->input = stdin;
	else 
		self->input = fopen(argv[3], "r");
	if (self->input == NULL) {
		fprintf(stderr,"No se pudo abrir el archivo ingresado como parámetro\n");
		return -1;
	}
	return 0;
}

//Se encarga de verificar si en el buffer se completa una línea obtenida
//desde el archivo de entrada. También verifica que no haya otra línea entera
//dentro de buffer.
//Retorna la cantidad de filas completas que hay dentro de buffer y setea al
//parámetro position en la posición del buffer donde se encontró la primera 
//ocurrencia. Retorna -1 en caso de error.
static int _is_msg_complete(buffer_t* line, char* buffer, int length,
														int* position) {
	char* end_line = memchr(buffer, '\n', length);
	int new_size;
	char* check_another_msg;
	int completed = 0;
	if (end_line == NULL) {
		if (buffer_concatenate(line, buffer, length) == -1)
			return -1;
	} else {
		new_size = end_line - buffer;
		*position = new_size;
		if(new_size+1 < length){
			check_another_msg = memchr(&buffer[new_size+1], '\n',length-new_size-1);
			if (check_another_msg != NULL && *check_another_msg != '\0')
				completed += 1;
		}
		buffer[new_size] = '\0';
		if (buffer_concatenate(line, buffer, new_size+1) == -1)
			return -1;
		completed += 1;
	}
	memset(buffer,0,length);
	return completed;
}

int client_recv_message(client_t* self) {
	char msg_recieve[3];
	protocol_t protocol = self->protocol;
	if(socket_recv(&(self->socket_client), msg_recieve, 3) == -1)
		return -1;
	printf("0x%08x: %s",protocol.id, msg_recieve);
	return 0;
}

int client_send_message(client_t* self, const uint8_t* buffer, 
												uint32_t length) {
	return socket_send(&(self->socket_client), buffer, length);
}

static int client_communicate_server(client_t* self, buffer_t* line) {
	uint8_t* message;
	int length_msg = protocol_encode_message(&(self->protocol), line->data,
																					line->length, &message);
	if (client_send_message(self, message, length_msg) == -1)
		return -1;
	if (client_recv_message(self) == -1)
		return -1;
	return 0;
}

int client_process_file(client_t* self) {
	char buffer[BUFFER_SIZE];
	buffer_t *line = buffer_create(0);
	int msg_complete = 0;
	int new_size;
	while (!feof(self->input)) {
		if (msg_complete)
			fseek(self->input, new_size+1-BUFFER_SIZE, SEEK_CUR);
		int buffer_read = fread(buffer, sizeof(char), BUFFER_SIZE, self->input);
		msg_complete = _is_msg_complete(line, buffer, buffer_read, &new_size);
		if (msg_complete) {
			client_communicate_server(self, line);
			buffer_destroy(line);
			line = buffer_create(0);
		}
		if (msg_complete > 1){
			fseek(self->input, new_size+1-buffer_read, SEEK_CUR);
			msg_complete = 0;
		}
	}
	buffer_destroy(line);
	return 0;
}

int client_run(client_t* self, int argc, const char* argv[]) {
	if (client_define_input(self, argc, argv) == -1)
		return -1; //No se pudo abrir el archivo input.
	if (client_connect_to(self, argv[1],argv[2]) == -1)
		return -1;
	if (client_process_file(self) == -1)
		return -1;
	return 0;
}

void client_close(client_t* self) {
	protocol_destroy(&(self->protocol));
	socket_shutdown(&(self->socket_client), SHUT_RDWR);
	fclose(self->input);
	socket_destroy(&(self->socket_client));
	return;
}
