#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "initialize.h"

#define SPDIR ".splinter"
#define MODE_DIR S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#define MODE_FILE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define N_P1 10
#define N_P2 12
#define N_P3 10
#define CONFIG_STR "/.splinter"
#define DAEMON "/splinter.daemon"

int initialize(char *);
char * makeName(void);
