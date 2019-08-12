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
#include <termios.h>

#include "client.h"
#include "splinter.h"
#include "connectioninfo.h"

#define BUFSIZE 20

int connect_server(int argc, char** argv)
{
	struct server *server = 0;
	int sockfd = -1;

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
	//client_pty(sockfd);

	int n;
	char buffer[BUFSIZE];
	pid_t pid2;
	pid2 = fork();
	if(pid2 < 0)
	{
		error(EXIT_FAILURE, errno, "fork failed");
	}
	else if(pid2 == 0)
	{
		// child reads socket -> stdout
		while(1)
		{
			n = read(sockfd, buffer, BUFSIZE);
			write(STDOUT_FILENO, buffer, n);
		}
	}
	else
	{
		// parent reads stdin -> socket
		while(1)
		{
			n = read(STDIN_FILENO, buffer, BUFSIZE);
			write(sockfd, buffer, n);
		}
	}

error:
	if(sockfd > 0)
		close(sockfd);

	if(server)
		free(server);

	return 0;
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
				nread = read(slavefd, buffer, BUFSIZE);
				write(sockfd, buffer, nread);
			}
		}
		else
		{
			while(1)
			{
				// socket -> slave
				nread = read(sockfd, buffer, BUFSIZE);
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
			nread = read(STDIN_FILENO, buffer, BUFSIZE);
			write(sockfd, buffer, nread);
			memset(buffer, 0, BUFSIZE);
		}
	}
	else
	{
		while(1)
		{
			nread = read(sockfd, buffer, BUFSIZE);
			write(STDOUT_FILENO, buffer, nread);
		}
	}
}
