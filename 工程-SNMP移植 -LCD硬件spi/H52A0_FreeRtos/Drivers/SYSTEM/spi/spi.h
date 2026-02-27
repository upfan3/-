#ifndef SPI_H
#define SPI_H

#include "gd32f4xx.h"

#define SPI_TRANSFER_SUCCESS 0x00
#define SPI_SEND_TIMEOUT_ERROR (int8_t)-1
#define SPI_RECEIVE_TIMEOUT_ERROR (int8_t)-2
#define SPI_TRANSMIT_ERROR (int8_t)-3
#define SPI_TIMEOUT 1000 // 超时时间，单位：循环次数

/* SPI总线速度设置 */
#define SPI_SPEED_2 0
#define SPI_SPEED_4 1
#define SPI_SPEED_8 2
#define SPI_SPEED_16 3
#define SPI_SPEED_32 4
#define SPI_SPEED_64 5
#define SPI_SPEED_128 6
#define SPI_SPEED_256 7

// SPI端口枚举
typedef enum
{
    SPI_0 = 0,
    SPI_1,
    SPI_2,
    SPI_3,
    SPI_4,
    SPI_MAX
} SPI_Port;

typedef enum
{
    NOUSE_DMA = 0,
    USE_TX_DMA,
    USE_RX_DMA,
    USE_ALL_DMA
} SPI_TX_RX;

class SPI
{
private:
    SPI_Port m_port;
    uint32_t m_spi;
    rcu_periph_enum m_periph;
    uint8_t m_isUseDMA;

    // DMA相关参数
    uint32_t m_dmaPeriph;
    dma_channel_enum m_txDmaChannel;
    dma_channel_enum m_rxDmaChannel;
    dma_subperipheral_enum m_txSub_periph;
    dma_subperipheral_enum m_rxSub_periph;
    uint8_t m_dma_tx_nvic_irq;
    uint8_t m_dma_rx_nvic_irq;

public:
    SPI(SPI_Port port);
    ~SPI(void);

    void init(uint8_t isUseDMA = NOUSE_DMA);
    // 静态实例数组，用于中断处理
    static SPI *s_instances[SPI_MAX];
    spi_parameter_struct m_spi_init_struct;
    void set_speed(uint8_t speed);
    uint8_t read_write_byte(uint8_t txdata, uint8_t *rxdata = NULL);

    uint8_t *m_txbuff;
    uint8_t *m_rxbuff;
};

#endif // SPI_H
