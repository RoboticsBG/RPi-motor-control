#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>


#define MOT_SPEED  128

int main(int argc, char *argv[])
{
        char c;

        int MPWM1=12;
        int MPWM2=13;
        int MDIR1=24;
        int MDIR2=25;
        int MEN=5;


        if (gpioInitialise() < 0)
                return 1;

        gpioSetMode(MPWM1, PI_OUTPUT);
        gpioSetMode(MPWM2, PI_OUTPUT);

        gpioSetMode(MDIR1, PI_OUTPUT);
        gpioSetMode(MDIR2, PI_OUTPUT);

        gpioSetMode(MEN, PI_OUTPUT);
        gpioSetPWMfrequency(MPWM1, 8000);
        gpioSetPWMfrequency(MPWM2, 8000);

        gpioPWM(MPWM1, 0);
        gpioPWM(MPWM2, 0);
        gpioWrite(MEN, 0);

        while(1){
                c  = getchar();

                if  (c == 'f'){
                        gpioWrite(MDIR1, 0);
                        gpioWrite(MDIR2, 1);
                        gpioPWM(MPWM1, MOT_SPEED);
                        gpioPWM(MPWM2, MOT_SPEED);
                }
                else if (c == 'b'){
                        gpioWrite(MDIR1, 1);
                        gpioWrite(MDIR2, 0);
                        gpioPWM(MPWM1, MOT_SPEED);
                        gpioPWM(MPWM2, MOT_SPEED);

                }
                else if (c == 's'){
                        gpioPWM(MPWM1, 0);
                        gpioPWM(MPWM2, 0);
                }
                else if (c == 'x'){

                        break;
                }

        }
        gpioTerminate();
}
