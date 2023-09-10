#include "tcpConn.hpp"

#include "logger.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

Socket tcpConn::initializeServer(const unsigned short port, const char IPv) {

	// switch the code on the protocol
	auto protCode = IPv == 6 ? AF_INET6 : AF_INET;

	// create the server socket descriptor, return -1 on failure
	auto serverSocket = socket(
	    protCode,                    // IPvx
	    SOCK_STREAM | SOCK_NONBLOCK, // reliable conn, multiple communication per socket, non blocking accept
	    IPPROTO_TCP);                // Tcp protocol

	if (serverSocket == INVALID_SOCKET) {
		log(LOG_FATAL, "[TCP] Impossible to create server Socket.\n	Reason: %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	log(LOG_DEBUG, "[TCP] Created server socket\n");

	int  enable = 1;
	auto sso    = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (sso == -1) {
		log(LOG_ERROR, "[TCP] Could not set resusabe address for the server socket.\n	Reason: %d %s\n", errno, strerror(errno));
	}

	log(LOG_DEBUG, "[TCP] Set REUSEADDR for the server socket\n");

	// input socket of the server

	// bind the socket	Reason: "activate the socket"
	// return -1 on failure
	int errorCode;

	if (IPv == 6) {

		// sockaddr_in serverAddr;
		sockaddr_in6 serverAddr6;
		inet_pton(AF_INET6, "::1", &serverAddr6.sin6_addr);

		serverAddr6.sin6_family = AF_INET6;    // again IPv6
		serverAddr6.sin6_port   = htons(port); // change to network byte order since its needed internally,
		// network byte order is Big Endian, this machine is Little Endian

		errorCode = bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr6), sizeof(serverAddr6));
	} else {
		// sockaddr_in serverAddr;
		sockaddr_in serverAddr;

		serverAddr.sin_family      = AF_INET;     // again IPv4
		serverAddr.sin_addr.s_addr = INADDR_ANY;  // accept any type of ipv4 address
		serverAddr.sin_port        = htons(port); // change to network byte order since its needed internally,
		// network byte order is Big Endian, this machine is Little Endian

		errorCode = bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr));
	}

	if (errorCode == -1) {
		log(LOG_FATAL, "[TCP] Bind failed.\n	Reason: %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	log(LOG_DEBUG, "[TCP] Server socket bounded\n");

	// setup this socket to listen for connection, with the queue of SOMAXCONN	Reason: 2^12
	// SOcket
	// MAXimum
	// CONNections
	errorCode = listen(serverSocket, SOMAXCONN);

	if (errorCode == -1) {
		log(LOG_FATAL, "[TCP] Listening failed.\n	Reason: %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	log(LOG_DEBUG, "[TCP] Socket server creation completed\n");

	return serverSocket;
}

Socket tcpConn::initializeClient(const unsigned short port, const char *server_name, const char IPv) {

	auto protCode = IPv == 6 ? AF_INET6 : AF_INET;

	Socket clientSock = socket(protCode, SOCK_STREAM, IPPROTO_TCP);

	if (clientSock == INVALID_SOCKET) {
		log(LOG_FATAL, "Impossible to create server Socket.\n	Reason: %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	hostent *server_hn = gethostbyname(server_name);

	if (server_hn == nullptr) {
		log(LOG_FATAL, "Hostname requested is unrechable.\n	Reason. %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	sockaddr_in serv_addr;

	// set the entire server address struct to 0
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = static_cast<short unsigned int>(protCode);

	// copy th server ip from the server hostname to the server socket internet address
	bcopy((char *)server_hn->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server_hn->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(clientSock, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) == -1) {
		log(LOG_FATAL, "Connectionn to server failed.\n	Reason. %d %s\n", errno, strerror(errno));
		return INVALID_SOCKET;
	}

	return clientSock;
}

void tcpConn::terminate(const Socket sck) {

	shutdownSocket(sck);
	closeSocket(sck);

	log(LOG_DEBUG, "[TCP] Server socket %d terminated\n", sck);
}

void tcpConn::closeSocket(const Socket sck) {

	auto res = close(sck);

	if (res < 0) {
		log(LOG_ERROR, "[TCP] Could not close socket %d\n	Reason: %d %s\n", sck, errno, strerror(errno));
	}
}

void tcpConn::shutdownSocket(const Socket sck) {

	// shutdown for both ReaD and WRite
	auto res = shutdown(sck, SHUT_RDWR);

	if (res < 0) {
		log(LOG_ERROR, "[TCP] Could not shutdown socket %d\n	Reason: %d %s\n", sck, errno, strerror(errno));
	}
}

ssize_t tcpConn::receiveSegmentC(const Socket sck, char **buff) {

	char recvbuf[DEFAULT_BUFLEN];
	// result is the amount of bytes received
	ssize_t bytesReceived    = DEFAULT_BUFLEN;
	ssize_t totBytesReceived = 0;

	while (bytesReceived == DEFAULT_BUFLEN) {
		bytesReceived = recv(sck, recvbuf, DEFAULT_BUFLEN, 0);
		*buff         = static_cast<char *>(realloc(*buff, bytesReceived + totBytesReceived + 1));
		memcpy(*buff, recvbuf, bytesReceived - totBytesReceived);
		totBytesReceived += bytesReceived;
		*buff[totBytesReceived] = '\0';
	}

	if (totBytesReceived > 0) {
		log(LOG_INFO, "[Socket %d] Received %ldB from client\n", sck, totBytesReceived);
	}

	if (totBytesReceived == 0) {
		log(LOG_INFO, "[Socket %d] Client has shut down the communication\n", sck);
	}

	if (totBytesReceived < 0) {
		log(LOG_ERROR, "[Socket %d] Failed to receive message\n	Reason: %d %s\n", sck, errno, strerror(errno));
	}

	return totBytesReceived;
}

ssize_t tcpConn::receiveSegment(const Socket sck, std::string &buff) {

	char recvbuf[DEFAULT_BUFLEN];
	// result is the amount of bytes received
	ssize_t bytesReceived    = DEFAULT_BUFLEN;
	ssize_t totBytesReceived = 0;

	while (bytesReceived == DEFAULT_BUFLEN) {
		bytesReceived = recv(sck, recvbuf, DEFAULT_BUFLEN, 0);
		totBytesReceived += bytesReceived;
		buff.append(recvbuf, totBytesReceived);
	}

	if (totBytesReceived > 0) {
		log(LOG_INFO, "[Socket %d] Received %ldB from client\n", sck, totBytesReceived);
	}

	if (totBytesReceived == 0) {
		log(LOG_INFO, "[Socket %d] Client has shut down the communication\n", sck);
	}

	if (totBytesReceived < 0) {
		log(LOG_ERROR, "[Socket %d] Failed to receive message\n	Reason: %d %s\n", sck, errno, strerror(errno));
	}

	return totBytesReceived;
}

long tcpConn::sendSegmentC(const Socket sck, const char *buff) {

	auto size = strlen(buff);

	auto bytesSent = send(sck, buff, size, 0);
	if (bytesSent < 0) {
		log(LOG_ERROR, "[TCP] Failed to send message\n	Reason: %d %s\n", errno, strerror(errno));
	}

	if (bytesSent != static_cast<ssize_t>(size)) {
		log(LOG_WARNING, "[TCP] Mismatch between buffer size (%ldb) and bytes sent (%ldb)\n", size, bytesSent);
	}

	log(LOG_INFO, "[TCP] Sent %ldB to client %d\n", bytesSent, sck);

	return bytesSent;
}

long tcpConn::sendSegment(const Socket sck, const std::string &buff) {

	return sendSegmentC(sck, buff.c_str());
}

Socket tcpConn::acceptClientSock(const Socket ssck) {

	// the client socket address
	sockaddr clientAddr;

	// size of the client socket address
	socklen_t clientSize = sizeof(clientAddr);

	// get the socket (int) and the socket address fot the client
	Socket client = accept(ssck, &clientAddr, &clientSize);

	// received and invalid socket
	if (client == INVALID_SOCKET) {
		return -1;
	}

	sockaddr_in *temp = reinterpret_cast<sockaddr_in *>(&clientAddr);

	// everything fine, communicate on stdout
	log(LOG_INFO, "[TCP] Accepted client IP %s:%u\n", inet_ntoa(temp->sin_addr), ntohs(temp->sin_port));

	return client;
}
