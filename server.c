#define _GNU_SOURCE
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
#include <setjmp.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <fcntl.h>

#include "splinter.h"
#include "connectioninfo.h"
#include "serverside.h"
#include "server.h"

#define _POSIX_SOURCE 1
#define BUFSIZE 4096
sig_atomic_t term;
sigjmp_buf jump;

void sig_hand(int i)
{
  term = 1;
  siglongjmp(jump, 0);
}

int server_start(int argc, char* argv[])
{
  int sock;
  int rc;
  char* buf;
  int bufsize = 4096;
  struct server *server = 0;
  int backlog = 10;
  int peer;
  pid_t pid;
  pthread_t tid;
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
  sigsetjmp(jump, 0);
  while (!term) {

    int on = 1;
    peer = s_accept(sock);
    setsockopt(peer, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(int));
		if (peer > 0) {
			printf("connected\n");

      if((pid = fork()) < 0) {
        error(EXIT_FAILURE, errno, "fork failed");
      }
      else if(pid == 0) {
          create_pty(peer);
          exit(0);
      }
      else
      {
        pthread_create(&tid, NULL, &thrd_fnc, (void *)&pid);
        sleep(1);
        close(peer);
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
  int pid = *(int *)arg;
  waitpid(pid, NULL, 0);
  printf("Thread closed, %d\n", pid);
  pthread_exit(NULL);
}

void
create_pty(int peer)
{
  int ptyslave, ptymaster;
  int nread;
  char name[50], *nameptr;
  char buffer[BUFSIZE];
  pid_t pid;
  struct pollfd pl[1];
  char *dummy_args[1];
  dummy_args[0] = NULL;
  // open PTY
  if((ptymaster = posix_openpt(O_RDWR | O_NOCTTY)) < 0) {
    // error in opening pty
    close(peer);
    error(EXIT_FAILURE, errno, "failure when opening pty");
  }
  if(grantpt(ptymaster) < 0) {
    close(peer);
    error(EXIT_FAILURE, errno, "grantpt failed");
  }
  if(unlockpt(ptymaster) < 0) {
    close(peer);
    error(EXIT_FAILURE, errno, "unlockpt failed");
  }

  // Get name of pty master opened
  if((nameptr = ptsname(ptymaster)) == NULL) {
    error(EXIT_FAILURE, errno, "failed to obtain name of pty-master");
  }
  strncpy(name, nameptr, 50);


  if((pid = fork()) < 0) {
    error(EXIT_FAILURE, errno, "fork failed");
  }
  else if(pid == 0)
  {
    // child
    setsid();
    // open pty slave
    if((ptyslave = open(name, O_RDWR)) < 0) {
      error(EXIT_FAILURE, errno, "failed to open pty slave");
    }
    close(ptymaster);
    dup2(ptyslave, STDIN_FILENO);
    dup2(ptyslave, STDOUT_FILENO);
    dup2(ptyslave, STDERR_FILENO);
    execv("./shell", dummy_args);
    error(EXIT_FAILURE, errno, "exec shell failed in child");
  }

  // Master
  pid_t pid2;
  pid2 = fork();
  if(pid2 == 0)
  {
    // CHILD reads pollin
    while(1) {
      if((nread = read(peer, buffer, BUFSIZE)) < 0) {
        error(EXIT_FAILURE, errno, "read failure from socket to master pty");
      }
      write(ptymaster, buffer, nread);
    }
  }
  else
  {
    // PARENT writes
    while(1) {
      if((nread = read(ptymaster, buffer, BUFSIZE)) < 0) {
        error(EXIT_FAILURE, errno, "read failure from master pty failed");
      }
      printf("Writing: %s\n", buffer);
      write(peer, buffer, nread);
      memset(buffer, 0, BUFSIZE);
    }
  }

  close(ptymaster);
  return;
}
/*
void thr_read(void * arg) {

}

void thr_write(void *arg) {

}
*/
