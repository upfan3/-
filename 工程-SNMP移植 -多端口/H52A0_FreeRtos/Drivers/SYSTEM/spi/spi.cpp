#include "spi.h"

SPI *SPI::s_instances[SPI_MAX] = {0};

SPI::SPI(SPI_Port port) : m_port(port),
						  m_txDmaChannel(DMA_CH0), m_rxDmaChannel(DMA_CH0), m_dmaPeriph(0),
						  m_txSub_periph(DMA_SUBPERI0), m_rxSub_periph(DMA_SUBPERI0), m_isUseDMA(0)
{
	// 保存实例指针，用于中断处理
	if (port < SPI_MAX)
	{
		s_instances[port] = this; // 将当前实例注册到静态数组
	}
};

void SPI::init(uint8_t isUseDMA)
{

	if (isUseDMA > 0)
	{
		switch (m_dmaPeriph)
		{
		case DMA0:
			rcu_periph_clock_enable(RCU_DMA0);
			break;
		case DMA1:
			rcu_periph_clock_enable(RCU_DMA1);
			break;

		default:
			break;
		}
	}

	if ((isUseDMA & 0x01) == USE_TX_DMA)
	{

		dma_single_data_parameter_struct dma_init_struct;

		dma_deinit(m_dmaPeriph, m_txDmaChannel);

		dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;

		dma_init_struct.memory0_addr = (uint32_t)m_txbuff;

		dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;

		dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(m_dmaPeriph);

		dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;

		dma_init_struct.periph_memory_width = DMA_MEMORY_WIDTH_8BIT;

		dma_init_struct.number = 0;

		dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;

		dma_init_struct.priority = DMA_PRIORITY_MEDIUM;

		dma_single_data_mode_init(m_dmaPeriph, m_txDmaChannel, &dma_init_struct);

		dma_channel_subperipheral_select(m_dmaPeriph, m_txDmaChannel, m_txSub_periph);

		dma_channel_disable(m_dmaPeriph, m_txDmaChannel);

		// 使能DMA中断通道
		nvic_irq_enable(m_dma_tx_nvic_irq, 2, 0);

		dma_interrupt_enable(m_dmaPeriph, m_txDmaChannel, DMA_CHXCTL_FTFIE);
	}

	if ((isUseDMA & 0x02) == USE_RX_DMA)
	{
		dma_single_data_parameter_struct dma_init_struct;

		dma_deinit(m_dmaPeriph, m_txDmaChannel);

		dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;

		dma_init_struct.memory0_addr = (uint32_t)m_rxbuff;

		dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;

		dma_init_struct.periph_addr = (uint32_t)&SPI_DATA(m_dmaPeriph);

		dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;

		dma_init_struct.periph_memory_width = DMA_MEMORY_WIDTH_8BIT;

		dma_init_struct.number = 0;

		dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;

		dma_init_struct.priority = DMA_PRIORITY_MEDIUM;

		dma_single_data_mode_init(m_dmaPeriph, m_rxDmaChannel, &dma_init_struct);

		dma_channel_subperipheral_select(m_dmaPeriph, m_rxDmaChannel, m_rxSub_periph);

		dma_channel_disable(m_dmaPeriph, m_rxDmaChannel);

		// 使能DMA中断通道
		nvic_irq_enable(m_dma_rx_nvic_irq, 2, 0);

		dma_interrupt_enable(m_dmaPeriph, m_rxDmaChannel, DMA_CHXCTL_FTFIE);
	}

	switch (m_port)
	{
	case SPI_0:

		m_spi = SPI0;
		m_periph = RCU_SPI0;
		rcu_periph_clock_enable(RCU_GPIOB);									 /* GPIOF时钟使能 */
		gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5); // PB3(SCK),PB4(MISO),PB5(MOSI)
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

		break;
	case SPI_1:

		m_spi = SPI1;
		m_periph = RCU_SPI1;
		rcu_periph_clock_enable(RCU_GPIOB);										/* GPIOF时钟使能 */
		gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15); // PB10(SCK),PB14(MISO),PB15(MOSI)
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15);

		break;
	case SPI_2:

		m_spi = SPI2;
		m_periph = RCU_SPI2;
		m_txDmaChannel = DMA_CH7;
		m_rxDmaChannel = DMA_CH0;
		m_txSub_periph = DMA_SUBPERI0;
		m_rxSub_periph = DMA_SUBPERI0;
		m_dmaPeriph = DMA0;
		m_isUseDMA = USE_ALL_DMA;

		rcu_periph_clock_enable(RCU_GPIOB);									 /* GPIOF时钟使能 */
		gpio_af_set(GPIOB, GPIO_AF_6, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5); // PB3(SCK),PB4(MISO),PB5(MOSI)
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

		m_dma_tx_nvic_irq = DMA0_Channel7_IRQn;
		m_dma_tx_nvic_irq = DMA0_Channel0_IRQn;

		break;
	case SPI_3:
		m_spi = SPI3;
		m_periph = RCU_SPI3;
		m_txDmaChannel = DMA_CH1;
		m_rxDmaChannel = DMA_CH0;
		m_txSub_periph = DMA_SUBPERI4;
		m_rxSub_periph = DMA_SUBPERI4;
		m_dmaPeriph = DMA1;
		m_isUseDMA = USE_TX_DMA;

		rcu_periph_clock_enable(RCU_GPIOE);											   /* GPIOF时钟使能 */
		gpio_af_set(GPIOE, GPIO_AF_5, GPIO_PIN_2 | GPIO_PIN_6);						   // PE2(SCK),PE6(MOSI)
		gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2 | GPIO_PIN_6); // 只配置输出
		gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2 | GPIO_PIN_6);

		m_dma_tx_nvic_irq = DMA1_Channel1_IRQn;
		m_dma_tx_nvic_irq = DMA1_Channel0_IRQn;

		break;
	case SPI_4:

		m_spi = SPI4;
		m_periph = RCU_SPI4;
		m_txDmaChannel = DMA_CH4;
		m_rxDmaChannel = DMA_CH3;
		m_txSub_periph = DMA_SUBPERI2;
		m_rxSub_periph = DMA_SUBPERI2;
		m_dmaPeriph = DMA1;
		m_isUseDMA = USE_ALL_DMA;

		rcu_periph_clock_enable(RCU_GPIOF);									 /* GPIOF时钟使能 */
		gpio_af_set(GPIOF, GPIO_AF_5, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9); // PF7(SCK),PF8(MISO),PF9(MOSI)
		gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
		gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);

		m_dma_tx_nvic_irq = DMA1_Channel4_IRQn;
		m_dma_tx_nvic_irq = DMA1_Channel3_IRQn;

		break;

	default:

		m_spi = SPI0;
		m_periph = RCU_SPI0;
		rcu_periph_clock_enable(RCU_GPIOB);									 /* GPIOF时钟使能 */
		gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5); // PB3(SCK),PB4(MISO),PB5(MOSI)
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);

		break;
	}

	rcu_periph_clock_enable(m_periph);

	// spi_i2s_deinit(m_spi);
	m_spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;		  /* 全双工模式 */
	m_spi_init_struct.device_mode = SPI_MASTER;						  /* 主机模式 */
	m_spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;				  /* 8位数据帧格式 */
	m_spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE; /* 空闲状态下,CLK保持高电平,在第二个时钟跳变沿采集第一个数据 */
	m_spi_init_struct.nss = SPI_NSS_SOFT;							  /* NSS软件模式,NSS电平取决于SWNSS位 */
	m_spi_init_struct.prescale = SPI_PSC_256;						  /* 默认使用256分频, 速度最低 */
	m_spi_init_struct.endian = SPI_ENDIAN_MSB;						  /* MSB先传输 */

	spi_init(m_spi, &m_spi_init_struct);

	spi_enable(m_spi);

	//	  spi_parameter_struct spi_init_struct;
	//

	//	  spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;   /* 全双工模式 */
	//    spi_init_struct.device_mode          = SPI_MASTER;                 /* 主机模式 */
	//    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;         /* 8位数据帧格式 */
	//    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;    /* 空闲状态下,CLK保持高电平,在第二个时钟跳变沿采集第一个数据 */
	//    spi_init_struct.nss                  = SPI_NSS_SOFT;               /* NSS软件模式,NSS电平取决于SWNSS位 */
	//    spi_init_struct.prescale             = SPI_PSC_256;                /* 默认使用256分频, 速度最低 */
	//    spi_init_struct.endian               = SPI_ENDIAN_MSB;             /* MSB先传输 */
	//    spi_init(m_spi, &spi_init_struct);

	spi_enable(m_spi);
}

