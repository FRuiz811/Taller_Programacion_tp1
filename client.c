#include "client.h"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

#define BUFFER_SIZE 32

int client_create(client_t* self) {
	socket_t socket;
	FILE* input = NULL;
	socket_create(&socket);
	self->socket_client = socket;
	return 0;
}

int client_is_already_connected(client_t* self) {
	return socket_is_connected(&(self->socket_client));
}

int client_connect_to(client_t* self, const char* host, const char* port) {
	if(client_is_already_connected(self))
		return -1;
	int connection_success = socket_connect(&(self->socket_client), host, port);
	return connection_success;
}

int client_define_input(client_t* self,int argc, const char* argv[]){
	if (argc == 3)
		self->input = stdin;
	else 
		self->input = fopen(argv[3], "r");
	if (self->input == NULL){
		fprintf(stderr, "No se ha podido abrir el archivo ingresado como parámetro\n");
		return -1;
	}
	return 0;
}

int client_run(client_t* self, int argc, const char* argv[]) {
	if(client_define_input(self, argc, argv) == -1)
		return -1; //No se pudo abrir el archivo input.
	if(client_connect_to(self, argv[1],argv[2]) == -1)
		return -1;
	char buffer[BUFFER_SIZE];
	int enviados = 0;	
	while (!feof(self->input)){
		fread(buffer, BUFFER_SIZE, 1, self->input);
		printf("%s\n",buffer);
		enviados = client_send_message(self,buffer,BUFFER_SIZE);
		memset(buffer,0,BUFFER_SIZE);
	}

	return 0;
}

int client_send_message(client_t* self, const char* buffer, size_t lenght) {
	return socket_send(&(self->socket_client), buffer, lenght);
}

int client_close(client_t* self) {
	socket_shutdown(&(self->socket_client), SHUT_RDWR);
	fclose(self->input);
	if(socket_destroy(&(self->socket_client)) == -1) 
		return -1;
	return 0;
}