#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <poll.h>
#include <error.h>
#include <errno.h>
#include "splinter.h"

char *options = "a:p:";


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
        server->host = DEAFULT_HOST;
        server->port = DEAFULT_PORT;
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
		case 'p': server->port = arg; break;
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
	int rc = 0;
  ssize_t rc_b;
  char* buf;
  int timeout = 5000;
  struct pollfd server;

  server.fd = server_fd;
  server.events = POLLIN;
  server.revents = 0;


  buf = malloc(LINEMAX);
  if (!buf) {
		return -1;
  }

  while (1) {
    int poll_rc;
    poll_rc = poll(&server, 1, timeout);

    if (poll_rc == 0) {
			printf("error on poll timeout\n");
			break;
	  }	
    if (poll_rc < 0) {
			printf("error on poll()");
			break;
    } else {
      memset(buf, 0, LINEMAX);
      rc_b = read(server_fd, buf, LINEMAX - 1);
      if (rc_b > 0) {
        fprintf(stdout, "%s", buf);
        if (EOT == buf[rc_b - 1]) {
          rc = 0;
					break;
        }
      } else {
					printf("error on read() serverresponse()\n");
					break;
      }
    } 
  } 

  if(buf) {
    free(buf);
	}
    
  return rc;
}
