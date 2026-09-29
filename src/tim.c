#include "tim.h"
TIM_HandleTypeDef htim3;
void TIM3_Init(void)
{
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 19999;
    HAL_TIM_PWM_Init(&htim3);
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1500;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin   = GPIO_PIN_6;            // TIM3_CH1 → PA6
        GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;       // 复用推挽
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  // F1 用这个宏
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}
void tim_set_Pulse(uint16_t pulse)
{
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,pulse);
}
