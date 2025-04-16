#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>


#define MOT_SPEED_L  	64
#define MOT_SPEED_R  	72

#define FWD		0
#define BACK	  	1


int main(int argc, char *argv[])
{
        char c;

        int MPWM_L=12;
        int MPWM_R=13;
        int MDIR_L=24;
        int MDIR_R=25;
        int MEN=5;


        if (gpioInitialise() < 0)
                return 1;

        gpioSetMode(MPWM_L, PI_OUTPUT);
        gpioSetMode(MPWM_R, PI_OUTPUT);

        gpioSetMode(MDIR_L, PI_OUTPUT);
        gpioSetMode(MDIR_R, PI_OUTPUT);

        gpioSetMode(MEN, PI_OUTPUT);
        gpioSetPWMfrequency(MPWM_L, 8000);
        gpioSetPWMfrequency(MPWM_R, 8000);

        gpioPWM(MPWM_L, 0);
        gpioPWM(MPWM_R, 0);
        gpioWrite(MEN, 0);

        while(1){
                c  = getchar();

                if  (c == 'f'){
                        gpioWrite(MDIR_L, FWD);
                        gpioWrite(MDIR_R, FWD);
                        gpioPWM(MPWM_L, MOT_SPEED_L);
                        gpioPWM(MPWM_R, MOT_SPEED_R);
                }
                else if (c == 'b'){
                        gpioWrite(MDIR_L, BACK);
                        gpioWrite(MDIR_R, BACK);
                        gpioPWM(MPWM_L, MOT_SPEED_L);
                        gpioPWM(MPWM_R, MOT_SPEED_R);

                }
		else if (c == 'l'){
                        gpioWrite(MDIR_L, BACK);
                        gpioWrite(MDIR_R, FWD);
                        gpioPWM(MPWM_L, MOT_SPEED_L);
                        gpioPWM(MPWM_R, MOT_SPEED_L);

                }
 		else if (c == 'r'){
                        gpioWrite(MDIR_L, FWD);
                        gpioWrite(MDIR_R, BACK);
                        gpioPWM(MPWM_L, MOT_SPEED_L);
                        gpioPWM(MPWM_R, MOT_SPEED_L);

                }

                else if (c == 's'){
                        gpioPWM(MPWM_L, 0);
                        gpioPWM(MPWM_R, 0);
                }
                else if (c == 'x'){

                        break;
                }

        }
        gpioTerminate();
}
