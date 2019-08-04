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

void
connect_splinter(void)
{
  int fd, len, n;
  struct sockaddr_un cli_un, serv_un;
  char buffer[100];
  //fd_set socks;

  // Create client socket end point
  if((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    error(EXIT_FAILURE, errno, "socket failed");

  // Set up client sockaddr struct
  memset(&cli_un, 0, sizeof(cli_un));
  cli_un.sun_family = AF_UNIX;
  sprintf(cli_un.sun_path, "%s%05ld", SOCKPTH, (long)getpid());
  len = offsetof(struct sockaddr_un, sun_path) + strlen(cli_un.sun_path);
  // If the socket already exists (old socket) unlink it
  unlink(cli_un.sun_path);
  if(bind(fd, (struct sockaddr *)&cli_un, len) < 0)
    error(EXIT_FAILURE, errno, "bind error");

  // set up sockaddr to splinter daemon socket
  memset(&serv_un, 0, sizeof(serv_un));
  serv_un.sun_family = AF_UNIX;
  strcpy(serv_un.sun_path, SERVER_PATH);
  len = offsetof(struct sockaddr_un, sun_path) + strlen(serv_un.sun_path);
  // try to connect to splinter daemon socket
  if(connect(fd, (struct sockaddr *)&serv_un, len) < 0) {
    error(EXIT_FAILURE, errno, "connect failed");
  }
  // got a connection, just write out for now
  printf("Secured connection\n");
  //FD_SET(fd, &socks);
  //FD_ZERO(&socks);
  //select(1, &socks, NULL, NULL, NULL);
  n = read(fd, buffer, 15);
  n = write(1, buffer, n);

  return;
}
