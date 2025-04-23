#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>          // for sleep()
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define BROADCAST_INTERVAL 1  // seconds
#define MESSAGE "Hello from UDP broadcast sender!"


static int sockfd;
static struct sockaddr_in bcastaddr;


int init_udp_sender()
{
    

    // 1. Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return(EXIT_FAILURE);
    }

    // 2. Enable broadcast option
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("Failed to set SO_BROADCAST");
        close(sockfd);
        return(EXIT_FAILURE);
    }

    // 3. Setup broadcast address
    memset(&bcastaddr, 0, sizeof(bcastaddr));
    bcastaddr.sin_family = AF_INET;
    bcastaddr.sin_port = htons(PORT);
    bcastaddr.sin_addr.s_addr = inet_addr("10.10.50.255");  // or your subnet's broadcast IP like 192.168.1.255

   printf("UDP sender ready on port %d...\n", PORT);


    return 0;
}

int send_udp_data(char *pstr)
{

	ssize_t sent = sendto(sockfd, pstr, strlen(pstr), 0,
                              (struct sockaddr *)&bcastaddr, sizeof(bcastaddr));
        if (sent < 0) {
            perror("Broadcast failed");
	    return -1;
        } else {
            //printf("[SENT] %s\n", pstr);
        }

	return 0;
}

void close_udp_sender()
{
	close(sockfd);

}
