#include "accelerometer.h"

// ACCEL GLOBAL VARIABLES ----------------------------------------------------------------------------------------------------

// static float hump_angle;
bool found_hump = false;

float hump_horizontal_dist;
// Height of last hump/depth of last ditch? passed
float hump_height;
// Cache greatest angle measured when moving over hump here/moving into a bowl.
float hump_max_angle = 0.0f;

// ACCEL GLOBAL VARIABLES ----------------------------------------------------------------------------------------------------

// I2C R/W METHODS ----------------------------------------------------------------------------------------------------

// Bashed together from an incomplete library, three broken Github projects in various languages,
// and one very confusing YouTube video featuring a kind Indian teacher.

// For reading the Accelerometer data
int16_t I2C_read_int16(int8_t readAddress)
{
    int value = 0;
    int value_scratch = 0;

    // Transmit the slave register address that we wish to read from (transmit mode)
    // Set master to transmit mode PL
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    // Clear any existing interrupt flag PL
    I2C_clearInterruptFlag(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    // Wait until ready to write
    while (I2C_isBusBusy(I2C_MODULE))
        ;

    // Initiate start and send address to read from
    I2C_masterSendMultiByteStart(I2C_MODULE, readAddress);

    // Wait for TX to finish
    while (!(I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0)))
        ;

    // Initiate stop only
    I2C_masterSendMultiByteStop(I2C_MODULE);

    // Wait for Stop to finish
    while (!I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_STOP_INTERRUPT))
        ;

    I2C_masterReceiveStart(I2C_MODULE);

    // Wait for RX buffer to fill
    while (!(I2C_getInterruptStatus(I2C_MODULE, EUSCI_B_I2C_RECEIVE_INTERRUPT0)))
        ;

    // Read from I2C RX register
    value = I2C_masterReceiveMultiByteNext(I2C_MODULE);

    // Receive second byte then send STOP condition
    value_scratch = I2C_masterReceiveMultiByteFinish(I2C_MODULE);

    // Shift value to top MSB
    value = (value << 8);

    // Read from I2C RX Register and write to LSB of value
    value |= value_scratch;

    // Return value
    return (int16_t)value;
}

