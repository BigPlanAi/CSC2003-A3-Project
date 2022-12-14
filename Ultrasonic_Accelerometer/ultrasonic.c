#include "ultrasonic.h"

bool isFirstCalc = true;

// Cache of latest distances from the sensors.
// Order: front, right, back, left
float latestSensorDist[4];

bool flagUtilTimerDelay;

// Can technically overflow but if it does this demo has gone on for too long
uint32_t sensorInterruptCountVar;

// Queue for Simple Moving Average filter to cache values for computation
FloatQueue *smaQueue;
// Queue for Exponential Moving Average filter to cache values for computation
FloatQueue *emaQueue;
// Last EMA value
float previousEMA;

// Configurations for ultrasonic sensors
// Front sensor
UltrasonicSensorConfig frontSensorConfig = {
    FRONT_ULTRASONIC_TRIGGER_PORT,
    FRONT_ULTRASONIC_TRIGGER_PIN,
    FRONT_ULTRASONIC_ECHO_PORT,
    FRONT_ULTRASONIC_ECHO_PIN,
    ULTRASONIC_TIMER_MODULE,
    &sensorInterruptCountVar,
    ULTRASONIC_BUFFER_FRONT_INDEX};

// Right sensor
UltrasonicSensorConfig rightSensorConfig = {
    RIGHT_ULTRASONIC_TRIGGER_PORT,
    RIGHT_ULTRASONIC_TRIGGER_PIN,
    RIGHT_ULTRASONIC_ECHO_PORT,
    RIGHT_ULTRASONIC_ECHO_PIN,
    ULTRASONIC_TIMER_MODULE,
    &sensorInterruptCountVar,
    ULTRASONIC_BUFFER_LEFT_INDEX};

// Back sensor
UltrasonicSensorConfig backSensorConfig = {
    BACK_ULTRASONIC_TRIGGER_PORT,
    BACK_ULTRASONIC_TRIGGER_PIN,
    BACK_ULTRASONIC_ECHO_PORT,
    BACK_ULTRASONIC_ECHO_PIN,
    ULTRASONIC_TIMER_MODULE,
    &sensorInterruptCountVar,
    ULTRASONIC_BUFFER_RIGHT_INDEX};

// Left sensor
UltrasonicSensorConfig leftSensorConfig = {
    LEFT_ULTRASONIC_TRIGGER_PORT,
    LEFT_ULTRASONIC_TRIGGER_PIN,
    LEFT_ULTRASONIC_ECHO_PORT,
    LEFT_ULTRASONIC_ECHO_PIN,
    ULTRASONIC_TIMER_MODULE,
    &sensorInterruptCountVar,
    ULTRASONIC_BUFFER_BACK_INDEX};

#pragma region QUEUE
// QUEUE METHODS ----------------------------------------------------------------------------------------------------
// Some lib that had helper methods for a queue. Butchered it for float.

// Create queue. static size, NOT dynamic. Initial size = 0
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

#pragma endregion

// Init T32_0 to be a free run, 32-bit delay timer.
// Divider set to 1 so we can feed in ticks
void initDelayTimer()
{
    // Init 32 Bit Timer
    Timer32_initModule(
        UTILITY_DELAY_TIMER,   // MCLK = 3Mhz
        UTILITY_DELAY_DIVIDER, // 3Mhz / 1 = 1s
        TIMER32_32BIT,         // 32 Bit timer
        TIMER32_FREE_RUN_MODE  // Free run mode
    );

    Interrupt_disableMaster();

    // Enable interrupt for INT_T32_INT1
    Interrupt_enableInterrupt(UTILITY_DELAY_INTERRUPT);

    Interrupt_enableMaster();
}

// Pass ms delay as argument to convert to ticks
uint32_t converMSDelayForRegisterValue(uint32_t msDelay)
{
    uint32_t timerFreq = CS_getMCLK() / UTILITY_DELAY_DIVIDER_INT;
    return (timerFreq / 1000) * msDelay;
}

