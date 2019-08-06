#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <wordexp.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "connect.h"

#define SOCKPTH "/var/tmp/"
#define SERVER_PATH "/tmp/splinter.sock"
#define offsetof(TYPE, MEMBER) ((long)&((TYPE *)0)->MEMBER)

void connect_splinter(void);
