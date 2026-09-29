#include "led.h"

/* 蓝丸板载 LED 接在 PC13。
 * 板子上 LED 阳极接 VCC、阴极接 PC13，所以 PC13 输出低电平时 LED 亮，
 * 输出高电平时 LED 灭。这是蓝丸/最小系统板的标准接法，不是你接反了。 */

void LED_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = GPIO_PIN_13;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;   /* 推挽输出，能强驱动高低电平 */
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;   /* LED 不挑速度，低速省电 */
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    LED_Off();   /* 上电默认灭 */
}

void LED_On(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  /* 低电平 = 亮 */
}

void LED_Off(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    /* 高电平 = 灭 */
}

void LED_Toggle(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
