#pragma once

/*
Thanks to -> https://stackoverflow.com/questions/7698488/turn-a-simple-socket-into-an-ssl-socket
und to -> https://gist.github.com/vedantroy/d2b99d774484cf4ea5165b200888e414
*/

#include "tcpConn.hpp"

#include <openssl/ssl.h>

namespace sslConn {

	/**
	 * intialize necessary libraries openssl libraries and strings
	 *
	 * must later call terminateServer to free up these resources
	 */
	void initializeServer();

	/**
	 * free up the resources initialized in initilizeServer
	 */
	void terminateServer();

	/**
	 * create a reference to create SSL objects
	 *
	 * must later call destroyContext to free up the allocated resources
	 */
	SSL_CTX *createContext();

	/**
	 * free up the resources allocated by createContext
	 *
	 * @param ctx the context to destroy
	 */
	void destroyContext(SSL_CTX *ctx);

	/**
	 * create an ssl connection object through which you can communicate
	 *
	 * @param ctx the context / settings / reference to create and ssl object
	 * @param client the fd for the client you want to connect to
	 *
	 * @return a pointer to an ssl connection to use for communication
	 */
	SSL *createConnection(SSL_CTX *ctx, const Socket client);

	/**
	 * destroy and free up the resources allocated by the ssl connection
	 *
	 * @param ssl the ssl connection to destroy
	 */
	void destroyConnection(SSL *ssl);

	/**
	 * receive a record from the given ssl communication and puts it in the given bufffer
	 *
	 * @param ssl the ssl connection to receive data from
	 * @param buff the buffer where to put the received data
	 *
	 * @return the number of bytes received, 0 if none received, -1 if an error occured
	 */
	int receiveRecord(SSL *ssl, std::string &buff);

	/**
	 * same as receiveRecord but uses a c interface
	 */
	int receiveRecordC(SSL *ssl, char **buff);

	/**
	 * send data through the given ssl connection
	 *
	 * @param ssl the ssl communication to send data into
	 * @param buff the buffer containing the data to send
	 *
	 * @return the bytes sent -1 if an error occurred
	 */
	int sendRecord(SSL *ssl, const std::string &buff);

	/**
	 * same as sendRecord but uses a c interface
	 */
	int sendRecordC(SSL *ssl, const char *buff);

	/**
	 * Attempts to accept
	 */
	int acceptClientConnection(SSL *ssl);
} // namespace sslConn
