#include "can.h"

/*------------------------------------------------

复用功能组9
使用PB8(CAN0_RX),PB9(CAN0_TX)引脚
关闭定时触发
总线关断自恢复打开
关闭自动唤醒
打开发送失败重传
FIFO覆盖关闭
FIFO顺序发送关闭



波特率设定

1，首先确定系统时钟频率及其配置
  时钟配置时调用函数为 system_clock_240m_25m_hxtal(void) 可知
    其配置时钟频率如下：
    AHB 时钟：配置为系统时钟（SYSCLK）不分频，即RCU_AHB_CKSYS_DIV1。
  APB2 时钟：设置为AHB/2，即RCU_APB2_CKAHB_DIV2。
  APB1 时钟：设置为AHB/4，即RCU_APB1_CKAHB_DIV4。

2,由于CAN0属于 APB1总线上的外设，其使用的时钟为APB1的时钟 Fpclk = 240M/4 = 60M
3,CAN0的目标波特率设置为125Kbps 即钟周期是 1/125000 = 8us
4.因此，设置 can_parameter.prescaler=60; 对Fpclk60分频,使CAN0 工作在1MHz,时钟周其为 1/1MHz = 1us的频率下。
5，设置 can_parameter.time_segment_1 = CAN_BT_BS1_3TQ，can_parameter.time_segment_2 = CAN_BT_BS2_4TQ
即（1+CAN_BT_BS1_3TQ+CAN_BT_BS2_4TQ）=8us

设置公式如下：
设波特率为B,分频数为P, time_segment_1为a,time_segment_2为b

1/B = (1+a+b)*P/Fpclk => P*(1+a+b) = Fpclk/B = 60M/125K = 480 = 60*(1+3+4)
所以P=60 ,a = 3, b = 4
can_parameter.prescaler=60
又由于要求使a>b，且取样点要求在总时长的 70%处, 取样点为 a+1,总时为 T=(1+a+b)

因此有 a+1=0.7T => a = 0.7T-1 b =0.3T
T = 8us
a = 0.7*8-1=4.6 取5
b = 0.3*8 = 2.4 取2

can_parameter.time_segment_1 = CAN_BT_BS1_5TQ (注：这里是3取库函数中对应的宏）
can_parameter.time_segment_2 = CAN_BT_BS2_2TQ (注：这里是4取库函数中对应的宏）
在配置 P,a,b三个值时,应尽可使P为整数,防止不是整数分频。


使能CAN接收中断，主优先级为0，子优选级为0
使能CAN送送中断，主优先级为0，子优选级为1

CAN接收滤波器,暂设为不滤波
-------------------------------------------------*/

