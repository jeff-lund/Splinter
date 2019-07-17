#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "splinter.h"
#include "connectioninfo.h"

#define _POSIX_SOURCE 1

sig_atomic_t g_terminate;

void signal_handler(int nr) { g_terminate = 1; }

int main(int argc, char* argv[])
{
    int sock;
    int rc;
    char* buf;
    int bufsize = 4096;
    struct server *server = 0;
    int backlog = 10;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);

    server = alloc_serverinfo();
    getconnectioninfo(server, argc, argv);

    sock = -1;

    buf = malloc(bufsize);
    if (!buf) {
        goto out;
    }
    memset(buf, 0, bufsize);

		sock = s_bind(host(server), port(server));
    if (sock < 0) {
        fprintf(stderr, "(%s:%d) %s(), Binding Operation Returned: %d\n", __FILE__, __LINE__, __FUNCTION__, sock);
        goto out;
    }

    rc = listen(sock, backlog);
    if (rc < 0) {
        fprintf(stderr, "(%s:%d) %s(), listen() returned: %d\n", __FILE__, __LINE__, __FUNCTION__, rc);
        goto out;
    }

    fprintf(stderr, "pid: %d\n", getpid());

    while (!g_terminate) {
        int peer;
        peer = s_accept(sock);
        if (peer > 0) {
            close(peer);
        } else {
            break;
        }
    } 

    fprintf(stderr, "good-bye.\n");

out:
    if (sock > 0) {
        close(sock);
    }

    if (buf) {
        free(buf);
    }

		if(server != 0)
			free(server);

    return 0;
}

