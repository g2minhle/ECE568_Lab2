#include "clientManager.h"

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

/* Write a message to the file descriptor 
 * then free the message string if beiing told to do so
 *
 * fd : int - the file descriptor
 * message : char* - the message want to be sent
 * willFreeString : int - either FREE_STRING or NOT_FREE_STRING
 *                        indicating if the message should be free afterward
 * return : void
 */
void sendMessage(int fd, char* message, int willFreeString){
	int len = strlen(message);
	char endMessage[2] = "\r\n";
	myWrite(fd, message, len);
	myWrite(fd, endMessage, 2);
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
	client->curpos = 0;
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
	prepareConnection(clientManager);
	FD_ZERO(allSet);
	FD_SET(clientManager->socketFD, allSet);
	for (i = 0; i < MAXCLIENTS; i++) {
		clearClientInfo(&(clients[i]));
	}
	clientManager->maxFD = clientManager->socketFD;
	clientManager->maxI = -1;
}

/* Add a client to the client manager
 * 
 * clientManager : ClientManager* - the client manager
 * fd : int - the file descriptor of the new client
 * return : void 
 */
void addClient(ClientManager* clientManager, int fd){
	int i;
	Client* clients = clientManager->clients;
	for (i = 0; i < MAXCLIENTS; i++){
		if (clients[i].soc < 0) {
			/* save descriptor */
			clients[i].soc = fd;
			FD_SET(fd, &(clientManager->allSet));
			if (clientManager->maxFD < fd)
				clientManager->maxFD = fd;
			if (clientManager->maxI < i) 
				clientManager->maxI = i;
			sendMessage(fd, "What is your name?", NOT_FREE_STRING);
			break;
		}
	}
	if (i == MAXCLIENTS) {
		// too many clients
		// send a reject message
		sendMessage(fd, "We are sorry :( We are having too many clients", NOT_FREE_STRING);
		// close connection right away
		myClose(fd);
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
	int readLen = read(c->soc, startptr, MAXLINE - c->curpos);
	int leftOverLength;
	char* endOfCommand;
	if(readLen == 0) {
		// Connection closed by client
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
	int clientFD;
	fd_set readSet = clientManager->allSet;	
	mySelect(clientManager->maxFD + 1 , &readSet);
	// if there is a client want to connect
	if (FD_ISSET(clientManager->socketFD, &readSet)){	
		// accept the new client 
		clientFD = myAccept(clientManager->socketFD);
		addClient(clientManager, clientFD);
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
