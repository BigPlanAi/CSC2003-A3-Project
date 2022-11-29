#include <mspio.h>

/**
 * Send the char through the selected UART
 */
void print_char(uint32_t UART, char c)
{
    uart_write(UART, (uint8_t *)&c, 1);
}

/**
 * Print while string is not the null character
 */
void print_string(uint32_t UART, char *string)
{
    while (*string)
    {
        print_char(UART, *string);
        string++;
    }
}

/**
 * Print a number in a given base.
 *
 * Thanks! https://github.com/grbl/grbl/blob/master/grbl/print.c
 */
void print_integer(uint32_t UART, int integer)
{
    if (integer == 0)
    {
        print_char(UART, '0');
    }

    if (integer < 0)
    {
        print_char(UART, '-');
    }

    char b[10];
    int digit = integer;

    uint8_t i = 0;
    while (digit)
    {
        b[i++] = digit % 10;
        digit /= 10;
    }

    while (i)
    {
        print_char(UART, '0' + b[i - 1]);
        i--;
    }
}

/*
 * A basic printf for the MSP432. In order to use it properly you need to initialize the correct UART peripheral.
 * The following formats are supported:
 * %c = for char variables
 * %s = for string variables
 * %i = for unsigned integers
 * USAGE...
 *
 * msprintf(EUSCI_A0_BASE, "Formated string %c, %s, %i", character, string, integer)
 * */
void msprintf(uint32_t UART, const char *fs, ...)
{
    va_list valist;
    va_start(valist, fs);
    int i;
    char *s;

    while (*fs)
    {
        if (*fs != '%')
        {
            uart_write(UART, (uint8_t *)fs, 1);
            fs++;
        }
        else
        {
            switch (*++fs)
            {
            case 'c':
                i = va_arg(valist, int);
                print_char(UART, (char)i);
                break;
            case 's':
                s = va_arg(valist, char *);
                print_string(UART, s);
                break;
            case 'i':
                i = va_arg(valist, int);
                print_integer(UART, i);
                break;
            }

            ++fs;
        }
    }
}

/*
 * A basic gets for the MSP432. In order to use it properly you need to initialize the correct UART peripheral.
 * USAGE...
 *
 * mspgets(EUSCI_A0_BASE, InputBuffer, SizeOfInputBuffer)
 */
int mspgets(uint32_t UART, char *b, int size)
{
    char c;
    uint32_t i = 0;

    while (1)
    {
        if (uart_read(UART, (uint8_t *)&c, 1) != 0)
        {
            // Put a '\n' and '\r' if it fits on the buffer
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
            // Erase data from buffer if backspace is received
            else if (c == 127 || c == 8)
            {
                i--;
                b[i] = 0;
            }
            // Store character on the buffer
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
