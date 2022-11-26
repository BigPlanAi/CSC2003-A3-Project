#include <driverlib.h>

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <ultrasonic.h>

// ACCEL INIT DEFS ----------------------------------------------------------------------------------------------------
// I2C Module used
#define I2C_MODULE EUSCI_B1_BASE

// Timers
#define TIMER_MODULE TIMER_A1_BASE
#define TIMER_TICK_PERIOD 32768
#define TIMER_DIVIDER TIMER_A_CLOCKSOURCE_DIVIDER_10

// GPIO PINS used for I2C Protocol
// SDA 6.4
#define ACCEL_SDA_PORT GPIO_PORT_P6
#define ACCEL_SDA_PIN GPIO_PIN4

// SCL 6.5
#define ACCEL_SCL_PORT GPIO_PORT_P6
#define ACCEL_SCL_PIN GPIO_PIN5

// Slave Address for I2C Slave MPU6050
#define I2C_SLAVE_ADDRESS 0x68

// ACCEL INIT DEFS ----------------------------------------------------------------------------------------------------

// ACCEL DEFS ----------------------------------------------------------------------------------------------------

// Addresses on MPU6050
#define RESET_ADDRESS 0x6B
#define ACCEL_OUT_ADDRESS 0x3B
#define GYRO_OUT_ADDRESS 0x43
#define TEMP_OUT_ADDRESS 0x41

// How many times to calibrate
#define RECALIBRATION_COUNT 200

// Threshold to detect hump
#define HUMP_PITCH_ANGLE_THRESHOLD 5.0
#define NEG_HUMP_PITCH_ANGLE_THRESHOLD -5.0

//  ACCEL DEFS ----------------------------------------------------------------------------------------------------

// ACCEL GLOBAL VARIABLES ----------------------------------------------------------------------------------------------------

static float accel_error_x;
static float accel_error_y;

static float gyro_error_x;
static float gyro_error_y;
static float gyro_error_z;

static float hump_angle;
// static bool found_hump;

// ACCEL GLOBAL VARIABLES ----------------------------------------------------------------------------------------------------

bool found_hump = false;

// I2C R/W METHODS ----------------------------------------------------------------------------------------------------

int8_t I2C_read_int8(int8_t readAddress)
{
    int value = 0;

    // Transmit the slave register address that we wish to read from (transmit mode)
    /* Set master to transmit mode PL */
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    /* Clear any existing interrupt flag PL */
    I2C_clearInterruptFlag(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    /* Wait until ready to write */
    while (I2C_isBusBusy(I2C_MODULE))
        ;

    /* Initiate start and send address to read from...  */
    I2C_masterSendMultiByteStart(I2C_MODULE, readAddress);

    /* Wait for TX to finish */
    while (!(I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0)))
        ;

    /* Initiate stop only */
    I2C_masterSendMultiByteStop(I2C_MODULE);

    /* Wait for Stop to finish */
    while (!I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_STOP_INTERRUPT))
        ;

    // Stop the transmit of the slave register address that we wish to read from
    // Start reading from the slave register (receive mode)

    /*
     * Generate Start condition and set it to receive mode.
     * This sends out the slave address and continues to read
     * until you issue a STOP
     */
    I2C_masterReceiveStart(I2C_MODULE);

    /* Wait for RX buffer to fill */
    while (!(I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_RECEIVE_INTERRUPT0)))
        ;

    /* Read from I2C RX register */
    /* Receive byte then send STOP condition */
    value = I2C_masterReceiveMultiByteFinish(I2C_MODULE);

    /* Return value */
    return (int8_t)value;
}

