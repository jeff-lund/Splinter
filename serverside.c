#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/acct.h>
#include <sys/time.h>
#include <error.h>
#include <errno.h>


#include <sys/socket.h>
#include "splinter.h"
#include "connectioninfo.h"

#define _POSIX_SOURCE 1

#define EOT 0x04

int 
exec_command_remotely(const char *buffer, int buffersize, int fd)
{
	int rc;
	Exec(buffer);
}

int 
server_loop(int client_fd, int log_fd)
{
	char *buffer;
	int buffersize;

	buffersize = MAX;

	buffer = malloc(buffersize);
	if(!buffer) {
		kill(getpid(), SIGUSR1);
		return 0;
	}
	
	while(1) {
		int rc;
		memset(buffer, 0, MAX);
		rc = read(client_fd, buffer, MAX - 1)
		if(rc == 0) {
			printf("The client hungup\n");
			break;
		}

		if(rc < 0) {
			printf("read returned greater then 0 in server_loop\n");
			break;
		}

		exec_command_remotely(buf, rc, client_fd);
		buf[0] = EOT;
		write(client_fd, buffer, 1);
	}

	if(buffer)
		free(buffer);
	return 0;
}
