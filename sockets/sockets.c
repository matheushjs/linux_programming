#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[]){
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


	return 0;
}
