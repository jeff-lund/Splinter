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
#include "splintersh.h"

#define _POSIX_SOURCE 1
#define EOT 0x04
#define MAX 4096
sig_atomic_t term;


int 
server_loop(int rdfd, int wrfd)
{
	char *buffer;
	int buffersize, rc;
  char good[] = {"good"};
  char bad[] = {"bad"};

	buffersize = MAX;

	buffer = malloc(buffersize);
	if(!buffer) {
		return 0;
	}

	while(!term) {
		memset(buffer, 0, buffersize);
		rc = read(rdfd, buffer, buffersize - 1);
		if(rc == 0)
			printf("The client hungup\n");
			break;

		if(rc < 0) {
			printf("Error on read from server_loop");
			break;
		}

		Exec(buffer);
	}

		if(term)
			write(wrfd, good, sizeof good);
		else
			write(wrfd, bad, sizeof bad);
		
	if(buffer)
		free(buffer);
	kill(getppid(), SIGUSR1);
	return 0;
}
