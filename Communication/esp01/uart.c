
#include <uart.h>

// UARTA0 Ring Buffer Global Variables
volatile uint8_t UARTA0Data[UARTA0_BUFFERSIZE];
volatile uint32_t UARTA0ReadIndex;
volatile uint32_t UARTA0WriteIndex;

#define UARTA0_ADVANCE_READ_INDEX                                \
    MAP_Interrupt_disableMaster();                               \
    UARTA0ReadIndex = (UARTA0ReadIndex + 1) % UARTA0_BUFFERSIZE; \
    MAP_Interrupt_enableMaster();
#define UARTA0_ADVANCE_WRITE_INDEX UARTA0WriteIndex = (UARTA0WriteIndex + 1) % UARTA0_BUFFERSIZE
#define UARTA0_BUFFER_EMPTY UARTA0ReadIndex == UARTA0WriteIndex ? true : false
#define UARTA0_BUFFER_FULL (UARTA0WriteIndex + 1) % UARTA0_BUFFERSIZE == UARTA0ReadIndex ? true : false

// UARTA2 Ring Buffer Global Variables
volatile uint8_t UARTA2Data[UARTA2_BUFFERSIZE];
volatile uint32_t UARTA2ReadIndex;
volatile uint32_t UARTA2WriteIndex;

#define UARTA2_ADVANCE_READ_INDEX                                \
    MAP_Interrupt_disableMaster();                               \
    UARTA2ReadIndex = (UARTA2ReadIndex + 1) % UARTA2_BUFFERSIZE; \
    MAP_Interrupt_enableMaster();
#define UARTA2_ADVANCE_WRITE_INDEX UARTA2WriteIndex = (UARTA2WriteIndex + 1) % UARTA2_BUFFERSIZE
#define UARTA2_BUFFER_EMPTY UARTA2ReadIndex == UARTA2WriteIndex ? true : false
#define UARTA2_BUFFER_FULL (UARTA2WriteIndex + 1) % UARTA2_BUFFERSIZE == UARTA2ReadIndex ? true : false
#define UARTA2_AVAILABLE UARTA2WriteIndex - UARTA2ReadIndex

/**
 * Initializes the UART for use.
 */
void uart_init(uint32_t UART, eUSCI_UART_Config UARTConfig)
{
    switch (UART)
    {
    case EUSCI_A0_BASE:
        MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
        MAP_UART_initModule(UART, &UARTConfig);
        MAP_UART_enableModule(UART);
        MAP_UART_enableInterrupt(UART, EUSCI_A_UART_RECEIVE_INTERRUPT);
        MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
        break;
    case EUSCI_A2_BASE:
        MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
        MAP_UART_initModule(UART, &UARTConfig);
        MAP_UART_enableModule(UART);
        MAP_UART_enableInterrupt(UART, EUSCI_A_UART_RECEIVE_INTERRUPT);
        MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
        break;
    default:
        break;
    }
}

/**
 * Write data into the UART.
 */
void uart_write(uint32_t UART, uint8_t *data, uint32_t size)
{
    uint32_t i;
    for (i = 0; i < size; i++)
    {
        MAP_UART_transmitData(UART, data[i]);
    }
}

/**
 * Read data from the UART.
 */
uint32_t uart_read(uint32_t UART, uint8_t *data, uint32_t size)
{
    uint32_t i;
    int8_t c;

    switch (UART)
    {
    case EUSCI_A0_BASE:
        for (i = 0; i < size; i++)
        {
            if (UARTA0_BUFFER_EMPTY)
            {
                return i;
            }
            else
            {
                c = UARTA0Data[UARTA0ReadIndex];
                UARTA0_ADVANCE_READ_INDEX;

                data[i] = c;
            }
        }
        break;

    case EUSCI_A2_BASE:
        for (i = 0; i < size; i++)
        {
            if (UARTA2_BUFFER_EMPTY)
            {
                return i;
            }
            else
            {
                c = UARTA2Data[UARTA2ReadIndex];
                UARTA2_ADVANCE_READ_INDEX;

                data[i] = c;
            }
        }
        break;
    default:
        return 0;
    }

    return i;
}

/**
 * Check if UART is available.
 */
uint32_t uart_available(uint32_t UART)
{
    switch (UART)
    {
    case EUSCI_A0_BASE:
        return UARTA0WriteIndex - UARTA0ReadIndex;
    case EUSCI_A2_BASE:
        return UARTA2WriteIndex - UARTA2ReadIndex;
    default:
        return 0;
    }
}

/**
 * Flush the UART.
 */
void uart_flush(uint32_t UART)
{
    switch (UART)
    {
    case EUSCI_A0_BASE:
        UARTA0WriteIndex = UARTA0ReadIndex = 0;
        break;
    case EUSCI_A2_BASE:
        UARTA2WriteIndex = UARTA2ReadIndex = 0;
        break;
    }
}

/**
 * EUSCI A0 UART ISR.
 */
void EUSCIA0_IRQHandler(void)
{
    uint8_t c;
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        c = MAP_UART_receiveData(EUSCI_A0_BASE);

        UARTA0Data[UARTA0WriteIndex] = c;
        UARTA0_ADVANCE_WRITE_INDEX;

        // Transmit data only if it made it to the buffer
        MAP_UART_transmitData(EUSCI_A0_BASE, c);
    }
}

/**
 * EUSCI A2 UART ISR.
 */
void EUSCIA2_IRQHandler(void)
{
    uint8_t c;
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT)
    {
        c = MAP_UART_receiveData(EUSCI_A2_BASE);

        UARTA2Data[UARTA2WriteIndex] = c;
        UARTA2_ADVANCE_WRITE_INDEX;
    }
}
