/* DriverLib Includes */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <float.h>

#pragma region DEFINITIONS

// DEFINES ----------------------------------------------------------------------------------------------------

#define UTILITY_DELAY_TIMER TIMER32_0_BASE
#define UTILITY_DELAY_INTERRUPT INT_T32_INT1
#define UTILITY_DELAY_DIVIDER TIMER32_PRESCALER_1
#define UTILITY_DELAY_DIVIDER_INT 1

// Pins used
#define LED1_PORT GPIO_PORT_P1
#define LED1_PIN GPIO_PIN0

// Trigger: P2.3, Echo: P1.7
#define FRONT_ULTRASONIC_TRIGGER_PORT GPIO_PORT_P2
#define FRONT_ULTRASONIC_TRIGGER_PIN GPIO_PIN3
#define FRONT_ULTRASONIC_ECHO_PORT GPIO_PORT_P1
#define FRONT_ULTRASONIC_ECHO_PIN GPIO_PIN7

// Trigger: P5.1, Echo: P5.0
#define RIGHT_ULTRASONIC_TRIGGER_PORT GPIO_PORT_P5
#define RIGHT_ULTRASONIC_TRIGGER_PIN GPIO_PIN1
#define RIGHT_ULTRASONIC_ECHO_PORT GPIO_PORT_P5
#define RIGHT_ULTRASONIC_ECHO_PIN GPIO_PIN0

// Trigger: P3.5, Echo: P5.2
#define BACK_ULTRASONIC_TRIGGER_PORT GPIO_PORT_P3
#define BACK_ULTRASONIC_TRIGGER_PIN GPIO_PIN5
#define BACK_ULTRASONIC_ECHO_PORT GPIO_PORT_P5
#define BACK_ULTRASONIC_ECHO_PIN GPIO_PIN2

// Trigger: P3.7, Echo: P3.6
#define LEFT_ULTRASONIC_TRIGGER_PORT GPIO_PORT_P3
#define LEFT_ULTRASONIC_TRIGGER_PIN GPIO_PIN6
#define LEFT_ULTRASONIC_ECHO_PORT GPIO_PORT_P3
#define LEFT_ULTRASONIC_ECHO_PIN GPIO_PIN7

// Timers
#define ULTRASONIC_TIMER_MODULE TIMER_A2_BASE
#define ULTRASONIC_TIMER_INT INT_TA2_0
#define TIMER_A_TICKPERIOD 1000

#define ULTRASONIC_BUFFER_FRONT_INDEX 0
#define ULTRASONIC_BUFFER_LEFT_INDEX 1
#define ULTRASONIC_BUFFER_RIGHT_INDEX 2
#define ULTRASONIC_BUFFER_BACK_INDEX 3

// Threshold (in cm). Too large? Check after integration.
#define ULTRASONIC_DETECT_OBJECT_THRESHOLD 30.0f

#define SMA_PERIOD 20
#define EMA_PERIOD 20
// Weighting value in favour of previous EMAs.
// Test diff values if time permits
#define EMA_FILTER_WEIGHTAGE 0.20f
// Datasheet value (in cm). Values higher than this are erroneous.
#define HIGH_PASS_CUTOFF 400.0f
// Datasheet value (in cm). Values lower than this are erroneous.
#define LOW_PASS_CUTOFF 2.0f

// DEFS ----------------------------------------------------------------------------------------------------

#pragma endregion

#pragma region STRUCTS

// Float queue structure from Y1 module work + references online
struct FloatQueue
{
    int size;
    unsigned capacity;
    float *array;
};
typedef struct FloatQueue FloatQueue;

// Because you wanted to accommodate multiple sensors.
struct UltrasonicSensorConfig
{
    uint_fast8_t triggerPort;
    uint_fast16_t triggerPin;
    uint_fast8_t echoPort;
    uint_fast16_t echoPin;
    uint32_t timer;
    uint32_t *sensorInterruptCount;
    // Index for buffer arr indexing to cache data
    uint8_t bufferIndex;
};
typedef struct UltrasonicSensorConfig UltrasonicSensorConfig;

#pragma endregion

#pragma region QUEUE

// QUEUE METHODS ----------------------------------------------------------------------------------------------------

// Some lib that had helper methods for a queue. Butchered it for float.

// Create queue. Static size, NOT dynamic. Initial size = 0
struct FloatQueue *createQueue(unsigned capacity);
// Queue is empty when size is 0
int isEmpty(struct FloatQueue *queue);
// Helper because queue reaches max capacity when size == capacity
int isFull(struct FloatQueue *queue);
float dequeue(struct FloatQueue *queue);
void enqueue(struct FloatQueue *queue, float element);

#pragma endregion

#pragma region TIMERDELAY

// Init T32_0 to be a free run, 32-bit delay timer.
// Divider set to 1 so we can feed in ticks
void initDelayTimer();
// Pass ms delay as argument to convert to ticks
uint32_t converMSDelayForRegisterValue(uint32_t msDelay);
// Handy timer for ms delays, use for ultrasonic sensor pulses
void utilityTimeDelay(uint32_t msDelay);

#pragma endregion

#pragma region FILTERS

// FILTERS ----------------------------------------------------------------------------------------------------

// Simple Moving Average (SMA); filter
float SMAFilter(FloatQueue *quue);
// Exponential Moving Average (EMA); filter
float EMAFilter(FloatQueue *queue, float value);
bool HighPassFilter(float dist);
bool LowPassFilter(float dist);
// FILTERS ----------------------------------------------------------------------------------------------------

#pragma endregion

#pragma region SENSOR_METHODS

// SENSOR METHODS ----------------------------------------------------------------------------------------------------

float Ultrasonic_getDistanceFromFrontSensor();
float Ultrasonic_getDistanceFromLeftSensor();
float Ultrasonic_getDistanceFromRightSensor();
float Ultrasonic_getDistanceFromBackSensor();

// Use to start ranging for sensor, use 1ms delay
void triggerUltrasonicSensor(UltrasonicSensorConfig *sensorConfig);
uint32_t getDuration(UltrasonicSensorConfig *sensorConfig);
float getDistance(UltrasonicSensorConfig *sensorConfig);
bool detectedObject(UltrasonicSensorConfig *sensorConfig);

bool UltrasonicDetectFront();
bool UltrasonicDetectRight();
bool UltrasonicDetectBack();
bool UltrasonicDetectLeft();

// SENSOR METHODS ----------------------------------------------------------------------------------------------------

#pragma endregion

#pragma region HANDLERS

// HANDLERS ----------------------------------------------------------------------------------------------------

void TA2_0_IRQHandler(void);
void T32_0_IRQHandler(void);

// HANDLERS ----------------------------------------------------------------------------------------------------

#pragma endregion

#pragma region INIT_METHODS

// INIT METHODS ----------------------------------------------------------------------------------------------------

void initGPIO();
void initInterrupts();
void initTimers();
void initFilterQueues();

void initUltrasonicSensors();

#pragma endregion
