#ifndef HARDWARE_UART_DRIVER_H_
#define HARDWARE_UART_DRIVER_H_

#include "stdio.h"
#include "driverlib.h"

#define UARTA0_BUFFERSIZE 2048
#define UARTA2_BUFFERSIZE 2048

/*UARTA0 Ring Buffer Global Variables*/
volatile uint8_t UARTA0Data[UARTA0_BUFFERSIZE];
volatile uint32_t UARTA0ReadIndex;
volatile uint32_t UARTA0WriteIndex;

/*UARTA2 Ring Buffer Global Variables*/
volatile uint8_t UARTA2Data[UARTA2_BUFFERSIZE];
volatile uint32_t UARTA2ReadIndex;
volatile uint32_t UARTA2WriteIndex;

void UART_Write(uint32_t UART, uint8_t *Data, uint32_t Size);
uint32_t UART_Read(uint32_t UART, uint8_t *Data, uint32_t Size);
int UART_Gets(uint32_t UART, char *Data, int Size);
void UART_Printf(uint32_t UART, const char *fs, ...);

#endif /* HARDWARE_UART_DRIVER_H_ */
