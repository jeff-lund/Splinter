#ifndef SPLINTER_H_
#define SPLINTER_H_

#define LINEMAX 4096
#define DEAFULT_HOST "10.0.0.69"
#define DEAFULT_PORT "8080"

#define EOT 0x04


struct server;

struct server *alloc_serverinfo(void);
int getconnectioninfo(struct server *server, int argc, char *argv[]);
int setparams(struct server *server, int opt, char *argv);
const char *host(struct server *server);
const char *port(struct server *server);
int serverresponse(int server_fd);

int splinter(int server_fd);
int server_loop(int client_fd, int log_fd);

int r_exec(const char* tokbuf, size_t tokbufsize, int fd);
int builtin(int argc, char** argv, int fd);



int s_bind(const char *host, const char *port);
int s_connect(const char *host, const char *port, int socketttype);
int s_listen(int, int);
int s_accept(int sockfd);

int ls(int argc, char* argv[], int wfd);

#endif
