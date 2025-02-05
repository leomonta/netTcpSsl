#pragma once

/*
Thanks to -> https://www.linuxhowtos.org/C_C++/socket.htm
*/
#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

#include <sys/types.h>
#define INVALID_SOCKET -1

#define DEFAULT_BUFLEN 8192
typedef int Socket;


/**
 * setup a server listening on the given port with the requested IPv protocol
 *
 * @param port the tcp por to listen to
 * @param IPv the IP protocol version, 4 or 6
 *
 * @return the constructed server socket
 */
Socket TCPinitializeServer(const unsigned short port, const char IPv);

/**
 * setup a client connected to server_name, with the requested IPv protocol
 *
 * @param port the tcp port to connect to
 * @param serverName the hostname or ip of the server to connect to
 * @param IPv the IP protocol version, 4 or 6
 *
 * @return the constructed client socket
 */
Socket TCPinitializeClient(const unsigned short port, const char *serverName, const char IPv);

/**
 * shorthand to close and shutdown a socket
 *
 * @param sck the socket to terminate
 */
void TCPterminate(const Socket sck);

/**
 * close the given socket, close the related fd
 *
 * @param sck the socket to close
 */
void TCPcloseSocket(const Socket sck);

/**
 * send the tcp shutdown message through the socket
 *
 * @param sck the socket to shutdown
 */
void TCPshutdownSocket(const Socket sck);

/**
 * receive a segment from the specified socket

 * @param sck the socket to received data from
 * @param buff the buffer where to put the data
 *
 * @return the amount of bytes received
 */
ssize_t TCPreceiveSegment(const Socket sck, char **buff);

/**
 * send a segment through specified socket
 * 
 * @param sck the socket to send data to
 * @param buff the data to send
 *
 * @return the amount of bytes sent
 */
long TCPsendSegment(const Socket sck, const char *buff);

/**
 * 
 * @param ssck server socket to accept client from
 * 
 * @return a client that wants to connect to this server
 */
Socket TCPacceptClientSock(const Socket ssck);

#if defined(__cplusplus)
}
#endif
