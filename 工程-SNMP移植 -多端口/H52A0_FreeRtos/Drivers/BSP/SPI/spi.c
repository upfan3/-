/************************************************
 * WKS GD32F470ZIT6核心板
 * SPI 驱动代码
 * 版本：V1.0
 ************************************************/

#include "./BSP/SPI/spi.h"

/**
 * @brief       SPI4初始化代码
 *   @note      主机模式,8位数据,禁止硬件片选
 * @param       无
 * @retval      无
 */
void spi4_init(void)
{
	spi_parameter_struct spi_init_struct;

	rcu_periph_clock_enable(RCU_GPIOF); /* GPIOF时钟使能 */
	rcu_periph_clock_enable(RCU_SPI4);	/* SPI4时钟使能 */

	/* 设置PF7,8,9  复用推挽输出 */
	gpio_af_set(GPIOF, GPIO_AF_5, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
	gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
	gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);

	spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;			/* 全双工模式 */
	spi_init_struct.device_mode = SPI_MASTER;						/* 主机模式 */
	spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;				/* 8位数据帧格式 */
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE; /* 空闲状态下,CLK保持高电平,在第二个时钟跳变沿采集第一个数据 */
	spi_init_struct.nss = SPI_NSS_SOFT;								/* NSS软件模式,NSS电平取决于SWNSS位 */
	spi_init_struct.prescale = SPI_PSC_256;							/* 默认使用256分频, 速度最低 */
	spi_init_struct.endian = SPI_ENDIAN_MSB;						/* MSB先传输 */
	spi_init(SPI4, &spi_init_struct);

	spi_enable(SPI4); /* 使能SPI4 */
}

/**
 * @brief       SPI4速度设置函数
 *   @note      SPI4时钟选择来自APB2, 即PCLK2, 为120Mhz
 *              SPI速度 = PCLK2 / 2^(speed + 1)
 * @param       speed   : SPI4时钟分频系数
 * @retval      无
 */
void spi4_set_speed(uint8_t speed)
{
	speed &= 0X07;				  /* 限制范围 */
	spi_disable(SPI4);			  /* SPI4失能 */
	SPI_CTL0(SPI4) &= ~(7 << 3);  /* 先清零 */
	SPI_CTL0(SPI4) |= speed << 3; /* 设置分频系数 */
	spi_enable(SPI4);			  /* SPI4使能 */
}

/**
 * @brief       SPI4读写一个字节数据
 * @param       txdata  : 要发送的数据(1字节)
 * @retval      接收到的数据(1字节)
 */
uint8_t spi4_read_write_byte(uint8_t txdata)
{
	while (RESET == spi_i2s_flag_get(SPI4, SPI_FLAG_TBE))
		; /* 等待发送缓冲区空 */

	spi_i2s_data_transmit(SPI4, txdata); /* 发送一个字节 */

	while (RESET == spi_i2s_flag_get(SPI4, SPI_FLAG_RBNE))
		; /* 等待接收缓冲区非空 */

	return spi_i2s_data_receive(SPI4); /* 返回收到的数据 */
}
