/* DriverLib Includes */
#include <driverlib.h>

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <stdio.h>
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


//
//extern void straight_PID(void);
//
extern Timer_A_PWMConfig pwmConfig;
extern Timer_A_PWMConfig pwmConfig2;
//extern volatile char motor_state;

/* Wheel Encoder Global variables */

extern volatile float wheel_Right_Velocity;
extern volatile float wheel_Left_Velocity;
extern volatile uint32_t leftWheelCount;
extern volatile uint32_t rightWheelCount;

/* Wheel Encoder Function*/
extern void setup_Wheel_Encoder(void);
extern void setup_TimerA1_1Mhz(void);
extern void wheel_Encoder_Right_IRQ(void);
extern void wheel_Encoder_Left_IRQ(void);
extern void wheel_Encoder_Timer_INT(void);
extern void wheelVelocity_Print(void);

extern float getRightWheelDistance(void);
extern float getLeftWheelDistance(void);

extern void caluclate_wheel_velocity(void);


/* Barcode section */

#define ARR_SIZE 10
#define BLACK '1'
#define WHITE '0'
#define WIDE '1'
#define SLIM '0'
extern void scan_barBuffer();
extern void setup_barcode(void);
extern void barcode_ADC14_IRQ(void);
extern void barcode_TIMERA1_IRQ(void);
extern void set_color_threshold(void );
//extern void prototype_barcode_ADC14_IRQ(void);
struct characterBuffer {
    char buffer[ARR_SIZE];
    short int rear_index;
};

extern struct characterBuffer charBuff;
extern struct characterBuffer reverse_charBuff;
