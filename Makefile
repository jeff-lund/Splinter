CC=gcc
CFLAGS=-g -Wall -pthread

main: driver.c server.c  splintersh.c serverside.c splinter.c connectioninfo.c 
	$(CC) $(CFLAGS) -o splinter driver.c server.c  splintersh.c serverside.c splinter.c connectioninfo.c client.c 

clean:
	rm splinter
