/*
  Creates a daemon for splinter to manage all system splinters
  Start up a local splinter
  Listen for incoming connection requests
  Connect users to running splinter
  Close a running splinter
*/
#include "splinterDaemon.h"

#define offsetof(TYPE, MEMBER) ((long)&((TYPE *)0)->MEMBER)

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
    // keep fd of lock open to hold onto it
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
  int fdlock, fdlog, fdsock, connfd;
  struct sockaddr_un cli_un;
  int size;
  struct sigaction sa;
  struct sockaddr_un sun;
  wordexp_t expanded;
  char *msg = "waiting on connect\n";
  char *msg2 = "connection recieved\n";

  if(makeSplinterDaemon(&fdlock, &sa)) {
    printf("daemon creation failed");
    return -1;
  }
  // open log
  if(wordexp(LOGFILE, &expanded, 0) != 0)
    error(EXIT_FAILURE, errno, "word expansion failed");
  if((fdlog = open(expanded.we_wordv[0], O_RDWR | O_CREAT | O_APPEND, LOCKMODE)) < 0)
    error(EXIT_FAILURE, 0, "cannot open daemon log");
  if(lockfile(fdlog) < 0)
    error(EXIT_FAILURE, 0, "cannot obtain log lock");
  wordfree(&expanded);
  // create UNIX socket at /tmp/
  unlink(SOCKFILE);
  sun.sun_family = AF_UNIX;
  strcpy(sun.sun_path, SOCKFILE);
  if((fdsock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    error(EXIT_FAILURE, errno, "socket failed");
  }
  size = offsetof(struct sockaddr_un, sun_path) + strlen(sun.sun_path);
  if(bind(fdsock, (struct sockaddr *)&sun, size) < 0) {
    error(EXIT_FAILURE, errno, "bind failed");
  }

  socklen_t len;

  if(listen(fdsock, MAX_CONN) < 0)
    error(EXIT_FAILURE, 0, "listen failure");

  while(1)
  {
    write(fdlog, msg, strlen(msg));
    if((connfd = accept(fdsock, (struct sockaddr_un *)&cli_un, &len)) < 0)
    {
      write(fdlog, "daemon encountered accept error\n", strlen("daemon encountered accept error\n"));
      continue;
    }
    // TODO log connection with pid of client
    write(fdlog, msg2, strlen(msg2));
    // create thread to initiate command
    write(connfd, "connection made", strlen("connection made"));
    sleep(1);
    close(connfd);
  }
  write(fdlog, "goodbye\n", 8);
  return 0;
}
