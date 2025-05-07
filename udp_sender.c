#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>          // for sleep()
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#define MULTICAST_IP "239.1.1.1"
#define PORT 8080
#define PORT2 8081
#define BUFFER_SIZE 1024
#define BROADCAST_INTERVAL 1  // seconds
#define MESSAGE "Hello from UDP broadcast sender!"

#define MAX_EVENTS	16


int sockfd, sockfd2;
//static struct sockaddr_in broadcast_addr;
static struct sockaddr_in multicast_addr;
static struct sockaddr_in listenaddr;
static char buffer[BUFFER_SIZE];


void event_tick();

int init_udp_listener()
{
	// 1. Create UDP socket
    	if ((sockfd2 = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0) {
        	perror("Socket creation failed");
        	exit(EXIT_FAILURE);
    	}

	// 3. Prepare address to bind to
    	memset(&listenaddr, 0, sizeof(listenaddr));
    	listenaddr.sin_family = AF_INET;
    	listenaddr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    	listenaddr.sin_port = htons(PORT2);

    	// 4. Bind socket
    	if (bind(sockfd2, (struct sockaddr *)&listenaddr, sizeof(listenaddr)) < 0) {
        	perror("Bind failed");
        	close(sockfd2);
        	exit(EXIT_FAILURE);
    	}

	printf("Motor control ready to accept commands on port %d...\n", PORT2);

	return 0;
}


int init_udp_sender()
{

    // 1. Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return(EXIT_FAILURE);
    }

/*	struct in_addr localInterface;
	localInterface.s_addr = inet_addr("10.10.50.142"); // replace with your IP
	setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &localInterface, sizeof(localInterface));
*/

    // 2. Enable broadcast option
    /*int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("Failed to set SO_BROADCAST");
        close(sockfd);
        return(EXIT_FAILURE);
    }*/

    // 3. Setup broadcast address
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(PORT);
    multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);


    return 0;
}

int send_udp_data(char *pstr)
{

	ssize_t sent = sendto(sockfd, pstr, strlen(pstr), 0,
                              (struct sockaddr *)&multicast_addr, sizeof(multicast_addr));
        if (sent < 0) {
            perror("sendto failed");
	    return -1;
        } else {
            //printf("[SENT] %s\n", pstr);
        }

	return 0;
}


int wait_cmd(char *pcmd)
{
	while (1) {
        	socklen_t addrlen = sizeof(listenaddr);
        	ssize_t n = recvfrom(sockfd2, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr *)&listenaddr, &addrlen);
        	if (n < 0) {
           	 	perror("recvfrom failed");
            		continue;
        	}

        	buffer[n] = '\0';  // Null-terminate the string
        	//printf("%s\n", buffer);
		strcpy(pcmd, buffer);
		return 0;
    	}
	return 0;
}

int recv_data(char *pcmd)
{
	struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        ssize_t len = recvfrom(sockfd2, buffer, BUFFER_SIZE - 1, 0,
        	(struct sockaddr *)&client_addr, &client_len);
       if (len > 0) {

        	buffer[len] = '\0';  // Null-terminate the string
        	//printf("%s\n", buffer);
		strcpy(pcmd, buffer);
		return 0;
	}
	return 1;
}


void close_udp_sender()
{
	close(sockfd);

}

void close_udp_listener()
{
	close(sockfd2);

}
