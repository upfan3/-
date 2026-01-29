#include "oled.h"
#include "f1216.h"
#include "./SYSTEM/SPI/spi.h"
#include "./SYSTEM/delay/delay.h"

SPI spi3(SPI_3);

OLED::OLED(void)
{
}

OLED::~OLED()
{
}

void OLED::init()
{
	rcu_periph_clock_enable(RCU_GPIOE); /*D/C */
	gpio_mode_set(OLED_RS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, OLED_RS_PIN);
	gpio_output_options_set(OLED_RS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, OLED_RS_PIN);

	rcu_periph_clock_enable(RCU_GPIOE); /* RST */
	gpio_mode_set(OLED_RST_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, OLED_RST_PIN);
	gpio_output_options_set(OLED_RST_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, OLED_RST_PIN);

	spi3.init();

	Clr_OLED_RST;
	delay_ms(10);
	Set_OLED_RST;
	delay_ms(10);

	write_cmd(0xae); // Set Display Off
	write_cmd(0xd5); // Display divide ratio/osc. freq. mode
	write_cmd(0x80);
	write_cmd(0xa8); // Multiplex ration mode:63
	write_cmd(0x3f);
	write_cmd(0xd3); // Set Display Offset
	write_cmd(0x00);
	write_cmd(0x40); // Set Display Start Line
	write_cmd(0xad); // DC-DC Control Mode Set
	write_cmd(0x8a); // 0X8A: OFF 0X8B: ON

	write_cmd(0xa0); // 设置屏幕翻180,
	write_cmd(0xc0); // 设置屏幕翻180,
	write_cmd(0xda);
	write_cmd(0x12); // Common pads hardware: alternative
	write_cmd(0x81); // Contrast control
	write_cmd(0xe0);
	write_cmd(0xd9); // Set pre-charge period
	write_cmd(0x22);

	write_cmd(0xdb); // VCOM deselect level mode
	write_cmd(0x18);
	write_cmd(0xa4); // 设置完整显示
	write_cmd(0xa6); // 设置正常显示
	write_cmd(0xaf); // 设置显示开

	clear();
	refresh_gram();
}

/*************************************************
名称：write_cmd
功能描述:OLED写命令
输入:OLED命令
返回:SPI异常码
*************************************************/

uint8_t OLED::write_cmd(uint8_t cmd)
{
	uint8_t res;
	Clr_OLED_RS; // RS 为0时写命令

	res = spi3.read_write_byte(cmd);

	Set_OLED_RS;

	return res;
}

/*************************************************
名称：write_data
功能描述:OLED写数据
输入::OLED数据
返回:SPI异常码
*************************************************/
uint8_t OLED::write_data(uint8_t dat)
{
	uint8_t res;
	Set_OLED_RS;
	; // RS 为1时写数据
	res = spi3.read_write_byte(dat);
	Set_OLED_RS;
	return res;
}
/*************************************************
名称：refresh_gram
功能描述:刷新显示到OLED
输入::
返回:
*************************************************/
void OLED::refresh_gram(void)
{
	uint8_t i, n;
	for (i = 0; i < 8; i++)
	{
		write_cmd(0xb0 + i); // 设置页地址（0~7）
		write_cmd(0x02);	 // 设置显示位置—列低地址,偏移了2列
		write_cmd(0x10);	 // 设置显示位置—列高地址
		for (n = 0; n < 128; n++)
			write_data(GRAM[n][i]); // 128
	}
}

/*************************************************
名称：clear
功能描述:清显示缓存，调用OLED_Refresh_Gram后，清屏
输入:
返回:无
*************************************************/
void OLED::clear(void)
{
	uint8_t i, n;
	for (i = 0; i < 8; i++)
	{
		for (n = 0; n < 128; n++) // 128
		{
			GRAM[n][i] = 0X00;
		}
	}
}

/*************************************************
名称：OLED_ShowChar
功能描述: 在显示缓存指定的坐标上显示一个字符,调用
OLED_Refresh_Gram后，在屏幕上显示一个字符
输入:  x:0~127,y:0~63,t:1 正常显示0,反白显示,
size:选择字体 16/12
返回:无
*************************************************/

void OLED::show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
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

void OLED::draw_point(uint8_t x, uint8_t y, uint8_t t)
{
	uint8_t pos, bx, temp = 0;
	if (x > 127 || y > 63)
		return; // 超出范围了.
	pos = 7 - y / 8;
	bx = y % 8;
	temp = 1 << (7 - bx);
	if (t)
		GRAM[127 - x][pos] |= temp;
	else
		GRAM[127 - x][pos] &= ~temp;
}
