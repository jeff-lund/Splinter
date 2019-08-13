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
#include <poll.h>
#include <fcntl.h>
#include <wait.h>

#include "client.h"
#include "splinter.h"
#include "connectioninfo.h"

#define BUFSIZE 4096
#define NAMESIZE 20
static volatile sig_atomic_t eof = 0;

void
sigterm(int signo)
{
	eof = 1;
}

void
sigpipe(int signo)
{
	eof = 1;
}

int connect_server(int argc, char** argv)
{
	int sockfd = -1;
	int n;
	char buffer[BUFSIZE];
	pid_t pid;

	struct server *server = 0;
	// sigpipe not working
	sigset_t set;
	struct sigaction sa;
	sigemptyset(&set);
	sa.sa_handler = sigpipe;
	sa.sa_mask = set;
	sa.sa_flags = 0;
	if(sigaction(SIGPIPE, &sa, NULL) < 0)
		error(EXIT_FAILURE, errno, "sigaction failed");
	// handle sigterm
	sa.sa_handler = sigterm;
	if(sigaction(SIGTERM, &sa, NULL) < 0)
		error(EXIT_FAILURE, errno, "sigaction failed");

	server = alloc_serverinfo();
	getconnectioninfo(server, argc, argv);

	printf("%s\n", host(server));
	printf("%s\n", port(server));

	sockfd = s_connect(host(server), port(server), SOCK_STREAM);

	if(sockfd < 0) {
		printf("Failed To Connect To Remote Host.\n");
		goto error;
	}
	printf("Connected to server\n");
	// write username to server
	username(sockfd);
	// fork off processes to read and write
	pid = fork();
	if(pid < 0)
	{
		error(EXIT_FAILURE, errno, "fork failed");
	}
	else if(pid == 0)
	{
		pid = getppid();
		printf("Parent pid: %d\n", pid);
		// child reads socket -> stdout
		while(!eof)
		{
			if((n = read(sockfd, buffer, BUFSIZE)) <= 0)
				break;
			if(write(STDOUT_FILENO, buffer, n) < 0)
				break;
		}
		printf("child exiting\n");
		exit(0);
	}
	else
	{
		printf("child pid: %d\n", pid);
		// parent reads stdin -> socket
		while(!eof)
		{
			// stuck in this loop
			if((n = read(STDIN_FILENO, buffer, BUFSIZE)) <= 0)
				break;
			if(write(sockfd, buffer, n) < 0)
			{
				break;
			}
		}
		printf("parent exiting\n");
		kill(pid, SIGTERM);
		wait(0);
	}

error:
	if(sockfd > 0)
		close(sockfd);

	if(server)
		free(server);

	return 0;
}

void
username(int fd)
{
	char name[NAMESIZE];
	strncpy(name, getenv("USER"), NAMESIZE);
	name[NAMESIZE - 1] = '\0';
	write(fd, name, strlen(name));
}

// Not working, output messed up
void
client_pty(int sockfd)
{
	int masterfd, slavefd;
	int nread;
	char buffer[BUFSIZE];
	char *nameptr, name[50];
	pid_t pid, pid2;
	// set up ptys
	if((masterfd = posix_openpt(O_RDWR)) < 0) {
		fprintf(stderr, "failed to aquire master pty on client\n");
		return;
	}
	if(unlockpt(masterfd) < 0) {
    close(sockfd);
    error(EXIT_FAILURE, errno, "unlockpt failed");
  }

  // Get name of pty master opened
  if((nameptr = ptsname(masterfd)) == NULL) {
    error(EXIT_FAILURE, errno, "failed to obtain name of pty-master");
  }
  strncpy(name, nameptr, 50);

	if((pid = fork()) < 0) {
    error(EXIT_FAILURE, errno, "fork failed");
  }
  else if(pid == 0)
  {
		// child
		// slave, gets input from master, sends to socket
    setsid();
    // open pty slave
    if((slavefd = open(name, O_RDWR)) < 0) {
      error(EXIT_FAILURE, errno, "failed to open pty slave");
    }
		close(masterfd);

		pid2 = fork();
		if(pid2 < 0)
			error(EXIT_FAILURE, errno, "fork failed");
		else if(pid2 == 0)
		{
			while(1) {
				// slave -> socket
				if((nread = read(slavefd, buffer, BUFSIZE)) <0)
					break;
				write(sockfd, buffer, nread);
			}
			exit(0);
		}
		else
		{
			while(1)
			{
				// socket -> slave
				if((nread = read(sockfd, buffer, BUFSIZE)) < 0)
					break;
				write(slavefd, buffer, nread);
			}
		}

		close(slavefd);
		exit(0);
	}
	// master
	// gets input from user, sends to slave
	// reads from slave and outputs to screen
	pid2 = fork();
	if(pid2 < 0)
		error(EXIT_FAILURE, errno, 0);
	else if(pid2 == 0)
	{
		while(1)
		{
			if((nread = read(STDIN_FILENO, buffer, BUFSIZE)) < 0)
				break;
			write(sockfd, buffer, nread);
			memset(buffer, 0, BUFSIZE);
		}
		exit(0);
	}
	else
	{
		while(1)
		{
			if((nread = read(sockfd, buffer, BUFSIZE)) < 0)
				break;
			write(STDOUT_FILENO, buffer, nread);
		}
		exit(0);
	}
}
