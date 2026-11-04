#ifndef __SERVO_H__
#define __SERVO_H__

#include "stm32f1xx_hal.h"
#include "tim.h"

/* 角度 → 50Hz PWM 脉宽：0~180° 映射到 500~2500µs（宏在 servo.c 顶部，可按舵机型号改） */
void servo_set(uint16_t angle);

/* 直接给脉宽（µs，500~2500 截断），用来手动找这个舵机的真实行程端点 */
void servo_set_pulse_us(uint16_t pulse_us);

#endif
