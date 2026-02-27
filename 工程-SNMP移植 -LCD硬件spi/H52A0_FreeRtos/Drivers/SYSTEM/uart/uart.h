#ifndef GD32F470_UART_H
#define GD32F470_UART_H

#include "gd32f4xx.h"

// 定义接收缓冲区大小
#define DEFAULT_RECEIVE_BUFFER_SIZE 1024
#define TRANSMIT_MAX_SIZE 1024

// 串口端口枚举
typedef enum
{
  UART_0,
  UART_1,
  UART_2,
  UART_3,
  UART_4,
  UART_5,
  UART_MAX
} UART_Port;

// 数据位枚举
typedef enum
{
  DATA_BITS_8,
  DATA_BITS_9
} DataBits;

// 停止位枚举
typedef enum
{
  STOP_BITS_1,
  STOP_BITS_0_5,
  STOP_BITS_2,
  STOP_BITS_1_5
} StopBits;

// 校验位枚举
typedef enum
{
  PARITY_NONE,
  PARITY_ODD,
  PARITY_EVEN
} Parity;

// 流控制枚举
typedef enum
{
  FLOW_CONTROL_NONE,
  FLOW_CONTROL_RTS,
  FLOW_CONTROL_CTS,
  FLOW_CONTROL_RTS_CTS
} FlowControl;

// 回调函数类型定义
typedef void (*ReceiveCallback)(void *userData);
typedef void (*TransmitCompleteCallback)(void *userData); // 新增发送完成回调

// 模板类，缓冲区大小通过模板参数指定
template <uint32_t BUFFER_SIZE = DEFAULT_RECEIVE_BUFFER_SIZE>
class UART
{
public:
  // 构造函数和析构函数
  UART(UART_Port port);
  ~UART();

  // 初始化串口
  bool init(uint32_t baudrate,
            DataBits databits = DATA_BITS_8,
            StopBits stopbits = STOP_BITS_1,
            Parity parity = PARITY_NONE,
            FlowControl flowcontrol = FLOW_CONTROL_NONE);

  // 发送数据
  bool send(const uint8_t *data, uint32_t length);

  // 检查发送是否完成
  bool isSendComplete() const;

  // 获取接收到的数据
  uint32_t getReceivedData(uint8_t *buffer, uint32_t bufferSize);

  // 清除接收缓冲区
  void clearReceiveBuffer();

  // 设置RS485控制引脚
  void setRs485ControlPins(uint32_t gpio_periph, uint32_t tx_en_pin);

  // 使能RS485发送模式
  void rs485EnableTransmit();

  // 使能RS485接收模式
  void rs485EnableReceive();

  // 设置接收回调函数
  void setReceiveCallback(ReceiveCallback callback, void *userData = 0);

  // 中断处理函数
  void handleInterrupt();

  // 获取接收长度
  bool getReceiveLenth(uint16_t *length);

  // 静态实例数组，用于中断处理
  static UART *s_instances[UART_MAX];

  //  static void receiveHandler(void* userData);

private:
  // 初始化GPIO
  void initGPIO();

  // 初始化DMA
  void initDMA();

  // 初始化中断
  void initInterrupt();

  // 串口端口
  UART_Port m_port;

  // 波特率
  uint32_t m_baudrate;

  // USART外设基地址
  uint32_t m_usart;

  // DMA相关参数
  uint32_t m_dmaPeriph;
  dma_channel_enum m_txDmaChannel;
  dma_channel_enum m_rxDmaChannel;
  dma_subperipheral_enum m_txSub_periph;
  dma_subperipheral_enum m_rxSub_periph;

  // RS485控制引脚
  uint32_t m_rs485Gpio;
  uint32_t m_rs485TxEnPin;

  // 接收缓冲区和相关变量
  uint8_t m_receiveBuffer[BUFFER_SIZE];
  uint32_t m_receiveLength;
  bool m_receiveComplete;

  // 发送相关状态
  bool m_transmitComplete; // 新增发送完成标志

  // 回调函数和用户数据
  ReceiveCallback m_receiveCallback;
  TransmitCompleteCallback m_transmitCompleteCallback; // 新增发送完成回调
  void *m_receiveUserData;
  void *m_transmitUserData; // 新增发送回调用户数据
};

template <uint32_t BUFFER_SIZE>
UART<BUFFER_SIZE> *UART<BUFFER_SIZE>::s_instances[UART_MAX] = {0};

// 显式实例化模板类,定义不同接缓冲区大小的串口类
template class UART<256>;
template class UART<1024>;
template class UART<2048>;

// 定义类别名,该别名绑定串口中断
typedef UART<256> uart0; // 绑定串口中断0
typedef UART<256> uart1; // 绑定串口中断1
typedef UART<256> uart2; // 绑定串口中断2
typedef UART<256> uart3; // 绑定串口中断3
typedef UART<256> uart4; // 绑定串口中断4
typedef UART<256> uart5; // 绑定串口中断5

// void receiveHandler(void* userData);
#endif // UART_H
