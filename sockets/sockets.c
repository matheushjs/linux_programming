#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

void do_some_filling(){
	int binaddr;
	inet_pton(AF_INET, "172.217.29.132", &binaddr); // presentation to network (text to binary)
	                   // The above is google's IP

	struct sockaddr_in sin = {
		.sin_family = AF_INET, // IPv4, AF_INET6 para IPv6
		.sin_port = htons(80), // Porta de destino, convertida para big endian
		.sin_addr = {
			.s_addr = binaddr
		},
		.sin_zero = {'\0'} // Initialize to 0
	};

	struct addrinfo ai = {
		.ai_flags  = AI_PASSIVE,
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM, // tipo do socket
		.ai_protocol = 0, // aceitamos qualquer protocolo
		.ai_addrlen = sizeof(sin),
		.ai_addr = (struct sockaddr *) &sin,
		.ai_canonname = "www.google.com",

		.ai_next = 0
	};
}

void get_some_addrinfo(){
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC, // tanto faz IPv4 ou v6
		.ai_socktype = SOCK_STREAM, // socket com conexão
		.ai_protocol = 0, // qualquer protocolo
		.ai_flags = AI_PASSIVE // Preenche meu IP automaticamente
	};
	
	struct addrinfo *res;

	int retval = getaddrinfo("www.google.com", "http", &hints, &res);

	printf("Retval: %d\n", retval);

	freeaddrinfo(res);
}

int main(int argc, char *argv[]){
	get_some_addrinfo();
	return 0;
}
