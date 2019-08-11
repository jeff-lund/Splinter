#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "splinter.h"

#define _POSIX_SOURCE 1


int main(int argc, char* argv[])
{
  int sock;
  int rc;
  char* buf;
  int bufsize = 4096;
  struct server *server = 0;
  int backlog = 10;
	pid_t child;

  server = alloc_serverinfo();
  getconnectioninfo(server, argc, argv);

  sock = -1;

  buf = malloc(bufsize);
  if (!buf) {
    goto out;
  }
  memset(buf, 0, bufsize);

	printf("%s\n", port(server));
	printf("%s\n", host(server));

	sock = s_bind(host(server), port(server));
  if (sock < 0) {
		printf("error on bind() when binding to host and port\n");
    goto out;
  }

  rc = s_listen(sock, backlog);
  if (rc < 0) {
		printf("Listen() returned when listening to sock.\n");
    goto out;
  }

  fprintf(stderr, "pid: %d\n", getpid());

  while (1) {
    int peer;
    peer = s_accept(sock);
    if((child = fork()) == 0) {
			while(1) { 
				if (peer > 0) {
					printf("connected\n");
					server_loop(peer, STDERR_FILENO);
		      close(peer);
		    } else {
			      break;   
				}
			}
    } 
	}

    fprintf(stderr, "good-bye.\n");

out:
  if(sock > 0) 
    close(sock);
    
  if(buf)
    free(buf); 

	if(server != 0)
		free(server);

  return 0;
}
