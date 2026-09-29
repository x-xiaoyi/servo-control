#ifndef __UART_H__
#define __UART_H__
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

void UART1_Init(void);
void uart_puts(const char *str);

#endif