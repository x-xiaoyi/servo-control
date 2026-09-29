#ifndef __OLED_FONT_H__
#define __OLED_FONT_H__
#include <stdint.h>

/* 8x16 ASCII 字库，覆盖 32(' ') ~ 126('~')
 * 每个字符 16 字节：前 8 字节是上半页(每字节一列，bit0 在最上)，后 8 字节是下半页 */
extern const uint8_t oled_font_8x16[95][16];

#endif
