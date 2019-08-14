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
#include <pthread.h>

#include "thread_read.h"
#include "client.h"
#include "splinter.h"
#include "connectioninfo.h"

#define BUFSIZE 4096
#define NAMESIZE 20
static volatile sig_atomic_t eof = 0;

int connect_server(int argc, char** argv)
{
	int sockfd = -1;
	pthread_t tidp1, tidp2;
	struct descriptors *fds;
	struct server *server = 0;

	server = alloc_serverinfo();
	getconnectioninfo(server, argc, argv);

	printf("%s\n", host(server));
	printf("%s\n", port(server));

	sockfd = s_connect(host(server), port(server), SOCK_STREAM);

	if(sockfd < 0) {
		printf("Failed To Connect To Remote Host.\n");
		return 0;
	}
	printf("Connected to server\n");
	// write username to server
	username(sockfd);
	
	// passes output from server to stdout
	fds = malloc(sizeof(struct descriptors));
	fds->read_in = sockfd;
	fds->write_out = STDOUT_FILENO;
	pthread_create(&tidp1, NULL, thrd_reader, (void *)fds);
	// passes input from stdin to the server
	fds = malloc(sizeof(struct descriptors));
	fds->read_in = STDIN_FILENO;
	fds->write_out = sockfd;
	pthread_create(&tidp2, NULL, thrd_reader, (void *)fds);

	pthread_join(tidp1, NULL);
	pthread_cancel(tidp2);

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
