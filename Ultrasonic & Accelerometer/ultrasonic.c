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

#define DEBUG_ULTRASONIC

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
// Thanks Terence for the tip about the weighting, might have forgotten it.
#define EMA_FILTER_WEIGHTAGE 0.20f
// Datasheet value (in cm). Values higher than this are erroneous.
#define HIGH_PASS_CUTOFF 400
// Datasheet value (in cm). Values lower than this are erroneous.
#define LOW_PASS_CUTOFF 2

// DEFS ----------------------------------------------------------------------------------------------------

#pragma endregion

struct FloatQueue
{
    int size;
    unsigned capacity;
    float *array;
};

typedef struct FloatQueue FloatQueue;

struct UltrasonicSensorConfig
{
    uint_fast8_t triggerPort;
    uint_fast16_t triggerPin;
    uint_fast8_t echoPort;
    uint_fast16_t echoPin;
    uint32_t timer;
    uint32_t *sensorInterruptCount;
    uint8_t bufferIndex; // Stores which buffer index to store data in
};

typedef struct UltrasonicSensorConfig UltrasonicSensorConfig;

// Can technically overflow but if it does this demo has gone on for too long
uint32_t sensor1InterruptCount;

// Front sensor
UltrasonicSensorConfig frontSensorConfig = {
    FRONT_ULTRASONIC_TRIGGER_PORT,
    FRONT_ULTRASONIC_TRIGGER_PIN,
    FRONT_ULTRASONIC_ECHO_PORT,
    FRONT_ULTRASONIC_ECHO_PIN,
    ULTRASONIC_TIMER_MODULE,
    &sensor1InterruptCount,
    ULTRASONIC_BUFFER_FRONT_INDEX};

// Right sensor
UltrasonicSensorConfig rightSensorConfig = {
    RIGHT_ULTRASONIC_TRIGGER_PORT,
    RIGHT_ULTRASONIC_TRIGGER_PIN,
    RIGHT_ULTRASONIC_ECHO_PORT,
    RIGHT_ULTRASONIC_ECHO_PIN,
    ULTRASONIC_TIMER_MODULE,
    &sensor1InterruptCount,
    ULTRASONIC_BUFFER_LEFT_INDEX};

// Back sensor
UltrasonicSensorConfig backSensorConfig = {
    BACK_ULTRASONIC_TRIGGER_PORT,
    BACK_ULTRASONIC_TRIGGER_PIN,
    BACK_ULTRASONIC_ECHO_PORT,
    BACK_ULTRASONIC_ECHO_PIN,
    ULTRASONIC_TIMER_MODULE,
    &sensor1InterruptCount,
    ULTRASONIC_BUFFER_RIGHT_INDEX};

// Left sensor
UltrasonicSensorConfig leftSensorConfig = {
    LEFT_ULTRASONIC_TRIGGER_PORT,
    LEFT_ULTRASONIC_TRIGGER_PIN,
    LEFT_ULTRASONIC_ECHO_PORT,
    LEFT_ULTRASONIC_ECHO_PIN,
    ULTRASONIC_TIMER_MODULE,
    &sensor1InterruptCount,
    ULTRASONIC_BUFFER_BACK_INDEX};

FloatQueue *smaQueue;
FloatQueue *emaQueue;
float previousEMA;
bool isFirstValue = true;

float latestSensorDistances[4]; // Stores the latest distances captured from sensor.

static bool delayFlag;

#pragma region QUEUE
// QUEUE METHODS ----------------------------------------------------------------------------------------------------
// Some lib that had helper methods for a queue. Butchered it for float.

// Create queue. Static size, NOT dynamic. Initial size = 0
struct FloatQueue *createQueue(unsigned capacity)
{
    struct FloatQueue *queue = (struct FloatQueue *)malloc(sizeof(struct FloatQueue));
    queue->capacity = capacity;
    queue->size = 0;

    // Static, queue is NOT dynamic.
    queue->array = (float *)malloc(queue->capacity * sizeof(float));

    // memset queue array values to 0
    memset(queue->array, 0, queue->capacity * sizeof(float));
    return queue;
}

// Queue is empty when size is 0
int isEmpty(struct FloatQueue *queue)
{
    return (queue->size == 0);
}

// Helper because queue reaches max capacity when size == capacity
int isFull(struct FloatQueue *queue)
{
    return (queue->size == queue->capacity);
}

float dequeue(struct FloatQueue *queue)
{
    // Queue empty, nothing to dequeue. Return FLT_MIN to signal that.
    if (isEmpty(queue))
        return FLT_MIN;

    // Get the dequeued element
    float dequeuedelement = queue->array[0];

    queue->size--;

    return dequeuedelement;
}

