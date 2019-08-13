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
#include <poll.h>
#include <fcntl.h>

#include "splinter.h"
#include "connectioninfo.h"
#include "server.h"
#include "thread_read.h"

#define _POSIX_SOURCE 1
#define BUFSIZE 4096
#define NAMESIZE 20

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
  int nread;
  int backlog = 10;
  int peer;
  char* buf;
  char uname[NAMESIZE];
  struct server *server = 0;
  pid_t pid;
  pthread_t tid;

  signal(SIGINT, sig_hand);
  signal(SIGTERM, sig_hand);
  signal(SIGUSR1, sig_hand);

  server = alloc_serverinfo();
  getconnectioninfo(server, argc, argv);

  sock = -1;

  buf = malloc(BUFSIZE);
  if (!buf) {
    goto out;
  }
  memset(buf, 0, BUFSIZE);

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
  // when server recieves signal return to start of loop to terminate
  sigsetjmp(jump, 0);
  while (!term) {
    peer = s_accept(sock);
		if (peer > 0) {
			printf("connected\n");
      if((nread = read(peer, buf, BUFSIZE)) < 0)
        error(EXIT_FAILURE, errno, "read failed");
      strncpy(uname, buf, NAMESIZE);
      if((pid = fork()) < 0) {
        error(EXIT_FAILURE, errno, "fork failed");
      }
      else if(pid == 0) {
          // fork off new process to create psuedoterminal
          // and exec a shell for the client
          create_pty(peer, uname);
          exit(0);
      }
      else {
        // create a thread to wait for the child
        pthread_create(&tid, NULL, &thrd_wait, (void *)&pid);
        sleep(1);
        close(peer);
      }
    }
		else{
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

void
create_pty(int peer, char *uname)
{
  int ptyslave, ptymaster;
  char name[50], *nameptr;
  char *shell_args[2] = {uname, NULL};
  pid_t pid;
  struct descriptors *fds;
  pthread_t tidp1, tidp2;
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
    // child opens pty slave
    setsid();
    if((ptyslave = open(name, O_RDWR)) < 0) {
      error(EXIT_FAILURE, errno, "failed to open pty slave");
    }
    // close the master fd and reroute all regular I/O fds to the salve fd
    close(ptymaster);
    dup2(ptyslave, STDIN_FILENO);
    dup2(ptyslave, STDOUT_FILENO);
    dup2(ptyslave, STDERR_FILENO);
    execv("./shell", shell_args);
    error(EXIT_FAILURE, errno, "exec shell failed in child");
  }
  else
  {
    // Master process
    // passes input from client into pty
    fds = malloc(sizeof(struct descriptors));
    fds->read_in = peer;
    fds->write_out = ptymaster;
    pthread_create(&tidp1, NULL, thrd_reader, (void *)fds);
    // passes output from pty to the client
    fds = malloc(sizeof(struct descriptors));
    fds->read_in = ptymaster;
    fds->write_out = peer;
    pthread_create(&tidp2, NULL, thrd_reader, (void *)fds);

    pthread_join(tidp1, NULL);
    pthread_join(tidp2, NULL);
  }
  waitpid(pid, 0, 0);

  return;
}
