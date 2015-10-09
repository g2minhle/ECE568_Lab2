#ifndef __CLIENTMANAGER_H__
#define	__CLIENTMANAGER_H__
	/* According to POSIX.1-2001 */
       #include <sys/select.h>
       /* According to earlier standards */
	#include <sys/types.h>
	#include <unistd.h>
	#include <string.h>
	#include <stdlib.h>
	#include <arpa/inet.h>
	#include "myLib.h"
	#include "utility.h"

	#ifndef PORT
		#define PORT 8765
	#endif

	#define MAXLINE 1024
	#define MAXCLIENTS 30
	#define LISTENQ 10

	#define INPUT_BUFFER_SIZE 256
	#define INPUT_ARG_MAX_NUM 5
	#define DELIM " \n"

	#define FREE_STRING 0
	#define NOT_FREE_STRING 1

	/* use these strings to tell the marker what is happening */
	#define FMT_ACCEPT_ERR "ECE568-SERVER: SSL accept error\n"
	#define FMT_CLIENT_INFO "ECE568-SERVER: %s %s\n"
	#define FMT_OUTPUT "ECE568-SERVER: %s %s\n"
	#define FMT_INCOMPLETE_CLOSE "ECE568-SERVER: Incomplete shutdown\n"

	typedef struct {
		int soc;
		int curpos; // point to the first \0 character
		int hasName;
		char buf[MAXLINE];
		char clientName[MAXLINE];
	} Client;

	typedef struct {
		int maxI; // the index of the biggest item/ the max index/ real index
		int maxFD;
		int socketFD;
		fd_set allSet;
		Client clients[MAXCLIENTS];
	} ClientManager;

	void intiManager(ClientManager* clientManager);
	fd_set getClientRespond(ClientManager* clientManager);
	void sendMessage(int fd, char* message, int willFreeString);	
	void removeClient(ClientManager* clientManager, int index);
	int readfromclient(Client *c, char** outputString);
	void setName(Client* client, char* userName);
#endif