// Add an element to queue.
// It changes rear and size
void enqueue(struct FloatQueue *queue, float element)
{
    // Add element to queue
    // if its not full, add to index size
    if (!isFull(queue))
    {
        queue->array[queue->size] = element;
    }

    // If its full, dequeue first element, then shift elements by 1
    else
    {
        dequeue(queue);
        int i;
        for (i = 1; i < queue->capacity; i++)
        {
            queue->array[i - 1] = queue->array[i];
        }

        queue->array[queue->size] = element;
    }
    queue->size++;
}

float peek(struct FloatQueue *queue)
{
    if (isEmpty(queue))
        return FLT_MIN;
    return queue->array[0];
}

// Print queue
void printQueue(struct FloatQueue *queue)
{

    // print all in queue
    printf("[");
    int i;
    for (i = 0; i < queue->size; i++)
    {
        (i < queue->size - 1) ? printf("%.2f, ", queue->array[i]) : printf("%.2f", queue->array[i]);
    }
    printf("]\n");
}

#pragma endregion

void UtilityTime_initDelayTimer()
{
    // Init 32 Bit Timer
    Timer32_initModule(
        UTILITY_DELAY_TIMER,   // MCLK = 3Mhz
        UTILITY_DELAY_DIVIDER, // 3Mhz/1 = 1s
        TIMER32_32BIT,         // 32 Bit timer
        TIMER32_FREE_RUN_MODE  // Free run mode
    );

    Interrupt_disableMaster();
    // Enable interrupt
    Interrupt_enableInterrupt(UTILITY_DELAY_INTERRUPT);
    Interrupt_enableMaster();
}

static uint32_t getRegisterValueToSet(uint32_t delayMs)
{
    uint32_t timerFreq = CS_getMCLK() / UTILITY_DELAY_DIVIDER_INT;
    return (timerFreq / 1000) * delayMs;
}

void UtilityTime_delay(uint32_t delayMs)
{
    // Reset delay Flag
    delayFlag = false;

    // Get register value to set
    // 3000 Ticks in Timer = 1ms
    uint32_t registerValue = getRegisterValueToSet(delayMs);

    // Set the timer register value
    Timer32_setCount(UTILITY_DELAY_TIMER, registerValue);
    uint32_t regValue = Timer32_getValue(UTILITY_DELAY_TIMER);

    // Start the timer
    // It will interrupt when stopped
    Timer32_startTimer(UTILITY_DELAY_TIMER, true);

    // wait for interrupt to occur, block.
    while (!delayFlag)
        ;

    Timer32_haltTimer(UTILITY_DELAY_TIMER);

    // interrupt has occured, done!
    return;
}

#pragma region FILTERS

// FILTERS ----------------------------------------------------------------------------------------------------

// Simple Moving Average (SMA) filter
float SMAFilter(FloatQueue *queue)
{
    int i;
    float sum = 0;

    for (i = 0; i < queue->size; i++)
    {
        sum += queue->array[i];
    }
    // SMA = average
    return sum / queue->size;
}

// Exponential Moving Average (EMA) filter
float EMAFilter(FloatQueue *queue, float value)
{
    float SMAValue = SMAFilter(queue);

    // EMA queue must be full to calculate, return SMA avg instead.
    if (!isFull(queue))
    {
        return SMAValue;
    }

    // For first calculation of EMA,
    if (isFirstValue)
    {
        isFirstValue = false;
        previousEMA = SMAValue;
        return previousEMA;
    }

    // Calculate EMA values with full queue.
    // 80:20 weightage for previous value to current value in case of spikes in data.
    float emaValue = (1 - EMA_FILTER_WEIGHTAGE) * previousEMA + EMA_FILTER_WEIGHTAGE * value;

    // Set next previousEMA value
    previousEMA = emaValue;
    return emaValue;
}

bool HighPassFilter(float dist)
{
    return (dist > HIGH_PASS_CUTOFF);
}

bool LowPassFilter(float dist)
{
    return (dist < LOW_PASS_CUTOFF);
}

// FILTERS ----------------------------------------------------------------------------------------------------

#pragma endregion

#pragma region SENSOR_METHODS

// SENSOR METHODS ----------------------------------------------------------------------------------------------------

float Ultrasonic_getDistanceFromFrontSensor()
{
    return latestSensorDistances[ULTRASONIC_BUFFER_FRONT_INDEX];
}

float Ultrasonic_getDistanceFromLeftSensor()
{
    return latestSensorDistances[ULTRASONIC_BUFFER_LEFT_INDEX];
}

float Ultrasonic_getDistanceFromRightSensor()
{
    return latestSensorDistances[ULTRASONIC_BUFFER_RIGHT_INDEX];
}

