#pragma once

/*
Thanks to -> https://www.linuxhowtos.org/C_C++/socket.htm
*/

#include <string>

#define INVALID_SOCKET -1

#define DEFAULT_BUFLEN 8192
typedef int Socket;

namespace tcpConn {

	/**
	 * setup a server listening on the given port with the requested IPv protocol
	 * @return the server socket
	 */
	Socket initializeServer(const unsigned short port, const char protocol = 4);

	/**
	 * setup a client connected to server_name, with the requested IPv protocol
	 * @return the client socket
	 */
	Socket initializeClient(const unsigned short port, const char *server_name, const char protocol = 4);

	/**
	 * shorthand to close and shutdown a socket
	 */
	void terminate(const Socket sck);

	/**
	 * close the given socket, close the related fd
	 */
	void closeSocket(const Socket sck);

	/**
	 * send the tcp shutdown message through the socket
	 */
	void shutdownSocket(const Socket sck);

	/**
	 * receive a segment from the specified socket
	 * @return the amount of bytes sent
	 */
	ssize_t receiveSegmentC(const Socket sck, char* &res);
	
	/**
	 * Same as receiveSegmentC but uses std::string
	 */
	ssize_t receiveSegment(const Socket sck, std::string &result);

	/**
	 * send a segment through specified socket
	 * @return the amount of bytes received
	 */
	long sendSegmentC(const Socket sck, const char* buff);

	/**
	 * Same as sendSegmentC but ueses std::string
	 */
	long sendSegment(const Socket sck, const std::string &buff);

	/**
	 * @return a client that wants to connect to this server
	 */
	Socket acceptClientSock(const Socket ssck);
} // namespace tcpConn