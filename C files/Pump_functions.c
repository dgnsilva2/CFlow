#include <unistd.h>
#include <pigpio.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define FPIN1 17
#define FPIN2 27
#define AFPIN1 19
#define AFPIN2 18

void Q_to_DtClf(float Q, float* DtClf, int* freq, int Pump_Number) { // function that converts wanted flow to frequency and duty cycle
    
    if (Pump_Number == 1) {
        if (300 < Q && Q <= 1100) {
            *DtClf = 0.93*Q*Q + 191.75*Q + 96666;
            *freq = 50;
        } else if (1050 < Q && Q <= 3720) {
            *DtClf = 0.0743*Q*Q - 52.723*Q + 168655;
            *freq = 200;
        } else {
            *freq = 0;
            *DtClf = 0;
            if (Q)
            {
                printf("Flow rate 1 out of range\n");
            }
            return;
        }
        
    } else if (Pump_Number == 2) {
        if (350 < Q && Q <= 1016) {
            *DtClf = 0.9307*Q*Q - 170.76*Q + 212513;
            *freq = 50;
        } else if (1016 < Q && Q <= 1380) {
            *DtClf = 609.17*Q - 273235;
            *freq = 100;
        } else if (1380 < Q && Q <= 3470) {
            *DtClf = 0.0493*Q*Q + 113.97*Q + 8194.7;
            *freq = 200;
        } else {
            *freq = 0;
            *DtClf = 0;
            if (Q)
            {
                printf("Flow rate 2 out of range\n");
            }
            return;
        }
        
    } else {
        printf("Pump number must be 1 or 2\n");
        return;
    }
    
    return;
}

void Constant_Flow(float Q1, float Q2, int T, int delay, int PW) {
    bool isrun = false;
    float Af = 500000;
    int f1, f2;
    float DtCl1f, DtCl2f; //variables to control the flow (f -> clock frequency of the controller) (DtClf -> cuty cycle for the PWM signal)
    
    if (gpioInitialise() < 0)
    {
        fprintf(stderr, "Error initializing Pigpio library\n");
        return;
    }
    
    // Convert flow rates from uL/min to frequency (Hz)
    Q_to_DtClf(Q1, &DtCl1f, &f1, 1);
    Q_to_DtClf(Q2, &DtCl2f, &f2, 2);

    printf("\nDC:%f\n", DtCl2f);
    printf("\nF:%d\n", f2);
    printf("Started\n");
    sleep(delay);

    // Setup data pins
    // Pump 1
    gpioSetMode(FPIN1, PI_OUTPUT);
    gpioSetMode(AFPIN1, PI_OUTPUT); 
    // Pump 2
    gpioSetMode(FPIN2, PI_OUTPUT);
    gpioSetMode(AFPIN2, PI_OUTPUT); 
    
    //Setup Frequency Pins PWM range
    gpioSetPWMrange(FPIN1, 1000); //1000 represents the maximum range for the duty cycle of the PWM signal (1000 = 100%, 500 = 50%...)
    gpioSetPWMrange(FPIN2, 1000);
    gpioPWM(FPIN1, 500); //500 = 1000/2, that way we can get a perfect square wave for the clock signal of the controller
    gpioPWM(FPIN2, 500);
    
    // Pump wash
    if (PW)
    {
        printf("Started Pump Wash\n");
        if (f1)
        {
            gpioHardwarePWM(AFPIN1, Af, 1000000); // Af is constant. DtCl1f will change to match flow accordingly
            gpioSetPWMfrequency(FPIN1, 200); // frequency of the clock signal, will change (or not) along with DtClf to get a certain flow

        }
        if (f2)
        {
            gpioHardwarePWM(AFPIN2, Af, 1000000);
            gpioSetPWMfrequency(FPIN2, 200);
        }
        sleep(10);
        printf("Completed Pump Wash\n");
    }

    // Assign PWM frequency
    gpioSetPWMfrequency(FPIN1, f1); // frequency of the clock signal, will change (or not) along with DtClf to get a certain flow
    gpioSetPWMfrequency(FPIN2, f2);

    // Start pumps and run for given time
    if (f1) //checks if the pump was give an okay value (if f=0 then the value was not okay)
    {
        gpioHardwarePWM(AFPIN1, Af, DtCl1f); // Af is constant. DtCl1f will change to match flow accordingly
        isrun = true;
    }
    if (f2)
    {
        gpioHardwarePWM(AFPIN2, Af, DtCl2f);
        isrun = true;
    }
    if (f1 || f2)
    {
        printf("Pumping\n");
        sleep(T); //pauses the program for T seconds
    }

    // Stop pumps and clean up
    gpioPWM(FPIN1, 0);
    gpioPWM(FPIN2, 0);
    gpioWrite(FPIN1, 0);
    gpioWrite(FPIN2, 0);
    gpioHardwarePWM(AFPIN1, 0, 0);
    gpioHardwarePWM(AFPIN2, 0, 0);
    gpioTerminate(); 

    printf("Completed\n");
}

int main() {
    Constant_Flow(0, 3000, 60, 0, 0); //1st value is pump 1 flow, 2nd value is pump 2 flow, 3rd value is time in seconds to run, 4th value is the delay until the pumps start running in seconds, 5th value (1/0) pump wash to clear air bubbles
}
