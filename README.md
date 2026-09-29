# servo control

STM32F103C8T6（蓝丸）上用 **TIM3 硬件 PWM** 驱动舵机来回扫角，并在 **SSD1306 OLED** 上显示当前下发的目标角度，串口同步打印 I2C / OLED 自检结果。

PlatformIO + STM32Cube HAL（`framework = stm32cube`）裸机工程，无 RTOS。

## 硬件连接

| 功能 | 引脚 | 外设 / 说明 |
| --- | --- | --- |
| 舵机信号 | PA6 | TIM3_CH1，50 Hz PWM，脉宽 0.5–2.5 ms |
| OLED SCL | PB6 | I2C1，100 kHz，软件模拟时序外的硬件 I2C |
| OLED SDA | PB7 | I2C1，SSD1306 7 位地址 0x3C |
| 调试串口 TX | PA9 | USART1，115200 8-N-1，只发不收 |
| 板载 LED | PC13 | 低电平点亮，主循环每轮翻转一次 |
| 下载 / 调试 | SWD 四线 | ST-Link：3V3 / GND / SWDIO / SWCLK |

## 工作流程

1. `HAL_Init()` → `SystemClock_Config()`：HSE 8 MHz × 9 = 72 MHz。
2. 先初始化 LED（判断程序有没有跑起来），再 `TIM3_Init()` / `UART1_Init()` / `I2c_Init()`。
3. 用 `HAL_I2C_IsDeviceReady()` 探测 0x3C：通了才 `OLED_Init()`，沿屏幕四边画框 + 打印 `OLED OK`，失败则串口输出 `I2C FAIL` 并跳过屏幕相关代码（`oled_ready` 标志）。
4. 主循环 0° → 90° → 180° 循环扫角，每步 `HAL_Delay(500)` 等舵机转到位。

PWM 时基：72 MHz / (71+1) = 1 MHz 计数，周期 20000 计数 = 20 ms = 50 Hz；`servo_set()` 把 0–180° 线性映射到 500–2500 µs，并对超范围角度做上限截断。

## 编译与烧录

```bash
pio run                 # 编译
pio run -t upload       # 用 ST-Link 烧录
pio device monitor      # 115200 看串口输出
```

工程已在 `platformio.ini` 里固定 `upload_protocol = stlink`，并用 `upload_flags = -c set CPUTAPID 0x2ba01477` 覆盖 OpenOCD 默认的 SWD ID 期望值。

## 目录结构

```
include/    各外设头文件（servo / tim / i2c / uart / oled / led / system）
src/        对应实现 + main.c
platformio.ini
```

## 已知限制

- **没有位置反馈**：屏幕上显示的 `angle` 是发给舵机的目标角度，不是实测轴角；`HAL_Delay(500)` 只是预计动作时间。
- 串口只有阻塞式发送（`HAL_UART_Transmit`），没有接收和环形缓冲。
- 舵机由板子 5V 供电，堵转电流较大时建议外接电源共地，别只靠 USB 供电。
