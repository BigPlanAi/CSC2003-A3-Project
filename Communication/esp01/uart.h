#ifndef LIB_UART_UART_H_
#define LIB_UART_UART_H_

#include "driverlib.h"
#include <rom.h>
#include <rom_map.h>
#include <interrupt.h>
#include <uart.h>
#include <gpio.h>

#define UARTA0_BUFFERSIZE 4096
#define UARTA2_BUFFERSIZE 4096

void uart_init(uint32_t UART, eUSCI_UART_Config UARTConfig);
void uart_write(uint32_t UART, uint8_t *Data, uint32_t Size);
uint32_t uart_read(uint32_t UART, uint8_t *Data, uint32_t Size);
uint32_t uart_available(uint32_t UART);
void uart_flush(uint32_t UART);

#endif /* LIB_UART_UART_H_ */
