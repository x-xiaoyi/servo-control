#ifndef __SYSTEM_H
#define __SYSTEM_H

/* system.c 里用到了 HAL 类型（RCC_xxxInitTypeDef 等），
 * 头文件必须先把 HAL 根头文件引进来，否则编译器报 unknown type name */
#include "stm32f1xx_hal.h"

/* 时钟树：把 F103 从默认的 HSI 8MHz 拉到 HSE 8MHz × 9 = 72MHz */
void SystemClock_Config(void);

/* HAL 库统一的错误陷阱：任何初始化失败都进这里死循环 */
void Error_Handler(void);

#endif /* __SYSTEM_H */