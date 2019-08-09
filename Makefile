CC=gcc
CFLAGS=-g -Wall

all: server client

server: server.o splintersh.o serverside.o splinter.o connectioninfo.o
	$(CC) $(CFLAGS)	-o server server.o splintersh.o serverside.o splinter.o	connectioninfo.o

client: client.o splintersh.o serverside.o splinter.o connectioninfo.o
	$(CC) $(CFLAGS)	-o client client.o splintersh.o serverside.o splinter.o	connectioninfo.o

clean:
	rm *.o

