#include "oled.h"
#include "oled_font.h"
#include "i2c.h"
#include <stdio.h>
#include <string.h>

/* SSD1306 的 7 位地址 0x3C，HAL 库要 8 位格式，左移一位 */
#define OLED_ADDR   (0x3C << 1)
#define OLED_PAGES  (OLED_HEIGHT / 8)   /* 64 行 / 8 = 8 页 */

/* 显存：8 页 × 128 列，每个 bit 对应屏幕上一个像素
 * SSD1306 是"分页式"的：一列 8 个像素纵向挤在一个字节里 */
static uint8_t OLED_GRAM[OLED_PAGES][OLED_WIDTH];

/* 发一条命令。控制字节 0x00 表示"后面跟的是命令" */
static void OLED_WriteCmd(uint8_t cmd)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 100);
}

void OLED_Init(void)
{
    static const uint8_t init_cmds[] = {
        0xAE,        /* 关显示（配置期间先关，配完再开） */
        0x20, 0x02,  /* 页寻址模式 */
        0xB0,        /* 定位到第 0 页 */
        0xC8,        /* COM 扫描方向反向（让 y 轴朝下） */
        0x00,        /* 列地址低 4 位 */
        0x10,        /* 列地址高 4 位 */
        0x40,        /* 起始行 = 0 */
        0x81, 0x7F,  /* 对比度 */
        0xA1,        /* 段重映射（让 x 轴朝右） */
        0xA6,        /* 正常显示，不反色 */
        0xA8, 0x3F,  /* 多路复用比 1/64，对应 64 行 */
        0xA4,        /* 显示内容跟随显存，不强制全亮 */
        0xD3, 0x00,  /* 显示偏移 = 0 */
        0xD5, 0x80,  /* 时钟分频 */
        0xD9, 0xF1,  /* 预充电周期 */
        0xDA, 0x12,  /* COM 引脚配置 */
        0xDB, 0x40,  /* VCOMH 电压 */
        0x8D, 0x14,  /* 打开内部电荷泵：不给这条屏幕不亮 */
    };

    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT,
                      (uint8_t *)init_cmds, sizeof(init_cmds), 100);

    HAL_Delay(100);
    OLED_Clear();
    OLED_WriteCmd(0xAF);   /* 开显示 */
}

/* 清屏 = 清显存 + 把显存推上屏 */
void OLED_Clear(void)
{
    memset(OLED_GRAM, 0x00, sizeof(OLED_GRAM));
    OLED_Refresh();
}

/* 把整个显存推到屏幕上。必须调用，否则只改显存屏幕不会有反应 */
void OLED_Refresh(void)
{
    for (uint8_t page = 0; page < OLED_PAGES; page++)
    {
        OLED_WriteCmd(0xB0 + page);   /* 定位到第 page 页 */
        OLED_WriteCmd(0x00);          /* 列地址低 4 位 = 0 */
        OLED_WriteCmd(0x10);          /* 列地址高 4 位 = 0 */

        /* 控制字节 0x40 表示"后面跟的是显示数据" */
        HAL_I2C_Mem_Write(&hi2c1, OLED_ADDR, 0x40, I2C_MEMADD_SIZE_8BIT,
                          OLED_GRAM[page], OLED_WIDTH, 100);
    }
}

void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t on)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }

    if (on)
    {
        OLED_GRAM[y / 8][x] |= (uint8_t)(1U << (y % 8));
    }
    else
    {
        OLED_GRAM[y / 8][x] &= (uint8_t)~(1U << (y % 8));
    }
}

/* 画直线，Bresenham 算法：只用整数加减，不用浮点，快且准 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int16_t x = x1, y = y1;
    int16_t dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int16_t dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int16_t sx = (x1 < x2) ? 1 : -1;
    int16_t sy = (y1 < y2) ? 1 : -1;
    int16_t err = dx - dy;

    while (1)
    {
        OLED_DrawPoint((uint8_t)x, (uint8_t)y, 1);

        if (x == x2 && y == y2)
        {
            break;
        }

        int16_t e2 = err * 2;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 <  dx) { err += dx; y += sy; }
    }
}

/* 画一个 8x16 的 ASCII 字符。
 * 字库里每个字符 16 字节：前 8 字节是上半页（每字节一列，bit0 在最上），后 8 字节是下半页。
 * y 最好取 16 的倍数，这样一个字正好占满两页，不会跨页错位 */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch)
{
    if (ch < ' ' || ch > '~')
    {
        ch = ' ';
    }

    const uint8_t *glyph = oled_font_8x16[(uint8_t)(ch - ' ')];

    for (uint8_t col = 0; col < 8; col++)
    {
        uint8_t upper = glyph[col];        /* 上半页：y ~ y+7 */
        uint8_t lower = glyph[col + 8];    /* 下半页：y+8 ~ y+15 */

        for (uint8_t row = 0; row < 8; row++)
        {
            OLED_DrawPoint((uint8_t)(x + col), (uint8_t)(y + row),     (upper >> row) & 1U);
            OLED_DrawPoint((uint8_t)(x + col), (uint8_t)(y + row + 8), (lower >> row) & 1U);
        }
    }
}

/* 连续写字符串。一屏 128 宽，8x16 的字一行放得下 16 个。
 * 注意：这里只改显存，写完要自己调 OLED_Refresh() */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
    while (*str != '\0')
    {
        OLED_ShowChar(x, y, *str);

        x = (uint8_t)(x + 8);
        if (x > OLED_WIDTH - 8)
        {
            x = 0;
            y = (uint8_t)(y + 16);
        }
        str++;
    }
}

/* 对外提供的角度显示接口。
 * 调用者只需给出角度；本函数负责清除旧内容、格式化并刷新屏幕。 */
void OLED_ShowAngle(int16_t angle)
{
    char line[17];

    /* 一行最多 16 个 8x16 字符，先用空格覆盖旧数字。 */
    OLED_ShowString(0, 24, "                ");
    snprintf(line, sizeof(line), "Angle:%4d deg", (int)angle);
    OLED_ShowString(0, 24, line);
    OLED_Refresh();
}
