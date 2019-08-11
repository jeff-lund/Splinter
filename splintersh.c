#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include "splinter.h"

char *prompt = "$ ";
char *delims = " \t\n,;:=";



int 
parseargs(char* buf, size_t bufsize, const char* delims, size_t* token_bufsize)
{
  int token_count = 0;
  char* begin;
  char* end; 
  char* dest;
  size_t delim_bytes, token_bytes, total_token_bytes = 0;

  begin = buf;
  end = buf;
  dest = buf;

  if (!buf || !delims) {
      return -1;
  }

  delim_bytes = strspn(end, delims);

  while (end - buf < bufsize) {
    begin = end + delim_bytes;
    if (!*begin) 
			break;
    token_bytes = strcspn(begin, delims);
    end = begin + token_bytes;
    delim_bytes = strspn(end, delims);
    memmove(dest, begin, token_bytes);
    dest = dest + token_bytes;
    *dest = 0;
    ++dest;

    total_token_bytes += token_bytes;

    if (token_bytes != 0) {
      ++token_count;
    }
  }

  if (token_bufsize) {
    *token_bufsize = total_token_bytes + token_count;
  }

  return token_count;


}


int 
splinter(int server_fd)
{
  size_t tokbufsize = 0;
  char *buf, *newbuf;
	int n;


  buf = malloc(LINEMAX);
  if (!buf) 
    return 0;
    
   while (1) {
     write(STDOUT_FILENO, prompt, strlen(prompt));
		 fflush(NULL);
     memset(buf, 0, LINEMAX);
     if ((n = read(STDIN_FILENO, buf, LINEMAX)) == 0) {
       break;
     }
     if (buf[0] == '\n') {
       continue;
     }

     parseargs(buf, LINEMAX, delims, &tokbufsize);
     newbuf = buf;
     write(server_fd, newbuf, tokbufsize);
     serverresponse(server_fd);
   }

   if (buf) {
     free(buf);
   }

   return 0;
}

