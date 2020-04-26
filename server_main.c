#include <stdio.h>
#include "socket.h"
#include <string.h>

int main(int argc, const char* argv[]){
	socket_t socket_server;
	socket_t accepter;
	char buffer[50];
	memset(buffer,0,sizeof(buffer));
	int bytes_recived;
	socket_create(&accepter);
	socket_create(&socket_server);
	socket_bind_and_listen(&socket_server, "8080", 5);
	socket_accept(&socket_server, &accepter);
	socket_recv(&accepter, buffer, 50);
	printf("%s\n",buffer );
	socket_destroy(&accepter);
	socket_destroy(&socket_server);
	return 0;
}