#include "uart.h"
#include "string.h"

template <uint32_t BUFFER_SIZE>
UART<BUFFER_SIZE>::UART(UART_Port port) : m_port(port), m_baudrate(0), m_usart(0),
                                          m_txDmaChannel(DMA_CH0), m_rxDmaChannel(DMA_CH0), m_dmaPeriph(0),
                                          m_txSub_periph(DMA_SUBPERI0), m_rxSub_periph(DMA_SUBPERI0),
                                          m_rs485Gpio(0), m_rs485TxEnPin(0),
                                          m_receiveLength(0), m_receiveComplete(false),
                                          m_transmitComplete(true), // 初始状态为发送完成
                                          m_receiveCallback(0), m_transmitCompleteCallback(0),
                                          m_receiveUserData(0), m_transmitUserData(0)
{
    // 保存实例指针，用于中断处理
    if (port < UART_MAX)
    {
        s_instances[port] = this; // 将当前实例注册到静态数组
    }
}

template <uint32_t BUFFER_SIZE>
UART<BUFFER_SIZE>::~UART()
{
    // 清除实例指针
    if (m_port < UART_MAX)
    {
        s_instances[m_port] = 0;
    }
}

template <uint32_t BUFFER_SIZE>
bool UART<BUFFER_SIZE>::init(uint32_t baudrate, DataBits databits, StopBits stopbits, Parity parity, FlowControl flowcontrol)
{
    m_baudrate = baudrate;

    // 使能相应的UART时钟并设置USART基地址和DMA通道
    switch (m_port)
    {
    case UART_0:
        rcu_periph_clock_enable(RCU_USART0);
        m_usart = USART0;
        m_txDmaChannel = DMA_CH7;
        m_rxDmaChannel = DMA_CH5;

        m_txSub_periph = DMA_SUBPERI4;
        m_rxSub_periph = DMA_SUBPERI4;

        m_dmaPeriph = DMA1;
        break;
    case UART_1:
        rcu_periph_clock_enable(RCU_USART1);
        m_usart = USART1;
        m_txDmaChannel = DMA_CH6;
        m_rxDmaChannel = DMA_CH5;

        m_txSub_periph = DMA_SUBPERI4;
        m_rxSub_periph = DMA_SUBPERI4;
        m_dmaPeriph = DMA0;
        break;
    case UART_2:
        rcu_periph_clock_enable(RCU_USART2);
        m_usart = USART2;
        m_txDmaChannel = DMA_CH3;
        m_rxDmaChannel = DMA_CH1;

        m_txSub_periph = DMA_SUBPERI4;
        m_rxSub_periph = DMA_SUBPERI4;
        m_dmaPeriph = DMA0;
        break;
    case UART_3:
        rcu_periph_clock_enable(RCU_UART3);
        m_usart = UART3;
        m_txDmaChannel = DMA_CH4;
        m_rxDmaChannel = DMA_CH2;

        m_txSub_periph = DMA_SUBPERI4;
        m_rxSub_periph = DMA_SUBPERI4;

        m_dmaPeriph = DMA0;
        break;
    case UART_4:
        rcu_periph_clock_enable(RCU_UART4);
        m_usart = UART4;
        m_txDmaChannel = DMA_CH7;
        m_rxDmaChannel = DMA_CH0;

        m_txSub_periph = DMA_SUBPERI4;
        m_rxSub_periph = DMA_SUBPERI4;

        m_dmaPeriph = DMA0;
        break;
    case UART_5:
        rcu_periph_clock_enable(RCU_USART5);
        m_usart = USART5;
        m_txDmaChannel = DMA_CH6;
        m_rxDmaChannel = DMA_CH2;

        m_txSub_periph = DMA_SUBPERI5;
        m_rxSub_periph = DMA_SUBPERI5;

        m_dmaPeriph = DMA1;
        break;

    default:
        return false;
    }

    // 初始化GPIO
    initGPIO();

    // 配置UART参数
    usart_deinit(m_usart);
    usart_baudrate_set(m_usart, baudrate);

    // 配置数据位
    switch (databits)
    {
    case DATA_BITS_8:
        usart_word_length_set(m_usart, USART_WL_8BIT);
        break;
    case DATA_BITS_9:
        usart_word_length_set(m_usart, USART_WL_9BIT);
        break;
    }

    // 配置停止位
    switch (stopbits)
    {
    case STOP_BITS_1:
        usart_stop_bit_set(m_usart, USART_STB_1BIT);
        break;
    case STOP_BITS_0_5:
        usart_stop_bit_set(m_usart, USART_STB_0_5BIT);
        break;
    case STOP_BITS_2:
        usart_stop_bit_set(m_usart, USART_STB_2BIT);
        break;
    case STOP_BITS_1_5:
        usart_stop_bit_set(m_usart, USART_STB_1_5BIT);
        break;
    }

    // 配置校验位
    switch (parity)
    {
    case PARITY_NONE:
        usart_parity_config(m_usart, USART_PM_NONE);
        break;
    case PARITY_ODD:
        usart_parity_config(m_usart, USART_PM_ODD);
        break;
    case PARITY_EVEN:
        usart_parity_config(m_usart, USART_PM_EVEN);
        break;
    }

    // 配置流控制
    switch (flowcontrol)
    {
    case FLOW_CONTROL_NONE:
        usart_hardware_flow_cts_config(m_usart, USART_CTS_DISABLE);
        usart_hardware_flow_rts_config(m_usart, USART_RTS_DISABLE);
        break;
    case FLOW_CONTROL_RTS:
        usart_hardware_flow_cts_config(m_usart, USART_CTS_DISABLE);
        usart_hardware_flow_rts_config(m_usart, USART_RTS_ENABLE);
        break;
    case FLOW_CONTROL_CTS:
        usart_hardware_flow_cts_config(m_usart, USART_CTS_ENABLE);
        usart_hardware_flow_rts_config(m_usart, USART_RTS_DISABLE);
        break;
    case FLOW_CONTROL_RTS_CTS:
        usart_hardware_flow_cts_config(m_usart, USART_CTS_ENABLE);
        usart_hardware_flow_rts_config(m_usart, USART_RTS_ENABLE);
        break;
    }

    // 使能发送和接收
    usart_transmit_config(m_usart, USART_TRANSMIT_ENABLE);
    usart_receive_config(m_usart, USART_RECEIVE_ENABLE);

    // 初始化DMA
    initDMA();

    // 初始化中断
    initInterrupt();

    // 使能UART
    usart_enable(m_usart);

    return true;
}

