#include "client.h"

ClientContext clientContext;

void shutdownClient(ClientContext* clientContext){	
	if(clientContext->ssl != NULL) {
		/* TODO comment here for incomplete shutdown */
		if( SSL_shutdown(clientContext->ssl) <= 0){
			if( SSL_shutdown(clientContext->ssl) <= 0){
				printf(FMT_INCORRECT_CLOSE);
			}
		}
		SSL_free(clientContext->ssl);
	}
	if(clientContext->sslContext != NULL){
		SSL_CTX_free(clientContext->sslContext);
	}
	close(clientContext->sock);
}

void sslMajorIssueHandler(ClientContext* clientContext){
	printf(FMT_INCORRECT_CLOSE);
	shutdownClient(clientContext);
	exit(1);
}

void sigpipeHandle(int param){
	sslMajorIssueHandler(&clientContext);
}

void terminateDuringSSLContextCreation(ClientContext* clientContext, char* errorMessage){
	printf(errorMessage);
	ERR_print_errors_fp (stdout);
	if(clientContext->sslContext != NULL){
		SSL_CTX_free(clientContext->sslContext);
	}
	exit(1);
}

void initOpenSSL(ClientContext* clientContext){
	// Register the error strings for libcrypto & libssl
	SSL_load_error_strings ();
	// Register the available ciphers and digests
	SSL_library_init ();
	printf("Done load OpenSSL lib\n");

	signal(SIGPIPE, sigpipeHandle);
	printf("Done setup SIGPIPE\n");

	clientContext->sslContext = SSL_CTX_new(OPENSSL_SSL_METHOD);
	if (clientContext->sslContext == NULL){
		terminateDuringSSLContextCreation(clientContext, "Can't create ssl context\n");
	}

	SSL_CTX_set_options(clientContext->sslContext, SSL_OP_NO_SSLv2);
	SSL_CTX_set_options(clientContext->sslContext, SSL_OP_NO_TLSv1_1);
	SSL_CTX_set_options(clientContext->sslContext, SSL_OP_NO_TLSv1_2);
	SSL_CTX_set_verify(clientContext->sslContext, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

	printf("Done create OpenSSl context\n");

	SSL_CTX_set_cipher_list(clientContext->sslContext, OPENSSL_CIPHER_USED);
	printf("Done set cipher list\n");

	if(!(SSL_CTX_use_certificate_chain_file(clientContext->sslContext, SERVER_CERT_FILE))){
		terminateDuringSSLContextCreation(clientContext, "Can't read certificate file\n");
	}
	printf("Done loading certificate file\n");

	//TODO : find how to set password and where to set password
	//SSL_CTX_set_default_passwd_cb(clientContext->sslContext, "password");

	if(!(SSL_CTX_use_PrivateKey_file(clientContext->sslContext, SERVER_KEY_FILE, SSL_FILETYPE_PEM))){
		terminateDuringSSLContextCreation(clientContext, "Can't read key file");
	}	
	printf("Done loading key file\n");

	if(!(SSL_CTX_load_verify_locations(clientContext->sslContext, SERVER_CA_FILE, 0))){
		terminateDuringSSLContextCreation(clientContext, "Can't read CA list");
	}
	printf("Done loading CA file\n");
}

void initClientContext(int argc, char **argv, ClientContext* clientContext){
	clientContext->port = DEFAULT_PORT;
	clientContext->hostName = DEFAULT_HOSTNAME;
  
	/*Parse command line arguments*/
	switch(argc){
		case 1:
			break;
		case 3:
			clientContext->hostName = argv[1];
			clientContext->port = atoi(argv[2]);
			if (clientContext->port < 1
			  ||clientContext->port > 65535){
				printErrorThenQuit("Invalid port number\n");
			}
			break;
		default:
			printf("Usage: %s server port\n", argv[0]);
			exit(0);
	}

	clientContext->curpos = 0;
	clientContext->buf[0] = '\0';

	initOpenSSL(clientContext);
}

void establishTCPConnection(ClientContext* clientContext){
	struct sockaddr_in addr;
	struct hostent* host_entry;

	/*get ip address of the host*/
	host_entry = gethostbyname(clientContext->hostName);

	if (!host_entry){
		printErrorThenQuit("Couldn't resolve host\n");
	}

	memset(&addr,0,sizeof(addr));
	addr.sin_addr=*(struct in_addr *) host_entry->h_addr_list[0];
	addr.sin_family=AF_INET;
	addr.sin_port=htons(clientContext->port);

	printf("Connecting to %s(%s):%d\n", 
		clientContext->hostName, 
		inet_ntoa(addr.sin_addr), 
		clientContext->port);

	/* open socket */
	clientContext->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(clientContext->sock < 0){
		printErrorThenQuit("Cannot bind to socket\n");
	}

	if(connect(clientContext->sock,(struct sockaddr *)&addr, sizeof(addr))<0){
		close(clientContext->sock);
		printErrorThenQuit("Cannot connect to server\n");
	}

	printf("Done connect to server\n");
}

void assertServerData(ClientContext* clientContext, char* serverData, char* clientData, char* errorMessage){
	if(strcasecmp(serverData, clientData) != 0){
		printf(errorMessage);
		shutdownClient(clientContext);
		exit(1);		
	}
}

void establishSSLConnection(ClientContext* clientContext){
	clientContext->ssl = SSL_new(clientContext->sslContext);
	establishTCPConnection(clientContext);
	SSL_set_fd(clientContext->ssl, clientContext->sock);
	if (SSL_connect(clientContext->ssl) < 1){
		unsigned long lastErrorCode = ERR_peek_last_error();
		printf("SSL error code :%lu\n", lastErrorCode);
		if(lastErrorCode == OPENSSEL_VERSION_ERROR 
		|| lastErrorCode == OPENSSEL_PROTOCOL_ERROR
     		|| lastErrorCode == OPENSSEL_PROTOCOL_SSLV2_ERROR){
			// protocol issue
			printf(FMT_CONNECT_ERR_WITH_LINE_BREAK);	
		} else {
			// some others issues (cipher type)
			printf(FMT_CONNECT_ERR_WITHOUT_LINE_BREAK);
			ERR_print_errors_fp (stdout);
		}		
		shutdownClient(clientContext);
		exit(1);
	}	
	printf("Done SSL connection\n");
}

void checkSSLCertificateValid(ClientContext* clientContext){

	X509* serverCertificate;
	char serverEmail[EMAIL_LENGTH];
	char serverCommonName[COMMON_NAME_LENGTH];
	char issuerCommonName[COMMON_NAME_LENGTH];

	int NID_email = OBJ_txt2nid("emailAddress");

	if(SSL_get_verify_result(clientContext->ssl) != X509_V_OK){
		/* something not right */
		printf(FMT_NO_VERIFY);
		shutdownClient(clientContext);
		exit(1);		
	}

	/*Check the cert chain. The chain length is automatically checked by OpenSSL when
	we set the verify depth in the ctx */

	/*Check the common name*/
	serverCertificate = SSL_get_peer_certificate(clientContext->ssl);
	X509_NAME* serverSubjectName = X509_get_subject_name(serverCertificate);
	X509_NAME* serverCertIssuerName = X509_get_issuer_name(serverCertificate);

	X509_NAME_get_text_by_NID(serverSubjectName, NID_email, serverEmail, EMAIL_LENGTH);
	X509_NAME_get_text_by_NID(serverSubjectName, NID_commonName, serverCommonName, COMMON_NAME_LENGTH);
	X509_NAME_get_text_by_NID(serverCertIssuerName, NID_commonName, issuerCommonName, COMMON_NAME_LENGTH);

	assertServerData(clientContext, serverEmail, SERVER_EMAIL, FMT_EMAIL_MISMATCH);
	assertServerData(clientContext, serverCommonName, SERVER_COMMON_NAME, FMT_CN_MISMATCH);

	printf(FMT_SERVER_INFO, serverCommonName, serverEmail, issuerCommonName);
}

/* Return a pointer point to the first \n or \r character in the string bufer
 *
 * bufer : char * - the bufer string
 * return : char* - a pointer point to the first \n or \r character in the string bufer
 */
char* findEndOfCommand(char*  bufer){
	int i;
	int len = strlen(bufer);
	for ( i = 0 ; i < len ; i ++){
		if ((bufer[i] == '\n') || (bufer[i] == '\r') )
			return &(bufer[i]);
	}
	return NULL;	
}

int readfromServer(ClientContext* c, char** outputString) {
	char* startptr = &(c->buf[c->curpos]);
	int readLen = SSL_read(c->ssl, startptr, MAXLINE - c->curpos);
	printf("Read %d character from client\n", readLen);
	int leftOverLength;
	char* endOfCommand;
	if(readLen < 0) {
		// Connection closed by client
		printf("Server closed the connection\n");
		/* do ssl shutdown to cause this is cause a sigpipe */
		shutdownClient(c);
		return 2;
	} else {
		c->curpos += readLen;
		c->buf[c->curpos] = '\0';
		/// Check if we have the whole command \r\n
		endOfCommand = findEndOfCommand(c->buf);
		if(endOfCommand) {		
			*endOfCommand = '\0';
			// get and return string from the beginging to endOfCommand
			*outputString = strCopy(c->buf);
			leftOverLength = strlen( &(endOfCommand[1]) ); 
			// Need to shift anything still in the buffer over
			// to beginning.
			memmove(c->buf, endOfCommand + 1, leftOverLength + 1);
			c->curpos = leftOverLength;
			return 0;
		}
		return 2;
	}
}

void transferData(ClientContext* clientContext){	
	char* respond;
	char request[REQUEST_MESSAGE_LENGTH] = REQUEST_MESSAGE;
	strncat(request, "\r\n", 2);
	printf("Sending request\n");
	/* TODO exite here for incomplete shutdown */
	int retVal = SSL_write(clientContext->ssl, request, REQUEST_MESSAGE_LENGTH);
	if(retVal <= 0){
		/* Connection is not closed yet, any thing happen is bad*/
		sslMajorIssueHandler(clientContext);
	}
	printf("Done sending request with retVal = %d\n", retVal);
	printf("Reading respond\n");
	while ((retVal = readfromServer(clientContext, &respond)) == 2 ){
	}
	/* TODO exite here for incomplete shutdown */
	if(retVal == 0) {
		printf(FMT_OUTPUT, REQUEST_MESSAGE, respond);
		free(respond);
	}
}

int main(int argc, char **argv)
{
	ClientContext clientContext;  
	initClientContext(argc, argv, &clientContext);
	establishSSLConnection(&clientContext);
	checkSSLCertificateValid(&clientContext);
	transferData(&clientContext);
	shutdownClient(&clientContext);
	return 0;
}
