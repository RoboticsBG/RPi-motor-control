#ifndef UDP_SENDER_H
#define UDP_SENDER_H

extern int sockfd2;

int init_udp_sender();
int init_udp_listener();
void close_udp_sender();
void close_udp_listener();
int send_udp_data(char *pstr);
int wait_cmd(char *pcmd);
int recv_data(char *pcmd);

#endif
