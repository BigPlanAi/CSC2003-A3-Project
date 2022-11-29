#include "UART_Driver.h"

void UART_Write(uint32_t UART, uint8_t *Data, uint32_t Size)
{
    uint32_t i;
    for (i = 0; i < Size; i++)
    {
        MAP_UART_transmitData(UART, Data[i]);
    }
}

uint32_t UART_Read(uint32_t UART, uint8_t *Data, uint32_t Size)
{
    uint32_t i;
    int8_t c;

    switch (UART)
    {
    case EUSCI_A0_BASE:
        for (i = 0; i < Size; i++)
        {
            bool checkEmpty = true;
            if (UARTA0ReadIndex == UARTA0WriteIndex)
                checkEmpty = true;
            else
                checkEmpty = false;

            if (checkEmpty)
            {
                return i;
            }
            else
            {
                c = UARTA0Data[UARTA0ReadIndex];

                MAP_Interrupt_disableMaster();
                UARTA0ReadIndex = (UARTA0ReadIndex + 1) % UARTA0_BUFFERSIZE;
                MAP_Interrupt_enableMaster();

                Data[i] = c;
            }
        }
        break;

    case EUSCI_A2_BASE:
        for (i = 0; i < Size; i++)
        {
            bool checkEmpty = true;
            if (UARTA2ReadIndex == UARTA2WriteIndex)
                checkEmpty = true;
            else
                checkEmpty = false;

            if (checkEmpty)
            {
                return i;
            }
            else
            {
                c = UARTA2Data[UARTA2ReadIndex];

                MAP_Interrupt_disableMaster();
                UARTA2ReadIndex = (UARTA2ReadIndex + 1) % UARTA2_BUFFERSIZE;
                MAP_Interrupt_enableMaster();

                Data[i] = c;
            }
        }
        break;
    /*More UART reading modules go here*/
    default:
        return 0;
    }

    return i;
}

/*A basic gets for the MSP432. In order to use it properly you need to initialize the correct UART peripheral.
 * USAGE...
 * UART_Gets(EUSCI_A0_BASE, InputBuffer, SizeOfInputBuffer)*/

int UART_Gets(uint32_t UART, char *b, int size)
{
    char c;
    uint32_t i = 0;

    while (1)
    {
        if (UART_Read(UART, (uint8_t *)&c, 1) != 0)
        {
            /*put a '\n' and '\r' if it fits on the buffer*/
            if (c == '\n' || c == '\r')
            {
                if (i + 3 > size)
                {
                    return size + 1;
                }

                b[i++] = '\r';
                b[i++] = '\n';
                b[i++] = 0;

                return i;
            }
            /*erase data from buffer if backspace is received*/
            else if (c == 127 || c == 8)
            {
                i--;
                b[i] = 0;
            }
            /*store character on the buffer*/
            else
            {
                if (i < size)
                {
                    b[i++] = c;
                }
                else
                {
                    return size + 1;
                }
            }
        }
    }
}

/*A basic printf for the MSP432. In order to use it properly you need to initialize the correct UART peripheral.
 * The following formats are supported:
 * %c = for char variables
 * %s = for string variables
 * %i = for unsigned integers
 * USAGE...
 * MSPrintf(EUSCI_A0_BASE, "Formated string %c, %s, %i", character, string, integer)*/

void UART_Printf(uint32_t UART, const char *fs, ...)
{
    va_list valist;
    va_start(valist, fs);

    int i;
    char *s;
    int tmpVal;

    while (*fs)
    {
        if (*fs != '%')
        {
            UART_Write(UART, (uint8_t *)fs, 1);
            fs++;
        }
        else
        {
            switch (*++fs)
            {
            case 'c':
                i = va_arg(valist, int);
                UART_Write(UART, (uint8_t *)&i, 1);
                break;
            case 's':
                s = va_arg(valist, char *);
                while (*s)
                {
                    UART_Write(UART, (uint8_t *)s, 1);
                    s++;
                }
                break;
            case 'i':
                i = va_arg(valist, int);
                if (i == 0)
                {
                    tmpVal = '0';
                    UART_Write(UART, (uint8_t *)&tmpVal, 1);
                }

                if (i < 0)
                {
                    tmpVal = '-';
                    UART_Write(UART, (uint8_t *)&tmpVal, 1);
                }

                char b[10];
                int digit = i;

                uint8_t i = 0;
                while (digit)
                {
                    b[i++] = digit % 10;
                    digit /= 10;
                }

                while (i)
                {
                    tmpVal = '0' + b[i - 1];
                    UART_Write(UART, (uint8_t *)&tmpVal, 1);
                    i--;
                }
                break;
            }

            ++fs;
        }
    }
}
