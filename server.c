#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include "splinter.h"
#include "connectioninfo.h"
#include "serverside.h"
#include "splintersh.h"
#include "server.h"

#define _POSIX_SOURCE 1

sig_atomic_t term;

void sig_hand(int i) { term = 1; }

int server_start(int argc, char* argv[])
{
  int sock;
  int rc;
  char* buf;
  int bufsize = 4096;
  struct server *server = 0;
  int backlog = 10;
  pid_t pid;

  signal(SIGINT, sig_hand);
  signal(SIGTERM, sig_hand);
  signal(SIGUSR1, sig_hand);

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

  while (!term) {
    int peer;
    peer = s_accept(sock);

		if (peer > 0) {
			printf("connected\n");
      if((pid = fork()) < 0)
      {
        error(EXIT_FAILURE, errno, "fork failed");
      }
      else if(pid == 0)
      {
        splinter(peer);
        exit(0);
      }
    }
		else
    {
      break;
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

void*
thrd_fnc(void * arg)
{
  int sockfd = *(int *)arg;
  splinter(sockfd);
  pthread_exit(NULL);
}
