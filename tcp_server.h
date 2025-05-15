#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#define MAX_CLIENTS 10

extern int server_fd;

int init_tcp_server();
void close_tcp_server();
int set_nonblocking(int fd);

/*int init_udp_listener();
void close_udp_sender();
void close_udp_listener();
int send_udp_data(char *pstr);
int wait_cmd(char *pcmd);
int recv_data(char *pcmd);
*/

#endif
