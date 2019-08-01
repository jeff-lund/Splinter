/******************************************************************************************
* Jeff Lund
* Main driver for splinter program
*
* OPTIONS:
* -i --init [path]: start a new splinter at directory specified by path (default: '.')
* -c --connect [splinter_name] : connects to a running splinter
* -s --start [splinter] : starts splinter with given name or splinter in current dir
******************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <getopt.h>
#include "initialize.h"

#define O_INIT 0x1
#define O_START 0x2
#define O_CONNECT 0x4
#define ARGSTRING "c:i:s:"
#define N_LONG_ARGS 3
#define MAND_ARG 1
#define OPTIONAL_ARG 2
#define ARG_CONNECT 0
#define ARG_INIT 1
#define ARG_START 2

char* parseOpts(char*, int, char**);

int
main(int argc, char *argv[])
{
  char *name, *path = NULL;
  char startFlag = 0x0;
  // check options connect/init/start
  path = parseOpts(&startFlag, argc, argv);

  if(startFlag == O_INIT)
  {
    // Initialize new splinter at path and start up
    printf("Init\n");
    if(initialize(path) < 0)
      error(EXIT_FAILURE, 0, "initialization failed");
    //start(path);
  }
  else if(startFlag == O_START)
  {
    // Start existing splinter
    printf("Start\n");
    //start(path);
  }
  else if(startFlag == O_CONNECT)
  {
    // Connect to running splinter
    printf("Connect\n");
  }
  else
  {
    error(EXIT_FAILURE, 0, "choose only one option init/start/connect");
  }
  free(path);
  return 0;
}

char*
parseOpts(char *flag, int argc, char **argv)
{
  char arg;
  char *path;
  struct option largs[N_LONG_ARGS + 1];
  const char *longargs[] = {"connect", "init", "start"};
  for(int i = 0; i < N_LONG_ARGS; ++i)
  {
    largs[i].name = longargs[i];
    largs[i].has_arg = MAND_ARG;
    largs[i].flag = NULL;
    largs[i].val = i;
  }
  largs[N_LONG_ARGS].name = NULL;
  largs[N_LONG_ARGS].has_arg = 0;
  largs[N_LONG_ARGS].flag = NULL;
  largs[N_LONG_ARGS].val = 0;

  while((arg = getopt_long(argc, argv, ARGSTRING, largs, NULL)) != -1)
  {
    switch(arg)
    {
      case 'c':
      case ARG_CONNECT: *flag |= O_CONNECT;
                        if(optarg) {
                          path = malloc(strlen(optarg) + 1);
                          strcpy(path, optarg);
                        }
                        break;
      case 'i':
      case ARG_INIT: *flag |= O_INIT;
                     if(optarg) {
                       path = malloc(strlen(optarg) + 1);
                       strcpy(path, optarg);
                     }
                     break;
      case 's':
      case ARG_START: *flag |= O_START;
                      if(optarg) {
                        path = malloc(strlen(optarg) + 1);
                        strcpy(path, optarg);
                      }
                      break;
      default: error(EXIT_FAILURE, errno, "invalid option \'%c\'", arg);
    }
  }

  return path;
}
