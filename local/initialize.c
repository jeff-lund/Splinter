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
#define CONFIG_STR "/.splinterconfig"
#define DAEMON "/splinter.daemon"
const char* partOne[] = {"Magical", "Mysterious", "Commanding", "Super",
                          "Voracious", "Confused", "Bubbly", "Fierce", "Cuddly",
                          "Loyal"};
const char* partTwo[] = {"Panther", "Tiger", "Turtle", "Wolf", "Bear", "Lynx",
                        "Salmon", "Falcon", "Parrot", "Eagle", "Alligator", "Llama"};
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
  int length;
  srand(time(NULL));
  p1 = partOne[rand() % N_P1];
  p2 = partTwo[rand() % N_P2];
  p3 = partThree[rand() % N_P3];
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
  char *name, *home, *p;
  char *buf = malloc(sizeof(char) * (strlen(path) + strlen(SPDIR) + 2));
  int fd1, fd2;
  p = getenv("HOME");
  home = malloc(strlen(p) + strlen(CONFIG_STR) + 1);
  strcpy(home, p);
  strcat(home, CONFIG_STR);
  strcpy(buf, path);
  strcat(buf, "/");
  strcat(buf, SPDIR);
  // Create .splinter directory at location
  if(access(buf, F_OK) == 0)
    error(EXIT_FAILURE, errno, "splinter already exists at this location");
  if(mkdir(buf, MODE_DIR) < 0)
    error(EXIT_FAILURE, errno, "mkdir error");
  name = makeName();
  // write into main splinter file
  // Create main splinter directory if it does not exist
  if(access(home, F_OK) < 0) {
    if(mkdir(home, MODE_DIR) < 0)
      error(EXIT_FAILURE, errno, "mkdir failed");
  }

  home = realloc(home, strlen(home) + strlen(DAEMON) + 1);
  strcat(home, DAEMON);
  fd2 = open(home, O_WRONLY | O_CREAT | O_APPEND,  MODE_FILE);
  if(fd2 < 0)
    error(EXIT_FAILURE, errno, "file creation failed");
  // Write out path \tab name of splinter in daemon file
  write(fd2, path, strlen(path));
  write(fd2, "\t", 1);
  write(fd2, name, strlen(name));
  // TODO remove magic number
  // 9 = strlen("/name.sp" + '\0')
  if((buf = realloc(buf, strlen(buf) + 9)) == NULL)
    error(EXIT_FAILURE, 0, "realloc failed");
  strcat(buf, "/name.sp");
  // create name file
  fd1 = open(buf, O_WRONLY | O_CREAT,  MODE_FILE);
  if(fd1 == -1)
    error(EXIT_FAILURE, errno, "file creation error");
  write(fd1, name, strlen(name));

  close(fd1);
  close(fd2);
  free(name);
  free(buf);
  free(home);

  return 0;
}
