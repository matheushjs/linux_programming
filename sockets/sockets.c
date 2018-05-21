#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
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
		.ai_family = AF_INET, // IPv4
		.ai_socktype = SOCK_STREAM, // socket com conexão
		.ai_protocol = 0, // qualquer protocolo
	};
	
	struct addrinfo *res;

	int retval = getaddrinfo("www.google.com", "http", &hints, &res);

	printf("Retval: %d\n", retval);

	struct addrinfo *iter = res;
	while(iter != NULL){
		printf("(socktype, protocol) = (%d, %d)\n", iter->ai_socktype, iter->ai_protocol);
		
		int binaddr = ((struct sockaddr_in *) iter->ai_addr)->sin_addr.s_addr;
		char textaddr[INET_ADDRSTRLEN];
		inet_ntop(iter->ai_addr->sa_family, &binaddr, textaddr, INET_ADDRSTRLEN);
		printf("Address: %s\n", textaddr);
		
		iter = iter->ai_next;
	}

	freeaddrinfo(res);
}

void bind_some_sockets(){
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE, // Place my IP
	};

	struct addrinfo *res;
	getaddrinfo(NULL, "5000", &hints, &res);

	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	printf("Socket fd: %d\n", sockfd);
	
	int ret = bind(sockfd, res->ai_addr, res->ai_addrlen); // Bind to a port in my computer
	printf("Bind ret: %d\n", ret);

	ret = listen(sockfd, 5);
	printf("Listen ret: %d\n", ret);

	close(sockfd);
	
	struct sockaddr_storage client_addr;
	socklen_t client_len = sizeof(struct sockaddr_storage);
	ret = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
	printf("Accept ret: %d\n", ret);
}

int main(int argc, char *argv[]){
	// do_some_filling();
	// get_some_addrinfo();
	bind_some_sockets();
	return 0;
}
