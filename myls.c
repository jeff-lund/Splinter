/**************************************************
* Jeff Lund
* CS 510 ALSP
* LS replacement - myls
* Build - gcc -o myls myls.c
* Usage: ./myls
*        ./ myls [OPTIONS] [FILES]
**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <sys/stat.h>
#include <dirent.h>
#include <glob.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/time.h>

#define OPTIONS "laFi1"
#define O_LIST      0x10
#define O_ALL       0x08
#define O_CLASSIFY  0x04
#define O_INODE     0x02
#define O_ONE       0x01
#define IS_EXEC 0x49
#define TRUE 1
#define FALSE 0

char ARGMAP = 0x0;
char *myname;

int
globerr(const char *path, int eerrno)
{
    fprintf(stderr, "%s: %s: %s\n", myname, path, strerror(eerrno));
    return 0;    /* let glob() keep going */
}


static void
Malloc(char **buf, uint size)
{
  *buf = malloc(size);
  if(!buf)
  {
    error(EXIT_FAILURE, 0, "malloc failed");
  }
}

static int
Stat(char *path, struct stat *st)
{
  int val;
  val = stat(path, st);
  if(val)
  {
    if(errno == ENOENT)
      return FALSE;
    else
      error(EXIT_FAILURE, errno, "%s stat failed", path);
  }
  return TRUE;
}

static void
Lstat(char *path, struct stat *st)
{
  if(lstat(path, st) < 0)
  {
    error(EXIT_FAILURE, errno, "%s stat failed", path);
  }
}

static int
isHidden(char *buf)
{
  char *token;
  char *last;
  char *copy;
  int ret;

  Malloc(&copy, sizeof(char) * (strlen(buf) + 1));
  strcpy(copy, buf);
  token = strtok(copy, "/");
  last = token;

  while((token = strtok(NULL, "/")))
    last = token;

  ret = last[0] == '.';
  free(copy);

  return ret;
}

static void
classify(struct stat *st)
{
  switch(st->st_mode & S_IFMT) {
    case S_IFDIR:  printf("/"); break; //directory
    case S_IFREG:  if(st->st_mode & IS_EXEC)
                      printf("*"); // executable
                   break;
    case S_IFIFO:  printf("|"); break; // FIFO == PIPE
    case S_IFLNK:  printf("@"); break; // link
    case S_IFSOCK: printf("="); break; // socket
  }
}

void
printType(struct stat *st)
{
  switch(st->st_mode & S_IFMT) {
    case S_IFDIR:  printf("d"); break;
    case S_IFREG:  printf("-"); break;
    case S_IFCHR:  printf("c"); break;
    case S_IFBLK:  printf("b"); break;
    case S_IFIFO:  printf("|"); break; // FIFO == PIPE
    case S_IFLNK:  printf("l"); break; // tricky to get right
    case S_IFSOCK: printf("s"); break;
  }
}

static void
printMode(struct stat *st)
{
  printf((st->st_mode & S_IRUSR)? "r":"-");
  printf((st->st_mode & S_IWUSR)? "w":"-");
  st->st_mode & S_ISUID ? printf("s"):printf((st->st_mode & S_IXUSR)? "x":"-");
  printf((st->st_mode & S_IRGRP)? "r":"-");
  printf((st->st_mode & S_IWGRP)? "w":"-");
  st->st_mode & S_ISGID ? printf("s"):printf((st->st_mode & S_IXGRP)? "x":"-");
  printf((st->st_mode & S_IROTH)? "r":"-");
  printf((st->st_mode & S_IWOTH)? "w":"-");
  printf((st->st_mode & S_IXOTH)? "x":"-");
}

static void
setFlags(int argc, char **argv)
{
  char arg;
  while((arg = getopt(argc, argv, OPTIONS)) != -1)
  {
    switch(arg)
    {
      case 'l':
          ARGMAP |= O_LIST;
          break;
      case 'a':
          ARGMAP |= O_ALL;
          break;
      case 'F':
          ARGMAP |= O_CLASSIFY;
          break;
      case 'i':
          ARGMAP |= O_INODE;
          break;
      case '1':
          ARGMAP |= O_ONE;
          break;
      default:
          error(EXIT_FAILURE, 0, "Unrecognized option.");
    }
  }

  return ;
}

/* from GNU ls.c:
  Consider a time to be recent if it is within the past six
  months.  A Gregorian year has 365.2425 * 24 * 60 * 60 ==
  31556952 seconds on the average.  Write this value as an
  integer constant to avoid floating point hassles.
*/
static void
printDate(struct stat *st)
{
  int dateStrSize = 14;
  char dateStr[dateStrSize];
  struct tm * file_date;
  struct timeval now;
  struct timeval six_months_ago;

  file_date = localtime(&st->st_mtim.tv_sec);
  gettimeofday(&now, NULL);
  six_months_ago.tv_sec = now.tv_sec - 31556952 / 2;
  if((st->st_mtim.tv_sec - six_months_ago.tv_sec) < 0)
   strftime(dateStr, dateStrSize, "%b %e %Y", file_date);
  else
   strftime(dateStr, dateStrSize, "%b %e %H:%M", file_date);
  printf("%s", dateStr);
  printf("  ");
}