// For resetting the MPU6050
void I2C_write_int8(int8_t writeAddress, int8_t writeByte)
{
    // Set master to transmit mode PL
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    // Clear any existing interrupt flag PL
    I2C_clearInterruptFlag(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    // Wait until ready to write PL
    while (I2C_isBusBusy(I2C_MODULE))
        ;

    // Initiate start and send first character
    I2C_masterSendMultiByteStart(I2C_MODULE, writeAddress);

    // Send the MSB to SENSOR
    I2C_masterSendMultiByteNext(I2C_MODULE, (unsigned char)(writeByte >> 8));
    I2C_masterSendMultiByteFinish(I2C_MODULE, (unsigned char)(writeByte & 0xFF));
}

// For resetting the MPU6050
void I2C_write_int16(int8_t writeAddress, int16_t writeBytes)
{
    // Set master to transmit mode PL
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    // Clear any existing interrupt flag PL
    I2C_clearInterruptFlag(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    // Wait until ready to write PL
    while (I2C_isBusBusy(I2C_MODULE))
        ;

    // Initiate start and send first character
    I2C_masterSendMultiByteStart(I2C_MODULE, writeAddress);

    // Send the MSB to SENSOR
    I2C_masterSendMultiByteNext(I2C_MODULE, (unsigned char)(writeBytes >> 8));
    I2C_masterSendMultiByteFinish(I2C_MODULE, (unsigned char)(writeBytes & 0xFF));
}

// I2C R/W METHODS ----------------------------------------------------------------------------------------------------

// MPU6050 METHODS ----------------------------------------------------------------------------------------------------

// LSB Sensitivity: 16384
static float readAccelX()
{
    // Read 16-bit measurement from 0x3B (ACCEL_XOUT_H, ACCEL_XOUT_L)
    // Divide by 16384, see register map datasheet page 29
    int16_t readValue = I2C_read_int16(ACCEL_OUT_ADDRESS);
    return (float)readValue / 16384;
}

static float readAccelY()
{
    // Read 16-bit measurement from 0x3D (ACCEL_YOUT_H, ACCEL_YOUT_L)
    // Offset ACCEL_X_OUT_H by 2
    // Divide by 16384, see register map datasheet page 29
    int16_t readValue = I2C_read_int16(ACCEL_OUT_ADDRESS + 2);
    return (float)readValue / 16384;
}

static float readAccelZ()
{
    // Read 16-bit measurement from 0x3F (ACCEL_ZOUT_H, ACCEL_ZOUT_L)
    // Offset ACCEL_X_OUT_H by 4
    // Divide by 16384, see register map datasheet page 29
    int16_t readValue = I2C_read_int16(ACCEL_OUT_ADDRESS + 4);
    return (float)readValue / 16384;
}

// LSB Sensitivity: 131.0
static float readGyroX()
{
    // Read 16-bit measurement from 0x43 (GYRO_XOUT_H, GYRO_XOUT_L)
    // Divide by 131.0, see register map datasheet page 31
    int16_t readValue = I2C_read_int16(GYRO_OUT_ADDRESS);
    return (float)readValue / 131.0;
}

static float readGyroY()
{
    // Read 16-bit measurement from 0x45 (GYRO_YOUT_H, GYRO_YOUT_L)
    // Offset GYRO_XOUT_H by 2
    // Divide by 131.0, see register map datasheet page 31
    int16_t readValue = I2C_read_int16(GYRO_OUT_ADDRESS + 2);
    return (float)readValue / 131.0;
}

static float readGyroZ()
{
    // Read 16-bit measurement from 0x47 (GYRO_ZOUT_H, GYRO_ZOUT_L)
    // Offset GYRO_XOUT_H by 4
    // Divide by 131.0, see register map datasheet page 31
    int16_t readValue = I2C_read_int16(GYRO_OUT_ADDRESS + 4);
    return (float)readValue / 131.0;
}

// From register map, page 30:
// The temperature in degrees C for a given register value may be computed as:
// Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity) / 340 + 36.53
static float readTemp()
{
    // Read 16-bit measurement from 0x41 (TEMP_OUT_H, TEMP_OUT_L)
    int16_t readValue = I2C_read_int16(TEMP_OUT_ADDRESS);

    return (readValue / 340.0) + 36.53;
}

// From register map, page 41:
// When using SPI interface, user should use DEVICE_RESET (register 107) as well as
// SIGNAL_PATH_RESET (register 104) to ensure the reset is performed properly. The sequence
// used should be:
// 1. Set DEVICE_RESET = 1 (register PWR_MGMT_1)
// 2. Wait 100ms
// 3. Set GYRO_RESET = ACCEL_RESET = TEMP_RESET = 1 (register SIGNAL_PATH_RESET)
// 4. Wait 100ms
// The reset value is 0x00 for all registers other than the registers below.
// Register 107 (PWR_MGMT_1): 0x40.
// Register 117 (WHO_AM_I): 0x68.
// Don't think it's necessary to do steps 2, 3 and 4 - extra steps to be careful. Doing it anyways.
// Use UtilityTime_delay from ultrasonic sensor side to delay for 100ms (uncomment the rest when the team has integrated)
void resetMPU6050()
{
    // Set DEVICE_RESET (PWR_MGMT_1 register) to 1. Resets the gyroscope analog and digital signal paths.
    I2C_write_int8(RESET_ADDRESS, 0x40);

    // Can't use this until integrated with ultrasonic sensor (and thus rest of team)
    // Uncomment when ready
    // // Extra steps if you want to be safe!
    // // Set delay of 100ms
    // UtilityTime_delay(100);
    // // Set register to 1. Resets the gyroscope analog and digital signal paths.
    // I2C_write_int16(GYRO_OUT_ADDRESS, 0x00);
    // // Set register to 1. Resets the accelerometer analog and digital signal paths.
    // I2C_write_int16(ACCEL_OUT_ADDRESS, 0x00);
    // // Set register to 1. Resets the temperature analog and digital signal paths.
    // I2C_write_int16(TEMP_OUT_ADDRESS, 0x00);
    // // Set delay of 100ms
    // UtilityTime_delay(100);

    printf("MPU6050 reset complete.\n");
}

// Get angle of car(if going over hump, etc.)
float getAngle()
{
    // Get values from MPU6050 using I2C
    // float accX = readAccelX();
    // float accY = readAccelY();
    // float accZ = readAccelZ();

    // Condensed formula for getting angle
    float angle = (atan(-1 * readAccelX() / sqrt(pow(readAccelY(), 2) + pow(readAccelZ(), 2))) * 180 / M_PI);

    printf("getAngle: %f\n", angle);

    return angle;
}

// Works when going forwards/backwards over humps when the car is going over it
// and the accelerometer is mounted such that the Y-axis is aligned with the forward/backward motion of the car.
// Unsure what happens if only one wheel goes over a hump - probably just an inaccurate reading.
// reading may not be accurate due to inbalance of height across the car, and the relative accelerometer positioning.
// Technically, I think this would also work to measure the steepest part of a decline into a bowl/ditch/hole because I don't check for upwards acceleration
void calculateHumpAngle()
{
    // Calculate the current angle of the hump
    // Absolute the value so we can account for going backwards over humps, or for if the car reverses direction.
    float humpAngle = fabsf(getAngle());

    if (found_hump == false)
    {
        // Hump detected, cache distance from start of hump to later compute hump horizontal length, then later the hump height
        if (humpAngle >= HUMP_ANGLE_THRESHOLD)
        {
            // printf("Hump detected!\n");

            found_hump = true;

            // Start measuring distance
            hump_horizontal_dist = Ultrasonic_getDistanceFromFrontSensor();
        }
    }
    // Detected hump
    else
    {
        if (humpAngle > hump_max_angle)
        {
            hump_max_angle = humpAngle;
        }
        // hump_max_angle = (hump_angle > hump_max_angle) ? hump_angle : hump_max_angle;
        // Exit condition for found_hump
        // Detect hump end when car becomes horizontal - angle is supposed to be 0.00,
        //  Detection may be inaccurate due to floor or sensor error drift.
        if (humpAngle < HUMP_ANGLE_THRESHOLD)
        {
            // printf("Hump finished!\n");
            found_hump = false;

            // End of hump, calculate and cache the hump height
            hump_height = calculateHumpHeight();
        }
    }
}

// Calculate and return the hump height.
float calculateHumpHeight()
{
    float height = 0;

    // Halve the value as to get distance from start of hump to maximum height of hump, instead of end of hump.
    // fabsf in case we are reversing over it.
    hump_horizontal_dist = fabsf((Ultrasonic_getDistanceFromFrontSensor() - hump_horizontal_dist)) * 0.5f;

    height = hump_horizontal_dist * tan(hump_max_angle * M_PI / 180);

    // printf("Hump width * 0.5: %.2f\n", hump_horizontal_dist);
    // printf("Hump max_angle: %.2f\n", hump_max_angle);
    // printf("Hump height: %.2f\n", height);

    hump_max_angle = 0.0f;

    return height;
}

// MPU6050 METHODS ----------------------------------------------------------------------------------------------------

// INIT METHODS ----------------------------------------------------------------------------------------------------

static void initGPIO()
{
    // Set up I2C Pins
    // Set up SDA PIN
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        ACCEL_SDA_PORT,
        ACCEL_SDA_PIN,
        GPIO_PRIMARY_MODULE_FUNCTION);

    // Set up SCL PIN
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        ACCEL_SCL_PORT,
        ACCEL_SCL_PIN,
        GPIO_PRIMARY_MODULE_FUNCTION);
}