int16_t I2C_read_int16(int8_t readAddress)
{
    int value = 0;
    int value_scratch = 0;

    // Transmit the slave register address that we wish to read from (transmit mode)
    /* Set master to transmit mode PL */
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    /* Clear any existing interrupt flag PL */
    I2C_clearInterruptFlag(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    /* Wait until ready to write */
    while (I2C_isBusBusy(I2C_MODULE))
        ;

    /* Initiate start and send address to read from...  */
    I2C_masterSendMultiByteStart(I2C_MODULE, readAddress);

    /* Wait for TX to finish */
    while (!(I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0)))
        ;

    /* Initiate stop only */
    I2C_masterSendMultiByteStop(I2C_MODULE);

    /* Wait for Stop to finish */
    while (!I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_STOP_INTERRUPT))
        ;

    // Stop the transmit of the slave register address that we wish to read from
    // Start reading from the slave register (receive mode)

    /*
     * Generate Start condition and set it to receive mode.
     * This sends out the slave address and continues to read
     * until you issue a STOP
     */
    I2C_masterReceiveStart(I2C_MODULE);

    /* Wait for RX buffer to fill */
    while (!(I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_RECEIVE_INTERRUPT0)))
        ;

    /* Read from I2C RX register */
    value = I2C_masterReceiveMultiByteNext(I2C_MODULE);

    /* Receive second byte then send STOP condition */
    value_scratch = I2C_masterReceiveMultiByteFinish(I2C_MODULE);

    /* Shift value to top MSB */
    value = (value << 8);

    /* Read from I2C RX Register and write to LSB of value */
    value |= value_scratch;

    /* Return value */
    return (int16_t)value;
}

