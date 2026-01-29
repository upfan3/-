#ifndef OLED_H
#define OLED_H

#include "gd32f4xx.h"
// D/C
#define OLED_RS_PORT GPIOE
#define OLED_RS_PIN GPIO_PIN_4
#define Set_OLED_RS                              \
    {                                            \
        gpio_bit_set(OLED_RS_PORT, OLED_RS_PIN); \
    }
#define Clr_OLED_RS                                \
    {                                              \
        gpio_bit_reset(OLED_RS_PORT, OLED_RS_PIN); \
    }
// RES
#define OLED_RST_PORT GPIOE
#define OLED_RST_PIN GPIO_PIN_3
#define Set_OLED_RST                               \
    {                                              \
        gpio_bit_set(OLED_RST_PORT, OLED_RST_PIN); \
    }
#define Clr_OLED_RST                                 \
    {                                                \
        gpio_bit_reset(OLED_RST_PORT, OLED_RST_PIN); \
    }

#define FONT_DATA_BASE 0x08040000
// uint16_t FONT_LEN=*(uint16_t *)FONT_DATA_BASE;//读取汉字长度

// uint32_t ASC2_1206=FONT_DATA_BASE+8;//ASCII字模起始位置 固定为偏地址后8个字节
// uint32_t FONT_1206=FONT_DATA_BASE+8+95*9;//汉字模起始位置 //总共95个ascii码，每个占9个字节 再加上8字节信息码
// uint32_t FONT_TXT=FONT_DATA_BASE+(*((uint16_t *)FONT_DATA_BASE+3));//起始位置第4个字节，指定了汉字内码的偏移地址

class OLED
{

private:
    uint8_t GRAM[128][8];

public:
    OLED(void);
    ~OLED();
    void init();
    uint8_t write_cmd(uint8_t cmd);
    uint8_t write_data(uint8_t dat);
    void refresh_gram(void);
    void clear(void);
    void show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);
    void draw_point(uint8_t x, uint8_t y, uint8_t t);
};

#endif
