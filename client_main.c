#include <stdio.h>
#include "socket.h"
#include "client.h"
#include <string.h>

int main(int argc, const char* argv[]) {
	if(argc < 3 && argc > 4) {
		printf("Parámetros inválidos. Debe ejecutarse 
			./client <host> <port> [<archivo>]\n");
		return -1;
	}

	client_t* client;
	client_create(&client);
	client_connect(&client, argv[1], argv[2]);
}