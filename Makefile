CC=gcc
CFLAGS=-g -Wall -pthread

main: driver.c server.c  splintersh.c serverside.c splinter.c connectioninfo.c 
	$(CC) $(CFLAGS) -o splinter driver.c server.c  splintersh.c serverside.c splinter.c connectioninfo.c client.c

all: server client

server: server.o splintersh.o serverside.o splinter.o connectioninfo.o
	$(CC) $(CFLAGS)	-o server server.o splintersh.o serverside.o splinter.o	connectioninfo.o

client: client.o splintersh.o serverside.o splinter.o connectioninfo.o
	$(CC) $(CFLAGS)	-o client client.o splintersh.o serverside.o splinter.o	connectioninfo.o

clean:
	rm *.o