// 全局实例指针，用于中断服务程序
template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
CAN<TX_BUFF_SIZE, RX_BUFF_SIZE> *CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::s_instances[2] = {NULL};

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
uint32_t CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::get_can_base(CAN_Periph periph)
{
    if (periph == CAN0_PERIPH)
    {

        can_base = CAN0;
        rx0_irq = CAN0_RX0_IRQn;
        rx1_irq = CAN0_RX1_IRQn;
        tx_irq = CAN0_TX_IRQn;
        err_irq = CAN0_EWMC_IRQn;
        can_rcu = RCU_CAN0;
        filter_base = 0;

        // 引脚配置
        rcu_periph_clock_enable(RCU_GPIOB);
        gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_8 | GPIO_PIN_9); // 复用功能选择 CAN0_RX,CAN0_TX
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8 | GPIO_PIN_9);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

        return CAN0;
    }
    else
    {

        can_base = CAN1;
        rx0_irq = CAN1_RX0_IRQn;
        rx1_irq = CAN1_RX1_IRQn;
        tx_irq = CAN1_TX_IRQn;
        err_irq = CAN1_EWMC_IRQn;
        can_rcu = RCU_CAN1;
        filter_base = 14;

        rcu_periph_clock_enable(RCU_GPIOB);
        gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_12 | GPIO_PIN_13); // 复用功能选择 CAN0_RX,CAN0_TX
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12 | GPIO_PIN_13);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);

        return CAN1;
    }
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::CAN(CAN_Periph periph) : can_periph(periph)
{
    can_base = get_can_base(periph);
    is_initialized = false;
    rx_head = 0;
    rx_tail = 0;

    tx_head = 0;
    tx_tail = 0;
    tx_callback = NULL;
    rx_callback = NULL;
    error_callback = NULL;

    // 初始化状态结构体
    can_status.tx_ok = false;
    can_status.rx_ok = false;
    can_status.tx_errors = 0;
    can_status.rx_errors = 0;
    can_status.bus_errors = 0;
    can_status.bus_off = false;
    can_status.error_passive = false;

    // 设置实例指针
    if (periph == CAN0_PERIPH)
    {
        s_instances[0] = this;
    }
    else
    {
        s_instances[1] = this;
    }
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
bool CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::init(CAN_Baudrate baud)
{
    // 使能CAN时钟

    rcu_periph_clock_enable(can_rcu);
    // CAN配置
    // can_parameter_struct can_parameter;
    /* initialize CAN */
    m_can_parameter.time_triggered = DISABLE;       // 关闭定时触发
    m_can_parameter.auto_bus_off_recovery = ENABLE; // ENABLE; //总线关断自恢复打开
    m_can_parameter.auto_wake_up = DISABLE;         // 关闭自动唤醒
    m_can_parameter.auto_retrans = ENABLE;          // ENABLE;          //打开发送失败重传
    m_can_parameter.rec_fifo_overwrite = DISABLE;   // FIFO覆盖关闭
    m_can_parameter.trans_fifo_order = DISABLE;     // FIFO顺序发送关闭
    m_can_parameter.working_mode = CAN_NORMAL_MODE;

    is_initialized = true;
    set_baudrate(baud);

    configure_filter(0, 0, CAN_FIFO0, FILTER_MODE_MASK, FILTER_SCALE_32BIT, 0);

    nvic_irq_enable(rx0_irq, 2, 0);
    // nvic_irq_enable(rx1_irq, 2,0);
    nvic_irq_enable(tx_irq, 2, 1);

    can_interrupt_enable(can_base, CAN_INT_RFNE0);
    can_interrupt_enable(can_base, CAN_INTEN_TMEIE);

    return true;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
bool CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::send_message(uint32_t id, uint8_t *data, uint8_t len, bool is_dataft, bool is_extended)
{
    if (!is_initialized || (len > 8))
    {
        return false;
    }

    can_trasnmit_message_struct tx_message;

    // 配置发送消息
    tx_message.tx_sfid = is_extended ? 0 : (id & 0x7FF);
    tx_message.tx_efid = is_extended ? (id & 0x1FFFFFFF) : 0;
    tx_message.tx_ff = is_extended ? CAN_FF_EXTENDED : CAN_FF_STANDARD;
    tx_message.tx_ft = is_dataft ? CAN_FT_DATA : CAN_FT_REMOTE;
    tx_message.tx_dlen = len;

    // 复制数据
    for (uint8_t i = 0; i < len; i++)
    {
        tx_message.tx_data[i] = data[i];
    }

    // 发送消息
    if (can_message_transmit(can_base, &tx_message) == CAN_NOMAILBOX) // 若发送失败,将发送数据加到缓存队列,在中断中发送
    {

        uint8_t next_head = (tx_head + 1) % TX_BUFF_SIZE;
        if (next_head != tx_tail)
        {
            tx_buffer[tx_head] = tx_message;
            tx_head = next_head;
        }
        return false;
    }
    return true;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
bool CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::receive_message(CAN_Data *msg)
{
    if (!is_initialized || rx_head == rx_tail)
    {
        return false;
    }

    // 从缓冲区读取消息
    *msg = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUFF_SIZE;

    return true;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
void CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::irg_rx0_handler(uint8_t fifo_x)
{

    can_receive_message_struct rx_message;
    CAN_Data msg;

    // 接收消息
    can_message_receive(can_base, fifo_x, &rx_message);

    // 提取消息信息

    for (uint8_t i = 0; i < rx_message.rx_dlen; i++)
    {
        msg.data[i] = rx_message.rx_data[i];
    }

    // 释放FIFO
    can_fifo_release(can_base, fifo_x);

    // 将消息存入缓冲区
    uint8_t next_head = (rx_head + 1) % RX_BUFF_SIZE;
    if (next_head != rx_tail)
    {
        rx_buffer[rx_head] = msg;
        rx_head = next_head;
    }

    // 调用回调函数
    if (rx_callback != NULL)
    {
        rx_callback(&msg);
    }

    can_status.rx_ok = true;
    can_interrupt_flag_clear(can_base, CAN_INT_FLAG_RFF0);
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
void CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::irg_tx_handler(void)
{
    // 处理发送中断
    can_trasnmit_message_struct tx_message;

    can_interrupt_flag_enum mailboxes[] = {CAN_INT_FLAG_MTF0, CAN_INT_FLAG_MTF1, CAN_INT_FLAG_MTF2};
    for (uint8_t i = 0; i < 3; i++)
    {
        if (can_interrupt_flag_get(can_base, mailboxes[i]) == SET)
        {
            // 清除当前邮箱的中断标志
            can_interrupt_flag_clear(can_base, mailboxes[i]);
            // 检查发送缓冲区是否有数据（假设tx_head是缓冲区头部索引）
            if (tx_tail != tx_head)
            { // 缓冲区非空

                can_status.tx_errors = 0;
                // 从缓冲区取出待发送消息
                tx_message = tx_buffer[tx_tail];

                // 尝试发送消息（库函数会自动选择空闲邮箱）
                uint8_t ret = can_message_transmit(can_base, &tx_message);
                if (ret != CAN_NOMAILBOX)
                {
                    // 发送成功，更新缓冲区尾部索引
                    tx_tail = (tx_tail + 1) % TX_BUFF_SIZE;

                    // 调用回调函数，传递成功状态和邮箱信息
                    uint8_t res[2] = {true, i};
                    if (tx_callback != NULL)
                    {
                        tx_callback(res); // 第二个参数为邮箱号（0/1/2）
                    }
                }
                else
                {
                    // 发送失败（无空闲邮箱），不更新索引（下次重试）
                    uint8_t res[2] = {false, i};
                    if (tx_callback != NULL)
                    {
                        tx_callback(res);
                    }
                }
            } // end if tx_head
        } // end if flag
    } // end for
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
bool CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::calculate_baudrate_params(CAN_Baudrate baud, uint32_t *prescaler,
                                                                uint32_t *seg1, uint32_t *seg2)
{
    uint32_t target_baud;
    uint32_t canclk_freq = rcu_clock_freq_get(CK_APB1); // 从APB1获取CAN时钟频率

    switch (baud)
    {
    case CAN_BAUD_125K:
        target_baud = 125000;
        break;
    case CAN_BAUD_250K:
        target_baud = 250000;
        break;
    case CAN_BAUD_500K:
        target_baud = 500000;
        break;
    case CAN_BAUD_1M:
        target_baud = 1000000;
        break;
    default:
        return false;
    }

    // 计算分频器和时间段，总位时间设为8个时间单位
    *prescaler = canclk_freq / (target_baud * 8);
    *seg1 = CAN_BT_BS1_5TQ; // 10;  // 时间段1包含10个时间单位
    *seg2 = CAN_BT_BS1_2TQ; // 5;   // 时间段2包含5个时间单位

    // 检查参数有效性
    if (*prescaler < 1 || *prescaler > 1024)
    {
        return false;
    }
    if (*seg1 < 1 || *seg1 > 16)
    {
        return false;
    }
    if (*seg2 < 1 || *seg2 > 8)
    {
        return false;
    }

    return true;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
bool CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::set_baudrate(CAN_Baudrate baud)
{
    if (!is_initialized)
    {
        return false;
    }

    m_baud = baud;
    uint32_t prescaler, seg1, seg2;
    if (!calculate_baudrate_params(baud, &prescaler, &seg1, &seg2))
    {
        return false;
    }

    // 设置波特率

    m_can_parameter.time_segment_1 = seg1;
    m_can_parameter.time_segment_2 = seg2;
    m_can_parameter.prescaler = prescaler;
    // 复位CAN外设
    can_deinit(can_base);
    can_init(can_base, &m_can_parameter);

    return true;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
bool CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::configure_filter(uint32_t filter_id, uint32_t filter_mask,
                                                       uint32_t filter_fifo, CAN_FilterMode mode,
                                                       CAN_FilterScale scale, uint32_t filter_number)
{
    if (!is_initialized)
    {
        return false;
    }

    // 检查参数有效性
    if (filter_number + filter_base >= 28)
    { // GD32F470有28个过滤器
        return false;
    }
    if (filter_fifo > 1)
    {
        return false;
    }

    // 初始化过滤器
    can_filter_parameter_struct filter_init_struct;

    filter_init_struct.filter_number = filter_number + filter_base;
    filter_init_struct.filter_mode = (mode == FILTER_MODE_MASK) ? CAN_FILTERMODE_MASK : CAN_FILTERMODE_LIST;
    filter_init_struct.filter_bits = (scale == FILTER_SCALE_32BIT) ? CAN_FILTERBITS_32BIT : CAN_FILTERBITS_16BIT;

    // 设置过滤器ID和掩码
    if (scale == FILTER_SCALE_32BIT)
    {
        filter_init_struct.filter_list_high = (filter_id >> 16) & 0xFFFF;
        filter_init_struct.filter_list_low = filter_id & 0xFFFF;
        filter_init_struct.filter_mask_high = (filter_mask >> 16) & 0xFFFF;
        filter_init_struct.filter_mask_low = filter_mask & 0xFFFF;
    }
    else
    {
        filter_init_struct.filter_list_high = filter_id & 0xFFFF;
        filter_init_struct.filter_list_low = filter_mask & 0xFFFF;
        filter_init_struct.filter_mask_high = 0;
        filter_init_struct.filter_mask_low = 0;
    }

    filter_init_struct.filter_fifo_number = filter_fifo;
    filter_init_struct.filter_enable = ENABLE;

    can_filter_init(&filter_init_struct);

    return true;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
void CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::set_callbacks(void (*tx_cb)(void *), void (*rx_cb)(CAN_Data *), void (*error_cb)(uint32_t))
{
    tx_callback = tx_cb;
    rx_callback = rx_cb;
    error_callback = error_cb;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
void CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::start(void)
{
    if (is_initialized)
    {
        // 退出初始化模式（进入正常工作模式）
        // 清除IWMOD位（初始工作模式位），退出初始化状态
        CAN_CTL(can_base) &= ~CAN_CTL_IWMOD;

        // 等待硬件确认退出初始化模式（通常通过状态寄存器判断）
        // 注意：不同芯片确认方式可能不同，以下为通用实现
        uint32_t timeout = 0xFFFF;
        while ((CAN_CTL(can_base) & CAN_CTL_IWMOD) && (timeout-- > 0))
            ;

        if (timeout == 0)
        {
            // 超时处理，可根据需要添加错误回调
            if (error_callback != NULL)
            {
                error_callback(0x01); // 0x01表示启动超时
            }
            return;
        }

        // 使能自动重传（如果需要）
        CAN_CTL(can_base) &= ~CAN_CTL_ARD; // 清除ARD位，启用自动重传

        // 禁用接收FIFO覆盖（可选配置）
        CAN_CTL(can_base) |= CAN_CTL_RFOD; // 置位RFOD，禁止FIFO覆盖
    }
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
void CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::stop(void)
{
    if (is_initialized)
    {
        // 进入初始化模式（停止CAN通信）
        // 置位IWMOD位（初始工作模式位），进入初始化状态
        CAN_CTL(can_base) |= CAN_CTL_IWMOD;

        // 等待硬件确认进入初始化模式
        uint32_t timeout = 0xFFFF;
        while (!(CAN_CTL(can_base) & CAN_CTL_IWMOD) && (timeout-- > 0))
            ;

        if (timeout == 0)
        {
            // 超时处理
            if (error_callback != NULL)
            {
                error_callback(0x02); // 0x02表示停止超时
            }
        }

        // 禁用自动重传（可选）
        // CAN_CTL(can_base) |= CAN_CTL_ARD;
    }
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
void CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::check_errors(void)
{

    if (!is_initialized)
    {
        return;
    }

    // 获取当前错误状态
    can_error_enum error = can_error_get(can_base);

    // 清除之前的错误状态
    can_status.tx_ok = false;
    can_status.rx_ok = false;

    // 根据错误类型更新状态并处理
    switch (error)
    {
    case CAN_ERROR_NONE:
        // 无错误，不做处理
        break;

    case CAN_ERROR_FILL:
        // 填充错误：位填充规则被违反（通常是总线干扰）
        can_status.bus_errors++;
        if (error_callback != NULL)
        {
            error_callback(0x01); // 0x01表示填充错误
        }
        break;

    case CAN_ERROR_FORMATE:
        // 格式错误：帧格式不符合规范（可能是错误的帧结构）
        can_status.rx_errors++;
        if (error_callback != NULL)
        {
            error_callback(0x02); // 0x02表示格式错误
        }
        break;

    case CAN_ERROR_ACK:
        // 应答错误：发送的帧未收到应答（可能是总线断路或无其他节点）
        can_status.tx_errors++;
        if (error_callback != NULL)
        {
            error_callback(0x03); // 0x03表示应答错误
        }
        // 应答错误频繁发生可能需要检查总线连接
        if (can_status.tx_errors > 500)
        {
            reset_bus(); // 超过5次错误尝试复位总线
        }
        break;

    case CAN_ERROR_BITRECESSIVE:
        // 位隐性错误：无法发送隐性位（总线被显性位占据）
        can_status.bus_errors++;
        if (error_callback != NULL)
        {
            error_callback(0x04); // 0x04表示位隐性错误
        }
        break;

    case CAN_ERROR_BITDOMINANTER:
        // 位显性错误：无法发送显性位（总线被隐性位占据）
        can_status.bus_errors++;
        if (error_callback != NULL)
        {
            error_callback(0x05); // 0x05表示位显性错误
        }
        break;

    case CAN_ERROR_CRC:
        // CRC错误：接收帧的CRC校验不匹配（数据传输错误）
        can_status.rx_errors++;
        if (error_callback != NULL)
        {
            error_callback(0x06); // 0x06表示CRC错误
        }
        break;

    case CAN_ERROR_SOFTWARECFG:
        // 软件配置错误：CAN控制器配置参数无效
        if (error_callback != NULL)
        {
            error_callback(0x07); // 0x07表示软件配置错误
        }
        // 配置错误需要重新初始化
        reset_bus();
        break;

    default:
        // 未知错误
        can_status.bus_errors++;
        if (error_callback != NULL)
        {
            error_callback(0xFF); // 0xFF表示未知错误
        }
        break;
    }
    // 检查总线关闭状态
    can_status.bus_off = (can_flag_get(can_base, CAN_FLAG_BOERR) == SET);

    // 检查错误被动状态
    can_status.error_passive = (can_flag_get(can_base, CAN_FLAG_PERR) == SET);
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
CAN_Status CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::get_status(void)
{
    check_errors();
    return can_status;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
bool CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::monitor_status(void)
{
    if (!is_initialized)
    {
        return false;
    }

    check_errors();

    // 如果总线关闭或错误计数过高，返回错误
    if (can_status.bus_errors > 10)
    {
        reset_bus();
        return false;
    }

    return true;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
bool CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::reset_bus(void)
{
    if (!is_initialized)
    {
        return false;
    }

    // 禁用中断
    can_interrupt_disable(can_base, CAN_INT_RFNE0);
    can_interrupt_disable(can_base, CAN_INT_RFNE1);
    can_interrupt_disable(can_base, CAN_INTEN_TMEIE);

    // 停止CAN
    stop();

    // 复位错误状态
    can_deinit(can_base);
    can_status.tx_errors = 0;
    can_status.rx_errors = 0;
    can_status.bus_errors = 0;
    can_status.bus_off = false;
    can_status.error_passive = false;

    // 清空接收缓冲区
    rx_head = 0;
    rx_tail = 0;

    // 重新初始化并启动
    init(m_baud);
    start();

    // 重新使能中断
    can_interrupt_enable(can_base, CAN_INT_RFNE0);
    can_interrupt_enable(can_base, CAN_INT_RFNE1);
    can_interrupt_enable(can_base, CAN_INTEN_TMEIE);

    return true;
}

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
void CAN<TX_BUFF_SIZE, RX_BUFF_SIZE>::send(CAN_Data *msg, uint8_t len, bool is_dataft, bool is_extended)
{
    send_message(msg->id, msg->data, len, is_dataft, is_extended);
}

// CAN0中断服务程序
extern "C" void CAN0_RX0_IRQHandler(void)
{
    if (CanPort0::s_instances[0] != NULL)
    {
        CanPort0::s_instances[0]->irg_rx0_handler(CAN_FIFO0);
    }
}

extern "C" void CAN0_TX_IRQHandler(void)
{
    if (CanPort0::s_instances[1] != NULL)
    {
        CanPort0::s_instances[1]->irg_tx_handler();
    }
}

// CAN0中断服务程序
extern "C" void CAN1_RX0_IRQHandler(void)
{
    if (CanPort1::s_instances[1] != NULL)
    {
        CanPort1::s_instances[1]->irg_rx0_handler(CAN_FIFO0);
    }
}

extern "C" void CAN1_TX_IRQHandler(void)
{
    if (CanPort1::s_instances[1] != NULL)
    {
        CanPort1::s_instances[1]->irg_tx_handler();
    }
}
