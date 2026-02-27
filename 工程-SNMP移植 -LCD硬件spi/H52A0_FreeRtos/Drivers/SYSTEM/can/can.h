#ifndef CAN_H
#define CAN_H

#include "gd32f4xx.h"

// #define TX_BUFF_SIZE 16
// #define RX_BUFF_SIZE 32

// CAN外设枚举
typedef enum
{
    CAN0_PERIPH,
    CAN1_PERIPH
} CAN_Periph;

// CAN波特率枚举
typedef enum
{
    CAN_BAUD_125K,
    CAN_BAUD_250K,
    CAN_BAUD_500K,
    CAN_BAUD_1M
} CAN_Baudrate;

// 过滤器模式
typedef enum
{
    FILTER_MODE_MASK,
    FILTER_MODE_LIST
} CAN_FilterMode;

// 过滤器尺度
typedef enum
{
    FILTER_SCALE_16BIT,
    FILTER_SCALE_32BIT
} CAN_FilterScale;

typedef struct
{
    uint32_t id;     // 消息ID
    uint8_t data[8]; // 数据
} CAN_Data;

// CAN消息结构体
typedef struct
{

    uint8_t len;       // 数据长度
    bool is_extended;  // 是否为扩展帧
    bool is_dataft;    // 是否为数据帧
    CAN_Data can_data; // 数据
} CAN_Message;

// CAN状态结构体
typedef struct
{
    bool tx_ok;          // 最后一次发送成功
    bool rx_ok;          // 最后一次接收成功
    uint32_t tx_errors;  // 发送错误计数
    uint32_t rx_errors;  // 接收错误计数
    uint32_t bus_errors; // 总线错误计数
    bool bus_off;        // 总线关闭状态
    bool error_passive;  // 错误被动状态
} CAN_Status;

template <uint16_t TX_BUFF_SIZE, uint16_t RX_BUFF_SIZE>
class CAN
{
private:
    CAN_Periph can_periph;                               // CAN外设
    uint32_t can_base;                                   // CAN基地址
    bool is_initialized;                                 // 初始化标志
    CAN_Status can_status;                               // CAN状态
    CAN_Data rx_buffer[RX_BUFF_SIZE];                    // 接收缓冲区
    can_trasnmit_message_struct tx_buffer[TX_BUFF_SIZE]; // 接收缓冲区
    uint8_t rx_head;                                     // 缓冲区头指针
    uint8_t rx_tail;                                     // 缓冲区尾指针

    uint8_t tx_head; // 缓冲区头指针
    uint8_t tx_tail; // 缓冲区尾指针

    IRQn rx0_irq;
    IRQn rx1_irq;
    IRQn tx_irq;
    IRQn err_irq;
    rcu_periph_enum can_rcu;
    uint8_t filter_base;

    CAN_Baudrate m_baud;
    can_parameter_struct m_can_parameter; // CAN配置参数保存

    // 回调函数指针
    void (*tx_callback)(void *);
    void (*rx_callback)(CAN_Data *);
    void (*error_callback)(uint32_t);

    // 获取CAN基地址
    uint32_t get_can_base(CAN_Periph periph);

    // 计算波特率参数
    bool calculate_baudrate_params(CAN_Baudrate baud, uint32_t *prescaler,
                                   uint32_t *seg1, uint32_t *seg2);

    // 检查错误状态
    void check_errors(void);

public:
    static CAN *s_instances[2];
    // 构造函数
    CAN(CAN_Periph periph);

    // 初始化CAN外设
    bool init(CAN_Baudrate baud);

    // 设置CAN波特率
    bool set_baudrate(CAN_Baudrate baud);

    // 配置CAN过滤器
    bool configure_filter(uint32_t filter_id, uint32_t filter_mask,
                          uint32_t filter_fifo, CAN_FilterMode mode,
                          CAN_FilterScale scale, uint32_t filter_number);

    // 设置回调函数
    void set_callbacks(void (*tx_cb)(void *), void (*rx_cb)(CAN_Data *), void (*error_cb)(uint32_t));

    // 启动CAN
    void start(void);

    // 停止CAN
    void stop(void);

    // 发送CAN消息
    bool send_message(uint32_t id, uint8_t *data, uint8_t len = 8, bool is_dataft = true, bool is_extended = true);

    void send(CAN_Data *msg, uint8_t len = 8, bool is_dataft = true, bool is_extended = true);

    // 接收CAN消息
    bool receive_message(CAN_Data *msg);

    // 获取CAN状态
    CAN_Status get_status(void);

    // 监控CAN工作状态
    bool monitor_status(void);

    // 重启CAN总线
    bool reset_bus(void);

    // 中断处理函数
    void irg_rx0_handler(uint8_t fifo_x);
    void irg_tx_handler(void);
    void irg_err_handler(void);
};

template class CAN<16, 32>;

typedef CAN<16, 32> CanPort0;
typedef CAN<16, 32> CanPort1;

#endif // CAN_H
