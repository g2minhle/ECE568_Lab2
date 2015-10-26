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

clientNoCert: clientNoCert.o myLib.o utility.o
	$(CC) $(LDFLAGS) -o clientNoCert client.o myLib.o utility.o

server: server.o myLib.o utility.o clientManager.o
	$(CC) $(LDFLAGS) -o server server.o myLib.o utility.o clientManager.o

serverNoSSLv3NoTSLv1: server.o myLib.o utility.o clientManagerNoSSLv3NoTSLv1.o
	$(CC) $(LDFLAGS) -o serverNoSSLv3NoTSLv1 server.o myLib.o utility.o clientManagerNoSSLv3NoTSLv1.o	
	
serverMD5: server.o myLib.o utility.o clientManagerMD5.o
	$(CC) $(LDFLAGS) -o serverMD5 server.o myLib.o utility.o clientManagerMD5.o		

client.o: client.c client.h
	$(CC) $(CFLAGS) -c -o client.o client.c

clientNoCert.o: client.c client.h
	$(CC) $(CFLAGS) -c -o client.o client.c -DOPENSSL_LOAD_CERT=0

server.o: server.c server.h
	$(CC) $(CFLAGS) -c -o server.o  server.c

myLib.o: myLib.c myLib.h
	$(CC) $(CFLAGS) -c -o myLib.o myLib.c

utility.o: utility.c utility.h
	$(CC) $(CFLAGS) -c -o utility.o utility.c

clientManagerNoSSLv3NoTSLv1.o: clientManager.c clientManager.h
	$(CC) $(CFLAGS) -c -o clientManagerNoSSLv3NoTSLv1.o clientManager.c  -DOPENSSL_SSL_METHOD=TLSv1_2_server_method\(\)

clientManagerMD5.o: clientManager.c clientManager.h
	$(CC) $(CFLAGS) -c -o clientManagerMD5.o clientManager.c -DOPENSSL_CIPHER_USED=\"MD5\"
	
testClientAndServer: serverNoSSLv3NoTSLv1 serverMD5 clientNoCert 

clean:
	$(RM) $(targets) $(sources:.c=.o) *~ *.o

