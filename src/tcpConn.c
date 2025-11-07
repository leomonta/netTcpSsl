#include "tcpConn.h"

#include "logger.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define TEST_ALLOC(ptr) \
if (ptr == NULL) {\
	llog(LOG_FATAL, "[ALLOCATION] Allocation returned NULL: %d, %s\n", errno, strerror(errno)); \
}

Socket TCP_initialize_server(const unsigned short port, const char IPv) {

	// switch the code on the protocol
	auto protocol_version = IPv == 6 ? AF_INET6 : AF_INET;

	// create the server socket descriptor, return -1 on failure
	auto server_socket = socket(
	    protocol_version,            // IPvx
	    SOCK_STREAM | SOCK_NONBLOCK, // reliable conn, multiple communication per socket, non blocking accept
	    IPPROTO_TCP);                // Tcp protocol

	if (server_socket == INVALID_SOCKET) {
		llog(LOG_FATAL, "[TCP] Impossible to create server Socket: %s\n", strerror(errno));
		return INVALID_SOCKET;
	}

	llog(LOG_DEBUG, "[TCP] Created server socket\n");

	int  enable    = 1;
	auto ss_option = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (ss_option == -1) {
		llog(LOG_ERROR, "[TCP] Could not set reusabe address for the server socket: %s\n", strerror(errno));
	}

	llog(LOG_DEBUG, "[TCP] Set REUSEADDR for the server socket\n");

	// bind the socket	Reason: "activate the socket"
	// return -1 on failure
	int errcode;

	if (IPv == 6) {

		// sockaddr_in serverAddr;
		struct sockaddr_in6 server_addr_6;
		inet_pton(AF_INET6, "::1", &server_addr_6.sin6_addr);

		server_addr_6.sin6_family = AF_INET6;    // again IPv6
		server_addr_6.sin6_port   = htons(port); // change to network byte order since its needed internally,
		// network byte order is Big Endian, this machine is Little Endian

		errcode = bind(server_socket, (struct sockaddr *)(&server_addr_6), sizeof(server_addr_6));
	} else {
		// sockaddr_in serverAddr;
		struct sockaddr_in server_addr_4;

		server_addr_4.sin_family      = AF_INET;     // again IPv4
		server_addr_4.sin_addr.s_addr = INADDR_ANY;  // accept any type of ipv4 address
		server_addr_4.sin_port        = htons(port); // change to network byte order since its needed internally,
		// network byte order is Big Endian, this machine is Little Endian

		errcode = bind(server_socket, (struct sockaddr *)(&server_addr_4), sizeof(server_addr_4));
	}

	if (errcode == -1) {
		llog(LOG_FATAL, "[TCP] Bind failed: %s\n", strerror(errno));
		TCP_terminate(server_socket);
		return INVALID_SOCKET;
	}

	llog(LOG_DEBUG, "[TCP] Server socket bounded\n");

	// setup this socket to listen for connection, with the queue of SOMAXCONN	Reason: 2^12
	// SOcket
	// MAXimum
	// CONNections
	errcode = listen(server_socket, SOMAXCONN);

	if (errcode == -1) {
		llog(LOG_FATAL, "[TCP] Listening failed.: %s\n", strerror(errno));
		TCP_terminate(server_socket);
		return INVALID_SOCKET;
	}

	llog(LOG_DEBUG, "[TCP] Socket server creation completed\n");

	llog(LOG_INFO, "[TCP] Server now listening at http://127.0.0.1:%d\n", port);

	return server_socket;
}

