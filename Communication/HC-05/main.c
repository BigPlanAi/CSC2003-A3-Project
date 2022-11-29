#include <Hardware/CS_Driver.h>
#include <Devices/MSPIO.h>

#define BUFFER_SIZE    128

/*Data Buffer*/
char Buffer[BUFFER_SIZE];

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
eUSCI_UART_Config UART0Config =
{
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     13,
     0,
     37,
     EUSCI_A_UART_NO_PARITY,
     EUSCI_A_UART_LSB_FIRST,
     EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE,
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 9600 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
eUSCI_UART_Config UART2Config =
{
     EUSCI_A_UART_CLOCKSOURCE_SMCLK,
     156,
     4,
     0,
     EUSCI_A_UART_NO_PARITY,
     EUSCI_A_UART_LSB_FIRST,
     EUSCI_A_UART_ONE_STOP_BIT,
     EUSCI_A_UART_MODE,
     EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

void main(void)
{
    MAP_WDT_A_holdTimer();

    /*MSP432 running at 24 MHz*/
    CS_Init();

    /*Initialize Hardware required for the HC-05*/
    UART_Init(EUSCI_A0_BASE, UART0Config);
    UART_Init(EUSCI_A2_BASE, UART2Config);

    MAP_Interrupt_enableMaster();

    while(1)
    {
        /*Get data from MSP432 PC UART*/
        MSPrintf(EUSCI_A0_BASE, "Enter data to be sent to HC-05: ");
        MSPgets(EUSCI_A0_BASE, Buffer, BUFFER_SIZE);
        MSPrintf(EUSCI_A0_BASE, "\r\n");

        /*Send data to HC-05*/
        MSPrintf(EUSCI_A2_BASE, Buffer);

        /*Get data from HC-05*/
        MSPrintf(EUSCI_A0_BASE, "Waiting for HC-05 response...\r\n");
        MSPgets(EUSCI_A2_BASE, Buffer, BUFFER_SIZE);

        /*Send data to serial terminal*/
        MSPrintf(EUSCI_A0_BASE, "Data received from HC-05: %s\r\n", Buffer);
    }
	
}