float Ultrasonic_getDistanceFromBackSensor()
{
    return latestSensorDistances[ULTRASONIC_BUFFER_BACK_INDEX];
}

static void triggerUltrasonicSensor(UltrasonicSensorConfig *sensorConfig)
{

    GPIO_setOutputHighOnPin(sensorConfig->triggerPort, sensorConfig->triggerPin);

    // Set delay of 1ms to trigger ultrasonic sensor
    UtilityTime_delay(1);

    GPIO_setOutputLowOnPin(sensorConfig->triggerPort, sensorConfig->triggerPin);
}

static uint32_t getDuration(UltrasonicSensorConfig *sensorConfig)
{

    uint32_t pulseTime = 0;

    pulseTime = *(sensorConfig->sensorInterruptCount) * TIMER_A_TICKPERIOD;
    pulseTime += Timer_A_getCounterValue(sensorConfig->timer);

    // Clear Timer
    Timer_A_clearTimer(sensorConfig->timer);

    return pulseTime;
}

static float getDistance(UltrasonicSensorConfig *sensorConfig)
{
    uint32_t pulseDuration = 0;
    float distance = 0;

    triggerUltrasonicSensor(sensorConfig);

    // Wait for positive edge from sensor
    while (GPIO_getInputPinValue(sensorConfig->echoPort, sensorConfig->echoPin) == 0)
        ;

    // Restart global interrupt count for sensor
    *(sensorConfig->sensorInterruptCount) = 0;

    // Start timer.
    Timer_A_clearTimer(sensorConfig->timer);
    Timer_A_startCounter(sensorConfig->timer, TIMER_A_UP_MODE);

    // Detect Negative edge
    while (GPIO_getInputPinValue(sensorConfig->echoPort, sensorConfig->echoPin) == 1)
        ;

    // Stop timer.
    Timer_A_stopTimer(sensorConfig->timer);

    // Obtain Pulse Width in microseconds
    pulseDuration = getDuration(sensorConfig);

    // Calculate distance in terms of cm
    distance = (float)pulseDuration / 58.0f;

    return distance;
}

static bool checkSensorDetectObject(UltrasonicSensorConfig *sensorConfig)
{
    bool hasObject = false;

    // Reset sensor interrupt count
    *(sensorConfig->sensorInterruptCount) = 0;

    // Get distance from object.
    float raw_distance = getDistance(sensorConfig);

    // Filter data

    // Check with Profs...
    if (HighPassFilter(raw_distance) || LowPassFilter(raw_distance))
    {
        // flagP3 = 0;
        // PCM_gotoLPM3InterruptSafe();
        // continue;
    }

    // KALMAN FILTER
    //    filteredValue = Filter_KalmanFilter(distance, &KALMAN_DATA);
    //    printf("FilteredValue: %.2f\n", filteredValue);

    // SMA FILTER!
    // Add to queue
    enqueue(smaQueue, raw_distance);

    // Get the SMA
    float smaFilterVal = SMAFilter(smaQueue);
    printf("\nSMA: %.2f\n", smaFilterVal);

    // EMA FILTER!
    // Add to queue
    enqueue(emaQueue, raw_distance);

    // Get the EMA
    float emaFilterVal = EMAFilter(emaQueue, raw_distance);
    printf("EMA: %.2f\n", emaFilterVal);

    // Sets global distance buffer that stores latest distances captured from sensor.
    latestSensorDistances[sensorConfig->bufferIndex] = raw_distance;

    // If distance is lower then threshold, object is near.
    hasObject = raw_distance < ULTRASONIC_DETECT_OBJECT_THRESHOLD;

    printf("Distance: %.2fcm\n", raw_distance);

    return hasObject;
}

bool Ultrasonic_checkFront()
{
    printf("Front -> ");
    return checkSensorDetectObject(&frontSensorConfig);
}

bool Ultrasonic_checkRight()
{
    printf("Right -> ");
    return checkSensorDetectObject(&rightSensorConfig);
}

bool Ultrasonic_checkBack()
{
    printf("Back -> ");
    return checkSensorDetectObject(&backSensorConfig);
}

bool Ultrasonic_checkLeft()
{
    printf("Left -> ");
    return checkSensorDetectObject(&leftSensorConfig);
}

// SENSOR METHODS ----------------------------------------------------------------------------------------------------

#pragma endregion

// HANDLERS ----------------------------------------------------------------------------------------------------

