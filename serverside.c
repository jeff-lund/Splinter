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
#include "splinter.h"

int 
setargv(char** argv, const char* buf, size_t bufsize, int count)
{
  int argc, i = 0;
  size_t offset = 0;

  while (i < count && offset < bufsize && buf[offset]) {
    argv[i] = (char*)(buf + offset);
    offset = offset + strlen(argv[i]) + 1;
    ++i;
  }
  argv[i] = 0;
  argc = i;

  return argc;
}

int 
argccount(const char* tokbuf, size_t tokbufsize)
{
  int count = 0;

  for (size_t i = 0; i<tokbufsize; ++i) {
    if ('\0' == tokbuf[i]) {
      ++count;
    }
  }

  return count;
}

int 
r_exec(const char* tokbuf, size_t tokbufsize, int fd)
{
  int rc = -1;
  int tokcount;
  int argc;
  char** argv;

  tokcount = argccount(tokbuf, tokbufsize);
  argv = malloc((tokcount + 1) * sizeof(char*));

  if (!argv) {
		return -1;
  }
  
  memset(argv, 0, sizeof argv);
  argc = setargv(argv, tokbuf, tokbufsize, tokcount);

  if (argc != tokcount) {
		return -1;
  }

  rc = builtin(argc, argv, fd);

  if (argv) {
    free(argv);
  }

  return rc;
}

int 
builtin(int argc, char** argv, int fd)
{
  if (0 == strncmp(argv[0], "ls", 2)) 
    ls(argc, argv, fd);
	

  return 1;
}

int 
server_loop(int client_fd, int log_fd)
{
  char *buf;
	int count;

  buf = malloc(LINEMAX);
  if (!buf) {
    return 0;
  }

  while (1) {
    memset(buf, 0, LINEMAX);
    count = read(client_fd, buf, LINEMAX - 1);
    if (0 == count) {
  		printf("client hungup\n");
      break;
    } else if (count < 0) {
    		printf("error on read\n");
        break;
    }

    r_exec(buf, count, client_fd);
    buf[0] = EOT;
    write(client_fd, buf, 1);
  }

  if (buf) {
    free(buf);
  }

  return 0;
}


