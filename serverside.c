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
#include "util.h"

#define _POSIX_SOURCE 1
#define EOT 0x04
#define MAX 4096
sig_atomic_t term;

int
argclloc(char *argv[], char *buffer, int buffersize, int count)
{
  int argc, i, offset = 0;

  while (i < count && offset < buffersize && buffer[offset]) {
    argv[i] = (char*)(buffer + offset);
    offset = offset + strlen(argv[i]) + 1;
    ++i;
  }
  argv[i] = 0;
  argc = i;

  return argc;

}

int
argvlloc(char *buffer, int buffersize)
{
  int count, i = 0;

  for(; i<buffersize; ++i) {
 		if (buffer[i] == '\0')
      ++count;
  }
  return count;
}

int
builtin(int argc, char *argv[], int fd)
{
	int rc = 1, length = 0;
	char *buffer;
	int buffersize = 512;

	buffer = malloc(buffersize);

	if(!buffer)
		return -1;

	memset(buffer, 0, buffersize);
  /*
	if(strncmp(argv[0], "ls", 2)) {
		ls(argc, argv, fd);
	} else {
		rc = 1;
	}
  */
	if(length > 0)
		write(fd, buffer, length);

	if(buffer)
		free(buffer);

	return rc;
}

int
r_exec(char *buffer, int buffersize, int fd)
{
	int rc, argc, argvn;
	char **argv;

	argvn = argvlloc(buffer, buffersize);
	argv = malloc((argvn + 1) * sizeof(char*));

	if(!argv)
		return -1;

	memset(argv, 0, sizeof(*argv));
	argc = argclloc(argv, buffer, buffersize, argvn);

	if(argc != argvn) {
		printf("Error on r_exec\n");
		return -1;
	}

	rc = builtin(argc, argv, fd);

	if(argv)
		free(argv);

	return rc;
}

int
server_loop(int client_fd, int log_fd)
{
	char *buffer;
	int buffersize, rc;

	buffersize = MAX;

	buffer = malloc(buffersize);
	if(!buffer) {
		return 0;
	}

	while(1) {
		memset(buffer, 0, buffersize);
		rc = read(client_fd, buffer, buffersize - 1);
		if(rc == 0) {
			printf("The client hungup\n");
			break;
		}

		if(rc < 0) {
			printf("Error on read from server_loop");
			break;
		}

		r_exec(buffer, rc, client_fd);
		buffer[0] = EOT;
		write(client_fd, buffer, 1);
	}

	if(buffer)
		free(buffer);

	return 0;
}