// Handy timer for ms delays, use for ultrasonic sensor pulses
void utilityTimeDelay(uint32_t msDelay)
{
    // Reset delay flag
    flagUtilTimerDelay = false;

    // Get register value to set
    // 3000 Ticks in Timer = 1ms
    // 32-bit because we set T32_0 to 32-bit.
    uint32_t registerValue = converMSDelayForRegisterValue(msDelay);

    // Set the timer register value
    Timer32_setCount(UTILITY_DELAY_TIMER, registerValue);
    // uint32_t checkValue = Timer32_getValue(UTILITY_DELAY_TIMER);

    // Start the timer. It will interrupt when stopped
    Timer32_startTimer(UTILITY_DELAY_TIMER, true);

    // This is the delay. Wait for interrupt to occur, block until it is complete.
    while (!flagUtilTimerDelay)
        ;

    // Stop timer
    Timer32_haltTimer(UTILITY_DELAY_TIMER);

    // Delay complete
    // return;
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
    if (isFirstCalc)
    {
        isFirstCalc = false;
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
    return (dist < HIGH_PASS_CUTOFF);
}

bool LowPassFilter(float dist)
{
    return (dist > LOW_PASS_CUTOFF);
}

// FILTERS ----------------------------------------------------------------------------------------------------

#pragma endregion

#pragma region SENSOR_METHODS

// SENSOR METHODS ----------------------------------------------------------------------------------------------------

float Ultrasonic_getDistanceFromFrontSensor()
{
    return latestSensorDist[ULTRASONIC_BUFFER_FRONT_INDEX];
}

float Ultrasonic_getDistanceFromLeftSensor()
{
    return latestSensorDist[ULTRASONIC_BUFFER_LEFT_INDEX];
}

float Ultrasonic_getDistanceFromRightSensor()
{
    return latestSensorDist[ULTRASONIC_BUFFER_RIGHT_INDEX];
}

float Ultrasonic_getDistanceFromBackSensor()
{
    return latestSensorDist[ULTRASONIC_BUFFER_BACK_INDEX];
}

// You only need to supply a short 10uS pulse to the trigger input to start the ranging
// Use to start ranging for sensor, use 1ms delay
void triggerUltrasonicSensor(UltrasonicSensorConfig *sensorConfig)
{

    GPIO_setOutputHighOnPin(sensorConfig->triggerPort, sensorConfig->triggerPin);

    // Set delay of 1ms to trigger input to start the ranging
    utilityTimeDelay(1);

    GPIO_setOutputLowOnPin(sensorConfig->triggerPort, sensorConfig->triggerPin);
}

uint32_t getDuration(UltrasonicSensorConfig *sensorConfig)
{

    uint32_t pulseTime = 0;

    pulseTime = *(sensorConfig->sensorInterruptCount) * TIMER_A_TICKPERIOD;
    pulseTime += Timer_A_getCounterValue(sensorConfig->timer);

    // Clear Timer
    Timer_A_clearTimer(sensorConfig->timer);

    return pulseTime;
}

float getDistance(UltrasonicSensorConfig *sensorConfig)
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

bool detectedObject(UltrasonicSensorConfig *sensorConfig)
{
    bool objectDetected = false;

    // Reset sensor interrupt count
    *(sensorConfig->sensorInterruptCount) = 0;

    // Get distance from object.
    float raw_distance = getDistance(sensorConfig);

    printf("Raw distance: %.2fcm\n", raw_distance);
    // printf("%.2f\n", raw_distance);

    // Filter data. High pass and low pass clears out false readings.
    if (HighPassFilter(raw_distance) && LowPassFilter(raw_distance))
    {
        // KALMAN FILTER example
        // float filteredValue = Filter_KalmanFilter(raw_distance, &KALMAN_DATA);
        // printf("FilteredValue: %.2f\n", filteredValue);

        // SMA filter
        enqueue(smaQueue, raw_distance);
        float smaFilterVal = SMAFilter(smaQueue);

        printf("SMA: %.2f\n", smaFilterVal);
        // printf("%.2f\n", smaFilterVal);

        // EMA filter
        enqueue(emaQueue, raw_distance);
        float emaFilterVal = EMAFilter(emaQueue, raw_distance);

        printf("EMA: %.2f\n", emaFilterVal);
        // printf("%.2f\n", emaFilterVal);

        // Cache value for retrieval by other mods.
        latestSensorDist[sensorConfig->bufferIndex] = emaFilterVal;

        // If distance is lower then threshold, object is near.
        objectDetected = raw_distance < ULTRASONIC_DETECT_OBJECT_THRESHOLD;
    }
    else
    {
        printf("Invalid SMA measurement obtained!\n");
        printf("Invalid EMA measurement obtained!\n");
    }
    return objectDetected;
}

bool UltrasonicDetectFront()
{
    printf("Front sensor: \n");
    return detectedObject(&frontSensorConfig);
}

bool UltrasonicDetectRight()
{
    printf("Right sensor: \n");
    return detectedObject(&rightSensorConfig);
}

bool UltrasonicDetectBack()
{
    printf("Back sensor: \n");
    return detectedObject(&backSensorConfig);
}

bool UltrasonicDetectLeft()
{
    printf("Left sensor: \n");
    return detectedObject(&leftSensorConfig);
}

// SENSOR METHODS ----------------------------------------------------------------------------------------------------

#pragma endregion

// HANDLERS ----------------------------------------------------------------------------------------------------

void TA3_0_IRQHandler(void)
{
    // Increment global interrupt count
    sensorInterruptCountVar++;

    /* Clear interrupt flag */
    Timer_A_clearCaptureCompareInterrupt(ULTRASONIC_TIMER_MODULE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

// Interrupt handler
void T32_0_IRQHandler(void)
{
    // Set delay flag to true, remove blocking mechanism
    flagUtilTimerDelay = true;

    // Clear interrupt flag
    Timer32_clearInterruptFlag(UTILITY_DELAY_TIMER);
}

// HANDLERS ----------------------------------------------------------------------------------------------------

#pragma region INIT_METHODS

// INIT METHODS ----------------------------------------------------------------------------------------------------

void initGPIO()
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

void initInterrupts()
{
    // Disable interrupts globally
    Interrupt_disableMaster();

    // Enable interrupt for INT_TA3_0
    Interrupt_enableInterrupt(ULTRASONIC_TIMER_INT);

    // Enable interrupts globally
    Interrupt_enableMaster();
}

void initTimers()
{
    // Halt watchdog timer so that the microcontroller does not restart.
    WDT_A_holdTimer();

    // Config INT_TA3_0 for interrupt counting. 1MHz clock. Interrupts are every 1ms.
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

void initFilterQueues()
{
    // SMA Queue
    smaQueue = createQueue(SMA_PERIOD);
    // EMA Queue
    emaQueue = createQueue(EMA_PERIOD);
}

void initUltrasonicSensors()
{
    // Initialise all Ports and Pins
    initGPIO();

    // Initialise all Timers
    initTimers();

    // Initialise interrupt settings
    initInterrupts();

    // Initialise ultrasonic filters queues
    initFilterQueues();

    printf("Ultrasonic sensors initialised.\n");
}

#pragma endregion

// // Main method for debugging and demonstrating ultrasonic sensor functionality.
// int main(void)
// {
//     /* Halting WDT */
//     WDT_A_holdTimer();

//     initDelayTimer();

//     initUltrasonicSensors();

//     printf("Ultrasonic sensors initialised.\n");

//     while (1)
//     {
//         // Use UltrasonicDetect<side> to get distance cached into the buffer,
//         // then read data using Ultrasonic_getDistanceFrom<Side>Sensor()
//         if (UltrasonicDetectFront())
//         {
//             // Do something
//             printf("Front distance: %.2fcm\n", Ultrasonic_getDistanceFromFrontSensor());
//         }
//         // UltrasonicDetectRight();
//         // UltrasonicDetectBack();
//         // UltrasonicDetectLeft();
//     }
// }
