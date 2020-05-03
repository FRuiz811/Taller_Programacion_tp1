#include <stdio.h>
#include "client.h"
#include <string.h>

int main(int argc, const char* argv[]) {
  if (argc < 3 || argc > 4) {
	  fprintf(stderr, "Parámetros inválidos. Debe ejecutarse "
	  	"./client <host> <port> [<archivo>]\n");
		return -1;
	}
	client_t client;
	client_create(&client);
	if (client_run(&client, argc, argv) == -1) {
		client_close(&client);
		return -1;
	}
	client_close(&client);
	return 0;
}
