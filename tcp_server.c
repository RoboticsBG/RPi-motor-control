#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>

#define PORT 8082
#define MAX_EVENTS 64
#define BUFFER_SIZE 1024

int server_fd;
static struct sockaddr_in server_addr;


int set_nonblocking(int fd) 
{
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


int init_tcp_server()
{
 // 1. Create listening socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(server_fd);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        return -1;
    }

    // 3. Listen
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        return -2;
    }

    printf("TCP server with epoll listening on port %d...\n", PORT);
    return 0;
}


void close_tcp_server()
{
	close(server_fd);
}
