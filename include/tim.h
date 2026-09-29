#ifndef __TIM_H__
#define __TIM_H__
#include "stm32f1xx_hal.h"
void TIM3_Init(void);
void tim_set_Pulse(uint16_t pulse);
#endif