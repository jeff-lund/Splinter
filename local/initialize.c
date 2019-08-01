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

#define N_P1 10
#define N_P2 11
#define N_P3 10

const char* partOne[] = {"Magical", "Mysterious", "Commanding", "Super",
                          "Voracious", "Confused", "Bubbly", "Fierce", "Cuddly",
                          "Loyal"};
const char* partTwo[] = {"Panther", "Tiger", "Turtle", "Wolf", "Bear", "Lynx",
                        "Salmon", "Falcon", "Parrot", "Eagle", "Alligator"};
const char *partThree[] = {"Volcano", "Oasis", "Lake", "Mountain", "Cloud",
                           "River", "Plateau", "Canyon", "Forest", "Cenote"};

/*
  Creates a random name from selections defined above.
*/
char*
makeName(void)
{
  char *buf;
  const char *p1, *p2, *p3;
  int r1, r2, r3, length;
  srand(time(NULL));
  r1 = rand() % N_P1;
  r2 = rand() % N_P2;
  r3 = rand() % N_P3;

  p1 = partOne[r1];
  p2 = partTwo[r2];
  p3 = partThree[r3];
  length = strlen(p1) + strlen(p2) + strlen(p3);
  buf = malloc(sizeof(char) * (length + 2));
  strcpy(buf, p1);
  strcat(buf, p2);
  strcat(buf, p3);
  buf[length] = '\n';
  return buf;
}

/*
  Initializes new splinter. Fails if a splinter already exists at the given location.
*/
int
initialize(char *path)
{
  char *name;
  char *buf = malloc(sizeof(char) * (strlen(path) + strlen(SPDIR) + 2));
  int fd;

  strcpy(buf, path);
  strcat(buf, "/");
  strcat(buf, SPDIR);
  if(!access(buf, F_OK))
    error(EXIT_FAILURE, errno, "splinter already exists at this location");
  if(mkdir(buf, MODE_DIR) < 0)
    error(EXIT_FAILURE, errno, "mkdir error");
  name = makeName();
  // TODO remove magic number
  // 9 = strlen("/name.sp" + '\0')
  buf = realloc(buf, strlen(buf) + 9);
  if(buf == NULL)
    error(EXIT_FAILURE, 0, "realloc failed");
  strcat(buf, "/name.sp");
  fd = open(buf, O_WRONLY | O_CREAT,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if(fd == -1)
    error(EXIT_FAILURE, errno, "file creation error");
  write(fd, name, strlen(name));
  close(fd);
  free(name);
  free(buf);

  return 0;
}
