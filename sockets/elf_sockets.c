
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BASEPORT 6018
#define RECVPORT(procID) (BASEPORT + procID)

/* ELF SOCKETS
 *
 * This file should impement send() and recv() routines for sending and
 *   receiving vectors of bytes (char) among different processes.
 */

static int myId;
static int recv_sockfd;

void ElfSocket_init(int procId){
	int ret;
	myId = procId;

	char recvport[10];
	sprintf(recvport, "%d", RECVPORT(procId));


	// Get recv socket
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC, // Any protocol
		.ai_socktype = SOCK_STREAM, // Reliable stream communication
	};
	struct addrinfo *res;
	getaddrinfo("localhost", recvport, &hints, &res);
	recv_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// Allow port reusage
	int optval = 1;
	ret = setsockopt(recv_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(ret == -1){
		perror("setsockopt: ");
		exit(1);
	}

	// Bind the recv socket
	ret = bind(recv_sockfd, res->ai_addr, res->ai_addrlen);
	if(ret != 0){
		perror("bind");
		close(recv_sockfd);
		freeaddrinfo(res);
		exit(2);
	}
	freeaddrinfo(res);

	// Listen on the receiving socket
	listen(recv_sockfd, 10);
}

void ElfSocket_send(void *buf, int bufLen, int dest){
	// Get destiny information
	char destPort[10];
	sprintf(destPort, "%d", RECVPORT(dest));
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC, // Any protocol
		.ai_socktype = SOCK_STREAM, // Reliable stream communication
	};
	struct addrinfo *res;
	getaddrinfo("localhost", destPort, &hints, &res);

	// Get a socket
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// Allow port reusage
	int optval = 1;
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(ret == -1){
		perror("setsockopt: ");
		exit(1);
	}


	// Connect to destiny
	while( connect(sockfd, res->ai_addr, res->ai_addrlen) != 0 ){
		perror("connect");
	}
	
	// Send data
	int bytesSent = 0;
	int bytesLeft = bufLen;
	while(bytesLeft != 0){
		int nbytes = send(sockfd, buf + bytesSent, bytesLeft, 0);
		
		if(dest == 0) printf("Process %d sent to %d %d bytes.\n", myId, dest, nbytes);
		
		bytesSent += nbytes;
		bytesLeft -= nbytes;
	}

	// Free stuff
	freeaddrinfo(res);
	close(sockfd);
}

void ElfSocket_recv(void *buf, int msgLen){
	// Get socket
	struct sockaddr_storage client_addr;
	socklen_t client_len = sizeof(struct sockaddr_storage);
	int client_sockfd = accept(recv_sockfd, (struct sockaddr *) &client_addr, &client_len);
	if(client_sockfd == -1){
		perror("accept");
		exit(3);
	}

	// Read data
	int bytesReceived = 0;
	int bytesLeft = msgLen;
	while(bytesLeft != 0){
		int nbytes = recv(client_sockfd, buf + bytesReceived, bytesLeft, 0);

		// printf("Process %d received %d bytes. %d/%d\n", myId, nbytes, bytesReceived, msgLen);

		bytesReceived += nbytes;
		bytesLeft -= nbytes;
	}

	// Free stuff
	close(client_sockfd);
}

void ElfSocket_finalize(){
	close(recv_sockfd);
}


