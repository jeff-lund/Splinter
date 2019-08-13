#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <wait.h>

#include "thread_read.h"
#define BUFSIZE 1024

void *
thrd_reader(void *arg)
{
  struct descriptors fds = *(struct descriptors *)arg;
  int nread;
  int err = 0;
  char buffer[BUFSIZE];
  while(1) {
    if((nread = read(fds.read_in, buffer, BUFSIZE)) < 0) {
        err = 1;
        break;
    }
    else if(nread == 0) {
      // eof detected
      break;
    }
    write(fds.write_out, buffer, nread);
    memset(buffer, 0, BUFSIZE);
  }
  free(arg);
  if(err)
    pthread_exit((void *) -1);
  else
    pthread_exit((void *) 0);
}

void*
thrd_wait(void * arg)
{
  int pid = *(int *)arg;
  waitpid(pid, NULL, 0);
  printf("Thread closed, %d\n", pid);
  pthread_exit(NULL);
}