template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::initGPIO()
{
    // 根据不同的UART端口配置相应的GPIO时钟和引脚
    switch (m_port)
    {

    case UART_0:
        rcu_periph_clock_enable(RCU_GPIOA);
        // 配置PA9 (TX) 和 PA10 (RX)
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9 | GPIO_PIN_10);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
        gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9 | GPIO_PIN_10);
        break;

    case UART_1:
        rcu_periph_clock_enable(RCU_GPIOD);
        // 配置PD5 (TX) 和 PD6 (RX)
        gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_5 | GPIO_PIN_6);
        gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
        gpio_af_set(GPIOD, GPIO_AF_7, GPIO_PIN_5 | GPIO_PIN_6);

        m_rs485Gpio = GPIOD;
        m_rs485TxEnPin = GPIO_PIN_4;
        gpio_mode_set(m_rs485Gpio, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, m_rs485TxEnPin);
        gpio_output_options_set(m_rs485Gpio, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, m_rs485TxEnPin);

        break;

    case UART_2:
        rcu_periph_clock_enable(RCU_GPIOC);
        // 配置PC10 (TX) 和 PC11 (RX)
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10 | GPIO_PIN_11);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
        gpio_af_set(GPIOC, GPIO_AF_7, GPIO_PIN_10 | GPIO_PIN_11);

        m_rs485Gpio = GPIOC;
        m_rs485TxEnPin = GPIO_PIN_12;
        gpio_mode_set(m_rs485Gpio, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, m_rs485TxEnPin);
        gpio_output_options_set(m_rs485Gpio, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, m_rs485TxEnPin);
        break;

    case UART_3:
        rcu_periph_clock_enable(RCU_GPIOA);
        // 配置PA0 (TX) 和 PA1 (RX)
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0 | GPIO_PIN_1);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
        gpio_af_set(GPIOA, GPIO_AF_8, GPIO_PIN_0 | GPIO_PIN_1);
        break;

    case UART_4:
        rcu_periph_clock_enable(RCU_GPIOC);
        rcu_periph_clock_enable(RCU_GPIOD);
        // 配置PC12 (TX) 和 PD2 (RX)
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);
        gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
        gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_12);
        gpio_af_set(GPIOD, GPIO_AF_8, GPIO_PIN_2);
        break;

    case UART_5:
        rcu_periph_clock_enable(RCU_GPIOG);
        // 配置PG14 (TX) 和 PG9 (RX)
        gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_14 | GPIO_PIN_9);
        gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
        gpio_af_set(GPIOG, GPIO_AF_8, GPIO_PIN_14 | GPIO_PIN_9);

        m_rs485Gpio = GPIOG;
        m_rs485TxEnPin = GPIO_PIN_13;
        gpio_mode_set(m_rs485Gpio, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, m_rs485TxEnPin);
        gpio_output_options_set(m_rs485Gpio, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, m_rs485TxEnPin);

        break;

    // 其他UART端口的GPIO配置可以根据数据手册添加
    default:
        break;
    }
}

