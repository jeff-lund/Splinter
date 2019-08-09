#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "splinter.h"
#include "splintersh.h"
#include "connectioninfo.h"

#define LINEMAX 4096

int connect_server(int argc, char** argv)
{
	struct server *server = 0;
	int sockfd = -1;
	int n;
	char buffer[LINEMAX];
	server = alloc_serverinfo();
	getconnectioninfo(server, argc, argv);

	printf("%s\n", host(server));
	printf("%s\n", port(server));

	sockfd = s_connect(host(server), port(server), SOCK_STREAM);

	if(sockfd < 0) {
		printf("Failed To Connect To Remote Host.\n");
		goto error;
	}
	
	while(1)
	{
		// print prompt
		n = read(sockfd, buffer, LINEMAX);
		write(STDOUT_FILENO, buffer, n);
		memset(buffer, 0, LINEMAX);
		// get input from user
		n = read(STDIN_FILENO, buffer, LINEMAX);
		write(sockfd, buffer, LINEMAX);
		memset(buffer, 0, LINEMAX);
		//write out response from server
		n = read(sockfd, buffer, LINEMAX);
		write(STDOUT_FILENO, buffer, LINEMAX);
	}


error:
	if(sockfd > 0)
		close(sockfd);

	if(server != 0)
		free(server);

	return 0;
}
