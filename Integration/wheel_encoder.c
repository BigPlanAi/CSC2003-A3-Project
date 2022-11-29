/*
 *  * Infrared (Encoder and barcode) Members:
 *    - Terence Teh Han Yuan (2101388)
 *    - Wong Jing Yong, Shawn (2101229)
 *    - Mirza Bin Mohamad Aljaru (2101543)
 *
 */



#include "modules.h"

#define WHEEL_CIRCUMFERENCE  22 // 22 CM
#define NOTCHES 20
#define TICKPERIOD 1000
#define MOVING_AVG_DELTA 0.25 //  exponential moving average delta to calculate moving average  Velocity to reduce noise from inertia
#define PAUSE_PERIOD 50000       // Velocity becomes 0 after 500 ms
//SET UP PIN FOR WHEEL ENCODER

#define WHEEL_ENCODER_LEFT_PORT GPIO_PORT_P3
#define WHEEL_ENCODER_LEFT_PIN GPIO_PIN2
#define WHEEL_ENCODER_LEFT_INTPORT INT_PORT3

#define WHEEL_ENCODER_RIGHT_PORT GPIO_PORT_P2
#define WHEEL_ENCODER_RIGHT_PIN GPIO_PIN3
#define WHEEL_ENCODER_RIGHT_INTPORT INT_PORT2


#define YES '1'
#define NO '0'


/* Global variables */
volatile uint32_t wheel_Right_Time_Counter = 0;
volatile uint32_t wheel_Right_Time_Start = 0;
volatile float wheel_Right_Velocity = 0.0;


volatile uint32_t wheel_Left_Time_Counter = 0;
volatile uint32_t wheel_Left_Time_Start = 0;
volatile float wheel_Left_Velocity = 0.0;

const float TIME_PER_CYCLE  = 0.00001;

const float WHEEL_CIRC_PER_NOTCH   = 1.0 *WHEEL_CIRCUMFERENCE/NOTCHES;   // In centimeters


//volatile char startLeftWheel = '0';
//volatile char startRightWheel = '0';

volatile char record_left_encoder_time = '0';
volatile char record_right_encoder_time = '0';
volatile uint32_t leftWheelCount = 0;
volatile uint32_t rightWheelCount = 0;

/* Functions */
void setup_Wheel_Encoder(void);
void setup_TimerA1_1Mhz(void);
void wheel_Encoder_Right_IRQ(void);
void wheel_Encoder_Left_IRQ(void);
void wheel_Encoder_Timer_INT(void);
void wheelVelocity_Print(void);

float getRightWheelDistance(void);
float getLeftWheelDistance(void);

void wheel_velocity(void);


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

//    if (GPIO_getInputPinValue(WHEEL_ENCODER_RIGHT_PORT, WHEEL_ENCODER_RIGHT_PIN) == 0){      // If current input value is LOW
//        GPIO_interruptEdgeSelect(WHEEL_ENCODER_RIGHT_PORT, WHEEL_ENCODER_RIGHT_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
//    }

    //Enable interrupt from P2 and Enable all interrupts
    Interrupt_enableInterrupt(WHEEL_ENCODER_RIGHT_INTPORT);

    GPIO_setAsInputPinWithPullUpResistor(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN);   // Set input pin with pull_up_resistor

    GPIO_clearInterruptFlag(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN);     //Clear interrupt flag  and Enable  interrupt

    GPIO_enableInterrupt(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN);       // Enable interrupt

    GPIO_interruptEdgeSelect(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN, GPIO_HIGH_TO_LOW_TRANSITION);    // Set  Interrupt edge HIGH TO LOW

//    if (GPIO_getInputPinValue(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN) == 0){      // If current input value of P2 Pin5 is LOW
//        GPIO_interruptEdgeSelect(WHEEL_ENCODER_LEFT_PORT, WHEEL_ENCODER_LEFT_PIN, GPIO_LOW_TO_HIGH_TRANSITION);
//    }

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


void wheelVelocity_Print(){

     printf("\n Left wheel velocity %0.2f",  wheel_Left_Velocity );
     printf(" || Right wheel velocity:  %0.2f ",  wheel_Right_Velocity);
     printf(" || Distance LEFT : %f ", getLeftWheelDistance());
     printf(" || Distance RIGHT : %f ", getRightWheelDistance());


}





