#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pigpio.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <signal.h>
#include "rotary_encoder.h"
#include "tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>


#define MAX_PWM		1000
#define PWM_FREQ	5000
#define MOT_SPEED_L  	300
#define MOT_SPEED_R  	300

#define FWD_L		0
#define FWD_R		1
#define BACK_L	  	1
#define BACK_R	  	0

#define MDIV		11
#define K_SPD		0.222F
#define K_WHEEL		0.262F
#define L_WHEEL		20.45F
#define K_FSPD		0.0002134F

#define MAX_EVENTS	16
#define TOUT1		200
#define MOT_TOUT	5

struct pi_motors
{
	int pos;
	int c_speed;
	int t_speed;
	uint16_t pwm;
	float f_speed;
	float f_dist;
};

int motor_run = 0;
int motor_timer = 0;

int MPWM_L=12;
int MPWM_R=13;
int MDIR_L=04;
int MDIR_R=27;
int MEN=24;

int gpioA_L = 17;
int gpioB_L = 18;
int gpioA_R = 22;
int gpioB_R = 23;


struct pi_motors pi_motL, pi_motR;

volatile sig_atomic_t running = 1;

void handle_sigint(int signum)
{
	//printf("\nCaught signal %d, exiting cleanly...\n", signum);
    	running = 0;

}



void set_lspeed(int spd);
void set_aspeed(int spd);

uint16_t adj_pwm(struct pi_motors *pm);

int listen_rx();
void event_tick();
int proc_data(char *pcmd);
int create_timerfd(int interval_ms);


void set_pwm_L(uint16_t pwm)
{
	 if (pwm > MAX_PWM) 
		pwm = MAX_PWM;
	 gpioHardwarePWM(MPWM_L,PWM_FREQ,pwm*1000);

}

void set_pwm_R(uint16_t pwm)
{
	 if (pwm > MAX_PWM) 
		pwm = MAX_PWM;

	 gpioHardwarePWM(MPWM_R,PWM_FREQ,pwm*1000);

}

void callback_L(int way, uint32_t td)
{
	static int div = 0;
	static uint32_t td_sum = 0;
	int upd = 0, dir;

	dir = way*-1;
	div += dir;
	td_sum += td;

	if (div > MDIV-1){
		div = 0;
		upd = 1;
   	}
	else if (div < 0){
                div = MDIV-1;
		upd = 1;
	}

	if (upd == 1){
		pi_motL.pos += dir;
		pi_motL.c_speed = 60*1000000/td_sum*dir;
   		//printf("L:%d\n", pi_mot.speed_L);
		pi_motL.f_dist = pi_motL.pos* K_WHEEL;
		pi_motL.f_speed = pi_motL.c_speed*(K_FSPD*L_WHEEL);
   		//sprintf(str,"\rLS:%.1f|RS:%.1f|L:%.1f|R:%.1f ",
		//	pi_motL.f_speed, pi_motR.f_speed, pi_motL.f_dist,pi_motR.f_dist);
		//send_udp_data(str);
		//fflush(stdout);
		td_sum = 0;

		pi_motL.pwm = adj_pwm(&pi_motL);
		set_pwm_L(pi_motL.pwm);


	}
}

void callback_R(int way, uint32_t td)
{
        static int div = 0;
        static uint32_t td_sum = 0;
        int upd = 0, dir;

        dir = way*1;
	div += dir;
        td_sum += td;

        if (div > MDIV-1){
                div = 0;
                upd = 1;
        }
        else if (div < 0){
                div = MDIV-1;
                upd = 1;
        }

        if (upd == 1){
                pi_motR.pos += dir;
                pi_motR.c_speed = 60*1000000/td_sum*dir;
                pi_motR.f_dist = pi_motR.pos* K_WHEEL;
                pi_motR.f_speed = pi_motR.c_speed*(K_FSPD*L_WHEEL);

		td_sum = 0;

		pi_motR.pwm = adj_pwm(&pi_motR);
		set_pwm_R(pi_motR.pwm);
        }
}




