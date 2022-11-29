/******************************************************************************
 * MSP432 Project CarStep
 *
 * Description: A robotic car project that uses DriverLib
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *******************************************************************************/
/* DriverLib Includes */
#include "driverlib.h"

#include <uart.h>
#include <http.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

const eUSCI_UART_Config UART0Config =
    {
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,
        13,
        0,
        37,
        EUSCI_A_UART_NO_PARITY,
        EUSCI_A_UART_LSB_FIRST,
        EUSCI_A_UART_ONE_STOP_BIT,
        EUSCI_A_UART_MODE,
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION};

const eUSCI_UART_Config UART2Config =
    {
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,
        13,
        0,
        37,
        EUSCI_A_UART_NO_PARITY,
        EUSCI_A_UART_LSB_FIRST,
        EUSCI_A_UART_ONE_STOP_BIT,
        EUSCI_A_UART_MODE,
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION};

int main(void)
{
    // Stop watchdog
    MAP_WDT_A_holdTimer();

    // Set clock source
    cs_init();

    // Init leds
    LED1_init();
    LED2_init();


    // UART A0 for MSPIO
    uart_init(EUSCI_A0_BASE, UART0Config);

    // UART A2 for ESP8266
    uart_init(EUSCI_A2_BASE, UART2Config);
    GPIO_setAsOutputPin(GPIO_PORT_P6,GPIO_PIN1);
    // Initialize sensors
    http_init();   // WIFI

    // Enable interrupts
    MAP_Interrupt_enableMaster();

    while (1)
    {
        http_request();
    }
}
