#include "client.h"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdbool.h>
#include "buffer_dinamico.h"
#include <stdint.h>


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

int client_is_already_connected(client_t* self) {
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
		fprintf(stderr, "No se ha podido abrir el archivo ingresado como parámetro\n");
		return -1;
	}
	return 0;
}

static bool _is_msg_complete(buffer_t* line, char* buffer, int* position) {
	char* end_line = memchr(buffer, '\n', BUFFER_SIZE);
	int new_size;
	bool completed;
	if (end_line == NULL) {
		buffer_concatenate(line, buffer, BUFFER_SIZE);
		completed = false;
	} else {
		new_size = end_line - buffer;
		*position = new_size;
		buffer[new_size] = '\0';
		buffer_concatenate(line, buffer, new_size+1);
		completed = true;
	}
	memset(buffer,0,BUFFER_SIZE);
	return completed;
}

int client_process_file(client_t* self) {
	char buffer[BUFFER_SIZE];
	buffer_t *line = buffer_create(0);
	bool msg_complete = false;
	int new_size;
	while (!feof(self->input)) {
		if (msg_complete)
			fseek(self->input, new_size+1-BUFFER_SIZE, SEEK_CUR);
		fread(buffer, sizeof(char), BUFFER_SIZE, self->input);
		msg_complete = _is_msg_complete(line, buffer, &new_size);
		if (msg_complete) {
			protocol_encode_message(&(self->protocol), line->data, line->length);
			buffer_t* header = self->protocol.header;
			void* data = buffer_get_data(header);
			client_send_message(self, data, header->length);
			buffer_t* body = self->protocol.body;
			if(body->length != 0){
				data = buffer_get_data(body);
				client_send_message(self, data, body->length);
			}
			client_recv_message(self);
			buffer_destroy(line);
			line = buffer_create(0);
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
	client_process_file(self);
	return 0;
}

int client_recv_message(client_t* self) {
	char msg_recieve[3];
	protocol_t protocol = self->protocol;
	if(socket_recv(&(self->socket_client), msg_recieve, 3) == -1)
		return -1;
	printf("0x%08lx: %s",protocol.id, msg_recieve);
	return 0;
}

int client_send_message(client_t* self, const void* buffer, size_t length) {
	return socket_send(&(self->socket_client), buffer, length);
}

int client_close(client_t* self) {
	protocol_destory(&(self->protocol));
	socket_shutdown(&(self->socket_client), SHUT_RDWR);
	fclose(self->input);
	if (socket_destroy(&(self->socket_client)) == -1) 
		return -1;
	return 0;
}