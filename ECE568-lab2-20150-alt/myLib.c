#include "myLib.h"

/*
 * The my* library
 *
 * These function imitate the real C function. 
 * However, if the original functrion return an error signal
 * all functions in my library with print out an error and exit 
 */

/*
 * If malloc successfuly executed then the function behave like normal malloc
 * If there is an error, the function wil terminate the program and print out the error
 *
 * size : size_t - the size of memory wanted to be allocated
 * error_message : char* - the output string if an error occurs
 *                         if NULL then the function will print Malloc error
 * return : void* - the pointer to newly allocated memory
 */
void* myMalloc(size_t size, char* error_message){
	void* result = malloc(size);
	if (result == NULL){
		if (error_message != NULL){
			perror(error_message);
		} else {
			perror("Malloc error");
		}
		exit(1);
	}
	return result;
}

/*
 * If realloc successfuly executed then the function behave like normal realloc
 * If there is an error, the function wil terminate the program and print out the error
 *
 * size : size_t - the size of memory wanted to be allocated
 * error_message : char* - the output string if an error occurs
 *                         if NULL then the function will print Malloc error
 * return : void* - the pointer to newly allocated memory
 */
void* myRealloc(void *ptr, size_t size, char* error_message){
	void* result = realloc(ptr, size);
	if (result == NULL){
		if (error_message != NULL){
			perror(error_message);
		} else {
			perror("Realloc error");
		}
		exit(1);
	}
	return result;
}

/*
 * If read successfuly executed then the function behave like normal read
 * If there is an error, the function wil terminate the program and print out the error
 *
 * fd : int - the file descriptor
 * buf : void* - the buffer
 * count : size_t - the number of bytes
 * return : ssize_t - the succesfull number of bytes read
 */
ssize_t myRead(int fd, void *buf, size_t count){
	int result = read(fd, buf, count);
	if (result < 0){
		perror("read error");
		exit(1);
	}
	return result;
}

/*
 * If write successfuly executed then the function behave like normal write
 * If there is an error, the function wil terminate the program and print out the error
 *
 * fd : int - the file descriptor
 * buf : void* - the buffer
 * count : size_t - the number of bytes want to be written
 * return : ssize_t - the succesfull number of bytes written
 */
ssize_t myWrite(int fd, void *buf, size_t count){
	int result = write(fd, buf, count);
	if (result < 0){
		perror("read error");
		exit(1);
	}
	return result;
}


/*
 * If accept successfuly executed then the function behave like normal accept
 * If there is an error, the function wil terminate the program and print out the error
 *
 * fd : int - the file descriptor
 * return : int - the file descriptor to communicate with the new client
 */
int myAccept(int fd){
	int  n;
	if ( (n = accept(fd, NULL, NULL)) < 0) {
		perror("accept error");
		exit(1);
	}
	return(n);
}

/*
 * If bind successfuly executed then the function behave like normal bind
 * If there is an error, the function wil terminate the program and print out the error
 *
 * fd : int - the file descriptor
 * sa : const struct sockaddr* - the information about the server
 * salen : socklen_t - the length of the information
 * return : void
 */
void myBind(int fd, const struct sockaddr *sa, socklen_t salen){
    if (bind(fd, sa, salen) < 0){
        perror("bind error");
        exit(1);
    }
}

/*
 * If listen successfuly executed then the function behave like normal listen
 * If there is an error, the function wil terminate the program and print out the error
 *
 * fd : int - the file descriptor
 * backlog : int - the total number of client can be in the queue
 * return : void
 */
void myListen(int fd, int backlog){
	if (listen(fd, backlog) < 0) {
		perror("listen error");
		exit(1);
	}
}


/*
 * If select successfuly executed then the function behave like normal select
 * If there is an error, the function wil terminate the program and print out the error
 *
 * fd : int - the file descriptor
 * readfds : fd_set* - the set of all opening file descriptor
 * return : void
 */
void mySelect(int nfds, fd_set *readfds){
	int n;
	if ( (n = select(nfds, readfds, NULL, NULL, NULL)) < 0) {
		perror("select error");
		exit(1);
	}
}


/*
 * If socket successfuly executed then the function behave like normal socket
 * If there is an error, the function wil terminate the program and print out the error
 *
 * family : int - the family type
 * type : int - the type of the connection
 * protocol : int - the protocol type 
 * return : int - the file descriptor for the socket
 */
int mySocket(int family, int type, int protocol){
	int n;
	if ( (n = socket(family, type, protocol)) < 0) {
		perror("socket error");
		exit(1);
	}
	return(n);
}

/*
 * If close successfuly executed then the function behave like normal close
 * If there is an error, the function wil terminate the program and print out the error
 *
 * fd : int - the file descriptor
 * return : void
 */
void myClose(int fd){
	if (close(fd) == -1) {
		perror("close error");
		exit(1);
	}
}
