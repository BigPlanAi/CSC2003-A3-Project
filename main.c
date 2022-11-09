/* DriverLib Includes */
#include <driverlib.h>

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>

#include <stdio.h>

/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_24,
        10000,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        0
};

Timer_A_PWMConfig pwmConfig2 =
{
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_24,
        10000,
        TIMER_A_CAPTURECOMPARE_REGISTER_2,
        TIMER_A_OUTPUTMODE_RESET_SET,
        0
};


int main(void)
 {
    /* Halting the watchdog */
    MAP_WDT_A_holdTimer();

    setup_Wheel_Encoder();
    printf("\nFinished Wheel Setup");

    setup_TimerA1_1Mhz();

    printf("\nFinished Timer Setup");
    printf("Finished Timer Setup");

    /* Configuring P4.2 and P4.0 as Output. P2.5 as peripheral output for PWM and P1.1 for button interrupt */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN4);

    /* Configuring P4.4 and P4.5 as Output. P2.4 as peripheral output for PWM and P1.1 for button interrupt */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);
    //To configure Wheel to move forward
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);



    /* Configuring Timer_A to have a period of approximately 80ms and an initial duty cycle of 10% of that (1000 ticks)  */
//    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);

    /* Enabling interrupts and starting the watchdog timer */
    Interrupt_enableInterrupt(INT_PORT1);
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();

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

    if (status & GPIO_PIN1)
    {
        if((pwmConfig.dutyCycle > 9000 || pwmConfig2.dutyCycle > 9000)){
            pwmConfig.dutyCycle = 0;
            pwmConfig2.dutyCycle = 0;
        }
        else {
            pwmConfig.dutyCycle += 3000;
            pwmConfig2.dutyCycle += 3000;
        }
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig2);
    }


    if (status & GPIO_PIN4)
        {
            GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN0);
            GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN2);
            GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN4);
            GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN5);
        }
}



void PORT2_IRQHandler(void)
{
    wheel_Encoder_Left_IRQ();

}
/* GPIO ISR */
void PORT3_IRQHandler(void)
{
    wheel_Encoder_Right_IRQ();

}

void TA1_0_IRQHandler(void)
{
    wheel_Encoder_Timer_INT();
    /* Clear interrupt flag */
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}
