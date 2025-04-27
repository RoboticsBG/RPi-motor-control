#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pigpio.h>
#include "rotary_encoder.h"
#include "udp_sender.h"

#define MAX_PWM		1000
#define PWM_FREQ	5000
#define MOT_SPEED_L  	300
#define MOT_SPEED_R  	300

#define FWD		0
#define BACK	  	1

#define MDIV		11
#define K_SPD		0.222F

struct pi_motors
{
	int pos;
	int c_speed;
	int t_speed;
	uint16_t pwm;
};

int MPWM_L=12;
int MPWM_R=13;
int MDIR_L=24;
int MDIR_R=25;
int MEN=5;

struct pi_motors pi_motL, pi_motR;

void set_lspeed(int spd);
void set_aspeed(int spd);

uint16_t adj_pwm(struct pi_motors *pm);


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
	char str[32];

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
   		sprintf(str,"\rLS:%d|RS:%d|L:%d|R:%d ", 
			pi_motL.c_speed, pi_motR.c_speed, pi_motL.pos,pi_motR.pos);
		send_udp_data(str);
		printf("%s",str);
		fflush(stdout);
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
                //printf("\rR:%d", pi_mot.pos_R);
		//fflush(stdout);
		//printf("R:%d\n", pi_mot.speed_R);
                td_sum = 0;

		pi_motR.pwm = adj_pwm(&pi_motR);
		set_pwm_R(pi_motR.pwm);


        }
}




int main(int argc, char *argv[])
{
        char c;
	char buffer[128];
	int val,ret;


	int gpioA_L = 17;
	int gpioB_L = 18;
	int gpioA_R = 20;
	int gpioB_R = 21;


 	Pi_Renc_t *renc1, *renc2;

	if (init_udp_sender() < 0){
		return 1;
	}


        if (gpioInitialise() < 0)
                return 1;

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
        gpioWrite(MEN, 0);

	renc1 = Pi_Renc(gpioA_L, gpioB_L, callback_L);
	renc2 = Pi_Renc(gpioA_R, gpioB_R, callback_R);

        while(1){
                fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = 0; 
		c = buffer[0];
		val = atoi(&buffer[1]);

                if  (c == 'm'){
			set_lspeed(val);
                }

		else if (c == 'r'){
			set_aspeed(val);

                }

                else if (c == 's'){
                	set_lspeed(0);
		}
                else if (c == 'x'){

                        break;
                }

        }

	close_udp_sender();
	Pi_Renc_cancel(renc1);
	Pi_Renc_cancel(renc2);

        gpioTerminate();
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

	 pi_motL.pwm =(uint16_t)(abs(spd)*K_SPD);
	 pi_motR.pwm =(uint16_t)(abs(spd)*K_SPD);
	 pi_motL.t_speed = spd;
	 pi_motR.t_speed = spd;

	// pi_motL.pwm = abs(spd);
	 //pi_motR.pwm = abs(spd);


	if (spd>=0){

		gpioWrite(MDIR_L, FWD);
        	gpioWrite(MDIR_R, FWD);
		set_pwm_L(pi_motL.pwm);
		set_pwm_R(pi_motR.pwm);
	}
	else{
		gpioWrite(MDIR_L, BACK);
        	gpioWrite(MDIR_R, BACK);
		set_pwm_L(pi_motL.pwm);
		set_pwm_R(pi_motR.pwm);
	}
}

void set_aspeed(int spd)
{

	 pi_motL.pwm =(uint16_t)(abs(spd)*K_SPD);
	 pi_motR.pwm =(uint16_t)(abs(spd)*K_SPD);

	 pi_motL.t_speed = spd;
	 pi_motR.t_speed = spd;

	if (spd>=0){

                gpioWrite(MDIR_L, BACK);
                gpioWrite(MDIR_R, FWD);
                set_pwm_L(pi_motL.pwm);
		set_pwm_R(pi_motR.pwm);

        }
        else{
                gpioWrite(MDIR_L, FWD);
                gpioWrite(MDIR_R, BACK);
                set_pwm_L(pi_motL.pwm);
		set_pwm_R(pi_motR.pwm);

        }
}
