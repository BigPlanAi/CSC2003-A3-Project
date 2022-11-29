#ifndef LIB_UTILS_UTILS_H_
#define LIB_UTILS_UTILS_H_
#include "driverlib.h"

void delay(uint32_t);
float min(float, float);
float max(float, float);

void GPIO_High(uint32_t GPIO_Port, uint16_t pins);
void GPIO_Low(uint32_t GPIO_Port, uint16_t pins);
void GPIO_Output(uint32_t GPIO_Port, uint16_t pins);
void GPIO_Pullup_Input(uint32_t GPIO_Port, uint16_t pins);

void LED1_init(void);
void LED2_init(void);
void LED1_RED_ON(void);
void LED1_RED_OFF(void);
void LED2_RED_ON(void);
void LED2_RED_OFF(void);
void LED2_GREEN_ON(void);
void LED2_GREEN_OFF(void);
void LED2_BLUE_ON(void);
void LED2_BLUE_OFF(void);

#endif /* LIB_UTILS_UTILS_H_ */