static void
printUID(uid_t uid)
{
  struct passwd *pw;
  if((pw = getpwuid(uid)) == NULL)
    printf("%i ", uid);
  else
    printf("%s", pw->pw_name);
  return;
}

static void
printGID(gid_t gid)
{
  struct group *gr;
  if((gr = getgrgid(gid)) == NULL)
    printf("%i ", gid);
  else
    printf("%s", gr->gr_name);
  return;
}

static void
printFile(char *name, struct stat *st, char *symbName, struct stat *symbst)
{
  if(!(ARGMAP & O_ALL) && isHidden(name))
  {
    return;
  }

  if(ARGMAP & O_INODE)
  {
    // INODE
    printf("%li  ", st->st_ino);
  }

  if(ARGMAP & O_LIST)
  {
    // MODE
    printType(st);
    printMode(st);
    printf("  ");
    //NLINK
    printf("%li  ", st->st_nlink);
    // OWNER
    printUID(st->st_uid);
    printf("  ");
    // GROUP
    printGID(st->st_gid);
    printf("  ");
    // SIZE
    printf("%li  ", st->st_size);
    // DATE
    printDate(st);
    printf("  ");
  }
  // NAME
  if(ARGMAP & O_LIST && S_ISLNK(st->st_mode))
  {
    printf("%s -> %s", name, symbName);
  }
  else
  {
    printf("%s", name);
  }
  // CLASSIFY
  if(ARGMAP & O_CLASSIFY)
  {
    if(ARGMAP & O_LIST && S_ISLNK(st->st_mode))
      classify(symbst);
    else
      classify(st);
  }
  printf("  ");
  if(ARGMAP & (O_LIST | O_ONE))
  {
    printf("\n");
  }
  return;
}

static void
listSymbLink(char *path, char* entryPath, struct stat *st)
{
  struct stat symbst; // stat of following symbolic link
  long int sz;
  char buffer[PATH_MAX];

  symbst.st_mode = 0;
  sz = readlink(entryPath, buffer, PATH_MAX);
  buffer[sz] = '\0';
  Stat(entryPath, &symbst);
  printFile(path, st, buffer, &symbst);
}

static void
listDir(char *path, struct stat *st)
{
  DIR *dp;
  struct dirent *dirp;
  char entryPath[PATH_MAX];

  dp = opendir(path);

  while((dirp = readdir(dp)))
  {
    memset(entryPath, 0, PATH_MAX);
    strcpy(entryPath, path);
    if(entryPath[strlen(entryPath) - 1] != '/')
      strcat(entryPath, "/");
    strcat(entryPath, dirp->d_name);
    Lstat(entryPath, st);

    if(S_ISLNK(st->st_mode))
      listSymbLink(dirp->d_name, entryPath, st);
    else
      printFile(dirp->d_name, st, NULL, NULL);
  }
}

static void
list(glob_t paths)
{
  struct stat st;
  char *path;
  char buf[PATH_MAX];

  for(int i = 0; i < paths.gl_pathc; ++i)
  {
    memset(buf, 0, PATH_MAX);
    path = paths.gl_pathv[i];
    Lstat(path, &st);
    if(S_ISDIR(st.st_mode))
      listDir(path, &st);
    else if(S_ISLNK(st.st_mode))
      listSymbLink(path, path, &st);
    else
      printFile(path, &st, NULL, NULL);
  }
  if(!(ARGMAP & (O_LIST | O_ONE)))
    printf("\n");

  return;
}

int
main(int argc, char *argv[])
{
  glob_t paths;
  setFlags(argc, argv);
  char cwd[PATH_MAX];
  int val;

  myname = argv[0];
  if(optind == argc)
  {
    getcwd(cwd, PATH_MAX);
    glob(cwd, 0, globerr, &paths);
  }
  else
  {
    for(int i = optind; i < argc; ++i)
    {
      val = glob(argv[i], GLOB_TILDE | (i == 0 ? 0 : GLOB_APPEND), globerr, &paths);
      switch (val)
      {
        case 0: break;
        case GLOB_NOMATCH: error(0, errno, "cannot access %s", argv[i]); break;
        case GLOB_NOSPACE: error(0, errno, "out of space"); break;
        case GLOB_ABORTED: error(0, errno, "read error"); break;
        default: fprintf(stderr, "unknown error"); break;
      }
    }
  }
  if(paths.gl_pathc)
  {
    list(paths);
  }
  globfree(&paths);
	return 0;
}
