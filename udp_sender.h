#ifndef UDP_SENDER_H
#define UDP_SENDER_H

int init_udp_sender();
int init_udp_listener();
void close_udp_sender();
void close_udp_listener();
int send_udp_data(char *pstr);
int wait_cmd(char *pcmd);

#endif