Socket TCP_initialize_client(const unsigned short port, const char *server_name, const char IPv) {

	auto protocol_version = IPv == 6 ? AF_INET6 : AF_INET;

	Socket client_socket = socket(protocol_version, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

	if (client_socket == INVALID_SOCKET) {
		llog(LOG_FATAL, "[TCP] Impossible to create server Socket: %s\n", strerror(errno));
		return INVALID_SOCKET;
	}

	struct hostent *server_hostname = gethostbyname(server_name);

	if (server_hostname == nullptr) {
		llog(LOG_FATAL, "[TCP] Hostname requested is unrechable: %s\n", strerror(errno));
		TCP_terminate(client_socket);
		return INVALID_SOCKET;
	}

	struct sockaddr_in serv_addr;

	// set the entire server address struct to 0
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = (short unsigned int)(protocol_version);

	// copy the server ip from the server hostname to the server socket internet address
	memcpy(&serv_addr.sin_addr.s_addr, server_hostname->h_addr_list[0], (size_t)(server_hostname->h_length));
	serv_addr.sin_port = htons(port);

	if (connect(client_socket, (struct sockaddr *)(&serv_addr), sizeof(serv_addr)) == -1) {
		if (errno == EINPROGRESS) {
			llog(LOG_DEBUG, "[TCP] EINPROGRESS returned. Waiting until the client socket is writible\n");
			fd_set fds;

			FD_ZERO(&fds);
			FD_SET(client_socket, &fds);

			struct timeval tv = {1000, 10};
			select(client_socket + 1, NULL, &fds, NULL, &tv);

			llog(LOG_DEBUG, "[TCP] EINPROGRESS returned. The client socket is now writable\n");
			return client_socket;
		} else {
			llog(LOG_FATAL, "[TCP] Connectionn to server failed: %s\n", strerror(errno));
			TCP_terminate(client_socket);
			return INVALID_SOCKET;
		}
	}

	return client_socket;
}

void TCP_terminate(const Socket sck) {

	TCP_shutdown_socket(sck);
	TCP_close_socket(sck);

	llog(LOG_DEBUG, "[TCP] Socket %d terminated\n", sck);
}

void TCP_close_socket(const Socket sck) {

	auto res = close(sck);

	if (res < 0) {
		llog(LOG_ERROR, "[TCP] Could not close socket %d: %s\n", sck, strerror(errno));
	}
}

void TCP_shutdown_socket(const Socket sck) {

	// shutdown for both ReaD and WRite
	auto res = shutdown(sck, SHUT_RDWR);

	if (res < 0) {
		llog(LOG_ERROR, "[TCP] Could not shutdown socket %d: %s\n", sck, strerror(errno));
	}
}

ssize_t TCP_receive_segment(const Socket sck, char **buff) {

	char recvbuf[DEFAULT_BUFLEN];
	// result is the amount of bytes received
	ssize_t total_bytes_received = 0;

	while (true) {
		ssize_t curr_bytes_received = recv(sck, recvbuf, DEFAULT_BUFLEN, 0);

		if (curr_bytes_received < 0) {
			llog(LOG_ERROR, "[TCP %d] Failed to receive message: %s\n", sck, strerror(errno));
			return -1;
		}

		if (curr_bytes_received == 0) {
			llog(LOG_INFO, "[TCP %d] Client sent 0 bytes. The communicatation might have been shut down\n", sck);
			return 0;
		}

		size_t realloc_sz = (size_t)(curr_bytes_received + total_bytes_received + 1);
		*buff             = (char *)(realloc(*buff, realloc_sz));
		TEST_ALLOC(*buff)

		memcpy(*buff + total_bytes_received, recvbuf, (size_t)(curr_bytes_received));

		total_bytes_received += curr_bytes_received;
		(*buff)[total_bytes_received] = '\0';

		if (curr_bytes_received < DEFAULT_BUFLEN) {
			break;
		}
	}

	llog(LOG_INFO, "[TCP %d] Received %ldB from client\n", sck, total_bytes_received);

	return total_bytes_received;
}

long TCP_send_segment(const Socket sck, const char *buff, const size_t size) {

	auto bytes_sent = send(sck, buff, size, 0);
	if (bytes_sent < 0) {
		llog(LOG_ERROR, "[TCP] Failed to send message: %s\n", strerror(errno));
	}

	if (bytes_sent != (ssize_t)(size)) {
		llog(LOG_WARNING, "[TCP] Mismatch between buffer size (%ldb) and bytes sent (%ldb)\n", size, bytes_sent);
	}

	llog(LOG_INFO, "[TCP] Sent %ldB to client %d\n", bytes_sent, sck);

	return bytes_sent;
}

Socket TCP_accept_client(const Socket ssck) {

	struct sockaddr client_address;

	socklen_t client_size = sizeof(client_address);

	Socket client = accept(ssck, &client_address, &client_size);

	if (client == INVALID_SOCKET) {
		return -1;
	}

	struct sockaddr_in *temp = (struct sockaddr_in *)(&client_address);

	// everything fine, communicate on stdout
	llog(LOG_INFO, "[TCP] Accepted client IP %s:%u\n", inet_ntoa(temp->sin_addr), ntohs(temp->sin_port));

	return client;
}