static void initInterrupts()
{
    // Enable interrupts globally
    Interrupt_disableMaster();

    // Clear the interrupt flags for I2C
    I2C_clearInterruptFlag(
        I2C_MODULE,
        EUSCI_B_I2C_TRANSMIT_INTERRUPT0 |
            EUSCI_B_I2C_RECEIVE_INTERRUPT0);

    //  Check if this line is needed
    // I2C_enableInterrupt(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_INTERRUPT1);
    Interrupt_enableInterrupt(I2C_MODULE_INTERRUPT_INT);

    // Enable interrupts for TIMER_A1_0
    Interrupt_enableInterrupt(TIMER_MODULE_INTERRUPT_INT);

    // Enable interrupts globally
    Interrupt_enableMaster();
}

static void initTimers()
{
    // Halt watchdog timer so that the microcontroller does not restart.
    WDT_A_holdTimer();

    // Enable timer needed for roll and pitch measurement
    const Timer_A_UpModeConfig upConfig =
        {
            TIMER_A_CLOCKSOURCE_ACLK,            // ACLK 32768Hz Clock Source
            TIMER_DIVIDER,                       // ACLK/10 = 3276.8 Hz
            TIMER_TICK_PERIOD,                   // 32768, so 0.1s interrupt
            TIMER_A_TAIE_INTERRUPT_DISABLE,      // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE, // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                     // Clear value
        };

    // Configure and reset timer
    Timer_A_configureUpMode(TIMER_MODULE, &upConfig);
    Timer_A_clearTimer(TIMER_MODULE);
}

static void configI2C(void)
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
    // Set Master in transmit mode
    I2C_setMode(I2C_MODULE, EUSCI_B_I2C_TRANSMIT_MODE);

    // Enable I2C Module to start operations
    I2C_enableModule(I2C_MODULE);

    return;
}

void initAccelerometer()
{
    // Init GPIO Pins required for MPU6050
    initGPIO();

    // Init I2C Configurations required for MPU6050 communication
    configI2C();

    // Init Timers
    initTimers();

    // Enable interrupts for I2C Protocol and Timers
    initInterrupts();

    // Reset MPU6050 on program start
    resetMPU6050();

    printf("Accelerometer initialised.\n");
}

// INIT METHODS ----------------------------------------------------------------------------------------------------

// // Main - testing only. Demonstrates how to use the methods to get accelerometer data
// void main(void)
// {
//     WDT_A_holdTimer();

//     initDelayTimer();

//     initUltrasonicSensors();

//     initAccelerometer();

//     printf("While loop for debugging and demonstration.\n");

//     while (1)
//     {
//         // Show we can read values

//         // printf("AccelX: %.2f\n", accX);
//         // printf("AccelY: %.2f\n", accY);
//         // printf("AccelZ: %.2f\n", accZ);
//         // printf("GyroX: %.2f\n", readGyroX());
//         // printf("GyroY: %.2f\n", readGyroY());
//         // printf("GyroZ: %.2f\n", readGyroZ());
//         // printf("Temp: %.2f\n", readTemp());

//         // if (UltrasonicDetectFront())
//         // {
//         //     // Do something
//         //     float temp = Ultrasonic_getDistanceFromFrontSensor();

//         //     printf("Temp: %.2f\n", temp);
//         // }

//         // Get angle of hump
//         calculateHumpAngle();

//         printf("hump_height: %.2f\n", hump_height);

//         // PCM_gotoLPM3InterruptSafe();
//     }
// }
