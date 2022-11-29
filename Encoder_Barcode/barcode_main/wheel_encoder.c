
#include "modules.h"

#define WHEEL_CIRCUMFERENCE  (uint32_t) 22 // 22 CM
#define NOTCHES (uint32_t)20
#define TICKPERIOD (uint32_t)1000
#define MOVING_AVG_DELTA (float)0.5  // Apply moving average on Velocity to reduce noise from inertia
#define PAUSE_PERIOD (uint32_t)1000        // Velocity becomes 0 after 500 ms
//SET UP PIN FOR WHEEL ENCODER

#define WHEEL_ENCODER_LEFT_PORT GPIO_PORT_P2
#define WHEEL_ENCODER_LEFT_PIN GPIO_PIN6
#define WHEEL_ENCODER_LEFT_INTPORT INT_PORT2

#define WHEEL_ENCODER_RIGHT_PORT GPIO_PORT_P3
#define WHEEL_ENCODER_RIGHT_PIN GPIO_PIN2
#define WHEEL_ENCODER_RIGHT_INTPORT INT_PORT3


/* Global variables */
volatile uint32_t wheel_Right_Time_Counter = 0;
volatile uint32_t wheel_Right_Time_Start = 0;
volatile float wheel_Right_Velocity = 0.0;


volatile uint32_t wheel_Left_Time_Counter = 0;
volatile uint32_t wheel_Left_Time_Start = 0;
volatile float wheel_Left_Velocity = 0.0;

const float TIME_PER_CYCLE  = 1.0/ 1000000;

const float WHEEL_CIRC_PER_NOTCH   = 1.0 *WHEEL_CIRCUMFERENCE/NOTCHES;   // In centimeters


volatile char startLeftWheel = '0';
volatile char startRightWheel = '0';
volatile uint32_t leftWheelCount = 0;
volatile uint32_t rightWheelCount = 0;

/* Functions */
void setup_Wheel_Encoder(void);
void setup_TimerA1_1Mhz(void);
void wheel_Encoder_Right_IRQ(void);
void wheel_Encoder_Left_IRQ(void);
void wheel_Encoder_Timer_INT(void);
void wheelVelocity_Print(void);

void startLeftWheelCount(void);
void startRightWheelCount(void);
void stopLeftWheelCount(void);
void stopRightWheelCount(void);
void clearLeftWheelCount(void);
void clearRightWheelCount(void);
float getRightWheelDistance(void);
float getLeftWheelDistance(void);

void startLeftWheelCount(){
    startLeftWheel = '1';
}

void startRightWheelCount(){
    startRightWheel = '1';
}
void stopLeftWheelCount(){
    startLeftWheel = '0';
}

void stopRightWheelCount(){
    startRightWheel = '0';
}

void clearLeftWheelCount(){
    leftWheelCount = 0;
}
void clearRightWheelCount(){
    rightWheelCount = 0;
}

float getLeftWheelDistance(){
    return leftWheelCount * WHEEL_CIRC_PER_NOTCH;
}

float getRightWheelDistance(){
    return rightWheelCount * WHEEL_CIRC_PER_NOTCH;
}


