#include <stdio.h>
#include "server.h"
#include <string.h>

int main(int argc, const char* argv[]){
	if (argc != 2) {
		fprintf(stderr, "Parámetros inválidos. Debe ejecutarse ./server <port>\n");
		return -1;
	}
	server_t server;
	server_create(&server);
	if (server_run(&server, argv) == -1) {
		server_close(&server);
		return -1;
	}
	server_close(&server);
	return 0;
}
