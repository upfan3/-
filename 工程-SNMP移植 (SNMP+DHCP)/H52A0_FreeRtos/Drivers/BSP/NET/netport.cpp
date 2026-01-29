#include "./SYSTEM/SPI/spi.h"
#include "./SYSTEM/delay/delay.h"

//	/******w5500Òý½Å¶¨Òå************
#define WIZ_SCS_RESET_PORT GPIOG
#define WIZ_SCS GPIO_PIN_11 //
#define WIZ_RESET_PIN GPIO_PIN_10

// #define WIZ_INT			  GPIO_PIN_7	//GPIO_Pin_7	// PD7

SPI spi2(SPI_2);

extern "C"
{
	void net_port_init(void)
	{

		rcu_periph_clock_enable(RCU_GPIOG); /*D/C */
		gpio_mode_set(WIZ_SCS_RESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, WIZ_SCS | WIZ_RESET_PIN);
		gpio_output_options_set(WIZ_SCS_RESET_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, WIZ_SCS | WIZ_RESET_PIN);

		spi2.init();
	}

	uint8_t net_read_write(uint8_t tx, uint8_t *rx)
	{
		spi2.read_write_byte(tx, rx);
	}

	void net_scs_ctrl(uint8_t val)
	{
		if (val == 0)
		{
			gpio_bit_reset(WIZ_SCS_RESET_PORT, WIZ_SCS);
		}
		else if (val == 1)
		{
			gpio_bit_set(WIZ_SCS_RESET_PORT, WIZ_SCS);
		}
	}

	void net_reset(void)
	{
		gpio_bit_reset(WIZ_SCS_RESET_PORT, WIZ_RESET_PIN);
		delay_us(2);
		gpio_bit_set(WIZ_SCS_RESET_PORT, WIZ_RESET_PIN);
		delay_ms(160);
	}
}