void setup_Wheel_Encoder(void){

    // Setup Right Wheel Encoder
    GPIO_setAsInputPinWithPullUpResistor(WHEEL_ENCODER_RIGHT_PORT, WHEEL_ENCODER_RIGHT_PIN);   // Set as input pin with pull_up_resistor

    GPIO_clearInterruptFlag(WHEEL_ENCODER_RIGHT_PORT, WHEEL_ENCODER_RIGHT_PIN);     //Clear interrupt flag for  and Enable as interrup

    GPIO_enableInterrupt(WHEEL_ENCODER_RIGHT_PORT, WHEEL_ENCODER_RIGHT_PIN);       // Enable interrupt

    GPIO_interruptEdgeSelect(WHEEL_ENCODER_RIGHT_PORT, WHEEL_ENCODER_RIGHT_PIN, GPIO_HIGH_TO_LOW_TRANSITION);    // Set Interrupt edge HIGH TO LOW

    if (GPIO_getInputPinValue(WHEEL_ENCODER_RIGHT_PORT, WHEEL_ENCODER_RIGHT_PIN) == 0){      // If current input value is LOW
        GPIO_interruptEdgeSelect(WHEEL_ENCODER_RIGHT_PORT, WHEEL_ENCODER_RIGHT_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    }

    //Enable interrupt from P2 and Enable all interrupts
    Interrupt_enableInterrupt(WHEEL_ENCODER_RIGHT_INTPORT);

    GPIO_setAsInputPinWithPullUpResistor(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN);   // Set input pin with pull_up_resistor

    GPIO_clearInterruptFlag(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN);     //Clear interrupt flag  and Enable  interrupt

    GPIO_enableInterrupt(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN);       // Enable interrupt

    GPIO_interruptEdgeSelect(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN, GPIO_HIGH_TO_LOW_TRANSITION);    // Set  Interrupt edge HIGH TO LOW

    if (GPIO_getInputPinValue(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN) == 0){      // If current input value of P2 Pin5 is LOW
        GPIO_interruptEdgeSelect(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
    }


    //Enable interrupt from P2 and Enable all interrupts
    Interrupt_enableInterrupt(WHEEL_ENCODER_LEFT_INTPORT);

}




void setup_TimerA1_1Mhz(void){
    /* Timer_A UpMode Configuration Parameter */
    const Timer_A_UpModeConfig upConfig = {
                TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
                TIMER_A_CLOCKSOURCE_DIVIDER_3,          // SMCLK/3 = 1MHz
                TICKPERIOD,                             // 1000 tick period , 1 ms per period
                TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
                TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
                TIMER_A_DO_CLEAR                        // Clear value
    };
    /* Configuring Timer_A0 for Up Mode */
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);
    /* Enabling interrupts and starting the timer */
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

}



void wheel_Encoder_Left_IRQ(void){

    uint32_t status = GPIO_getEnabledInterruptStatus(WHEEL_ENCODER_LEFT_PORT);
    // Check pin
    if(status & WHEEL_ENCODER_LEFT_PIN)
    {
        if (startLeftWheel == '1'){
            leftWheelCount++;
        }

        //P2->IES ^=  WHEEL_ENCODER_LEFT_PIN;

        if (wheel_Left_Time_Counter   < 1){

            wheel_Left_Time_Start=  Timer_A_getCounterValue(TIMER_A1_BASE);
            wheel_Left_Time_Counter = wheel_Left_Time_Start + 1;
        }
        else{
            wheel_Left_Time_Counter = wheel_Left_Time_Counter + Timer_A_getCounterValue(TIMER_A1_BASE) - wheel_Left_Time_Start - 1;
            if (wheel_Left_Velocity <  0.1){
                wheel_Left_Velocity = WHEEL_CIRC_PER_NOTCH  /   (wheel_Left_Time_Counter * TIME_PER_CYCLE) ;
                //wheel_Left_Velocity = 1  /   (wheel_Left_Time_Counter * TIME_PER_CYCLE) ;
            }
            else{
                wheel_Left_Velocity = WHEEL_CIRC_PER_NOTCH  /   (wheel_Left_Time_Counter * TIME_PER_CYCLE) * MOVING_AVG_DELTA +  wheel_Left_Velocity * (1-MOVING_AVG_DELTA);
                //wheel_Left_Velocity = 1   /   (wheel_Left_Time_Counter * TIME_PER_CYCLE) * MOVING_AVG_DELTA +  wheel_Left_Velocity * (1 - MOVING_AVG_DELTA);
                //pid_Left_Straight();

            }

            wheel_Left_Time_Start=  Timer_A_getCounterValue(TIMER_A1_BASE);
            wheel_Left_Time_Counter =  1;
        }
        GPIO_clearInterruptFlag(WHEEL_ENCODER_LEFT_PORT, status);
    }
}

void wheel_Encoder_Right_IRQ(void){
   uint32_t status = GPIO_getEnabledInterruptStatus(WHEEL_ENCODER_RIGHT_PORT);
    /* Toggling the output on the LED */
    if(status &  WHEEL_ENCODER_RIGHT_PIN)
    {
        //P3->IES ^=  WHEEL_ENCODER_RIGHT_PIN;
        if (startRightWheel == '1'){
            rightWheelCount++;
        }

        if (wheel_Right_Time_Counter <1){
            wheel_Right_Time_Start = Timer_A_getCounterValue(TIMER_A1_BASE);
            wheel_Right_Time_Counter =  wheel_Right_Time_Start + 1;
        }

        else{
            wheel_Right_Time_Counter = wheel_Right_Time_Counter + Timer_A_getCounterValue(TIMER_A1_BASE) - wheel_Right_Time_Start - 1;
            if (wheel_Left_Velocity <  0.1){
                wheel_Right_Velocity = WHEEL_CIRC_PER_NOTCH  / (wheel_Right_Time_Counter * TIME_PER_CYCLE);
            }
            else{
                wheel_Right_Velocity = WHEEL_CIRC_PER_NOTCH  / (wheel_Right_Time_Counter * TIME_PER_CYCLE) * (MOVING_AVG_DELTA) +  wheel_Right_Velocity * (1 -MOVING_AVG_DELTA);
               //pid_Right_Straight();

            }

            wheel_Right_Time_Start = Timer_A_getCounterValue(TIMER_A1_BASE);
            wheel_Right_Time_Counter =  1;
        }
        GPIO_clearInterruptFlag(WHEEL_ENCODER_RIGHT_PORT, status);
    }
}



void wheelVelocity_Print(){
    static uint32_t print_counter = 0;
    print_counter++ ;
    if (print_counter > 2000){
         printf("\nLeft wheel velocity:  %0.2f ",  wheel_Left_Velocity );
         printf("\nRight wheel velocity:  %0.2f ",  wheel_Right_Velocity);
         printf("\nLEFT PWM: %d",pwmConfig2.dutyCycle);
         printf("\nRIGHT PWM: %d",pwmConfig.dutyCycle);
         print_counter = 0;
    }
}

void wheel_Encoder_Timer_INT(void){
    /* Increment global variable (count number of interrupt occurred) */
    static uint32_t timer_count = 0;
    timer_count++;
    if (timer_count > 700 && motor_state == '1'){
        ///pid_Left_Straight();
        //pid_Right_Straight();
        straight_PID();
        timer_count = 0;
    }



    if (wheel_Right_Time_Counter > 0) {
        wheel_Right_Time_Counter += TICKPERIOD;
    }
    if (wheel_Left_Time_Counter > 0){
        wheel_Left_Time_Counter += TICKPERIOD;
    }

    if (wheel_Left_Time_Counter /TICKPERIOD > PAUSE_PERIOD){  // After 0.5s reset velocity
        wheel_Left_Velocity= 0.0;
        wheel_Left_Time_Counter = 0;
    }
    if (wheel_Right_Time_Counter /TICKPERIOD  > PAUSE_PERIOD){ // After 0.5s reset velocity
        wheel_Right_Velocity= 0.0;
        wheel_Right_Time_Counter = 0;
    }
    wheelVelocity_Print();
}

//int main(void)
//{
//
//    /* Halting the Watchdog */
//    WDT_A_holdTimer();
//
//    //Run wheel Encoder Setup and TimerA0 to 1Mhz
//    setup_Wheel_Encoder();
//    printf("\nFinished Wheel Setup");
//
//    setup_TimerA0_1Mhz();
//
//    printf("\nFinished Timer Setup");
//
//    /* Enabling MASTER interrupts */
//
//    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
//
//    Interrupt_enableMaster();
//
//    /* Going to LPM3 */
//    while(1)
//    {
//        PCM_gotoLPM3InterruptSafe();
//    }
//}

/*
float get_wheel_velocity(float *wheel_velocity , int *wheel_time_counter){
    *wheel_velocity =   WHEEL_CIRC_PER_NOTCH / (wheel_time_counter  + Timer_A_getCounterValue(TIMER_A0_BASE) );
    *wheel_time_counter = 0.0;

}*/



