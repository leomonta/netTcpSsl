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
 * must later call `terminate` to free up these resources
 */
void SSL_initialize();

/**
 * free up the resources initialized in `initialize`
 */
void SSL_terminate();

/**
 * create a reference to create SSL objects
 *
 * must later call destroyContext to free up the allocated resources
 *
 * @param[in] `cert_filename` the file path of the openssl certificate
 * @param[in] `key_filename` the file path of the openssl key
 */
SSL_CTX *SSL_create_context(const char* cert_filename, const char* key_filename);

/**
 * free up the resources allocated by createContext
 *
 * @param[in] `ctx` the context to destroy
 */
void SSL_destroy_context(SSL_CTX *ctx);

/**
 * create an ssl connection object through which you can communicate
 *
 * @param[in] `ctx` the context / settings / reference to create and ssl object
 * @param[in] `client` the fd for the client you want to connect to
 *
 * @return a pointer to an ssl connection to use for communication
 */
SSL *SSL_create_connection(SSL_CTX *ctx, const Socket client);

/**
 * destroy and free up the resources allocated by the ssl connection
 *
 * @param[in] `ssl` the ssl connection to destroy
 */
void SSL_destroy_connection(SSL *ssl);

/**
 * receive a record from the given ssl communication and puts it in the given bufffer
 *
 * @param[in] `ssl` the ssl connection to receive data from
 * @param[in] `buff` the buffer where to put the received data
 *
 * @return the number of bytes received, 0 if none received, -1 if an error occured
 */
int SSL_receive_record(SSL *ssl, char **buff);

/**
 * send data through the given ssl connection
 *
 * @param[in] `ssl` the ssl communication to send data into
 * @param[in] `buff` the buffer containing the data to send
 *
 * @return the bytes sent -1 if an error occurred
 */
int SSL_send_record(SSL *ssl, const char *buff, const size_t len);

/**
 * Attempts to accept
 * @param[in] `ssl` the ssl communication to accept a client one
 *
 * @return 0 on success -1 if an error occurred
 */
int SSL_accept_client(SSL *ssl);

#if defined(__cplusplus)
}
#endif