int main(int argc, char *argv[])
{
	int ret;

 	Pi_Renc_t *renc1, *renc2;


	if (init_tcp_server() < 0){
		return 1;
	}

/*	if (init_udp_listener() < 0){
		return 1;
	}
*/

        if (gpioInitialise() < 0)
                return 1;

	signal(SIGINT, handle_sigint);
	signal(SIGTERM, handle_sigint);

	//pi_mot.pwm_L = MOT_SPEED_L;
	//pi_mot.pwm_R = MOT_SPEED_R;
	//pi_mot.target_s = S_TARGET1;


        gpioSetMode(MPWM_L, PI_ALT0);
        gpioSetMode(MPWM_R, PI_ALT0);

        gpioSetMode(MDIR_L, PI_OUTPUT);
        gpioSetMode(MDIR_R, PI_OUTPUT);

        gpioSetMode(MEN, PI_OUTPUT);
        //gpioSetPWMfrequency(MPWM_L, 1000);
        //gpioSetPWMfrequency(MPWM_R, 1000);

        //gpioPWM(MPWM_L, 0);
        //gpioPWM(MPWM_R, 0);
	gpioHardwarePWM(MPWM_L,PWM_FREQ,0);
	gpioHardwarePWM(MPWM_R,PWM_FREQ,0);
        gpioWrite(MEN, 1);

	renc1 = Pi_Renc(gpioA_L, gpioB_L, callback_L);
	renc2 = Pi_Renc(gpioA_R, gpioB_R, callback_R);

	//set_aspeed(1000);
	//set_rspeed(1000);


	listen_rx();

	close_tcp_server();
	//close_udp_sender();
	//close_udp_listener();
	Pi_Renc_cancel(renc1);
	Pi_Renc_cancel(renc2);
	set_pwm_L(0);
        set_pwm_R(0);

        gpioTerminate();
}

int client_fds[MAX_CLIENTS] = {0};
#define BUFFER_SIZE 256

/**
 * @brief  The primary function that listens on CAN sockets.
 * @retval  ret 
 */
int listen_rx()
{
        int event_count;
        int  fd_epoll;
        int i, ret;
        int tfd, fd;
	uint64_t expirations;

	//struct epoll_event event_setup[2], events[MAX_EVENTS];
	struct epoll_event t_event, events[MAX_EVENTS];

	char buffer[BUFFER_SIZE];

	tfd = create_timerfd(100);  // fire every  100ms
	if (tfd < 0)
		return -1;


        fd_epoll = epoll_create(1);
        if (fd_epoll < 0) {
                perror("epoll_create");
                return -2;
        }

        t_event.events = EPOLLIN;
        t_event.data.fd = server_fd;


        if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, server_fd, &t_event)) {
                perror("failed to add TCP listen socket to epoll");
                return -3;
        }

	t_event.events = EPOLLIN;
        t_event.data.fd = tfd;

        if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, tfd, &t_event)) {
                perror("failed to add timerfd to epoll");
                return -4;
        }

 	while(running){
                event_count = epoll_wait(fd_epoll, events, MAX_EVENTS, -1);
                //printf("%d ready events\n", event_count);
                if (event_count < 0){
                        running = 0;
                        continue;
                }
                for(i = 0; i < event_count; i++){
			fd = events[i].data.fd;
             		if (fd  == server_fd) {
				struct sockaddr_in client_addr;
                		socklen_t addrlen = sizeof(client_addr);
                		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);

                		if (client_fd >= 0) {
                    			set_nonblocking(client_fd);
                    			t_event.events = EPOLLIN | EPOLLET;
                    			t_event.data.fd = client_fd;
                    			epoll_ctl(fd_epoll, EPOLL_CTL_ADD, client_fd, &t_event);

                    			// Track client
                    			for (int j = 0; j < MAX_CLIENTS; j++) {
                        			if (client_fds[j] == 0) {
                            				client_fds[j] = client_fd;
                           			 	break;
                       			 	}
                    			}
                    			printf("New connection from %s:%d\n",
                           		inet_ntoa(client_addr.sin_addr),
                           		ntohs(client_addr.sin_port));
                		}

                	}
		        else if (fd == tfd) {
                		read(tfd, &expirations, sizeof(expirations)); 
				event_tick();
                	}
			else {
                		// Data from client
                		ssize_t r = read(fd, buffer, BUFFER_SIZE - 1);
                		if (r <= 0) {
                    			// Disconnect
                    			printf("Client fd %d disconnected.\n", fd);
                    			epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd, NULL);
                    			close(fd);

                    			for (int j = 0; j < MAX_CLIENTS; j++) {
                        			if (client_fds[j] == fd) {
                            				client_fds[j] = 0;
                            				break;
                        			}
                    			}
                		} else {
                    			buffer[r] = '\0';
                    			//printf("[Client %d] %s\n", fd, buffer);
					proc_data(buffer);

                		}
            		}
		}

        }
        return 0;
}


