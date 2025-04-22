#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>
#include "rotary_encoder.h"


#define MOT_SPEED_L  	300
#define MOT_SPEED_R  	300

#define FWD		0
#define BACK	  	1

#define MDIV		11
#define S_TARGET1	1200

struct pi_motors
{
	int pos_L;
	int pos_R;
	int speed_L;
	int speed_R;
	uint16_t pwm_L;
	uint16_t pwm_R;
	int target_s;

};

int MPWM_L=12;
int MPWM_R=13;


struct pi_motors pi_mot;

void set_pwm_L(uint16_t pwm)
{
	 if (pwm > 1000) pwm = 1000;
	 gpioHardwarePWM(MPWM_L,5000,pwm*1000);

}

void set_pwm_R(uint16_t pwm)
{
	 if (pwm > 1000) pwm = 1000;
	 gpioHardwarePWM(MPWM_R,5000,pwm*1000);

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
		pi_mot.pos_L += dir;
		pi_mot.speed_L = 60*1000000/td_sum;
   		//printf("L:%d\n", pi_mot.speed_L);
   		printf("\rLS:%d RS:%d L:%d R:%d", 
			pi_mot.speed_L, pi_mot.speed_R, pi_mot.pos_L,pi_mot.pos_R);
		fflush(stdout);
		td_sum = 0;

		/*if (pi_mot.speed_L > pi_mot.target_s){ 
			pi_mot.pwm_L--;
			set_pwm_L(pi_mot.pwm_L);

		}
		else if (pi_mot.speed_L < pi_mot.target_s){
			 pi_mot.pwm_L++;
			 set_pwm_L(pi_mot.pwm_L);
		}*/
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
                pi_mot.pos_R += dir;
                pi_mot.speed_R = 60*1000000/td_sum;
                //printf("\rR:%d", pi_mot.pos_R);
		//fflush(stdout);
		//printf("R:%d\n", pi_mot.speed_R);
                td_sum = 0;

		/*if (pi_mot.speed_R > pi_mot.target_s) 
			pi_mot.pwm_R--;
		else if (pi_mot.speed_R < pi_mot.target_s) 
			pi_mot.pwm_R++;
		set_pwm_R(pi_mot.pwm_R); */
        }
}




int main(int argc, char *argv[])
{
        char c;

        int MDIR_L=24;
        int MDIR_R=25;
        int MEN=5;

	int gpioA_L = 17;
	int gpioB_L = 18;
	int gpioA_R = 20;
	int gpioB_R = 21;


 	Pi_Renc_t *renc1, *renc2;


        if (gpioInitialise() < 0)
                return 1;

	pi_mot.pwm_L = MOT_SPEED_L;
	pi_mot.pwm_R = MOT_SPEED_R;
	pi_mot.target_s = S_TARGET1;


        gpioSetMode(MPWM_L, PI_ALT0);
        gpioSetMode(MPWM_R, PI_ALT0);

        gpioSetMode(MDIR_L, PI_OUTPUT);
        gpioSetMode(MDIR_R, PI_OUTPUT);

        gpioSetMode(MEN, PI_OUTPUT);
        //gpioSetPWMfrequency(MPWM_L, 1000);
        //gpioSetPWMfrequency(MPWM_R, 1000);

        //gpioPWM(MPWM_L, 0);
        //gpioPWM(MPWM_R, 0);
	gpioHardwarePWM(MPWM_L,8000,0);
	gpioHardwarePWM(MPWM_R,8000,0);
        gpioWrite(MEN, 0);

	renc1 = Pi_Renc(gpioA_L, gpioB_L, callback_L);
	renc2 = Pi_Renc(gpioA_R, gpioB_R, callback_R);


        while(1){
                c  = getchar();

                if  (c == 'f'){
                        gpioWrite(MDIR_L, FWD);
                        gpioWrite(MDIR_R, FWD);
			set_pwm_L(pi_mot.pwm_L);
			set_pwm_R(pi_mot.pwm_R);
                        //gpioPWM(MPWM_L, pi_mot.pwm_L);
                        //gpioPWM(MPWM_R, pi_mot.pwm_R);
                }
                else if (c == 'b'){
                        gpioWrite(MDIR_L, BACK);
                        gpioWrite(MDIR_R, BACK);
                        set_pwm_L(pi_mot.pwm_L);
			set_pwm_R(pi_mot.pwm_R);


                }
		else if (c == 'l'){
			pi_mot.pwm_R++;
			set_pwm_R(pi_mot.pwm_R);
		        //printf("%d\n", pi_mot.pwm_R);



                      /*  gpioWrite(MDIR_L, BACK);
                        gpioWrite(MDIR_R, FWD);
                        gpioPWM(MPWM_L, MOT_SPEED_L);
                        gpioPWM(MPWM_R, MOT_SPEED_L); */

                }
 		else if (c == 'r'){
			pi_mot.pwm_R--;
			set_pwm_R(pi_mot.pwm_R);

			//printf("%d\n", pi_mot.pwm_R);

                       /* gpioWrite(MDIR_L, FWD);
                        gpioWrite(MDIR_R, BACK);
                        gpioPWM(MPWM_L, MOT_SPEED_L);
                        gpioPWM(MPWM_R, MOT_SPEED_L); */

                }

                else if (c == 's'){
                	set_pwm_R(0);
                	set_pwm_L(0);
                }
                else if (c == 'x'){

                        break;
                }

        }
	Pi_Renc_cancel(renc1);
	Pi_Renc_cancel(renc2);

        gpioTerminate();
}