void TA2_0_IRQHandler(void)
{
    // Increment global interrupt count
    sensor1InterruptCount++;

    /* Clear interrupt flag */
    Timer_A_clearCaptureCompareInterrupt(ULTRASONIC_TIMER_MODULE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

// Interrupt handler
void T32_0_IRQHandler(void)
{

    // Set delay flag to true, remove blocking mechanism
    delayFlag = true;

    // Clear interrupt flag...
    Timer32_clearInterruptFlag(UTILITY_DELAY_TIMER);
}

// HANDLERS ----------------------------------------------------------------------------------------------------

#pragma region INIT_METHODS

// INIT METHODS ----------------------------------------------------------------------------------------------------

static void initPortsAndPins()
{
    // Configure LED1 pin as output
    GPIO_setOutputLowOnPin(LED1_PORT, LED1_PIN);
    GPIO_setAsOutputPin(LED1_PORT, LED1_PIN);

    // Configure echo pins for sensors as input
    GPIO_setAsInputPinWithPullDownResistor(FRONT_ULTRASONIC_ECHO_PORT, FRONT_ULTRASONIC_ECHO_PIN);
    GPIO_setAsInputPinWithPullDownResistor(RIGHT_ULTRASONIC_ECHO_PORT, RIGHT_ULTRASONIC_ECHO_PIN);
    GPIO_setAsInputPinWithPullDownResistor(BACK_ULTRASONIC_ECHO_PORT, BACK_ULTRASONIC_ECHO_PIN);
    GPIO_setAsInputPinWithPullDownResistor(LEFT_ULTRASONIC_ECHO_PORT, LEFT_ULTRASONIC_ECHO_PIN);

    // Configure trigger pins for sensors as output
    GPIO_setOutputLowOnPin(FRONT_ULTRASONIC_TRIGGER_PORT, FRONT_ULTRASONIC_TRIGGER_PIN);
    GPIO_setAsOutputPin(FRONT_ULTRASONIC_TRIGGER_PORT, FRONT_ULTRASONIC_TRIGGER_PIN);

    GPIO_setOutputLowOnPin(RIGHT_ULTRASONIC_TRIGGER_PORT, RIGHT_ULTRASONIC_TRIGGER_PIN);
    GPIO_setAsOutputPin(RIGHT_ULTRASONIC_TRIGGER_PORT, RIGHT_ULTRASONIC_TRIGGER_PIN);

    GPIO_setOutputLowOnPin(BACK_ULTRASONIC_TRIGGER_PORT, BACK_ULTRASONIC_TRIGGER_PIN);
    GPIO_setAsOutputPin(BACK_ULTRASONIC_TRIGGER_PORT, BACK_ULTRASONIC_TRIGGER_PIN);

    GPIO_setOutputLowOnPin(LEFT_ULTRASONIC_TRIGGER_PORT, LEFT_ULTRASONIC_TRIGGER_PIN);
    GPIO_setAsOutputPin(LEFT_ULTRASONIC_TRIGGER_PORT, LEFT_ULTRASONIC_TRIGGER_PIN);
}

static void initInterrupts()
{
    // Disable interrupts globally
    Interrupt_disableMaster();

    // Enable interrupt for INT_TA2_0
    Interrupt_enableInterrupt(ULTRASONIC_TIMER_INT);

    // Enable interrupts globally
    Interrupt_enableMaster();
}

static void initTimers()
{
    // Halt watchdog timer so that the microcontroller does not restart.
    WDT_A_holdTimer();

    // Config INT_TA2_0 for interrupt counting. 1MHz clock. Interrupts are every 1ms.
    const Timer_A_UpModeConfig upConfig =
        {
            TIMER_A_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_3,      // SMCLK/3 = 1MHz
            TIMER_A_TICKPERIOD,                 // 1000 tick period.
            TIMER_A_TAIE_INTERRUPT_DISABLE,     // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                    // Clear value
        };

    Timer_A_configureUpMode(ULTRASONIC_TIMER_MODULE, &upConfig);
    Timer_A_clearTimer(ULTRASONIC_TIMER_MODULE);
}

static void initFilters()
{
    // SMA Queue
    smaQueue = createQueue(SMA_PERIOD);
    // EMA Queue
    emaQueue = createQueue(EMA_PERIOD);
}

void Ultrasonic_init()
{
    // Initialize all Ports and Pins
    initPortsAndPins();

    // // No configs needed for I2C, amen
    // initI2CConfigs();

    // Initialize all Timers
    initTimers();

    // Initialize interrupt settings
    initInterrupts();

    // Initialize queues for ultrasonic filters
    initFilters();
}

#pragma endregion

int main(void)
{
    /* Halting WDT */
    WDT_A_holdTimer();

    UtilityTime_initDelayTimer();
    Ultrasonic_init();

    printf("Ultrasonic sensors initialised.\n");

    while (1)
    {
        Ultrasonic_checkLeft();

        printf("back dist: %.f\n", Ultrasonic_getDistanceFromBackSensor());

        PCM_gotoLPM3InterruptSafe();
        // PCM_gotoLPM3();
    }
}
