/* DriverLib Includes */
#include <driverlib.h>

/* Standard Includes */
#include <stdint.h>

#include <stdbool.h>
#include <stdio.h>
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

extern Timer_A_PWMConfig pwmConfig;
extern Timer_A_PWMConfig pwmConfig2;


/* Wheel Encoder Global variables */
extern volatile uint32_t wheel_Right_Time_Counter;
extern volatile uint32_t wheel_Right_Time_Start;
extern volatile float wheel_Right_Velocity;
extern volatile uint32_t wheel_Left_Time_Counter;
extern volatile uint32_t wheel_Left_Time_Start;
extern volatile float wheel_Left_Velocity;
extern const float TIME_PER_CYCLE;
extern const float WHEEL_CIRC_PER_NOTCH;

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

