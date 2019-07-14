#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "splinter.h"
#include "connectioninfo.h"

int main(int argc, char** argv)
{
	struct server *server = 0;
	int socketfd = -1;

	server = alloc_serverinfo();
	socketfd = s_connect(host(server), port(server), SOCK_STREAM);

	if(socketfd < 0) {
		printf("Failed To Connect To Remote Host.\n");
		goto error;
	}


error:
	if(socketfd > 0)
		close(socketfd);

	if(server != 0)
		free(server);	

	return 0;
}

