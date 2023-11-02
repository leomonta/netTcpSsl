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
	 *
	 * @param port the tcp por to listen to
	 * @param IPv the IP protocol version, 4 or 6
	 *
	 * @return the constructed server socket
	 */
	Socket initializeServer(const unsigned short port, const char IPv = 4);

	/**
	 * setup a client connected to server_name, with the requested IPv protocol
	 *
	 * @param port the tcp port to connect to
	 * @param serverName the hostname or ip of the server to connect to
	 * @param IPv the IP protocol version, 4 or 6
	 *
	 * @return the constructed client socket
	 */
	Socket initializeClient(const unsigned short port, const char *serverName, const char IPv = 4);

	/**
	 * shorthand to close and shutdown a socket
	 *
	 * @param sck the socket to terminate
	 */
	void terminate(const Socket sck);

	/**
	 * close the given socket, close the related fd
	 *
	 * @param sck the socket to close
	 */
	void closeSocket(const Socket sck);

	/**
	 * send the tcp shutdown message through the socket
	 *
	 * @param sck the socket to shutdown
	 */
	void shutdownSocket(const Socket sck);

	/**
	 * receive a segment from the specified socket

	 * @param sck the socket to received data from
	 * @param buff the buffer where to put the data
	 *
	 * @return the amount of bytes received
	 */
	ssize_t receiveSegmentC(const Socket sck, char **buff);

	/**
	 * Same as receiveSegmentC but uses std::string
	 */
	ssize_t receiveSegment(const Socket sck, std::string &buff);

	/**
	 * send a segment through specified socket
	 * 
	 * @param sck the socket to send data to
	 * @param buff the data to send
	 *
	 * @return the amount of bytes sent
	 */
	long sendSegmentC(const Socket sck, const char *buff);

	/**
	 * Same as sendSegmentC but ueses std::string
	 */
	long sendSegment(const Socket sck, const std::string &buff);

	/**
	 * 
	 * @param ssck server socket to accept client from
	 * 
	 * @return a client that wants to connect to this server
	 */
	Socket acceptClientSock(const Socket ssck);
} // namespace tcpConn