template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::initDMA()
{
    // 使能相应的DMA时钟
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

    // 配置发送DMA
    dma_deinit(m_dmaPeriph, m_txDmaChannel);

    dma_single_data_parameter_struct dma_init_struct;

    dma_single_data_para_struct_init(&dma_init_struct);

    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;        /*   DMA通道数据传输方向为从存储器到外设 */
    dma_init_struct.memory0_addr = (uint32_t)0;              /* DMA 存储器0基地址*/
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE; /* 存储器地址增量模式 */

    dma_init_struct.periph_addr = (uint32_t)(m_usart + 0x04); // USART_DATA寄存器偏移地址
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE; /* 外设地址固定模式 */

    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT; /* 外设数据传输宽度:8位 */

    dma_init_struct.priority = DMA_PRIORITY_MEDIUM; /* 优先级中 */
    dma_init_struct.number = 0;                     /* DMA通道数据传输数量清零, 后续在dma_enable函数设置 */
    dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;

    dma_single_data_mode_init(m_dmaPeriph, m_txDmaChannel, &dma_init_struct);

    dma_channel_subperipheral_select(m_dmaPeriph, m_txDmaChannel, m_txSub_periph); /* DMA通道外设选择 */

    // 配置接收DMA
    dma_deinit(m_dmaPeriph, m_rxDmaChannel);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;         /* DMA通道数据传输方向为从外设到存储器 */
    dma_init_struct.memory0_addr = (uint32_t)m_receiveBuffer; /* DMA 存储器0基地址*/
    dma_init_struct.number = BUFFER_SIZE;
    dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE; /* 关闭循环模式 */

    dma_single_data_mode_init(m_dmaPeriph, m_rxDmaChannel, &dma_init_struct);
    dma_channel_subperipheral_select(m_dmaPeriph, m_rxDmaChannel, m_rxSub_periph); /* DMA通道外设选择 */

    // 使能接收DMA
    dma_channel_enable(m_dmaPeriph, m_rxDmaChannel);

    // 使能UART的DMA发送和接收

    usart_dma_receive_config(m_usart, USART_RECEIVE_DMA_ENABLE);
}

template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::initInterrupt()
{
    // 配置中断优先级
    uint8_t irqNum = 0;
    switch (m_port)
    {
    case UART_0:
        irqNum = USART0_IRQn;
        break;
    case UART_1:
        irqNum = USART1_IRQn;
        break;
    case UART_2:
        irqNum = USART2_IRQn;
        break;
    case UART_3:
        irqNum = UART3_IRQn;
        break;
    case UART_4:
        irqNum = UART4_IRQn;
        break;
    case UART_5:
        irqNum = USART5_IRQn;
        break;
    default:
        return;
    }

    // 设置中断优先级
    nvic_irq_enable(irqNum, 2, 0);

    // 使能空闲中断
    usart_interrupt_enable(m_usart, USART_INT_IDLE); // 使能空闲中断
    usart_interrupt_enable(m_usart, USART_INT_TC);   // 使能发送完成中断
}

