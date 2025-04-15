#pragma once

/*
Thanks to -> https://www.linuxhowtos.org/C_C++/socket.htm
*/
#if defined(__cplusplus)
extern "C" { // Prevents name mangling of functions
#endif

#include <sys/types.h>
// man SOCKET(3P)
//        Upon successful completion, socket() shall return a non-negative integer, the socket file descriptor.  Otherwise, a value of -1 shall be returned and errno set to indicate the error.
// so -1 is an invalid socket
#define INVALID_SOCKET -1

#define DEFAULT_BUFLEN 8192
typedef int Socket;

/**
 * setup a server listening on the given port with the requested IPv protocol
 *
 * @param[in] `port` the tcp por to listen to
 * @param[in] `IPv` the IP protocol version, 4 or 6
 *
 * @return the constructed server socket
 */
Socket TCP_initialize_server(const unsigned short port, const char IPv);

/**
 * setup a client connected to server_name, with the requested IPv protocol
 *
 * @param[in] `port` the tcp port to connect to
 * @param[in] `serverName` the hostname or ip of the server to connect to
 * @param[in] `IPv` the IP protocol version, 4 or 6
 *
 * @return the constructed client socket
 */
Socket TCP_initialize_client(const unsigned short port, const char *serverName, const char IPv);

/**
 * shorthand to close and shutdown a socket
 *
 * @param[in] `sck` the socket to terminate
 */
void TCP_terminate(const Socket sck);

/**
 * close the given socket, close the related fd
 *
 * @param[in] `sck` the socket to close
 */
void TCP_close_socket(const Socket sck);

/**
 * send the tcp shutdown message through the socket
 *
 * @param[in] `sck` the socket to shutdown
 */
void TCP_shutdown_socket(const Socket sck);

/**
 * receive a segment from the specified socket

 * @param[in] `sck` the socket to received data from
 * @param[in] `buff` the buffer where to put the data
 *
 * @return the amount of bytes received
 */
ssize_t TCP_receive_segment(const Socket sck, char **buff);

/**
 * send a segment through specified socket
 *
 * @param[in] `sck` the socket to send data to
 * @param[in] `buff` the data to send
 * @param[in] `size` the amount of bytes to send
 *
 * @return the amount of bytes sent
 */
long TCP_send_segment(const Socket sck, const char *buff, const size_t size);

/**
 *
 * @param[in] `ssck` server socket to accept client from
 *
 * @return a client that wants to connect to this server
 */
Socket TCP_accept_client(const Socket ssck);

#if defined(__cplusplus)
}
#endif
