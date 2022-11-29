#include "driverlib.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "ultrasonic.h"

// ACCEL INIT DEFS ----------------------------------------------------------------------------------------------------
// I2C Module used
#define I2C_MODULE EUSCI_B1_BASE
#define I2C_MODULE_INTERRUPT_INT INT_EUSCIB1

// Timers
#define TIMER_MODULE TIMER_A1_BASE
#define TIMER_MODULE_INTERRUPT_INT INT_TA1_0
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

// Addresses on MPU6050, see register map datasheet for details.
#define RESET_ADDRESS 0x6B
#define ACCEL_OUT_ADDRESS 0x3B
#define GYRO_OUT_ADDRESS 0x43
#define TEMP_OUT_ADDRESS 0x41

// Threshold to detect hump start and end, needs testing post-integration
#define HUMP_ANGLE_THRESHOLD 5.0f

//  ACCEL DEFS ----------------------------------------------------------------------------------------------------

// I2C R/W METHODS ----------------------------------------------------------------------------------------------------

// Bashed together from an incomplete library, three broken Github projects in various languages,
// and one very confusing YouTube video featuring a kind Indian teacher.

// For reading the Accelerometer data
int16_t I2C_read_int16(int8_t readAddress);
// For resetting the MPU6050
void I2C_write_int8(int8_t writeAddress, int8_t writeByte);
// For resetting the MPU6050
void I2C_write_int16(int8_t writeAddress, int16_t writeBytes);

// I2C R/W METHODS ----------------------------------------------------------------------------------------------------

// MPU6050 METHODS ----------------------------------------------------------------------------------------------------

// LSB Sensitivity: 16384
static float readAccelX();
static float readAccelY();
static float readAccelZ();

// LSB Sensitivity: 131.0
static float readGyroX();
static float readGyroY();
static float readGyroZ();

// From register map, page 30:
// The temperature in degrees C for a given register value may be computed as:
// Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity) / 340 + 36.53
static float readTemp();

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
void resetMPU6050();

// Get angle of car(if going over hump, etc.)
float getAngle();
// Works when going forwards/backwards over humps when the car is going over it
// and the accelerometer is mounted such that the Y-axis is aligned with the forward/backward motion of the car.
// Unsure what happens if only one wheel goes over a hump - probably just an inaccurate reading.
// reading may not be accurate due to inbalance of height across the car, and the relative accelerometer positioning.
// Technically, I think this would also work to measure the steepest part of a decline into a bowl/ditch/hole because I don't check for upwards acceleration
void calculateHumpAngle();
// Calculate and return the hump height.
float calculateHumpHeight();

// MPU6050 METHODS ----------------------------------------------------------------------------------------------------

// INIT METHODS ----------------------------------------------------------------------------------------------------

static void initGPIO();
static void initInterrupts();
static void initTimers();
static void configI2C(void);
void initAccelerometer();

// INIT METHODS ----------------------------------------------------------------------------------------------------
