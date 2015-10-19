#ifndef __MYLIB_H__
#define __MYLIB_H__
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/types.h> 
	#include <sys/socket.h>

	void* myMalloc(size_t size, char* error_message);
	void* myRealloc(void *ptr, size_t size, char* error_message);
	void myBind(int fd, const struct sockaddr *sa, socklen_t salen);
	void myListen(int fd, int backlog);
	void mySelect(int nfds, fd_set *readfds);
	int mySocket(int family, int type, int protocol);
	void myClose(int fd);
#endif 
