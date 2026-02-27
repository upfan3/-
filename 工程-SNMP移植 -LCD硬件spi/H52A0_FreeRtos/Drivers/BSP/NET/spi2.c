#include "Tqueue.h"
#include "config.h"

void w5500_dma_init(void)
{

	rcu_periph_clock_enable(RCU_DMA0);
	dma_deinit(DMA0, DMA_CH1);
	dma_deinit(DMA0, DMA_CH2);

	dma_channel_disable(DMA0, DMA_CH1);
	dma_channel_disable(DMA0, DMA_CH2);
}

void w5500_gpio_init(void)
{

	/******w5500引脚定义************
		SPI2_CK	 89(PB3)
		SPI2_MI	 90(PB4)
		SPI2_MO	 91(PB5)

	  W5500_SCSN   64(PC7)
	  W5500_INT    65(PC8)
	  W5500_RST    66(PC9)
	*******************************/
	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_SPI2);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);

	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

	gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_9);

	gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
}

void WIZ_SPI_Init(void)
{

	w5500_gpio_init();
	w5500_dma_init();

	spi_parameter_struct spi_init_struct;
	spi_i2s_deinit(SPI2);
	spi_struct_para_init(&spi_init_struct);

	/* SPI Config -------------------------------------------------------------*/
	spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode = SPI_MASTER;
	spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE; // SPI_CK_PL_HIGH_PH_2EDGE;
	spi_init_struct.nss = SPI_NSS_SOFT;
	spi_init_struct.prescale = SPI_PSC_2;
	spi_init_struct.endian = SPI_ENDIAN_MSB;
	spi_init(SPI2, &spi_init_struct);

	while (spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET)
	{
		;
	} // 等待发送寄存器为空
	spi_enable(SPI2);
}

void WIZ_CS(uint8_t val)
{
	if (val == LOW)
	{

		gpio_bit_reset(GPIOC, WIZ_SCS);
	}
	else if (val == HIGH)
	{

		gpio_bit_set(GPIOC, WIZ_SCS);
	}
}

uint8_t SPI2_SendByte(uint8_t byte)
{
	while (spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET)
		; // 等待发送寄存器为空
	spi_i2s_data_transmit(SPI2, byte);
	while (spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE) == RESET)
		; // 等待送发寄存器不为空
	return (uint8)spi_i2s_data_receive(SPI2);
}
