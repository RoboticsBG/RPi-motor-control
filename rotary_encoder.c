#include <stdio.h>
#include <stdlib.h>

#include <pigpio.h>

#include "rotary_encoder.h"

struct _Pi_Renc_s
{
   int gpioA;
   int gpioB;
   Pi_Renc_CB_t callback;
   int levA;
   int levB;
   int lastGpio;
   uint32_t lastTick;
};


/*

             +---------+         +---------+      0
             |         |         |         |
   A         |         |         |         |
             |         |         |         |
   +---------+         +---------+         +----- 1

       +---------+         +---------+            0
       |         |         |         |
   B   |         |         |         |
       |         |         |         |
   ----+         +---------+         +---------+  1

*/

static void _cb(int gpio, int level, uint32_t tick, void *user)
{
   Pi_Renc_t *renc;
   
   uint32_t delta;
   renc = user;


   if (gpio == renc->gpioA) renc->levA = level; 
   else renc->levB = level;

   if (gpio != renc->lastGpio) /* debounce */
   {
      renc->lastGpio = gpio;

      if ((gpio == renc->gpioA) && (level == 1))
      {
         delta = tick - renc->lastTick;
	renc->lastTick = tick;
	if (renc->levB) (renc->callback)(1,delta);
        else (renc->callback)(-1,delta);
      }
     /* else if ((gpio == renc->gpioB) && (level == 1))
      {
         if (renc->levA) (renc->callback)(1);
      }*/
   }
}

Pi_Renc_t * Pi_Renc(int gpioA, int gpioB, Pi_Renc_CB_t callback)
{
   Pi_Renc_t *renc;

   renc = malloc(sizeof(Pi_Renc_t));

   renc->gpioA = gpioA;
   renc->gpioB = gpioB;
   renc->callback = callback;
   renc->levA=0;
   renc->levB=0;
   renc->lastGpio = -1;
   renc->lastTick = gpioTick();

   gpioSetMode(gpioA, PI_INPUT);
   gpioSetMode(gpioB, PI_INPUT);

   /* pull up is needed as encoder common is grounded */

   gpioSetPullUpDown(gpioA, PI_PUD_UP);
   gpioSetPullUpDown(gpioB, PI_PUD_UP);

   /* monitor encoder level changes */

   gpioSetAlertFuncEx(gpioA, _cb, renc);
   gpioSetAlertFuncEx(gpioB, _cb, renc);

   return renc;
}

void Pi_Renc_cancel(Pi_Renc_t *renc)
{
   if (renc)
   {
      gpioSetAlertFunc(renc->gpioA, 0);
      gpioSetAlertFunc(renc->gpioB, 0);

      free(renc);
   }
}

