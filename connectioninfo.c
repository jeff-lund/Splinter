#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <poll.h>
#include "connectioninfo.h"

#define LINEMAX 4096


static const char* DEFAULT_HOST = "10.0.0.69"; // means "any suitable interface"
static const char* DEFAULT_PORT = "8080";

static const char* options = "a:p:";

struct server {
    const char* host;
    const char* port;
};

struct server *
alloc_serverinfo(void)
{
    struct server* server;
    server = malloc(sizeof(struct server));
    if (server != 0) {
        memset(server, 0, sizeof *server);
        server->host = DEFAULT_HOST;
        server->port = DEFAULT_PORT;
    }
    return server;
}

int
getconnectioninfo(struct server *server, int argc, char *argv[])
{
	int optc;
	
	if(!server)	
		goto error;

	while(-1 != (optc = getopt(argc, argv, options))) {
		if('?' == optc) {
			optc = optopt;
		}
		setparams(server, optc, optarg);
	}
error:
	return -1;
}

int
setparams(struct server *server, int opt, char *arg)
{

	if(!server)
		goto error;

	switch(opt) {
		case 'a':	server->host = arg; break;
		case 'p': server->host = arg; break;
		default: break;
	}

error:
	return -1;

}

const char *
host(struct server *server)
{
	const char *host = 0;
	if(server)
		host = server->host;

	return host;
}

const char *
port(struct server *server)
{
	const char *port = 0;
	if(server)
		port = server->port;

	return port;
}

int
serverresponse(int server_fd)
{
	int rc, timeout, buffersize, finished;
	char *buffer;
	struct pollfd server;
	
	server.fd = server_fd;
	server.events = POLLIN;
	server.revents = 0;

	int done = 0;
	timeout = 5000; //Wait till server times out

	buffersize = LINEMAX;
	buffer = malloc(buffersize);

	if(!buffer)
		return -1;
	
	while(!finished) {
		int poll_server;

		poll_server = poll(&server, 1, timeout);

		if(poll_server == 0) {
			printf("Error serverresponse(), poll returned 0, timed out");
			done = 1;
		}
		
		if(poll_server < 0) {
			printf("Error serverresponse(), poll returned greter then 1");
			done = 1;
		}

		else {
			int server_r = 0;
			memset(buffer, 0, buffersize);
			server_r = read(server_fd, buffer, buffersize - 1);
			if(server_r > 0) {
				fprintf(stdout, "%s", buffer);
				done = 1;
			}
			
			else {
				printf("Issue with read in serverresponse()");
				done = 1;
			}	
		}
	}

	if(buffer)
		free(buffer);

	return rc;
}