uint8_t SPI::read_write_byte(uint8_t txdata, uint8_t *rxdata)
{
	// 等待发送缓冲区空
	uint32_t send_timeout = SPI_TIMEOUT;
	while ((RESET == spi_i2s_flag_get(m_spi, SPI_FLAG_TBE)) && (send_timeout > 0))
	{
		send_timeout--;
	}
	if (send_timeout == 0)
	{
		// 发送超时，返回错误码
		return SPI_SEND_TIMEOUT_ERROR;
	}

	spi_i2s_data_transmit(m_spi, txdata);

	// 等待接收缓冲区非空
	uint32_t receive_timeout = SPI_TIMEOUT;
	while ((RESET == spi_i2s_flag_get(m_spi, SPI_FLAG_RBNE)) && (receive_timeout > 0))
	{
		receive_timeout--;
	}
	if (receive_timeout == 0)
	{
		// 接收超时，返回错误码
		return SPI_RECEIVE_TIMEOUT_ERROR;
	}

	// 设置接收数据
	uint8_t tmp;
	tmp = spi_i2s_data_receive(m_spi);
	if (rxdata != NULL)
		*rxdata = tmp;
	return SPI_TRANSFER_SUCCESS;

	//	  while(RESET == spi_i2s_flag_get(m_spi, SPI_FLAG_TBE));    /* 等待发送缓冲区空 */
	//
	//	  spi_i2s_data_transmit(m_spi, txdata);                     /* 发送一个字节 */
	//
	//	  while(RESET == spi_i2s_flag_get(m_spi, SPI_FLAG_RBNE));   /* 等待接收缓冲区非空 */
	//
	//	  *rxdata =spi_i2s_data_receive(m_spi);                       /* 返回收到的数据 */

	//   return SPI_TRANSFER_SUCCESS;
}

void SPI::set_speed(uint8_t speed)
{
	speed &= 0X07;				   /* 限制范围 */
	spi_disable(m_spi);			   /* SPI4失能 */
	SPI_CTL0(m_spi) &= ~(7 << 3);  /* 先清零 */
	SPI_CTL0(m_spi) |= speed << 3; /* 设置分频系数 */
	spi_enable(m_spi);			   /* SPI4使能 */
}

SPI::~SPI()
{
}