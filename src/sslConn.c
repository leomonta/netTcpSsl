#include "sslConn.h"

#include "logger.h"

#include <openssl/err.h>

const char *SSL_ERR_STRING[] = {

    "SSL_ERROR_NONE",
    "SSL_ERROR_SSL",
    "SSL_ERROR_WANT_READ",
    "SSL_ERROR_WANT_WRITE",
    "SSL_ERROR_WANT_X509_LOOKUP",
    "SSL_ERROR_SYSCALL",
    "SSL_ERROR_ZERO_RETURN",
    "SSL_ERROR_WANT_CONNECT",
    "SSL_ERROR_WANT_ACCEPT",
    "SSL_ERROR_WANT_ASYNC",
    "SSL_ERROR_WANT_ASYNC_JOB",
    "SSL_ERROR_WANT_CLIENT_HELLO_CB",
    "SSL_ERROR_WANT_RETRY_VERIFY",
};

void SSL_initialize() {
	// make libssl and libcrypto errors readable, before library_init
	SSL_load_error_strings();
	llog(LOG_DEBUG, "[SSL] Loaded error strings\n");

	// Should be called before everything else
	// Registers digests  and cyphers(whatever that means)
	SSL_library_init();
	llog(LOG_DEBUG, "[SSL] Initialized library\n");

	// populate all digest and cypher algos to an internal table
	OpenSSL_add_ssl_algorithms();
	llog(LOG_DEBUG, "[SSL] Loaded the cyphers and digest algorithms\n");
}

void SSL_terminate() {
	ERR_free_strings();

	llog(LOG_DEBUG, "[SSL] Error string fred\n");
}

SSL_CTX *SSL_create_context(const char* cert_filename, const char* key_filename) {
	// TLS is the newer version of ssl
	// use SSLv23_server_method() for sslv2, sslv3 and tslv1 compartibility
	// create a framework to create ssl struct for connections
	auto res = SSL_CTX_new(TLS_server_method());

	// maybe needed
	// SSL_CTX_set_options(res, SSL_OP_SINGLE_DH_USE);

	// the context could not be created
	if (res == nullptr) {
		ERR_print_errors_fp(stderr);
		// Should alos always use my logger
		llog(LOG_FATAL, "[SSl] Could not create context\n");
		return nullptr;
	}

	llog(LOG_DEBUG, "[SSL] Created context with the best protocol available\n");

	// Load the keys and cetificates

	auto errcode = SSL_CTX_use_certificate_file(res, cert_filename, SSL_FILETYPE_PEM);

	if (errcode != 1) {
		ERR_print_errors_fp(stderr);
		llog(LOG_FATAL, "[SSL] Could not load certificate file '%s'\n", cert_filename);
		return nullptr;
	}

	llog(LOG_DEBUG, "[SSL] Loaded server certificate '%s'\n", cert_filename);

	errcode  = SSL_CTX_use_PrivateKey_file(res, key_filename, SSL_FILETYPE_PEM);

	if (errcode != 1) {
		ERR_print_errors_fp(stderr);
		llog(LOG_FATAL, "[SSL] Could not load private key file '%s'\n", key_filename);
		return nullptr;
	}

	llog(LOG_DEBUG, "[SSL] Loaded server private key '%s'\n", key_filename);

	return res;
}

void SSL_destroy_context(SSL_CTX *ctx) {
	// destroy and free the context
	SSL_CTX_free(ctx);

	llog(LOG_DEBUG, "[SSL] Context destroyed\n");
}

SSL *SSL_create_connection(SSL_CTX *ctx, const Socket client) {
	// ssl is the actual struct that hold the connectiondata
	auto res = SSL_new(ctx);

	// the conn failed
	if (res == NULL) {
		ERR_print_errors_fp(stderr);
		llog(LOG_ERROR, "[SSL] Could not create a connection\n");
		return nullptr;
	}

	llog(LOG_DEBUG, "[SSL] Created new connection\n");

	auto err = SSL_set_fd(res, client);
	if (err != 1) {
		ERR_print_errors_fp(stderr);
		llog(LOG_ERROR, "[SSL] Could not set the tcp client fd to the ssl connection\n");
		return nullptr;
	}

	llog(LOG_DEBUG, "[SSL] Connection linked to the socket\n");

	llog(LOG_INFO, "[SSL] Successfully created and linked a connection to the client sokcet %d\n", client);

	return res;
}

