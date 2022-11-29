#include <utils.h>

/**
 * Simple delay function using for loop.
 */
void delay(uint32_t loop)
{
    volatile uint32_t index;

    for (index = 0; index < loop; index++);
}

/**
 * Get the smaller number between two numbers.
 */
float min(float a, float b)
{
    return a < b ? a : b;
}

/**
 * Get the larger number between two numbers.
 */
float max(float a, float b)
{
    return a > b ? a : b;
}

/**
 * Set GPIO pin to high.
 */
void GPIO_High(uint32_t GPIO_Port, uint16_t pins)
{
    MAP_GPIO_setOutputHighOnPin(GPIO_Port, pins);
}

/**
 * Set GPIO pin to low.
 */
void GPIO_Low(uint32_t GPIO_Port, uint16_t pins)
{
    MAP_GPIO_setOutputLowOnPin(GPIO_Port, pins);
}

/**
 * Set GPIO pin as output.
 */
void GPIO_Output(uint32_t GPIO_Port, uint16_t pins)
{
    MAP_GPIO_setAsOutputPin(GPIO_Port, pins);
}

/**
 * Set GPIO pin as input with pull up resister.
 */
void GPIO_Pullup_Input(uint32_t GPIO_Port, uint16_t pins)
{
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_Port, pins);
    MAP_GPIO_clearInterruptFlag(GPIO_Port, pins);
    MAP_GPIO_enableInterrupt(GPIO_Port, pins);
}

/**
 * Initialize LED1
 */
void LED1_init(void)
{
    GPIO_Output(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_Low(GPIO_PORT_P1, GPIO_PIN0);
}

/**
 * Initialize LED2
 */
void LED2_init(void)
{
    GPIO_Output(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_Output(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_Output(GPIO_PORT_P2, GPIO_PIN2);

    GPIO_Low(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_Low(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_Low(GPIO_PORT_P2, GPIO_PIN2);
}

/**
 * Turn on RED LED for LED1.
 */
void LED1_RED_ON(void)
{
    GPIO_High(GPIO_PORT_P1, GPIO_PIN0);
}

/**
 * Turn off RED LED for LED1.
 */
void LED1_RED_OFF(void)
{
    GPIO_Low(GPIO_PORT_P1, GPIO_PIN0);
}

/**
 * Turn on RED LED For LED2.
 */
void LED2_RED_ON(void)
{
    GPIO_High(GPIO_PORT_P2, GPIO_PIN0);
}

/**
 * Turn off RED LED for LED2.
 */
void LED2_RED_OFF(void)
{
    GPIO_Low(GPIO_PORT_P2, GPIO_PIN0);
}

/**
 * Turn on GREEN LED for LED2.
 */
void LED2_GREEN_ON(void)
{
    GPIO_High(GPIO_PORT_P2, GPIO_PIN1);
}

/**
 * Turn off GREEN LED for LED2.
 */
void LED2_GREEN_OFF(void)
{
    GPIO_Low(GPIO_PORT_P2, GPIO_PIN1);
}

/**
 * Turn on BLUE LED for LED2.
 */
void LED2_BLUE_ON(void)
{
    GPIO_High(GPIO_PORT_P2, GPIO_PIN2);
}

/**
 * Turn off BLUE LED for LED2.
 */
void LED2_BLUE_OFF(void)
{
    GPIO_Low(GPIO_PORT_P2, GPIO_PIN2);
}
