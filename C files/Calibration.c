#include <unistd.h>
#include <pigpio.h>
#include <stdio.h>
#include <math.h>

#define FPIN1 17
#define FPIN2 27
#define AFPIN1 19
#define AFPIN2 18

int main()
{
    float A1, A2, DtCl1faux ;
    
    float f1 = 200; // Pump 1 frequency in Hz
    float f2 = 200; // Pump 2 frequency in Hz
    float Af = 500000; // Amplitude pin frequency 0.2-1Mhz
    float DtCl1f = 1000000;
    float DtCl2f = 1000000;
    int T = 100; // Run duration in sec

    if (gpioInitialise() < 0)
    {
        fprintf(stderr, "Error initializing Pigpio library\n");
        return 1;
    }
    
    // Pump 1
    gpioSetMode(FPIN1, PI_OUTPUT);
    gpioSetMode(AFPIN1, PI_OUTPUT); 
    
    // Pump 2
    gpioSetMode(FPIN2, PI_OUTPUT);
    gpioSetMode(AFPIN2, PI_OUTPUT); 

    // Assign PWM frequency
    gpioSetPWMrange(FPIN1, 1000);
    gpioSetPWMrange(FPIN2, 1000);
    gpioSetPWMfrequency(FPIN1, f1);
    gpioSetPWMfrequency(FPIN2, f2);

    // Start pumps and run for given time
    printf("Started\n");
    gpioPWM(FPIN1, 500);
    gpioPWM(FPIN2, 500);
    gpioHardwarePWM(AFPIN1, Af, DtCl1f);
    //gpioHardwarePWM(AFPIN2, Af, DtCl2f);
    
    DtCl2f = 300000;   //Change duty cycle here
    gpioHardwarePWM(AFPIN2, Af, DtCl2f);    
    gpioSetPWMfrequency(FPIN2, 200); //Change frequency here
    
    sleep(T);

    // Stop pumps and clean up
    gpioPWM(FPIN1, 0);
    gpioPWM(FPIN2, 0);
    gpioWrite(FPIN1, 0);
    gpioWrite(FPIN2, 0);
    gpioHardwarePWM(AFPIN1, 0, 0);
    gpioHardwarePWM(AFPIN2, 0, 0);
    gpioTerminate();

    printf("Completed\n");
    
    return 0;
}
