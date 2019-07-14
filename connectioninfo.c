#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "connectioninfo.h"

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