void I2C_write_int8(int8_t writeAddress, int8_t writeByte)
{
    /* Set master to transmit mode PL */
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    /* Clear any existing interrupt flag PL */
    I2C_clearInterruptFlag(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    /* Wait until ready to write PL */
    while (I2C_isBusBusy(I2C_MODULE))
        ;

    /* Initiate start and send first character */
    I2C_masterSendMultiByteStart(I2C_MODULE, writeAddress);

    /* Send the MSB to SENSOR */
    I2C_masterSendMultiByteNext(I2C_MODULE, (unsigned char)(writeByte >> 8));
    I2C_masterSendMultiByteFinish(I2C_MODULE, (unsigned char)(writeByte & 0xFF));

    printf("reset MPU6050!\n");
}

void I2C_write_int16(int8_t writeAddress, int16_t writeBytes)
{
    /* Set master to transmit mode PL */
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    /* Clear any existing interrupt flag PL */
    I2C_clearInterruptFlag(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    /* Wait until ready to write PL */
    while (I2C_isBusBusy(I2C_MODULE))
        ;

    /* Initiate start and send first character */
    I2C_masterSendMultiByteStart(I2C_MODULE, writeAddress);

    /* Send the MSB to SENSOR */
    I2C_masterSendMultiByteNext(I2C_MODULE, (unsigned char)(writeBytes >> 8));
    I2C_masterSendMultiByteFinish(I2C_MODULE, (unsigned char)(writeBytes & 0xFF));

    printf("reset MPU6050!\n");
}

// I2C R/W METHODS ----------------------------------------------------------------------------------------------------

// MPU6050 METHODS ----------------------------------------------------------------------------------------------------

// LSB Sensitivity: 16384
static float readAccelX()
{
    // Read 16 bits from 0x3B (ACCEL_XOUT_H, ACCEL_XOUT_L)
    // Divide by 16384 according to datasheet
    int16_t readValue = I2C_read_int16(ACCEL_OUT_ADDRESS);
    return (float)readValue / 16384;
}

static float readAccelY()
{
    // Read 16 bits from 0x3D (ACCEL_YOUT_H, ACCEL_YOUT_L)
    // Offset ACCEL_X_OUT_H by 2
    // Divide by 16384 according to datasheet
    int16_t readValue = I2C_read_int16(ACCEL_OUT_ADDRESS + 2);
    return (float)readValue / 16384;
}

static float readAccelZ()
{
    // Read 16 bits from 0x3F (ACCEL_ZOUT_H, ACCEL_ZOUT_L)
    // Offset ACCEL_X_OUT_H by 4
    // Divide by 16384 according to datasheet
    int16_t readValue = I2C_read_int16(ACCEL_OUT_ADDRESS + 4);
    return (float)readValue / 16384;
}

// LSB Sensitivity: 131.0
static float readGyroX()
{
    // Read 16 bits from 0x43 (GYRO_XOUT_H, GYRO_XOUT_L)
    // Divide by 131.0 according to datasheet
    int16_t readValue = I2C_read_int16(GYRO_OUT_ADDRESS);
    return (float)readValue / 131.0;
}

static float readGyroY()
{
    // Read 16 bits from 0x45 (GYRO_YOUT_H, GYRO_YOUT_L)
    // Offset GYRO_XOUT_H by 2
    // Divide by 131.0 according to datasheet
    int16_t readValue = I2C_read_int16(GYRO_OUT_ADDRESS + 2);
    return (float)readValue / 131.0;
}

static float readGyroZ()
{
    // Read 16 bits from 0x47 (GYRO_ZOUT_H, GYRO_ZOUT_L)
    // Offset GYRO_XOUT_H by 4
    // Divide by 131.0 according to datasheet
    int16_t readValue = I2C_read_int16(GYRO_OUT_ADDRESS + 4);
    return (float)readValue / 131.0;
}

// From register map:
// The temperature in degrees C for a given register value may be computed as:
// Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity) / 340 + 36.53
static float readTemp()
{
    // Read 16 bits from 0x41 (TEMP_OUT_H, TEMP_OUT_L)
    int16_t readValue = I2C_read_int16(TEMP_OUT_ADDRESS);

    return (readValue / 340.0) + 36.53;
}

void recalibrateMPU6050()
{
    // We can call this function in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
    // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
    // Read accelerometer values RECALIBRATION_COUNT times

    // Delay variable
    volatile uint32_t ii;

    // Loop variable
    int16_t i;

    // Sum of all accel values
    float accValueXSum = 0;
    float accValueYSum = 0;

    // Sum of all gyro values
    float gyroValueXSum = 0;
    float gyroValueYSum = 0;
    float gyroValueZSum = 0;

    for (i = 0; i < RECALIBRATION_COUNT; i++)
    {

        // Delay between Transmissions...
        for (ii = 0; ii < 500; ii++)
            ;

        // Read values from MPU6050
        float accValueX = readAccelX();
        float accValueY = readAccelY();
        float accValueZ = readAccelZ();

        float gyroValueX = readGyroX();
        float gyroValueY = readGyroY();
        float gyroValueZ = readGyroZ();

        // Sum all accel values
        accValueXSum += ((atan((accValueY) / sqrt(pow((accValueX), 2) + pow((accValueZ), 2))) * 180 / M_PI));
        accValueYSum += ((atan(-1 * (accValueX) / sqrt(pow((accValueY), 2) + pow((accValueZ), 2))) * 180 / M_PI));

        // Sum all gyro values
        gyroValueXSum += gyroValueX;
        gyroValueYSum += gyroValueY;
        gyroValueZSum += gyroValueZ;

        printf("count: %d/%d\n", i, RECALIBRATION_COUNT);
    }

    // Calculate the errors
    accel_error_x = accValueXSum / RECALIBRATION_COUNT;
    accel_error_y = accValueYSum / RECALIBRATION_COUNT;

    gyro_error_x = gyroValueXSum / RECALIBRATION_COUNT;
    gyro_error_y = gyroValueYSum / RECALIBRATION_COUNT;
    gyro_error_z = gyroValueZSum / RECALIBRATION_COUNT;

    printf("accel_error_x: %.2f\n", accel_error_x);
    printf("accel_error_y: %.2f\n", accel_error_y);

    printf("gyro_errorX: %.2f\n", gyro_error_x);
    printf("gyro_error_y: %.2f\n", gyro_error_y);
    printf("gyro_error_z: %.2f\n", gyro_error_z);
}

void resetMPU6050()
{
    // Write 0x00 to 0x6B on MPU6050 PWR_MGMT_1
    I2C_write_int8(RESET_ADDRESS, 0x00);
}

void PORT1_IRQHandler(void)
{
    uint32_t pinActivated;
    printf("button interrupt\n");
    pinActivated = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, pinActivated);
}

// Get number of seconds...
float getTimerDuration()
{

    int16_t totalTimerTicks;

    // Add the remaining value in TIMER_MODULE's register
    totalTimerTicks = Timer_A_getCounterValue(TIMER_MODULE);

    // get time elapsed In seconds
    return totalTimerTicks * (1 / (CS_getACLK() / (TIMER_DIVIDER) / 1.0));
}

float getPitchAngle()
{

    // Get values from MPU6050 using I2C
    float accX = readAccelX();
    float accY = readAccelY();
    float accZ = readAccelZ();
    //    printf("accel Value \t-> X: %.2f, Y: %.2f, Z: %.2f\n", accX, accY, accZ);

    // Formula to get accAngleX and accAngleY
    float accAngleX = (atan(accY / sqrt(pow(accX, 2) + pow(accZ, 2))) * 180 / M_PI) - accel_error_x;
    float accAngleY = (atan(-1 * accX / sqrt(pow(accY, 2) + pow(accZ, 2))) * 180 / M_PI) - accel_error_y;

    //    printf("accel Angles \t-> X: %.2f, Y: %.2f\n", accAngleX, accAngleY);

    // Stop the timer and get total elapsed time...
    Timer_A_stopTimer(TIMER_MODULE);
    float durationInSeconds = getTimerDuration();

    // Get Gyro values, and use calibrated error values to calculate accurate gyro values
    float gyroX = readGyroX() - gyro_error_x;
    float gyroY = readGyroY() - gyro_error_y;
    float gyroZ = readGyroZ() - gyro_error_z;

    //    printf("gyro Value \t-> X: %.2f, Y: %.2f, Z: %.2f\n", gyroX, gyroY, gyroZ);

    // *0.04 due to gyroscope error drifting
    float gyroAngleY = gyroY * durationInSeconds;
    float pitch = (0.96 * gyroAngleY) + (0.04 * accAngleY);

    // Restart the timer
    Timer_A_clearTimer(TIMER_MODULE);
    Timer_A_startCounter(TIMER_MODULE, TIMER_A_UP_MODE);

    return pitch;

}

float getAngle(){
    int accAngleY =0;

    // Get values from MPU6050 using I2C
        float accX = readAccelX();
        float accY = readAccelY();
        float accZ = readAccelZ();

    float accAngleZ = (atan(-1 * accX / sqrt(pow(accY, 2) + pow(accZ, 2))) * 180 / M_PI) - accel_error_x;
    accAngleZ = accAngleZ + accAngleY;
    accAngleY = accAngleZ;

    return accAngleY;

}

//float getHeight(){
//    float height = 0;
//    //float distance;
//
//    height = Ultrasonic_getDistanceFromFrontSensor() * tan(getAngle()* PI/180);
//
//    return height;
//}

void calculateHumpAngle()
{

    // Calculate the current pitch angle...
    float pitchAngle = getAngle();

    printf("pitch \t\t-> %.2f deg\n", pitchAngle);

    // pitch angle has detected a hump
    if (pitchAngle > HUMP_PITCH_ANGLE_THRESHOLD)
    {
        printf("Hump detected!\n");

        found_hump = true;

        // Start measuring distance...
        float Ultrasonic_getDistanceFromFrontSensor();


    }

    // Detect hump end when car becomes horizontal - angle is supposed to be 0.00//
    if (pitchAngle == 0.00)
    {

        printf("Hump finish!\n");
        found_hump = false;

        // Get distance
        // Perform calculations here...
        float height = 0;
        float distance;

        height = Ultrasonic_getDistanceFromFrontSensor() * tan(getAngle()* PI/180);
        //
        return height;

        printf("Hump height: %.2f\n", height);
        // Send hump to comms / mapping
    }

    // Detected hump... add angle
    else if (found_hump)
    {
        hump_angle += pitchAngle;
    }
}

// MPU6050 METHODS ----------------------------------------------------------------------------------------------------

// INIT METHODS ----------------------------------------------------------------------------------------------------

static void initPortsAndPins()
{
    // Set up I2C Pins
    // Set up SDA PIN...
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        ACCEL_SDA_PORT,
        ACCEL_SDA_PIN,
        GPIO_PRIMARY_MODULE_FUNCTION);

    // Set up SCL PIN...
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        ACCEL_SCL_PORT,
        ACCEL_SCL_PIN,
        GPIO_PRIMARY_MODULE_FUNCTION);

    // --------------------------------------------
    // Set up button pin for testing
    // Set Button 1 as pull up resistor
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

