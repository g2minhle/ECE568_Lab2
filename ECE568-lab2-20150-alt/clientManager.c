#include "clientManager.h"

void terminateDuringSSLContextCreation(ClientManager* clientManager, char* errorMessage){
	printf(errorMessage);
	ERR_print_errors_fp (stdout);
	if(clientManager->sslContext != NULL){
		SSL_CTX_free(clientManager->sslContext);
	}
	exit(1);
}

int isSSLCertificateValid(SSL* clientSSL){

	X509* clientCertificate;
	char clientEmail[EMAIL_LENGTH];
	char clientCommonName[COMMON_NAME_LENGTH];
	int NID_email = OBJ_txt2nid("emailAddress");
	if(SSL_get_verify_result(clientSSL) != X509_V_OK){
		/* something not right */
		printf(FMT_ACCEPT_ERR_WITHOUT_LINEBREAK);
		ERR_print_errors_fp (stdout);
		return 0;
	} 
	/*Check the cert chain. The chain length is automatically checked by OpenSSL when
	we set the verify depth in the ctx */

	/*Check the common name*/
	clientCertificate = SSL_get_peer_certificate(clientSSL);
	X509_NAME* clientSubjectName = X509_get_subject_name(clientCertificate);
	X509_NAME_get_text_by_NID(clientSubjectName, NID_email, clientEmail, EMAIL_LENGTH);
	X509_NAME_get_text_by_NID(clientSubjectName, NID_commonName, clientCommonName, COMMON_NAME_LENGTH);

	//assertServerData(clientContext, clientEmail, SERVER_EMAIL, FMT_EMAIL_MISMATCH);
	//assertServerData(clientContext, clientCommonName, SERVER_COMMON_NAME, FMT_CN_MISMATCH);

	printf(FMT_CLIENT_INFO, clientCommonName, clientEmail);
	return 1;
}



/* Init ssl context for server
 *
 * clientManager : ClientManager* - the client manager
 * return : void
 */
