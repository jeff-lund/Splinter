CC=gcc
CFLAGS=-g

all: server client

server: server.o splintersh.o serverside.o splinter.o	connectioninfo.o myls.o
	$(CC) $(CFLAGS)	-o server server.o splintersh.o serverside.o splinter.o	connectioninfo.o myls.o

client: client.o splintersh.o serverside.o splinter.o	connectioninfo.o myls.o
	$(CC) $(CFLAGS)	-o client client.o splintersh.o serverside.o splinter.o	connectioninfo.o myls.o

clean:
	rm *.o

