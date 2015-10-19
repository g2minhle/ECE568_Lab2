CC := gcc
CFLAGS := -Wall -Werror -g
LIBS := -lssl -lcrypto
LDFLAGS := $(LIBS)
RM := rm -f

sources := client.c server.c myLib.c utility.c clientManager.c
targets := client server 

.PHONY: clean default all

default: all
all: utility.o myLib.o clientManager.o $(targets)

client: client.o myLib.o utility.o
	$(CC) $(LDFLAGS) -o client client.o myLib.o utility.o

server: server.o myLib.o utility.o clientManager.o
	$(CC) $(LDFLAGS) -o server server.o myLib.o utility.o clientManager.o

client.o: client.c client.h
	$(CC) $(CFLAGS) -c -o client.o client.c

server.o: server.c server.h
	$(CC) $(CFLAGS) -c -o server.o  server.c

myLib.o: myLib.c myLib.h
	$(CC) $(CFLAGS) -c -o myLib.o myLib.c

utility.o: utility.c utility.h
	$(CC) $(CFLAGS) -c -o utility.o utility.c

clientManager.o: clientManager.c clientManager.h
	$(CC) $(CFLAGS) -c -o clientManager.o clientManager.c

clean:
	$(RM) $(targets) $(sources:.c=.o) *~

