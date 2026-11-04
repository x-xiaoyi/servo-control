#include "servo.h"

/* ---------- 舵机脉宽范围（µs） ----------
 * 不同型号的实际行程端点会差：SG90/MG90S 常见 500~2500，
 * 但有些舵机 1000~2000 就已经是满行程。
 * 上板发现"打到极限还在吱吱叫"或"转不到头"，先改这两个宏，
 * 别去改角度换算 —— 换算本身是对的，是舵机行程对不上。 */
#define SERVO_PULSE_MIN_US   500U
#define SERVO_PULSE_MAX_US   2500U
#define SERVO_ANGLE_MAX      180U

/* 角度 → 脉宽用整数运算，不引浮点：
 *     pulse = MIN + angle * (MAX - MIN) / 180
 * Cortex-M3 没有 FPU，float 除法会走 __aeabi_fdiv 软件库（几十个周期）。
 * 这里不在中断里、性能无所谓，但整数写法结果可预测、没有 0.5µs 的舍入抖动。
 * 顺序必须"先乘后除"：angle*2000 最大 360000，uint32_t 装得下，不溢出也不丢精度；
 * 反过来先除再乘会把 2000/180 的余数丢掉，180° 只能打到 2480µs。 */
void servo_set(uint16_t angle)
{
    if (angle > SERVO_ANGLE_MAX)
    {
        angle = SERVO_ANGLE_MAX;   /* 上限截断：宁可不转到头，也不能把脉宽顶出机械行程 */
    }

    uint32_t pulse = SERVO_PULSE_MIN_US
                   + ((uint32_t)angle * (SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US))
                     / SERVO_ANGLE_MAX;

    tim_set_Pulse((uint16_t)pulse);
}

/* 直接下发脉宽（µs），给"手动找端点"用：
 * 拿它从 500 一点点加到 2500（每次 50µs，间隔 1s），听声音、看轴，
 * 记下这个舵机真正能到的上下限，再回填上面的两个宏。
 * 找不到端点就说明齿轮已经顶死，继续加只会烧电机。 */
void servo_set_pulse_us(uint16_t pulse_us)
{
    if (pulse_us < SERVO_PULSE_MIN_US)
    {
        pulse_us = SERVO_PULSE_MIN_US;
    }
    if (pulse_us > SERVO_PULSE_MAX_US)
    {
        pulse_us = SERVO_PULSE_MAX_US;
    }

    tim_set_Pulse(pulse_us);
}
