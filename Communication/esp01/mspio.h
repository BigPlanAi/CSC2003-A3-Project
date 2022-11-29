#ifndef LIB_UTILS_MSPIO_H_
#define LIB_UTILS_MSPIO_H_

#include <stdio.h>
#include <uart.h>

void msprintf(uint32_t UART, const char *fs, ...);
int mspgets(uint32_t UART, char *b, int size);

#endif /* LIB_UTILS_MSPIO_H_ */
