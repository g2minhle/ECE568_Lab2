CC := gcc
CFLAGS := -Wall -Werror -g
LIBS := -lssl -lcrypto
LDFLAGS := $(LIBS)
RM := rm -f

sources := client.c server.c lists.c myLib.c utility.c clientManager.c
targets := client server 

.PHONY: clean default all

default: all
all: utility.o myLib.o lists.o clientManager.o $(targets)

client: client.o
	$(CC) $(LDFLAGS) -o client client.o

server: server.o lists.o myLib.o utility.o clientManager.o
	$(CC) $(LDFLAGS) -o server server.o lists.o myLib.o utility.o clientManager.o

client.o: client.c
	$(CC) $(CFLAGS) -c -o client.o client.c

server.o: server.c
	$(CC) $(CFLAGS) -c -o server.o  server.c

lists.o:
	$(CC) $(CFLAGS) -c -o lists.o lists.c

myLib.o: 
	$(CC) $(CFLAGS) -c -o myLib.o myLib.c

utility.o: 
	$(CC) $(CFLAGS) -c -o utility.o utility.c

clientManager.o:
	$(CC) $(CFLAGS) -c -o clientManager.o clientManager.c

clean:
	$(RM) $(targets) $(sources:.c=.o) *~

