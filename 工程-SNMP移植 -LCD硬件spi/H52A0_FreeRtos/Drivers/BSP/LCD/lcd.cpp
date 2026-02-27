#include "lcd.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/SPI/spi.h"
#include "f1216.h"


SPI spi3(SPI_3);

// 显示缓存GRAM（初始化全0）
uint8_t LCD_GRAM[128][8] = {0};

// -------------------------- 类构造/析构函数 --------------------------
LCD_ST7567::LCD_ST7567(void)
{
}

LCD_ST7567::~LCD_ST7567(void)
{
}

// -------------------------- GPIO初始化--------------------------
void LCD_ST7567::init(void)
{
    // 1. 使能引脚时钟
    rcu_periph_clock_enable(RCU_GPIOE); 
    rcu_periph_clock_enable(RCU_GPIOD); 

    // 2. 配置CS引脚（推挽输出，高速）
    gpio_mode_set(LCD_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_CS_PIN);
    gpio_output_options_set(LCD_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, LCD_CS_PIN);

    // 3. 配置RST引脚（推挽输出，高速）
    gpio_mode_set(LCD_RST_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_RST_PIN);
    gpio_output_options_set(LCD_RST_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, LCD_RST_PIN);

    // 4. 配置A0引脚（推挽输出，高速）
    gpio_mode_set(LCD_A0_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_A0_PIN);
    gpio_output_options_set(LCD_A0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, LCD_A0_PIN);
	
		spi3.init();

    // 5. 初始化引脚默认状态
    Set_LCD_CS;    // 片选拉高，释放模块
    Set_LCD_RST;   // 复位拉高
    Clr_LCD_A0;    // 默认命令模式
		
		Set_LCD_RST; delay_ms(100);
    Clr_LCD_RST; delay_ms(100);
    Set_LCD_RST; delay_ms(300);
		
		    // 2. 初始化命令序列
    write_cmd(0xE2); // 软复位
    delay_ms(200);     
    
    write_cmd(0xA2); // 偏压比 1/9
    delay_ms(10);
    
    write_cmd(0xA0); // SEG方向
    delay_ms(10);
    
    write_cmd(0xC8); // COM方向 (6点钟视角)
    delay_ms(10);
    
    write_cmd(0x24); // 调节电阻比率
    delay_ms(10);
    
    write_cmd(0x81); // 电子音量命令
    delay_ms(10);
    write_cmd(0x1F); // 对比度值 (0x00-0x3F)
    delay_ms(10);	
    
    write_cmd(0x2F); // 开启电源 (内置升压)
    delay_ms(200);     

    write_cmd(0xB0); // 设置页
    write_cmd(0x10); // 设置列高4位
    write_cmd(0x00); // 设置列低4位
    write_cmd(0xAF); // 显示开启
    write_cmd(0xA6); // 正常显示
    delay_ms(10);

    clear();
		refresh_gram();
}


// -------------------------- 写命令（硬件SPI传输，适配A0引脚控制）--------------------------
uint8_t LCD_ST7567::write_cmd(uint8_t cmd)
{
    uint8_t res;
    Clr_LCD_CS;    // 片选拉低
    Clr_LCD_A0;    // A0拉低=命令模式
    delay_us(1);   // 时序稳定延时
    res = spi3.read_write_byte(cmd);  // 硬件SPI传输
    delay_us(1);
    Set_LCD_CS;    // 片选拉高
    return res;
}

// -------------------------- 写数据（硬件SPI传输，适配A0引脚控制）--------------------------
uint8_t LCD_ST7567::write_data(uint8_t dat)
{
    uint8_t res;
    Clr_LCD_CS;    // 片选拉低
    Set_LCD_A0;    // A0拉高=数据模式
    delay_us(1);   // 时序稳定延时
    res = spi3.read_write_byte(dat);  // 硬件SPI传输
    delay_us(1);
    Set_LCD_CS;    // 片选拉高
    return res;
}

// -------------------------- 刷新缓存到屏幕--------------------------
void LCD_ST7567::refresh_gram(void)
{
		uint8_t i, n;
	for (i = 0; i < 8; i++)
	{
		write_cmd(0xb0 + i); // 设置页地址（0~7）
		write_cmd(0x00);	 
		write_cmd(0x10);	 // 设置显示位置—列高地址
		for (n = 0; n < 128; n++)
			write_data(LCD_GRAM[n][i]); // 128
	}
		
		
}

// -------------------------- 清缓存（填充0）--------------------------
void LCD_ST7567::clear(void)
{
	uint8_t i, n;
	for (i = 0; i < 8; i++)
	{
		for (n = 0; n < 128; n++) // 128
		{
			LCD_GRAM[n][i] = 0X00;
		}
	}


}

// -------------------------- 画点（适配GRAM缓存映射）--------------------------
void LCD_ST7567::draw_point(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t pos, bx, temp = 0;
    if (x > 127 || y > 63)
        return; 

    pos = y / 8;          
    bx = y % 8;           
    temp = 1 << bx;       
    
    if (t)
        LCD_GRAM[x][pos] |= temp; 
    else
        LCD_GRAM[x][pos] &= ~temp;
}

// -------------------------- 显示单个字符--------------------------
void LCD_ST7567::show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t mode)
{
	uint8_t temp, t, t1, x0 = x;
	uint8_t y0 = y;
	chr = chr - 0x20;
	if (chr > 94)
		return; // 0~94共95个字模
	//////////////////////字模上半部分 宽6点，高8点/////////////////////////////
	for (t = 0; t < 6; t++)
	{
		y = y0; // 纵坐标复位

		// temp=*((uint8_t *)ASC2_1206+9*chr+t);
		temp = *(uint8_t *)(&font6x12table[chr * 9] + t);
		for (t1 = 0; t1 < 8; t1++)
		{
			if (temp & 0x80)
				draw_point(x, y, !mode);
			else
				draw_point(x, y, mode);
			temp <<= 1;
			y++;
		}
		x++;
	}
	//////////////////////字模下半部分 宽6点，高4点////////////////////////////
	x = x0;
	for (; t < 9; t++)
	{
		y = y0 + 8; ////纵坐标指向下半位置

		// temp=*((uint8_t *)ASC2_1206+9*chr+t);
		temp = *(uint8_t *)(&font6x12table[chr * 9] + t);
		for (t1 = 0; t1 < 8;) // t1，不能置于循环括号里加加
		{
			if (temp & 0x80)
				draw_point(x, y, !mode);
			else
				draw_point(x, y, mode);
			temp <<= 1;
			y++;
			t1++;		 // 若t1置于循环括号里加加，下面的判断就错过一个循环周期
			if (t1 == 4) // 如果取字模超半个字节，则
			{
				y = y0 + 8; // 纵坐标复位
				x++;		// 模坐档加1
			}
		}
		x++; // 完成一个字节取模模坐标加1
	}

}


void LCD_ST7567::show_full_screen_numbers(void)
{
    uint8_t x, y;
    uint8_t num = 0;
    
    // 清空缓存（避免残留）
    clear();
    
    // 循环填充屏幕：y轴按12像素步长（6x12点阵高度），x轴按8像素步长（预留间距）
    // 128列 ÷ 8列/个 = 16个/行；64行 ÷ 12行/行 = 5行（共80个数字，铺满屏幕）
    for (y = 0; y < 60; y += 12)
    {
        for (x = 0; x < 128; x += 8)
        {
            // 显示0-9循环的数字，正常模式（mode=0）
            show_char(x, y, '0' + (num % 10), 0);
            num++;
        }
    }
    
    // 刷新缓存到屏幕（关键：必须执行）
    refresh_gram();
}