static void initInterrupts()
{
    // Enable interrupts globally...
    Interrupt_disableMaster();

    // Clear the interrupt flags for I2C
    I2C_clearInterruptFlag(
        I2C_MODULE,
        EUSCI_B_I2C_TRANSMIT_INTERRUPT0 |
            EUSCI_B_I2C_RECEIVE_INTERRUPT0);

    // DEBUG --------------------------------------------
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    Interrupt_enableInterrupt(INT_PORT1);
    // --------------------------------------------

    // I added
    I2C_enableInterrupt(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT1);
    Interrupt_enableInterrupt(INT_EUSCIB1);

    // Enable interrupts for TIMER_A1
    Interrupt_enableInterrupt(INT_TA1_0);

    // Enable interrupts globally...
    Interrupt_enableMaster();
}

static void initTimers()
{
    // Halt watchdog timer so that the microcontroller does not restart.
    WDT_A_holdTimer();

    // Enable timer needed for roll and pitch measurement (1MHz clock)
    const Timer_A_UpModeConfig upConfig =
        {
            TIMER_A_CLOCKSOURCE_ACLK,            // ACLK 32768Hz Clock Source
            TIMER_DIVIDER,                       // ACLK/10 = 3276.8 Hz
            TIMER_TICK_PERIOD,                   // 10 sec interrupt
            TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE, // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                     // Clear value
        };

    // Configure and reset timer...
    Timer_A_configureUpMode(TIMER_MODULE, &upConfig);
    Timer_A_clearTimer(TIMER_MODULE);
}

