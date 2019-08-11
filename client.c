#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <fcntl.h>

#include "splinter.h"
#include "splintersh.h"
#include "connectioninfo.h"

#define LINEMAX 4096


int connect_server(int argc, char** argv)
{
	struct server *server = 0;
	struct pollfd pl[1];
	int sockfd = -1;
	int n;
	char buffer[LINEMAX];

	server = alloc_serverinfo();
	getconnectioninfo(server, argc, argv);

	printf("%s\n", host(server));
	printf("%s\n", port(server));

	sockfd = s_connect(host(server), port(server), SOCK_STREAM);
	// 0 - reader
	pl[0].fd = sockfd;
	pl[0].events = POLLIN | POLLOUT;

	if(sockfd < 0) {
		printf("Failed To Connect To Remote Host.\n");
		goto error;
	}

	while(1)
	{
		if(poll(pl, 1, 100) < 0)
		{
			break;
		}

		if(pl[0].revents & POLLOUT)
		{
			// get input from user
			n = read(STDIN_FILENO, buffer, LINEMAX);
			write(sockfd, buffer, LINEMAX);
			memset(buffer, 0, LINEMAX);
		}
		if(pl[0].revents & POLLIN) {
			while((n = recv(sockfd, buffer, LINEMAX, MSG_DONTWAIT)) > 0)
			{
				write(STDOUT_FILENO, buffer, n);
			}
			memset(buffer, 0, LINEMAX);
		}
	}


error:
	if(sockfd > 0)
		close(sockfd);

	if(server != 0)
		free(server);

	return 0;
}
