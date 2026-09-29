#include "stm32f1xx_hal.h"
#include "system.h"
#include "servo.h"
#include "i2c.h"
#include "uart.h"
#include "oled.h"
#include "led.h"

static uint8_t oled_ready = 0;

/* 当前没有位置反馈，所以这里显示的是已经发给舵机的目标角度。
 * 500ms 是预计动作时间，不代表已经测得真实轴角。 */
static void Servo_MoveAndShow(uint16_t angle)
{
    servo_set(angle);
    HAL_Delay(500);

    if (oled_ready)
    {
        OLED_ShowAngle((int16_t)angle);
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    LED_Init();        /* 最先初始化 LED，方便判断程序有没有跑起来 */
    TIM3_Init();
    UART1_Init();
    I2c_Init();

    /* 先用串口确认 I2C 通不通，通了再初始化屏幕 */
    if (HAL_I2C_IsDeviceReady(&hi2c1, 0x3C << 1, 3, 100) == HAL_OK)
    {
        uart_puts("I2C OK\r\n");

        OLED_Init();
        OLED_Clear();

        /* 沿屏幕四边画个框，验证整屏像素都能点亮 */
        OLED_DrawLine(0, 0, OLED_WIDTH - 1, 0);
        OLED_DrawLine(0, OLED_HEIGHT - 1, OLED_WIDTH - 1, OLED_HEIGHT - 1);
        OLED_DrawLine(0, 0, 0, OLED_HEIGHT - 1);
        OLED_DrawLine(OLED_WIDTH - 1, 0, OLED_WIDTH - 1, OLED_HEIGHT - 1);

        /* 在屏幕上也打一行字，位置避开边框 */
        OLED_ShowString(36, 24, "OLED OK");

        OLED_Refresh();
        oled_ready = 1;

        uart_puts("OLED OK\r\n");
    }
    else
    {
        uart_puts("I2C FAIL\r\n");
    }

    while (1)
    {
        LED_Toggle();   /* 每轮翻转一次，闪烁说明主循环仍在运行 */

        Servo_MoveAndShow(0);
        HAL_Delay(500);

        Servo_MoveAndShow(90);
        HAL_Delay(500);

        Servo_MoveAndShow(180);
        HAL_Delay(500);
    }
}