void send_mot_data()
{
	char str[64];
	static int cnt;

	sprintf(str,"T:%d|LS:%.1f|RS:%.1f|L:%.1f|R:%.1f    ",
		cnt,pi_motL.f_speed, pi_motR.f_speed, pi_motL.f_dist,pi_motR.f_dist);
        //send_udp_data(str);
	//printf("%s",str);
	// Broadcast to all clients
        for (int j = 0; j < MAX_CLIENTS; j++) {
        	if (client_fds[j] > 0) {
                	send(client_fds[j], str, strlen(str), 0);
                }
        }

	cnt ++;
	if (cnt >= 100)
		cnt = 0;

}


void event_tick()
{
        if (motor_timer != 0){
		motor_timer --;
		if (motor_timer == 0){
			set_lspeed(0);
		}
	}
	else{
		pi_motR.f_speed =0.0;
		pi_motL.f_speed =0.0;

	}

	send_mot_data();
	//printf("1000ms tick\n");
}

int proc_data(char *pcmd)
{
	char c;
	int val;

		c = *pcmd++;
		val = atoi(pcmd);

                if  (c == 'm'){
			set_lspeed(val);
			motor_timer = MOT_TOUT;
                }

		else if (c == 'r'){
			set_aspeed(val);
			motor_timer = MOT_TOUT;

                }

                else if (c == 's'){
                	set_lspeed(0);
		}
	return 0;
}



int create_timerfd(int interval_ms) 
{

	struct itimerspec ts;
	int tfd;

	tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    	if (tfd == -1) {
        	perror("timerfd_create");
		return -1;
    	}

    	ts.it_value.tv_sec = interval_ms / 1000;
    	ts.it_value.tv_nsec = (interval_ms % 1000) * 1000000;
    	ts.it_interval.tv_sec = interval_ms / 1000;
   	 ts.it_interval.tv_nsec = (interval_ms % 1000) * 1000000;

    	if (timerfd_settime(tfd, 0, &ts, NULL) == -1) {
        	perror("timerfd_settime");
		return -1;
    	}

    	return tfd;
}

uint16_t adj_pwm(struct pi_motors *pm)
{

	if (abs(pm->c_speed) > abs(pm->t_speed)){ 
		if  (pm->pwm > 0){
			    pm->pwm--;
		}

	}
	else if (abs(pm->c_speed) < abs(pm->t_speed)){
		if  (pm->pwm < MAX_PWM){
			    pm->pwm++;
		}
	}

	return pm->pwm;
}

void set_lspeed(int spd)
{

	if (spd == pi_motL.t_speed)
		return;
	 pi_motL.pwm =(uint16_t)(abs(spd)*K_SPD);
	 pi_motR.pwm =(uint16_t)(abs(spd)*K_SPD);
	 pi_motL.t_speed = spd;
	 pi_motR.t_speed = spd;

	// pi_motL.pwm = abs(spd);
	 //pi_motR.pwm = abs(spd);


	if (spd>=0){

		gpioWrite(MDIR_L, FWD_L);
        	gpioWrite(MDIR_R, FWD_R);
		set_pwm_L(pi_motL.pwm);
		set_pwm_R(pi_motR.pwm);
	}
	else{
		gpioWrite(MDIR_L, BACK_L);
        	gpioWrite(MDIR_R, BACK_R);
		set_pwm_L(pi_motL.pwm);
		set_pwm_R(pi_motR.pwm);
	}
}

void set_aspeed(int spd)
{
 	if (spd == pi_motL.t_speed)
                return;

	 pi_motL.pwm =(uint16_t)(abs(spd)*K_SPD);
	 pi_motR.pwm =(uint16_t)(abs(spd)*K_SPD);

	 pi_motL.t_speed = spd;
	 pi_motR.t_speed = spd;

	if (spd>=0){

                gpioWrite(MDIR_L, BACK_L);
                gpioWrite(MDIR_R, FWD_R);
                set_pwm_L(pi_motL.pwm);
		set_pwm_R(pi_motR.pwm);

        }
        else{
                gpioWrite(MDIR_L, FWD_L);
                gpioWrite(MDIR_R, BACK_R);
                set_pwm_L(pi_motL.pwm);
		set_pwm_R(pi_motR.pwm);

        }
}
