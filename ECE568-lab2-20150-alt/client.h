#ifndef __CSC458CLIENT_H__
#define	__CSC458CLIENT_H__
	#include <unistd.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	/* For openssl */
	#include <openssl/rand.h>
	#include <openssl/ssl.h>
	#include <openssl/err.h>

	#include "utility.h"

	#define DEFAULT_HOSTNAME "localhost"
	#define DEFAULT_PORT 8765
	#define REQUEST_MESSAGE "This is a top secrete message."
	#define REQUEST_MESSAGE_LENGTH 33

	#define MAXLINE 1024

	/* use these strings to tell the marker what is happening */
	#define FMT_CONNECT_ERR_WITH_LINE_BREAK "ECE568-CLIENT: SSL connect error\n"
	#define FMT_CONNECT_ERR_WITHOUT_LINE_BREAK "ECE568-CLIENT: SSL connect error "
	#define FMT_SERVER_INFO "ECE568-CLIENT: %s %s %s\n"
	#define FMT_OUTPUT "ECE568-CLIENT: %s %s\n"
	#define FMT_CN_MISMATCH "ECE568-CLIENT: Server Common Name doesn't match\n"
	#define FMT_EMAIL_MISMATCH "ECE568-CLIENT: Server Email doesn't match\n"
	#define FMT_NO_VERIFY "ECE568-CLIENT: Certificate does not verify\n"
	#define FMT_INCORRECT_CLOSE "ECE568-CLIENT: Premature close\n"

	/* OpenSSL related */
	#define OPENSSL_CIPHER_USED "SHA1"
	#define SERVER_KEY_FILE "alice.pem"
	#define SERVER_CERT_FILE "alice.pem"
	#define SERVER_CA_FILE "568ca.pem"
	#define OPENSSL_SSL_METHOD SSLv23_client_method()
	#define OPENSSL_SSL_SETTING SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | SSL_VERIFY_FAIL_IF_NO_PEER_CERT	

	#ifndef OPENSSL_LOAD_CERT
		#define OPENSSL_LOAD_CERT 1
	#endif

	#define COMMON_NAME_LENGTH 256
	#define SERVER_COMMON_NAME "Bob's Server"

	#define EMAIL_LENGTH 256
	#define SERVER_EMAIL "ece568bob@ecf.utoronto.ca"

	#define OPENSSEL_VERSION_ERROR 336032814
	#define OPENSSEL_CERT_ERROR 336134278
	#define OPENSSEL_PROTOCOL_ERROR 336032002
	#define OPENSSEL_PROTOCOL_SSLV2_ERROR 0
	#define OPENSSEL_CIPHER_ERROR  336032784
	
	#define PRIVATE_KEY_PASSWORD "password"
	#define PRIVATE_KEY_PASSWORD_LENGTH 8

	typedef struct {
		int port;
		int sock;
		char* hostName;
		int curpos; // point to the first \0 character
		char buf[MAXLINE];
		/* SSL component */
		SSL* ssl;
		SSL_CTX* sslContext;
	} ClientContext;
#endif
