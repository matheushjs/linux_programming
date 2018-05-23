#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "elf_sockets.h"

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

void node_listen(int myId){
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
	};

	printf("Node %d is listening\n", myId);

	struct addrinfo *res;
	getaddrinfo("10.0.2.15", "5009", &hints, &res);

	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	printf("My sockfd: %d\n", sockfd);

	int optval = 1;
	
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(ret == -1){
		perror("setsockopt");
	}

	ret = bind(sockfd, res->ai_addr, res->ai_addrlen);

	if(ret != 0){
		printf("Failed binding socket\n");
		perror(NULL);
		close(sockfd);
		return;
	}

	ret = listen(sockfd, 5);
	printf("Listening? %d\n", ret);


	// Accept data
	int counter = 0;
	while(counter < 3){
		struct sockaddr_storage client_addr;
		socklen_t client_len = sizeof(struct sockaddr_storage);
		
		int client_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
		
		// Read data
		char message[24];
		int ret = recv(client_sockfd, message, 24 * sizeof(char), 0);
		message[ret] = '\0';

		// PRINTTTTT!!!@!@!#$@$!@%$@&!%#*&
		printf("Received: %s, with return value %d\n", message, ret);

		close(client_sockfd);

		counter++;
	}

	printf("Node %d will stop listening.\n", myId);
	close(sockfd);
}

void node_send(int myId){
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
	};

	sleep(1);

	struct addrinfo *res;
	
	while( 0 != getaddrinfo("10.0.2.15", "5009", &hints, &res) ){
		printf("Error in process %d\n", myId);
	}

	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	int ret = connect(sockfd, res->ai_addr, res->ai_addrlen);
	
	if(myId == 1){
		printf("Connected? %d\n", ret);
	}

	char message[24] = { '\0' };
	sprintf(message, "Hello from process: %d", myId);

	// Send!
	ret = send(sockfd, message, strlen(message), 0);

	if(myId == 1){
		printf("Sent? %d\n", ret);
	}

	sleep(1);
	close(sockfd);
}

int main(int argc, char *argv[]){
	// We fork 4 processes and identify each of them with an ID
	int ret1 = fork();
	int ret2 = fork();

	// Normalize variables to 0 or 1
	ret1 = (ret1 != 0);
	ret2 = (ret2 != 0);

	// Get the id
	// the pair (ret2, ret1) is seen as a binary number.
	int myId = ret2*2 + ret1;

	// Node 0 will listen and print to what the other nodes send
	if(myId == 0){
		// node_listen(myId);
	} else {
		// node_send(myId);
	}
	
	ElfSocket_init(myId, 4);

	// In this test, we create a vector of size 10000 with numbers from 0 to 9999
	//   multiplied by the (processId + 1)
	// Processes 0, 1, 2, 3 are in a logical ring topology
	// Each process should send the vector to the next one, and receive from the previous
#define TEST_SIZE 1000
	static int message[TEST_SIZE];
	static int buffer[TEST_SIZE];
	int i;
	for(i = 0; i < TEST_SIZE; i++) message[i] = i * myId;

	int nextProc = (myId + 1) % 4;
	int prevProc = (myId - 1 + 4) % 4;

	if(myId%2 == 0){
		ElfSocket_send(message, TEST_SIZE * sizeof(int), nextProc);
		ElfSocket_recv(buffer, TEST_SIZE * sizeof(int), prevProc);
	} else {
		ElfSocket_recv(buffer, TEST_SIZE * sizeof(int), prevProc);
		ElfSocket_send(message, TEST_SIZE * sizeof(int), nextProc);
	}

	// Just so we can visualize better later
	if(myId == 0) sleep(1);

	int good = 1;
	for(i = 0; i < TEST_SIZE; i++){
		if(buffer[i] != i * prevProc) good = 0;
		if(myId == 0) printf("%d ", buffer[i]);
	}

	if(good)
		printf("Process %d says GOOD!\n", myId);

	ElfSocket_finalize();

	return 0;
}
