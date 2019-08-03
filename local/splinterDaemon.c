/*
  Creates a daemon for splinter to manage all system splinters
  Start up a local splinter
  Listen for incoming connection requests
  Connect users to running splinter
  Close a running splinter
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <wordexp.h>

#define LOCKFILE "~/.splinter/splintered.pid"
#define LOCKMODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define DAE_EXISTS -2

int
lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return(fcntl(fd, F_SETLK, &fl));
}

/*
  Checks if the daemon is already running.
  If the daemon is running 1 is returned.
  If the daemon is not running, the file is written with the process pid.
  The lock fd is stored in the provided filedescriptor.

*/
int
running(int *fd)
{
  char buf[100];
  wordexp_t lock;

  if(wordexp(LOCKFILE, &lock, 0) != 0)
    error(EXIT_FAILURE, errno, "word expansion failed");
  if(*lock.we_wordv == NULL)
    error(EXIT_FAILURE, 0, "word expansion empty");
  *fd = open(lock.we_wordv[0], O_RDWR | O_CREAT, LOCKMODE);
  wordfree(&lock);

  if(*fd < 0)
  {
      // error in opening lockfile
      printf("\nError in opening lockfile\n");
      return -1;
  }
  if(lockfile(*fd) < 0)
  {
    if(errno == EACCES || errno == EAGAIN)
    {
      printf("\nFile locked\n");
      close(*fd);
      return 1;
    }
    // other locking error
    printf("\nOther locking error\n");
    close(*fd);
    return -1;
  }
  ftruncate(*fd, 0);
  sprintf(buf, "%ld", (long)getpid());
  write(*fd, buf, strlen(buf) + 1);

  return 0;
}

/*
  Makes a splinter daemon.
  Returns 0 on success, -1 on error.
*/

int
makeSplinterDaemon(int * fdlock, struct sigaction *sa)
{
  int fd0, fd1, fd2;
  pid_t pid;
  struct rlimit rl;

  umask(0);
  getrlimit(RLIMIT_NOFILE, &rl);
  // fork and exit parent
  if((pid = fork()) != 0)
    exit(0);
  // create new session
  setsid();

  sa->sa_handler = SIG_IGN;
  sigemptyset(&(sa->sa_mask));
  sa->sa_flags = 0;
  sigaction(SIGHUP, sa, NULL);

  if((pid = fork()) != 0)
    exit(0);
  // change to root directory
  chdir(getenv("HOME"));

  // get max number of file descriptors
  if(rl.rlim_max == RLIM_INFINITY)
    rl.rlim_max = 1024;

  switch(running(fdlock))
  {
    case -1: return -1; // failure
    case 0: break; // success
    case 1: printf("Daemon already running\n");
            return DAE_EXISTS;
  }

  // close all file descriptors
  for(int i = 0; i < rl.rlim_max; ++i) {
    // keep fd of lock open to hold it
    if(i == *fdlock)
      continue;
    close(i);
  }

  fd0 = open("/dev/null", O_RDWR);
  fd1 = dup(0);
  fd2 = dup(0);
  if(fd0 != 0 || fd1 != 1 || fd2 != 2)
  {
    return -1;
  }

  return 0;
}

int
splinterDaemon()
{
  int fdlock;
  struct sigaction sa;
  if(makeSplinterDaemon(&fdlock, &sa)) {
    printf("daemon creation failed");
    return -1;
  }
  sleep(10);
  return 0;
}

int main(void)
{
  splinterDaemon();
  return 0;
}
