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
<<<<<<< HEAD
=======
#define MAX 1028
>>>>>>> ae6b7a72f45666638871305c411fc2c73cb7ea03
#define EOT 0x04
#define MAX 4096
sig_atomic_t term;


int 
server_loop(int rdfd, int wrfd)
{
	char *buffer;
	int buffersize, rc;
  char ack[] = {"aye"};
  char nack[] = {"nay"};

	buffersize = MAX;

	buffer = malloc(buffersize);
	if(!buffer) {
		return 0;
	}
<<<<<<< HEAD

	while(!term) {
		memset(buffer, 0, buffersize);
		rc = read(rdfd, buffer, buffersize - 1);
		if(rc == 0)
=======
	
	while(1) {
		int rc;
		memset(buffer, 0, MAX);
		rc = read(client_fd, buffer, MAX - 1);
		if(rc == 0) {
			printf("The client hungup\n");
>>>>>>> ae6b7a72f45666638871305c411fc2c73cb7ea03
			break;

		if(rc < 0) {
			printf("Error on read from server_loop");
			break;
		}

<<<<<<< HEAD
		Exec(buffer);
=======
		exec_command_remotely(buffer, rc, client_fd);
		buffer[0] = EOT;
		write(client_fd, buffer, 1);
	}
>>>>>>> ae6b7a72f45666638871305c411fc2c73cb7ea03

		if(term)
			write(wrfd, nack, sizeof nack);
		else
			write(wrfd, ack, sizeof ack);
		
	}
	if(buffer)
		free(buffer);
	kill(getppid(), SIGUSR1);
	return 0;
}
