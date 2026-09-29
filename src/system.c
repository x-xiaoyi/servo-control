#include "system.h"

/* ============================================================================
 * 时钟树配置：HSE 8MHz 晶振 × PLL9 = 72MHz（F103 的最高主频）
 *
 *   8MHz HSE ──> PREDIV1(÷1) ──> PLL(×9) ──> 72MHz SYSCLK
 *                                              ├─ AHB  ÷1 ──> HCLK  72MHz
 *                                              ├─ APB1 ÷2 ──> PCLK1 36MHz
 *                                              └─ APB2 ÷1 ──> PCLK2 72MHz
 *
 * 为什么必须用 HSE 而不是 HSI？
 *   HSI 是芯片内部的 RC 振荡器，精度只有 ±1% 左右，且温漂大；
 *   更关键的是：HSI 走 PLL 时先要 ÷2 变成 4MHz，而 F1 的 PLLMUL 最大只到 16，
 *   4MHz × 16 = 64MHz，根本凑不出 72MHz。所以 72MHz 只能用板载 8MHz 晶振 × 9。
 * ============================================================================ */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* --- 第一步：开振荡器 + 配 PLL 倍频 --- */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;              /* 打开板载 8MHz 晶振 */
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;     /* 进 PLL 前不分频，保持 8MHz */
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;              /* HSI 也留着，做 HSE 失效时的兜底时钟源 */
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;       /* PLL 的输入选 HSE */
    RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;            /* 8MHz × 9 = 72MHz */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        /* 常见原因：板子没焊 8MHz 晶振（部分山寨蓝丸省料）→ 卡在 Error_Handler 里 */
        Error_Handler();
    }

    /* --- 第二步：把 SYSCLK 切到 PLL，并配三条总线的分频 --- */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK; /* 系统时钟源 = PLL 输出 */
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;         /* HCLK = 72MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;           /* APB1 = 36MHz（TIM2 在这条总线上） */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;           /* APB2 = 72MHz（GPIOA 在这条总线上） */

    /* FLASH_LATENCY_2 不能省：
     * Flash 是 3.3V 供电，72MHz 已经超出它单周期取指的能力，
     * 必须插 2 个等待周期，否则 CPU 读到的指令可能是错的（跑飞）。
     * 规则：0~24MHz→0；24~48MHz→1；48~72MHz→2 */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ============================================================================
 * SysTick 中断服务函数（全工程唯一）
 *
 * 名字必须叫 SysTick_Handler：启动文件 startup_stm32f103xb.s 里的向量表
 * 是按名字找函数的，写成别的名字永远进不来。
 *
 * 启动文件把它声明成 .weak（弱符号），所以我们这里写一个同名的强符号
 * 就能把它覆盖掉 —— 这正是 HAL 库"回调靠同名覆盖"的机制。
 *
 * 作用：每 1ms 触发一次，把 HAL 的内部计数器 uwTick 加 1。
 * 没有它，HAL_Delay() 会死等在一个永远不增加的变量上 → 程序假死。
 * ============================================================================ */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/* ============================================================================
 * 错误陷阱：HAL 初始化任何一步失败都会跳进来
 * 调试时在这里打断点，就能一眼看出是哪个外设没配起来
 * ============================================================================ */
void Error_Handler(void)
{
    __disable_irq(); /* 关中断，保证死循环不会被打断 */
    while (1)
    {
    }
}