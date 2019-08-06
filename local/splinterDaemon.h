#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <wordexp.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LOCKFILE "~/.splinter/splintered.pid"
#define LOGFILE "~/.splinter/splinter.log"
#define SOCKFILE "/tmp/splinter.sock"

#define LOCKMODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define DAE_EXISTS -2
#define MAX_CONN 20

int lockfile(int);
int running(int*);
int makeSplinterDaemon(int *, struct sigaction*);
int splinterDaemon();
