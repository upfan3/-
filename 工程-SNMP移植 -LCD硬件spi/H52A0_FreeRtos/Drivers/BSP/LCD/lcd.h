#ifndef LCD_H
#define LCD_H

#include "stdint.h"


#define LCD  1;

// -------------------------- 硬件引脚定义（严格遵循规格书Page5/6）--------------------------
// 1号引脚：CS1（片选）
#define LCD_CS_PORT    GPIOD  // 请替换为你的硬件接线端口（如GPIOE）
#define LCD_CS_PIN     GPIO_PIN_7  // 替换为实际引脚（如GPIO_PIN_4）
#define Set_LCD_CS     gpio_bit_set(LCD_CS_PORT, LCD_CS_PIN)
#define Clr_LCD_CS     gpio_bit_reset(LCD_CS_PORT, LCD_CS_PIN)

// 2号引脚：RES（硬件复位）
#define LCD_RST_PORT   GPIOE  // 替换为实际端口（如GPIOE）
#define LCD_RST_PIN    GPIO_PIN_3  // 替换为实际引脚（如GPIO_PIN_3）
#define Set_LCD_RST    gpio_bit_set(LCD_RST_PORT, LCD_RST_PIN)
#define Clr_LCD_RST    gpio_bit_reset(LCD_RST_PORT, LCD_RST_PIN)

// 3号引脚：A0（命令/数据选择，H=数据，L=命令）
#define LCD_A0_PORT    GPIOE  // 替换为实际端口（如GPIOD）
#define LCD_A0_PIN     GPIO_PIN_4  // 替换为实际引脚（如GPIO_PIN_7）
#define Set_LCD_A0     gpio_bit_set(LCD_A0_PORT, LCD_A0_PIN)
#define Clr_LCD_A0     gpio_bit_reset(LCD_A0_PORT, LCD_A0_PIN)

// -------------------------- 全局常量/缓存定义 --------------------------
#define LCD_WIDTH  128  // 规格书Page4：128x64点阵
#define LCD_HEIGHT 64
#define LCD_PAGE   8    // 1/64Duty，分8页（Page4）

// 显示缓存GRAM（[列][页]，与规格书页式存储一致）
extern uint8_t LCD_GRAM[128][8];

// -------------------------- ST7567指令定义（严格遵循规格书Page22）--------------------------
#define LCD_CMD_DISPLAY_OFF    0xAE  // 显示关闭
#define LCD_CMD_DISPLAY_ON     0xAF  // 显示开启
#define LCD_CMD_SOFT_RESET     0xE2  // 软件复位
#define LCD_CMD_SEG_DIR_NORMAL 0xA0  // 段扫描正向
#define LCD_CMD_SEG_DIR_REVERSE 0xA1 // 段扫描反向
#define LCD_CMD_COM_DIR_NORMAL 0xC0  // COM扫描正向
#define LCD_CMD_COM_DIR_REVERSE 0xC8 // COM扫描反向（适配6点钟视角）
#define LCD_CMD_SET_BIAS_1_9   0xA2  // 偏压比1/9（Page4规格）
#define LCD_CMD_SET_REGULATOR   0x24  // 调节电阻比例
#define LCD_CMD_POWER_CONTROL   0x2F  // 电源控制（开启内置电源）
#define LCD_CMD_SET_EV_START    0x81  // 电子音量（对比度）调节起始
#define LCD_CMD_INVERSE_OFF     0xA6  // 正常显示
#define LCD_CMD_INVERSE_ON      0xA7  // 反显
#define LCD_CMD_ALL_PIXEL_ON    0xA5  // 全亮
#define LCD_CMD_ALL_PIXEL_NORMAL 0xA4 // 正常像素
#define LCD_CMD_SET_PAGE        0xB0  // 设置页地址（0xB0~0xB7）
#define LCD_CMD_SET_COL_LOW     0x02  // 列地址低4位（0x00~0x0F）
#define LCD_CMD_SET_COL_HIGH    0x10  // 列地址高4位（0x10~0x1F）

// -------------------------- 类定义（沿用OLED代码风格）--------------------------
class LCD_ST7567
{
public:
    LCD_ST7567(void);
    ~LCD_ST7567(void);
    void init(void);
    uint8_t write_cmd(uint8_t cmd);
    uint8_t write_data(uint8_t dat);
    void refresh_gram(void);
    void clear(void);
    void show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t mode);
    void draw_point(uint8_t x, uint8_t y, uint8_t t);
    void show_full_screen_numbers(void);

};

#endif // LCD_H