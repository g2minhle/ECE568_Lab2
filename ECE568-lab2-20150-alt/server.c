#include "server.h"

/* Process user command
 *
 * userInput : char* - the user input
 * return : void
 */
void processUserInput(ClientManager* clientManager, char* userInput, int i){
	Client* currentClient = &(clientManager->clients[i]);
	char* respondMessage = "(And this is the respond message)";
	sendMessageToClient(currentClient, respondMessage, 33, NOT_FREE_STRING);
	printf(FMT_OUTPUT, userInput, respondMessage);
}

/* Grab commands from clients and process commands
 * Remove clients if the connection is lost of the client want to quit
 * 
 * clientManager : ClientManager* - the client manager
 * readSet : fd_set* - the set of file descriptors that has information to be read 
 * return : void
 */
void processClientsRequests(ClientManager* clientManager, fd_set* readSet){
	char* userInput;
	Client* clients = clientManager->clients;
	int i, connectingStatus;
	int maxCount = clientManager->maxI + 1; 
	for ( i = 0 ; i < maxCount ; i++){
		if (clients[i].soc > 0){
			if (FD_ISSET(clients[i].soc, readSet)){				
				connectingStatus = readfromclient(&(clients[i]), &userInput);
				if (connectingStatus == 0){
					printf("Got full command from client %d\n", i);
					processUserInput(clientManager, userInput, i);
					removeClient(clientManager, i);
				}
				else if (connectingStatus == 1){
					// readfromclient returns 1 when client lost connection 
					removeClient(clientManager, i);
				}
				// readfromclient returns 2 when a full command is not
				free(userInput);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	// The list head
	fd_set readSet;
	ClientManager clientManager;
	intiManager(&clientManager);
	while(1){
		// Waiting for fd to be ready
		readSet = getClientRespond(&clientManager);
		// read from other clients
		processClientsRequests(&clientManager, &readSet);
	}
	return 0;
}
