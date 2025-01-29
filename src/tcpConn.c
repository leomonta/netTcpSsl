#include "tcpConn.h"

#include "logger.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

Socket TCPinitializeServer(const unsigned short port, const char IPv) {

	// switch the code on the protocol
	auto protCode = IPv == 6 ? AF_INET6 : AF_INET;

	// create the server socket descriptor, return -1 on failure
	auto serverSocket = socket(
	    protCode,                    // IPvx
	    SOCK_STREAM | SOCK_NONBLOCK, // reliable conn, multiple communication per socket, non blocking accept
	    IPPROTO_TCP);                // Tcp protocol

	if (serverSocket == INVALID_SOCKET) {
		llog(LOG_FATAL, "[TCP] Impossible to create server Socket.\n\tReason: %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	llog(LOG_DEBUG, "[TCP] Created server socket\n");

	int  enable = 1;
	auto sso    = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (sso == -1) {
		llog(LOG_ERROR, "[TCP] Could not set reusabe address for the server socket.\n\tReason: %d %s\n", errno, strerror(errno));
	}

	llog(LOG_DEBUG, "[TCP] Set REUSEADDR for the server socket\n");

	// bind the socket	Reason: "activate the socket"
	// return -1 on failure
	int errorCode;

	if (IPv == 6) {

		// sockaddr_in serverAddr;
		struct sockaddr_in6 serverAddr6;
		inet_pton(AF_INET6, "::1", &serverAddr6.sin6_addr);

		serverAddr6.sin6_family = AF_INET6;    // again IPv6
		serverAddr6.sin6_port   = htons(port); // change to network byte order since its needed internally,
		// network byte order is Big Endian, this machine is Little Endian

		errorCode = bind(serverSocket, (struct sockaddr *)(&serverAddr6), sizeof(serverAddr6));
	} else {
		// sockaddr_in serverAddr;
		struct sockaddr_in serverAddr;

		serverAddr.sin_family      = AF_INET;     // again IPv4
		serverAddr.sin_addr.s_addr = INADDR_ANY;  // accept any type of ipv4 address
		serverAddr.sin_port        = htons(port); // change to network byte order since its needed internally,
		// network byte order is Big Endian, this machine is Little Endian

		errorCode = bind(serverSocket, (struct sockaddr *)(&serverAddr), sizeof(serverAddr));
	}

	if (errorCode == -1) {
		llog(LOG_FATAL, "[TCP] Bind failed.\n\tReason: %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	llog(LOG_DEBUG, "[TCP] Server socket bounded\n");

	// setup this socket to listen for connection, with the queue of SOMAXCONN	Reason: 2^12
	// SOcket
	// MAXimum
	// CONNections
	errorCode = listen(serverSocket, SOMAXCONN);

	if (errorCode == -1) {
		llog(LOG_FATAL, "[TCP] Listening failed.\n\tReason: %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	llog(LOG_DEBUG, "[TCP] Socket server creation completed\n");

	llog(LOG_INFO, "[TCP] Server now listening at http://127.0.0.1:%d\n", port);

	return serverSocket;
}

Socket TCPinitializeClient(const unsigned short port, const char *server_name, const char IPv) {

	auto protCode = IPv == 6 ? AF_INET6 : AF_INET;

	Socket clientSock = socket(protCode, SOCK_STREAM, IPPROTO_TCP);

	if (clientSock == INVALID_SOCKET) {
		llog(LOG_FATAL, "Impossible to create server Socket.\n\tReason: %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	struct hostent *server_hn = gethostbyname(server_name);

	if (server_hn == nullptr) {
		llog(LOG_FATAL, "Hostname requested is unrechable.\n\tReason. %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	struct sockaddr_in serv_addr;

	// set the entire server address struct to 0
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = (short unsigned int)(protCode);

	// copy the server ip from the server hostname to the server socket internet address
	memcpy(&serv_addr.sin_addr.s_addr, server_hn->h_addr_list[0], (size_t)(server_hn->h_length));
	serv_addr.sin_port = htons(port);

	if (connect(clientSock, (struct sockaddr *)(&serv_addr), sizeof(serv_addr)) == -1) {
		llog(LOG_FATAL, "Connectionn to server failed.\n\tReason. %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	return clientSock;
}

void TCPterminate(const Socket sck) {

	TCPshutdownSocket(sck);
	TCPcloseSocket(sck);

	llog(LOG_DEBUG, "[TCP] Socket %d terminated\n", sck);
}

void TCPcloseSocket(const Socket sck) {

	auto res = close(sck);

	if (res < 0) {
		llog(LOG_ERROR, "[TCP] Could not close socket %d\n\tReason: %d %s\n", sck, errno, strerror(errno));
	}
}

void TCPshutdownSocket(const Socket sck) {

	// shutdown for both ReaD and WRite
	auto res = shutdown(sck, SHUT_RDWR);

	if (res < 0) {
		llog(LOG_ERROR, "[TCP] Could not shutdown socket %d\n\tReason: %d %s\n", sck, errno, strerror(errno));
	}
}

ssize_t TCPreceiveSegment(const Socket sck, char **buff) {

	char recvbuf[DEFAULT_BUFLEN];
	// result is the amount of bytes received
	ssize_t bytesReceived    = DEFAULT_BUFLEN;
	ssize_t totBytesReceived = 0;

	while (bytesReceived == DEFAULT_BUFLEN) {
		bytesReceived = recv(sck, recvbuf, DEFAULT_BUFLEN, 0);
		*buff         = (char *)(realloc(*buff, (size_t)(bytesReceived + totBytesReceived + 1)));

		memcpy(*buff, recvbuf, (size_t) (bytesReceived - totBytesReceived));

		totBytesReceived += bytesReceived;
		(*buff)[totBytesReceived] = '\0';
	}

	if (totBytesReceived > 0) {
		llog(LOG_INFO, "[Socket %d] Received %ldB from client\n", sck, totBytesReceived);
	}

	if (totBytesReceived == 0) {
		llog(LOG_INFO, "[Socket %d] Client has shut down the communication\n", sck);
	}

	if (totBytesReceived < 0) {
		llog(LOG_ERROR, "[Socket %d] Failed to receive message\n\tReason: %d %s\n", sck, errno, strerror(errno));
	}

	return totBytesReceived;
}

long TCPsendSegment(const Socket sck, const char *buff) {

	auto size = strlen(buff);

	auto bytesSent = send(sck, buff, size, 0);
	if (bytesSent < 0) {
		llog(LOG_ERROR, "[TCP] Failed to send message\n\tReason: %d %s\n", errno, strerror(errno));
	}

	if (bytesSent != (ssize_t)(size)) {
		llog(LOG_WARNING, "[TCP] Mismatch between buffer size (%ldb) and bytes sent (%ldb)\n", size, bytesSent);
	}

	llog(LOG_INFO, "[TCP] Sent %ldB to client %d\n", bytesSent, sck);

	return bytesSent;
}

Socket TCPacceptClientSock(const Socket ssck) {

	// the client socket address
	struct sockaddr clientAddr;

	// size of the client socket address
	socklen_t clientSize = sizeof(clientAddr);

	// get the socket (int) and the socket address fot the client
	Socket client = accept(ssck, &clientAddr, &clientSize);

	// received and invalid socket
	if (client == INVALID_SOCKET) {
		return -1;
	}

	struct sockaddr_in *temp = (struct sockaddr_in *)(&clientAddr);

	// everything fine, communicate on stdout
	llog(LOG_INFO, "[TCP] Accepted client IP %s:%u\n", inet_ntoa(temp->sin_addr), ntohs(temp->sin_port));

	return client;
}
