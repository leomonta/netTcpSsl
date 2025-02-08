#pragma once

/*
Thanks to -> https://stackoverflow.com/questions/7698488/turn-a-simple-socket-into-an-ssl-socket
und to -> https://gist.github.com/vedantroy/d2b99d774484cf4ea5165b200888e414
*/

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

#include "tcpConn.h"

#include <openssl/ssl.h>

/**
 * intialize necessary libraries openssl libraries and strings
 *
 * must later call terminateServer to free up these resources
 */
void SSLinitializeServer();

/**
 * free up the resources initialized in initilizeServer
 */
void SSLterminateServer();

/**
 * create a reference to create SSL objects
 *
 * must later call destroyContext to free up the allocated resources
 *
 * @param certificateFilename the file path of the openssl certificate
 * @param keyFilename the file path of the openssl key
 */
SSL_CTX *SSLcreateContext(const char* certificateFilename, const char* keyFilename);

/**
 * free up the resources allocated by createContext
 *
 * @param ctx the context to destroy
 */
void SSLdestroyContext(SSL_CTX *ctx);

/**
 * create an ssl connection object through which you can communicate
 *
 * @param ctx the context / settings / reference to create and ssl object
 * @param client the fd for the client you want to connect to
 *
 * @return a pointer to an ssl connection to use for communication
 */
SSL *SSLcreateConnection(SSL_CTX *ctx, const Socket client);

/**
 * destroy and free up the resources allocated by the ssl connection
 *
 * @param ssl the ssl connection to destroy
 */
void SSLdestroyConnection(SSL *ssl);

/**
 * receive a record from the given ssl communication and puts it in the given bufffer
 *
 * @param ssl the ssl connection to receive data from
 * @param buff the buffer where to put the received data
 *
 * @return the number of bytes received, 0 if none received, -1 if an error occured
 */
int SSLreceiveRecord(SSL *ssl, char **buff);

/**
 * send data through the given ssl connection
 *
 * @param ssl the ssl communication to send data into
 * @param buff the buffer containing the data to send
 *
 * @return the bytes sent -1 if an error occurred
 */
int SSLsendRecord(SSL *ssl, const char *buff, const size_t len);

/**
 * Attempts to accept
 */
int SSLacceptClientConnection(SSL *ssl);

#if defined(__cplusplus)
}
#endif
