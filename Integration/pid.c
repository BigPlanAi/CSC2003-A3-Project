#include "modules.h"

// PID variables
float KP = 200;
float KI = 10;
float KD = 5;


//error variables
float int_error = 0;
float prev_error = 0;
volatile int sum_error=0 ;
volatile short int lastError=0;

void straight_PID(){
    int error = leftWheelCount - rightWheelCount; //calculate difference between wheel count
    //reset wheel count
    clearLeftWheelCount();
    clearRightWheelCount();

    printf("\nerror : %d", error);
    sum_error += error; //calculate sum of error
    printf(" sum_error : %d\n", sum_error);

    //calculate difference in error
    long dError = (error - lastError)*0.05;
    int_error = int_error + 0.05*error;

    // calculate adjust amount for error correction
    float adjust =     (KP*error)/100.0f
                    + (KI*int_error)/100.0f;
                    + (KD*dError)/100.0f;


    // adjust both wheels
    pwmConfig2.dutyCycle += adjust/2;
    pwmConfig.dutyCycle += adjust/2;
    printf("Duty cycle 1: %d\t", pwmConfig.dutyCycle);
    printf("Duty cycle 2: %d\n", pwmConfig2.dutyCycle);

    // generate adjusted PWM
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    // previous error
    lastError = error;
}