void SSL_destroy_connection(SSL *ssl) {
	// close the connection and free the data in the struct
	SSL_shutdown(ssl);
	SSL_free(ssl);

	llog(LOG_DEBUG, "[SSL] Connection destroyed\n");
}

int SSL_receive_record(SSL *ssl, char **buff) {

	char recvbuf[DEFAULT_BUFLEN];

	int curr_bytes_received    = DEFAULT_BUFLEN;
	int total_bytes_received = 0;

	do {
		// if bytes received <= DEFAULT_BUFLEN, return the exact amount of byes received
		curr_bytes_received = SSL_read(ssl, recvbuf, DEFAULT_BUFLEN);

		if (curr_bytes_received > 0) {
			llog(LOG_INFO, "[SSL] Received %dB from connection\n", curr_bytes_received);
			size_t realloc_sz   = (size_t)(curr_bytes_received + total_bytes_received + 1);
			*buff               = (char *)(realloc(*buff, realloc_sz));

			memcpy(*buff + total_bytes_received, recvbuf, (size_t)(curr_bytes_received));

			total_bytes_received += curr_bytes_received;
			(*buff)[total_bytes_received] = '\0';
		}

		if (curr_bytes_received < 0) {
			ERR_print_errors_fp(stderr);
			llog(LOG_ERROR, "[SSL] Could not read from the ssl connection\n");
		}

		auto errcode = SSL_get_error(ssl, curr_bytes_received);
		if (errcode == SSL_ERROR_SSL || errcode == SSL_ERROR_SYSCALL) {
			llog(LOG_FATAL, "[SSL] The SSL library encoutered a fatal error %d -> %s\n", errno, strerror(errno));
			return -1;
		}

		if (errcode == SSL_ERROR_ZERO_RETURN) {
			llog(LOG_DEBUG, "[SSL] Read failed. Client stopped sending data\n");
			return 0;
		}

		if (errcode != SSL_ERROR_NONE) {
			llog(LOG_DEBUG, "[SSL] Read failed with: %s\n", SSL_ERR_STRING[errcode]);
		}
	} while (SSL_pending(ssl) > 0);

	return curr_bytes_received;
}

int SSL_send_record(SSL *ssl, const char *buff, const size_t size) {

	int errcode   = SSL_ERROR_NONE;
	int bytes_sent = 0;

	do {
		bytes_sent = SSL_write(ssl, buff, (int)(size));

		errcode = SSL_get_error(ssl, bytes_sent);

		if (bytes_sent < 0) {
			ERR_print_errors_fp(stderr);
			switch (errcode) {
			case SSL_ERROR_ZERO_RETURN:
				llog(LOG_DEBUG, "[SSL] Client shutdown connection\n");
				break;
			default:
				llog(LOG_DEBUG, "[SSL] Write failed with: %s\n", SSL_ERR_STRING[errcode]);
				llog(LOG_ERROR, "[SSL] Could not send Record to client\n");
			};

			return -1;
		}
	} while (errcode == SSL_ERROR_WANT_WRITE);

	if (bytes_sent != (int)(size)) {
		llog(LOG_WARNING, "[SSL] Mismatch between buffer size (%ldb) and bytes sent (%ldb)\n", size, bytes_sent);
	}

	llog(LOG_INFO, "[SSL] Sent %ldB of data to client\n", bytes_sent);

	return bytes_sent;
}

int SSL_accept_client(SSL *ssl) {

	auto res = SSL_accept(ssl);

	if (res != 1) {

		ERR_print_errors_fp(stderr);
		auto errcode = SSL_get_error(ssl, res);

		llog(LOG_DEBUG, "[SSL] Read failed with: %s\n", SSL_ERR_STRING[errcode]);
		llog(LOG_ERROR, "[SSL] Could not accept ssl connection\n");
		return -1;
	}

	llog(LOG_INFO, "[SSL] Accepted secure connection\n");
	return 0;
}