/* wheel_Encoder_Timer_INT() , executed in a Timer Interrupt handler
 * - Increments wheel_Right_Time_Counter by TICKPERIOD
 * - Increments wheel_Left_Time_Counter by TICKPERIOD
 * */

void wheel_Encoder_Timer_INT(void){
    if (record_right_encoder_time == YES){
        wheel_Right_Time_Counter +=TICKPERIOD;
    }
    if (record_left_encoder_time == YES){
        wheel_Left_Time_Counter +=TICKPERIOD;
    }
}

/*
 *  wheel_Encoder_Right_IRQ() is executed in interrupt handler of Right Wheel
 *  - Increments rightWheelCount (Number of left wheel notches completed )
 */
void wheel_Encoder_Right_IRQ(void){

    uint32_t status = GPIO_getEnabledInterruptStatus(WHEEL_ENCODER_RIGHT_PORT);
    GPIO_clearInterruptFlag(WHEEL_ENCODER_RIGHT_PORT, status);
    /* Toggling the output on the LED */
    if(status &  WHEEL_ENCODER_RIGHT_PIN)
    {
        rightWheelCount++;

    }
}

/*
 *  wheel_Encoder_Left_IRQ() is executed in interrupt handler of Left Wheel Interrupt
 *  - Increments leftWheel Count (Number of left wheel notches completed )
 */
void wheel_Encoder_Left_IRQ(void){
    uint32_t status = GPIO_getEnabledInterruptStatus(WHEEL_ENCODER_LEFT_PORT);
    GPIO_clearInterruptFlag(WHEEL_ENCODER_LEFT_PORT, status);
    /* Toggling the output on the LED */
    if(status &  WHEEL_ENCODER_LEFT_PIN)
    {
        leftWheelCount++;
    }
}

/*
 * caluclate_wheel_velocity() is executed in the main() and run continuously
 *
 * What it does:
 * - Calculates right wheel velocity with Exponential moving average
 * - Calculates left wheel velocity  with Exponential moving average
 *
 */
void caluclate_wheel_velocity(void){
    static uint32_t prev_left_wheel_count = 0;
    static uint32_t prev_right_wheel_count = 0;
    volatile uint32_t current_Time;
    volatile uint16_t diff;

    diff = leftWheelCount - prev_left_wheel_count;
    prev_left_wheel_count= leftWheelCount;
    if(diff){
        //current_Time = wheel_Left_Time_Counter;
        current_Time = wheel_Left_Time_Counter +  Timer_A_getCounterValue(TIMER_A1_BASE) - wheel_Left_Time_Start ;
        wheel_Left_Time_Start = Timer_A_getCounterValue(TIMER_A1_BASE);
        wheel_Left_Time_Counter = 1 ;

        if(record_left_encoder_time == YES){
            wheel_Left_Velocity = (diff *   WHEEL_CIRC_PER_NOTCH  / (current_Time * TIME_PER_CYCLE)) * MOVING_AVG_DELTA + (1.0 -MOVING_AVG_DELTA) * wheel_Left_Velocity  ;
        }
        else{
            record_left_encoder_time = YES;
        }
    }
    else{
        if (wheel_Left_Time_Counter > PAUSE_PERIOD){
            record_left_encoder_time = NO;
            wheel_Left_Time_Counter = 0 ;
            wheel_Left_Velocity = 0.0;

        }
    }

    diff = rightWheelCount - prev_right_wheel_count;
    prev_right_wheel_count = rightWheelCount;
    if(diff){
        //current_Time = wheel_Right_Time_Counter;
        current_Time = wheel_Right_Time_Counter + Timer_A_getCounterValue(TIMER_A1_BASE) - wheel_Right_Time_Start ;
        wheel_Right_Time_Start = Timer_A_getCounterValue(TIMER_A1_BASE);
        wheel_Right_Time_Counter = 1 ;

        if(record_right_encoder_time == YES){
            wheel_Right_Velocity = (diff *   WHEEL_CIRC_PER_NOTCH  / (current_Time * TIME_PER_CYCLE)) * MOVING_AVG_DELTA + (1.0 -MOVING_AVG_DELTA) * wheel_Right_Velocity;
        }
        else{
            record_right_encoder_time = YES;
        }
    }
    else{
        if (wheel_Right_Time_Counter > PAUSE_PERIOD){
            record_right_encoder_time = NO;
            wheel_Right_Time_Counter = 0;
            wheel_Right_Velocity = 0.0;

        }
    }


}