void initOpenSSL(ClientManager* clientManager){
	// Register the error strings for libcrypto & libssl
	SSL_load_error_strings ();
	// Register the available ciphers and digests
	SSL_library_init ();

	printf("Done load OpenSSL lib\n");

	// New context saying we are a client, and using SSL 2 or 3 or hihger
	clientManager->sslContext = SSL_CTX_new(OPENSSL_SSL_METHOD);
	if (clientManager->sslContext == NULL){
		terminateDuringSSLContextCreation(clientManager, "Can't create ssl context\n");
	}
	
	SSL_CTX_set_verify(clientManager->sslContext, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	printf("Done create OpenSSl context\n");
	SSL_CTX_set_cipher_list(clientManager->sslContext, OPENSSL_CIPHER_USED);
	printf("Done set cipher list\n");

	if(!(SSL_CTX_use_certificate_chain_file(clientManager->sslContext, SERVER_CERT_FILE))){
		terminateDuringSSLContextCreation(clientManager, "Can't read certificate file\n");
	}
	printf("Done loading certificate file\n");

	//TODO : find how to set password and where to set password
	//SSL_CTX_set_default_passwd_cb(clientManager->sslContext, "password");

	if(!(SSL_CTX_use_PrivateKey_file(clientManager->sslContext, SERVER_KEY_FILE, SSL_FILETYPE_PEM))){
		terminateDuringSSLContextCreation(clientManager, "Can't read key file");
	}	
	printf("Done loading key file\n");

	if(!(SSL_CTX_load_verify_locations(clientManager->sslContext, SERVER_CA_FILE, 0))){
		terminateDuringSSLContextCreation(clientManager, "Can't read CA list");
	}
	printf("Done loading CA file\n");

}

/* Prepare for the connection for the client manager
 *
 * clientManager : ClientManager* - the client manager
 * return : void
 */
void prepareConnection(ClientManager* clientManager){
	int yes = 1;
	int socketFD;
	struct sockaddr_in self; 
	// configure the server
	self.sin_family = AF_INET;
	self.sin_port = htons(PORT);
	self.sin_addr.s_addr = INADDR_ANY;
	bzero(&(self.sin_zero), 8);
	// open socket
	socketFD = mySocket(AF_INET, SOCK_STREAM, 0);
	if((setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
		perror("setsockopt");
	}
	myBind(socketFD, (const struct sockaddr*)&self, sizeof(self));
	myListen(socketFD, 5);
	clientManager->socketFD = socketFD;
}

/* Write a message to a client
 * then free the message string if beiing told to do so
 *
 * client : Client* - the client
 * message : char* - the message want to be sent
 * willFreeString : int - either FREE_STRING or NOT_FREE_STRING
 *                        indicating if the message should be free afterward
 * return : void
 */
void sendMessageToClient(Client* client, char* message, int willFreeString){
	printf("Sending a \"%s\" to client\n", message);
	int len = strlen(message);
	char endMessage[2] = "\r\n";
	SSL_write(client->ssl, message, len);
	SSL_write(client->ssl, endMessage, 2);
	if (willFreeString == FREE_STRING){
		free(message);
	}
}

/* Clear all information relating to the client to remove the client
 * 
 * client : Client* - the client to be cleared
 * return : void
 */
void clearClientInfo(Client* client){
	client->soc = -1; /* -1 indicates available entry */
	client->hasName = 0;
	client->curpos = 0;
	client->buf[0] = '\0';
	client->clientName[0] = '\0';
}

/* Initialize the client manager
 *
 * clientManager : ClientManager* - the client manager
 * return : void
 */
void intiManager(ClientManager* clientManager){
	int i;
	Client* clients = clientManager->clients;
	fd_set* allSet = &(clientManager->allSet);
	// Prepare connection
	initOpenSSL(clientManager);
	prepareConnection(clientManager);
	FD_ZERO(allSet);
	FD_SET(clientManager->socketFD, allSet);
	for (i = 0; i < MAXCLIENTS; i++) {
		clearClientInfo(&(clients[i]));
	}
	clientManager->maxFD = clientManager->socketFD;
	clientManager->maxI = -1;
	printf("Start waiting for client\n");
}

/* Add a client to the client manager
 * 
 * clientManager : ClientManager* - the client manager
 * fd : int - the file descriptor of the new client
 * return : void 
 */
void addClient(ClientManager* clientManager, int fd, SSL* clientSSL){
	int i;	
	Client* clients = clientManager->clients;
	for (i = 0; i < MAXCLIENTS; i++){
		if (clients[i].soc < 0) {
			/* save descriptor */				
			FD_SET(fd, &(clientManager->allSet));
			if (clientManager->maxFD < fd)
				clientManager->maxFD = fd;
			if (clientManager->maxI < i) 
				clientManager->maxI = i;
			clients[i].soc = fd;
			clients[i].ssl = clientSSL;
			break;
		}
	}
	if (i == MAXCLIENTS) {
		// too many clients
		// send a reject message
		Client mockClient;
		mockClient.ssl = clientSSL;
		sendMessageToClient(&mockClient, "We are sorry :( We are having too many clients", NOT_FREE_STRING);
		// close connection right away
		myClose(fd);
		SSL_shutdown(clientSSL);
		SSL_free(clientSSL);
	}
}

/* Find the biggest file descriptor in the client manager
 * Note : this function is used when a client is removed
 * 
 * clientManager : ClientManager* - the client manager
 * return : int - the biggest file descriptor
 */
int findMaxFD(ClientManager* clientManager){
	int i;
	int max = -1;
	int maxIndex = clientManager->maxI + 1;
	Client* clients = clientManager->clients;
	for (i = 0 ; i < maxIndex; i++){
		if (max < clients[i].soc){
			max = clients[i].soc;
		}
	}
	if (max < clientManager->socketFD){
		max = clientManager->socketFD;
	}
	return max;
}

/* Find the biggest index of a client in the client manager
 * Return -1 if there is no client
 * Note : this function is used when a client is removed
 * 
 * clientManager : ClientManager* - the client manager
 * return : int - the biggest index
 */
int findMaxI(ClientManager* clientManager){
	int i;
	Client* clients = clientManager->clients;
	//  * Note : this function is used when a client is removed
	// then i can start from 
	for ( i = clientManager->maxI ; i > -1; i--){
		if (clients[i].soc > -1){
			return i;
		}
	}
	return i;
}

/* Remove the client with index i from the client manager
 * 
 * clientManager : ClientManager* - the client manager
 * index : int - the index of the client wanted to be removed
 * return : void
 */
void removeClient(ClientManager* clientManager, int index){
	Client* client = &(clientManager->clients[index]);
	if(client->ssl != NULL){
		// shutdown SSL
		SSL_shutdown(client->ssl);
		SSL_free(client->ssl);
	}
	// close connection
	myClose(client->soc);
	// remove of the list of FD
	FD_CLR(client->soc, &(clientManager->allSet));
	clearClientInfo(client);
	// update the max info
	clientManager->maxFD = findMaxFD(clientManager);
	clientManager->maxI = findMaxI(clientManager);
	printf("client removed\n");
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

/* Read a command from client c into outputString
 * Retrun 0 if there is some full commnad in put into outputString
 * Return 1 if the connection with client is lost
 * Return 2 if there is no full command from the client
 * 
 * c : cientt* - the client
 * outputString - char* - the output string
 * return : int - Retrun 0 if there is some full commnad in put into outputString
		  Return 1 if the connection with client is lost
		  Return 2 if there is no full command from the client
 */
int readfromclient(Client *c, char** outputString) {
	char* startptr = &(c->buf[c->curpos]);
	int readLen = SSL_read(c->ssl, startptr, MAXLINE - c->curpos);
	printf("Read %d character from client\n", readLen);
	int leftOverLength;
	char* endOfCommand;
	if(readLen == 0) {
		// Connection closed by client
		printf("Client closed the connection\n");
		return 1;
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
			/*
			sigfault creator*/
			//int* a = (int*)1;
			//*a = 1;
			//close(c->soc);
			
			return 0;
		}
		return 2;
	}
	return 0;
}

/* Return the list of descriptors that has information waiting to be read
 * This function will automatically add a new client if possible, but the return list
 * will not contain new client.
 *
 * clientManager : ClientManager* - the client manager
 * return : fd_set - the set of all file descriptors
 *                   that have information waiting to be read
 */
fd_set getClientRespond(ClientManager* clientManager){
	int retVal;
	int clientFD;
	fd_set readSet = clientManager->allSet;	
	mySelect(clientManager->maxFD + 1 , &readSet);
	// if there is a client want to connect
	if (FD_ISSET(clientManager->socketFD, &readSet)){	
		printf("Accepting client\n");
		// accept the new client 
		clientFD = accept(clientManager->socketFD, NULL, NULL);
		if ( clientFD < 0) {
			printf("Failed accepting tcp client\n");			
		} else {
			printf("Done accepting tcp client\n");
			SSL* clientSSL = SSL_new(clientManager->sslContext);
			SSL_set_fd(clientSSL, clientFD);
			retVal = SSL_accept(clientSSL);
			// verityfy the 
			if(retVal == 1 && isSSLCertificateValid(clientSSL) ){
				printf("Done accepting client\n");
				addClient(clientManager, clientFD, clientSSL);
								printf("c\n");
			} else {
				printf("Failed accepting client\n");
				SSL_free(clientSSL);
				close(clientFD);
			}
		}
	}
	return readSet;
}	

/* Set name of the give client
 * 
 * client : Client* - the client
 * userName : char* - the name of the client
 * return : void 
 */
void setName(Client* client, char* userName){
	int length = strlen(userName);
	client->hasName = 1;
	strncpy(client->clientName, userName, length);
}
