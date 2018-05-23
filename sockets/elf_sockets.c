
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
#define RECVPORT(recvProc, sendProc, nProc) (BASEPORT + recvProc*nProc + sendProc)

#define MAX_PROCNUM 32 // Maximum number of processes

/* ELF SOCKETS
 *
 * This file should impement send() and recv() routines for sending and
 *   receiving vectors of bytes (char) among different processes.
 */

static int g_myId;
static int g_nProc;
static int g_recv_sockfd[MAX_PROCNUM];

static
int get_recv_socket(int recvProc, int sendProc){
	char recvport[10];
	sprintf(recvport, "%d", RECVPORT(recvProc, sendProc, g_nProc));

	// Set up our hints struct
	static struct addrinfo hints = {
		.ai_family = AF_UNSPEC, // Any protocol
		.ai_socktype = SOCK_STREAM, // Reliable stream communication
	};

	// Get recv socket
	struct addrinfo *res;
	getaddrinfo("localhost", recvport, &hints, &res);
	int recv_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if(recv_sockfd == -1){
		perror("socket:");
		exit(5);
	}

	// Allow port reusage
	int optval = 1;
	int ret = setsockopt(recv_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
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

	return recv_sockfd;
}

void ElfSocket_init(int procId, int nProc){
	g_myId = procId;
	g_nProc = nProc;

	if(nProc > MAX_PROCNUM){
		printf("Exceeded maximum number (%d) of processes.", MAX_PROCNUM);
		exit(4);
	}

	// Get a recv_socket for each other process but ourselves
	int i;
	for(i = 0; i < g_nProc; i++){
		if(i == procId){
			g_recv_sockfd[procId] = 0;
			continue;
		}

		g_recv_sockfd[i] = get_recv_socket(procId, i);
	}
}

void ElfSocket_send(void *buf, int bufLen, int dest){
	// Get destiny port
	char destPort[10];
	sprintf(destPort, "%d", RECVPORT(dest, g_myId, g_nProc));


	// Get address info
	static struct addrinfo hints = {
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
		printf("Connect error at process %d\n", g_myId);
		sleep(1);
	}
	
	// Send data
	int bytesSent = 0;
	int bytesLeft = bufLen;
	while(bytesLeft != 0){
		int nbytes = send(sockfd, buf + bytesSent, bytesLeft, 0);
		
		if(dest == 0) printf("Process %d sent to %d %d bytes.\n", g_myId, dest, nbytes);
		
		bytesSent += nbytes;
		bytesLeft -= nbytes;
	}

	// Free stuff
	freeaddrinfo(res);
	close(sockfd);
}

void ElfSocket_recv(void *buf, int msgLen, int src){
	// Get our recv socket to the process with id 'src'
	int recv_sockfd = g_recv_sockfd[src];

	// Get client socket
	struct sockaddr_storage client_addr;
	socklen_t client_len = sizeof(struct sockaddr_storage);
	int client_sockfd = accept(recv_sockfd, (struct sockaddr *) &client_addr, &client_len);
	if(client_sockfd == -1){
		perror("accept");
		printf("Socket number: %d\n", client_sockfd);
		exit(3);
	}

	// Read data
	int bytesReceived = 0;
	int bytesLeft = msgLen;
	while(bytesLeft != 0){
		int nbytes = recv(client_sockfd, buf + bytesReceived, bytesLeft, 0);

		if(src == 0) printf("Process %d received %d bytes from %d. %d/%d\n", g_myId, nbytes, src, bytesReceived, msgLen);

		bytesReceived += nbytes;
		bytesLeft -= nbytes;
	}

	// Free stuff
	close(client_sockfd);
}

void ElfSocket_finalize(){
	int i;
	for(i = 0; i < g_nProc; i++){
		if(i == g_myId) continue;
		close(g_recv_sockfd[i]);
	}
}


