
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BASEPORT 6000
#define RECVPORT(procID) (BASEPORT + 2*procID)
#define SENDPORT(procID) (BASEPORT + 2*procID + 1)

/* ELF SOCKETS
 *
 * This file should impement send() and recv() routines for sending and
 *   receiving vectors of bytes (char) among different processes.
 */

static int recv_sockfd;
static int send_sockfd;


void ElfSocket_init(int procId){
	int ret;

	char sendport[10];
	char recvport[10];
	sprintf(recvport, "%d", RECVPORT(procId));
	sprintf(sendport, "%d", SENDPORT(procId));


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
		perror(NULL);
		close(recv_sockfd);
		exit(2);
	}

	
	// Get send socket
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo("localhost", sendport, &hints, &res);
	send_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	optval = 1;
	ret = setsockopt(send_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(ret == -1){
		perror("setsockopt: ");
		exit(1);
	}

	// Bind the send socket
	ret = bind(send_sockfd, res->ai_addr, res->ai_addrlen);
	if(ret != 0){
		perror(NULL);
		close(send_sockfd);
		exit(3);
	}


	// Listen on the receiving socket
	listen(recv_sockfd, 10);
}

void ElfSocket_finalize(){
	close(recv_sockfd);
	close(send_sockfd);
}