static void initI2CConfigs(void)
{
    // I2C Config
    const eUSCI_I2C_MasterConfig i2cConfig =
        {
            EUSCI_B_I2C_CLOCKSOURCE_SMCLK,     // SMCLK Clock Source
            3000000,                           // SMCLK = 3MHz
            EUSCI_B_I2C_SET_DATA_RATE_400KBPS, // Desired I2C Clock of 400khz
            0,                                 // No byte counter threshold
            EUSCI_B_I2C_NO_AUTO_STOP           // No Autostop
        };

    // Init I2C Master Block for specified I2C_MODULE
    I2C_initMaster(I2C_MODULE, &i2cConfig);

    // Specify slave address
    I2C_setSlaveAddress(I2C_MODULE, I2C_SLAVE_ADDRESS);

    // check if work ty
    /* Set Master in transmit mode */
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    // reset I2C module
    // I2C_disableModule(I2C_MODULE);

    /* Enable I2C Module to start operations */
    I2C_enableModule(I2C_MODULE);

    return;
}

void Accelerometer_init()
{
    // Init GPIO Pins required for MPU6050
    initPortsAndPins();

    // Init I2C Configurations required for MPU6050 communication
    initI2CConfigs();

    // Init Timers
    initTimers();

    // Enable interrupts for I2C Protocol and Timers
    initInterrupts();

    // Reset MPU6050 on program start
    resetMPU6050();
}

// INIT METHODS ----------------------------------------------------------------------------------------------------

// Main
void main(void)
{
    Accelerometer_init();

    //    recalibrateMPU6050();

    printf("While loop!\n");
    while (1)
    {
        // Can read
        printf("AccelX: %f\n", readAccelX());
        printf("AccelY: %f\n", readAccelY());
        printf("AccelZ: %f\n", readAccelZ());
        printf("GyroX: %04.2f\n", readGyroX());
        printf("GyroY: %04.2f\n", readGyroY());
        printf("GyroZ: %04.2f\n", readGyroZ());
        printf("Temp: %04.2f\n", readTemp());

        // Get angle properly
//        printf("getPitchAngle: %f\n", getPitchAngle());
        printf("getAngle: %f\n", getAngle());
//        calculateHumpAngle();
        //printf("getHeight: %f\n", getHeight());

        // accX = readTemp();
        // printf("AccelX: %04.2f\n", readAccelX());

        //        accX = readAccelX();
        //        accY = readAccelY();
        //        accZ = readAccelZ();
        //        sprintf(print_buffer, "AccelX: %04.2f, AccelY: %04.2f, AccelZ: %04.2f \n\r",
        //                accX, accY, accZ);
        //        uPrintf(print_buffer);

        // PCM_gotoLPM3InterruptSafe();
    }
}
