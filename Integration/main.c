/* DriverLib Includes */
#include <driverlib.h>
//#include "pid.c"

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>

#include <stdio.h>

// declare functions
void Moving_Forward(void);
void Moving_Backward(void);
void Moving_Right(void);
void Moving_Left(void);
void Stop_Moving(void);

/* Timer_A PWM Configuration Parameter for Right Motor */
Timer_A_PWMConfig pwmConfig =
    {
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_24,
        10000,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        0 // initialise initial pwm to 0
};

/* Timer_A PWM Configuration2 Parameter for Left Motor*/
Timer_A_PWMConfig pwmConfig2 =
    {
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_24,
        10000,
        TIMER_A_CAPTURECOMPARE_REGISTER_2,
        TIMER_A_OUTPUTMODE_RESET_SET,
        0 // initialise initial pwm to 0
};

int main(void)
{
    /* Halting the watchdog */
    MAP_WDT_A_holdTimer();

    setup_Wheel_Encoder(); // setup wheel encoder
    printf("\nFinished Wheel Setup");

    setup_TimerA1_1Mhz(); // set up timer

    printf("\nFinished Timer Setup");

    /* Configuring P4.2 and P4.0 as Output. P2.5 as peripheral output for PWM */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring P4.4 and P4.5 as Output. P2.4 as peripheral output for PWM */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);
    // To configure Wheel to move forward
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);

    // Enable button interrupt for switch 1.1 and 1.4
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN4);

    /* Configuring Timer_A to have a period of approximately 80ms and an initial duty cycle of 0% of that   */
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);

    // Set up barcode
    runBarcode();

    /* Enabling interrupts */
    Interrupt_enableInterrupt(INT_PORT1);
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();

    // Start reading values from the line sensor
    startConversion();

    /* Sleeping when not in use */
    while (1)
    {
        PCM_gotoLPM3InterruptSafe();
    }
}

/* Port1 ISR - This ISR will progressively step up the duty cycle of the PWM on a button press */
void PORT1_IRQHandler(void)
{
    uint32_t status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);
    static uint32_t state_mode = 0;

    if (status & GPIO_PIN1)
    {
        if ((pwmConfig.dutyCycle > 9000 && pwmConfig2.dutyCycle > 9000))
        { // configure duty cycle to 0 if duty cycle more than 9000
            pwmConfig.dutyCycle = 0;
            pwmConfig2.dutyCycle = 0;
        }
        else
        { // add 3000 to duty cycle
            pwmConfig.dutyCycle += 3000;
            pwmConfig2.dutyCycle += 3000;
        }

        // generate PWM
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
    }

    if (status & GPIO_PIN4)
    {
        switch (state_mode % 9)
        {
        case 0: // stop
            state_mode++;
            Stop_Moving();
            break;
        case 1: // backward
            state_mode++;
            Moving_Backward();
            break;
        case 2: // stop
            state_mode++;
            Stop_Moving();
            break;
        case 3: // right
            state_mode++;
            Moving_Right();
            break;
        case 4: // stop
            state_mode++;
            Stop_Moving();
            break;
        case 5: // forward
            state_mode++;
            Moving_Forward();
            break;
        case 6: // stop
            state_mode++;
            Stop_Moving();
            break;
        case 7: // left
            state_mode++;
            Moving_Left();
            break;
        case 8: // stop
            state_mode++;
            Stop_Moving();
            break;
        case 9: // forward
            state_mode++;
            Moving_Forward();
            break;
        }
    }
}

void PORT2_IRQHandler(void) // interrupt for left wheel encoder
{
    wheel_Encoder_Left_IRQ();
}
/* GPIO ISR */
void PORT3_IRQHandler(void) // interrupt for right wheel encoder
{
    wheel_Encoder_Right_IRQ();
}

void TA1_0_IRQHandler(void) // interrupt for wheel encoder timer
{
    wheel_Encoder_Timer_INT();
    /* Clear interrupt flag */
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

void Moving_Forward(void) // function to move forward
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2);

    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
}

void Moving_Backward(void) // function to move backward
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2);

    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5);
}

void Moving_Right(void) // function to move right
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2);

    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5);
}

void Moving_Left(void) // function to move left
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2);

    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
}

void Stop_Moving(void) // function to stop moving
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2);

    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
}
