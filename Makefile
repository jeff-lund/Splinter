CC=gcc
CFLAGS=-g -Wall

all: server client

server: server.o serverside.o connectioninfo.o splintersh.o splinter.o ls.o
	$(CC) $(CFLAGS)	-o $@ server.o serverside.o connectioninfo.o splintersh.o splinter.o ls.o

client: client.o serverside.o connectioninfo.o splintersh.o splinter.o ls.o
	$(CC) $(CFLAGS)	-o $@ client.o serverside.o connectioninfo.o splintersh.o splinter.o ls.o

clean:
	rm *.o