template <uint32_t BUFFER_SIZE>
bool UART<BUFFER_SIZE>::send(const uint8_t *data, uint32_t length)
{
    if (isSendComplete() && data && length > 0 && length <= TRANSMIT_MAX_SIZE)
    {
        // 禁用发送DMA
        dma_channel_disable(m_dmaPeriph, m_txDmaChannel);

        // 清除标志位
        dma_flag_clear(m_dmaPeriph, m_txDmaChannel, DMA_FLAG_FTF);
        // 配置新的DMA请求源
        // dma_channel_subperipheral_select(m_dmaPeriph, m_txDmaChannel, m_txSub_periph);  /* DMA通道外设选择 */
        // 重置发送完成标志
        m_transmitComplete = false;
        // 设置发送数据地址
        dma_memory_address_config(m_dmaPeriph, m_txDmaChannel, DMA_MEMORY_0, (uint32_t)data);

        // 设置发送数据长度
        dma_transfer_number_config(m_dmaPeriph, m_txDmaChannel, length);

        // 使能发送DMA
        rs485EnableTransmit();                                         // 若为RS485接口，设置为发送模式
        usart_dma_transmit_config(m_usart, USART_TRANSMIT_DMA_ENABLE); // 使能串口DMA发送
        dma_channel_enable(m_dmaPeriph, m_txDmaChannel);               // 使能DMA通道
        return true;
    }
    return false;
}

template <uint32_t BUFFER_SIZE>
bool UART<BUFFER_SIZE>::isSendComplete() const
{
    // 检查DMA是否已把数据移到串口发送寄存器中。
    // return dma_flag_get(m_dmaPeriph, m_txDmaChannel, DMA_FLAG_FTF) == SET;
    return m_transmitComplete;
}

template <uint32_t BUFFER_SIZE>
uint32_t UART<BUFFER_SIZE>::getReceivedData(uint8_t *buffer, uint32_t bufferSize)
{
    if (!buffer || bufferSize == 0)
        return 0;

    uint32_t length = 0;
    if (m_receiveComplete)
    {
        // 确保不超过缓冲区大小
        length = (m_receiveLength > bufferSize) ? bufferSize : m_receiveLength;

        // 复制数据
        memcpy(buffer, m_receiveBuffer, length);

        // 清除接收标志和缓冲区
        m_receiveComplete = false;
        m_receiveLength = 0;
        memset(m_receiveBuffer, 0, BUFFER_SIZE);

        // 禁用DMA接收
        dma_channel_disable(m_dmaPeriph, m_rxDmaChannel); /* 关闭DMA通道 */

        if (dma_flag_get(m_dmaPeriph, m_rxDmaChannel, DMA_FLAG_FTF) == SET) // 清除DMA接收标志
        {
            dma_flag_clear(m_dmaPeriph, m_rxDmaChannel, DMA_FLAG_FTF);
        }
        dma_transfer_number_config(m_dmaPeriph, m_rxDmaChannel, BUFFER_SIZE); // 更新接收长度
        dma_channel_enable(m_dmaPeriph, m_rxDmaChannel);
    }

    return length;
}

template <uint32_t BUFFER_SIZE>
bool UART<BUFFER_SIZE>::getReceiveLenth(uint16_t *length)
{
    if (m_receiveComplete)
    {
        *length = m_receiveLength;
        return true;
    }
    return false;
}
template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::clearReceiveBuffer()
{
    m_receiveComplete = false;
    m_receiveLength = 0;
    memset(m_receiveBuffer, 0, BUFFER_SIZE);

    // 重新启动DMA接收
    dma_channel_disable(m_dmaPeriph, m_rxDmaChannel);
    dma_transfer_number_config(m_dmaPeriph, m_rxDmaChannel, BUFFER_SIZE);
    dma_channel_enable(m_dmaPeriph, m_rxDmaChannel);
}

