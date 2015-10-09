#include "server.h"

/* Tokenize the input string then put into the array of string tokens
 * then return the number of tokens in the string
 *
 * fd : int - the file descriptor of the current client
 * input : char* - the input string
 * tokens : tokens - the output list of tokens
 * return : int - the total number of tokens
 */
int tokenizeTheInput(int fd, char* input, char** tokens){
	/* Tokenize arguments */
	int cmd_argc;
	cmd_argc = 0;
	char *next_token = strtok(input, DELIM);
	while (next_token != NULL) {
		if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
			sendMessage(fd, "Too many arguments!", NOT_FREE_STRING);
			cmd_argc = 0;
			break;
		}
		tokens[cmd_argc] = next_token;
		cmd_argc++;
		next_token = strtok(NULL, DELIM);
	}
	tokens[cmd_argc] = NULL;	
	return cmd_argc;
}

/* Read and process buxfer commands
 * 
 * return : int - 1 iff the client send a valid quit command
 */
int processCommand(ClientManager* clientManager, int cmd_argc, char **cmd_argv, Group **group_list_addr, int i) {
	int j;
	Group *theGroup = NULL;
	char* outputString = NULL;
	User *current_user = NULL;
	Group *group_list = *group_list_addr; 
	Client* currentClient = &(clientManager->clients[i]);
	Client* clients = clientManager->clients;
	if (cmd_argc <= 0) return 0;
	if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
		sendMessage(currentClient->soc, "Goodbye!", NOT_FREE_STRING);
		return 1;        
	} else if (strcmp(cmd_argv[0], "add_group") == 0 && cmd_argc == 2) {
		outputString = add_group(group_list_addr, cmd_argv[1]);
		sendMessage(currentClient->soc, outputString, FREE_STRING);
		group_list = *group_list_addr; 
		theGroup = find_group(group_list, cmd_argv[1]);
		outputString = add_user(theGroup, currentClient->clientName);
		sendMessage(currentClient->soc, outputString, NOT_FREE_STRING);
		// send to every one
		current_user = theGroup->users;	
		while (current_user != NULL) {
			int maxCount = clientManager->maxI + 1;
			for ( j = 0 ; j < maxCount ; j ++ ){
				if (i != j){
					if (strcmp(current_user->name, clients[j].clientName) == 0){
						sendMessage(clients[j].soc, outputString, NOT_FREE_STRING);
					}
				}
			}
			current_user = current_user->next;
		}
		free(outputString);
	} else if (strcmp(cmd_argv[0], "list_groups") == 0 && cmd_argc == 1) {
		if (group_list == NULL) return 0;
		outputString = list_groups(group_list);
		sendMessage(currentClient->soc, outputString, FREE_STRING);
	} else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 2) {
		if(group_list == NULL) return 0;
		theGroup = find_group(group_list, cmd_argv[1]);
		if (theGroup == NULL) {
			sendMessage(currentClient->soc, "Group does not exist", NOT_FREE_STRING);
		} else {
			outputString = list_users(theGroup);
			sendMessage(currentClient->soc, outputString, FREE_STRING);
		}        
	} else if (strcmp(cmd_argv[0], "user_balance") == 0 && cmd_argc == 2) {
		theGroup = find_group(group_list, cmd_argv[1]);
		if (theGroup == NULL) {        
			sendMessage(currentClient->soc, "Group does not exist", NOT_FREE_STRING);
		} else {
			outputString = user_balance(theGroup, currentClient->clientName);
			sendMessage(currentClient->soc, outputString, FREE_STRING);
		}
	} else if (strcmp(cmd_argv[0], "add_xct") == 0 && cmd_argc == 3) {
		theGroup = find_group(group_list, cmd_argv[1]);
		if (theGroup == NULL) {
			sendMessage(currentClient->soc, "Group does not exist", NOT_FREE_STRING);
		} else {
			char* end;
			double amount = strtod(cmd_argv[2], &end);
			if (end == cmd_argv[2]) {
				sendMessage(currentClient->soc, "Incorrect number format", NOT_FREE_STRING);
			} else {
				outputString = add_xct(theGroup, currentClient->clientName, amount);
				sendMessage(currentClient->soc, outputString, FREE_STRING);
			}
		}
	} else {
		sendMessage(currentClient->soc, "Incorrect syntax", NOT_FREE_STRING);
	}
	return 0;
}


/* Change the group list according to user command
 *
 * userInput : char* - the user input
 * group_list : Group** - the group list
 * return : int - the function returns 1 iff the user want to quit
 */
int processUserInput(ClientManager* clientManager, char* userInput, Group** group_list, int i){
	int cmd_argc;
	int quitCommand;
	char* cmd_argv[INPUT_ARG_MAX_NUM];
	char* tmpString;
	Client* currentClient = &(clientManager->clients[i]);
	if (currentClient->hasName){
		cmd_argc = tokenizeTheInput(currentClient->soc, userInput, cmd_argv);
		quitCommand = processCommand(clientManager, cmd_argc, cmd_argv, group_list, i);
		if (cmd_argc > 0 &&  quitCommand == 1) return 1;
		return 0;
	} else {
		// try to read for name
		tmpString = buildString("Welcome, %s! Please enter Buxfer commands",userInput);
		sendMessage(currentClient->soc, tmpString, FREE_STRING);
		setName(currentClient, userInput);
		return 0;
	}
	return 0;
}

/* Grab commands from clients and process commands
 * Remove clients if the connection is lost of the client want to quit
 * 
 * clientManager : ClientManager* - the client manager
 * readSet : fd_set* - the set of file descriptors that has information to be read 
 * group_list : Group** - the main group link list contains.
 * return : void
 */
void processClientsRequests(ClientManager* clientManager, fd_set* readSet, Group** group_list){
	char* userInput;
	Client* clients = clientManager->clients;
	int i, quit, connectingStatus;
	int maxCount = clientManager->maxI + 1; 
	for ( i = 0 ; i < maxCount ; i++){
		if (clients[i].soc > 0){
			if (FD_ISSET(clients[i].soc, readSet)){				
				connectingStatus = readfromclient(&(clients[i]), &userInput);
				if (connectingStatus == 0){
					// readfromclient retruns 0 when a full command is recieved
					// execute the command
					quit = processUserInput(clientManager, userInput, group_list, i);
					// remove client if client  want to quit
					if (quit) removeClient(clientManager, i);
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
	Group *group_list = NULL;
	fd_set readSet;
	ClientManager clientManager;
	intiManager(&clientManager);
	while(1){
		// Waiting for fd to be ready
		readSet = getClientRespond(&clientManager);
		// read from other clients
		processClientsRequests(&clientManager, &readSet, &group_list);
	}
	return 0;
}