template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::setRs485ControlPins(uint32_t gpio_periph, uint32_t tx_en_pin)
{
    m_rs485Gpio = gpio_periph;
    m_rs485TxEnPin = tx_en_pin;

    // 配置RS485控制引脚为输出
    if (m_rs485Gpio && m_rs485TxEnPin)
    {
        // 使能GPIO时钟
        if (m_rs485Gpio == GPIOA)
            rcu_periph_clock_enable(RCU_GPIOA);
        else if (m_rs485Gpio == GPIOB)
            rcu_periph_clock_enable(RCU_GPIOB);
        else if (m_rs485Gpio == GPIOC)
            rcu_periph_clock_enable(RCU_GPIOC);
        else if (m_rs485Gpio == GPIOD)
            rcu_periph_clock_enable(RCU_GPIOD);
        else if (m_rs485Gpio == GPIOE)
            rcu_periph_clock_enable(RCU_GPIOE);
        else if (m_rs485Gpio == GPIOF)
            rcu_periph_clock_enable(RCU_GPIOF);
        else if (m_rs485Gpio == GPIOG)
            rcu_periph_clock_enable(RCU_GPIOG);

        gpio_mode_set(m_rs485Gpio, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, m_rs485TxEnPin);
        gpio_output_options_set(m_rs485Gpio, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, m_rs485TxEnPin);

        // 默认设置为接收模式
        rs485EnableReceive();
    }
}

template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::rs485EnableTransmit()
{
    if (m_rs485Gpio && m_rs485TxEnPin)
    {
        gpio_bit_set(m_rs485Gpio, m_rs485TxEnPin);
    }
}

template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::rs485EnableReceive()
{
    if (m_rs485Gpio && m_rs485TxEnPin)
    {
        gpio_bit_reset(m_rs485Gpio, m_rs485TxEnPin);
    }
}

template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::setReceiveCallback(ReceiveCallback callback, void *userData)
{
    m_receiveCallback = callback;
    m_receiveUserData = userData;
}

template <uint32_t BUFFER_SIZE>
void UART<BUFFER_SIZE>::handleInterrupt()
{

    // 检查发送完成中断标志
    if (usart_interrupt_flag_get(m_usart, USART_INT_FLAG_TC) != RESET)
    {
        // 清除发送完成中断标志
        usart_interrupt_flag_clear(m_usart, USART_INT_FLAG_TC);

        // 设置发送完成标志
        m_transmitComplete = true;

        // 关闭DMA串口发送
        usart_dma_transmit_config(m_usart, USART_TRANSMIT_DMA_DISABLE);

        // 如果使用RS485，自动切换到接收模式
        rs485EnableReceive();

        // 调用发送完成回调函数
        if (m_transmitCompleteCallback)
        {
            m_transmitCompleteCallback(m_transmitUserData);
        }
    }
    // 检查空闲中断标志
    if (usart_interrupt_flag_get(m_usart, USART_INT_FLAG_IDLE) != RESET)
    {
        // 清除空闲中断标志
        usart_data_receive(m_usart); // 读取数据寄存器清除标志
        usart_interrupt_flag_clear(m_usart, USART_INT_FLAG_IDLE);

        // 计算接收到的数据长度
        m_receiveLength = BUFFER_SIZE - dma_transfer_number_get(m_dmaPeriph, m_rxDmaChannel);

        // 禁用DMA接收
        //        dma_channel_disable(m_dmaPeriph, m_rxDmaChannel);  /* 关闭DMA通道 */
        //
        //
        //
        //			 if(dma_flag_get(m_dmaPeriph, m_rxDmaChannel, DMA_FLAG_FTF) == SET)//清除DMA接收标志
        //								 {
        //											dma_flag_clear(m_dmaPeriph, m_rxDmaChannel, DMA_FLAG_FTF);
        //								 }
        //         dma_transfer_number_config(m_dmaPeriph, m_rxDmaChannel, BUFFER_SIZE);		//更新接收长度
        //         dma_channel_enable(m_dmaPeriph, m_rxDmaChannel);

        // 设置接收完成标志
        m_receiveComplete = true;

        // 如果设置了回调函数，则调用
        if (m_receiveCallback)
        {
            m_receiveCallback(m_receiveUserData);
        }
    }
}
