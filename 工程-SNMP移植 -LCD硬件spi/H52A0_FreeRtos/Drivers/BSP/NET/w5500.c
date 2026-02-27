#include "w5500.h"
#include "netport.h"
#include <string.h>
#include "stdint.h"
// #include "FreeRTOS.h"
// #include "task.h"
// #include "timers.h"

static uint8 I_STATUS[MAX_SOCK_NUM];
static uint16 SSIZE[MAX_SOCK_NUM]; /**< Max Tx buffer size by each channel */
static uint16 RSIZE[MAX_SOCK_NUM]; /**< Max Rx buffer size by each channel */
#define DATA_BUF_SIZE    2048
uint8 SPI2_DMA_TxBuff[DATA_BUF_SIZE+3];

void w5500_reset(void)
{

  net_reset();
}

void WIZ_SPI_Init(void)
{

  net_port_init();
}

void WIZ_CS(uint8 val)
{

  net_scs_ctrl(val);
}

uint8 getISR(uint8 s)
{
  return I_STATUS[s];
}
void putISR(uint8 s, uint8 val)
{
  I_STATUS[s] = val;
}

uint16 getIINCHIP_RxMAX(uint8 s)
{
  return RSIZE[s];
}
uint16 getIINCHIP_TxMAX(uint8 s)
{
  return SSIZE[s];
}
void IINCHIP_CSoff(void)
{
  WIZ_CS(LOW);
}
void IINCHIP_CSon(void)
{
  WIZ_CS(HIGH);
}
u8 IINCHIP_SpiSendData(uint8 dat)
{
  // return(SPI2_SendByte(dat));
  uint8 res = 0;
  net_read_write(dat, &res);
  return res;
}

void IINCHIP_WRITE(uint32 addrbsb, uint8 data)
{
  uint8 spi_data[4];

#if !defined(SPI_DMA)
  // IINCHIP_ISR_DISABLE();                        // Interrupt Service Routine Disable
  IINCHIP_CSoff();                                   // CS=0, SPI start
  IINCHIP_SpiSendData((addrbsb & 0x00FF0000) >> 16); // Address byte 1
  IINCHIP_SpiSendData((addrbsb & 0x0000FF00) >> 8);  // Address byte 2
  IINCHIP_SpiSendData((addrbsb & 0x000000F8) + 4);   // Data write command and Write data length 1
  IINCHIP_SpiSendData(data);                         // Data write (write 1byte data)
  IINCHIP_CSon();                                    // CS=1,  SPI end
  // IINCHIP_ISR_ENABLE();                         // Interrupt Service Routine Enable

#else
  spi_data[0] = (addrbsb & 0x00FF0000) >> 16;
  spi_data[1] = (addrbsb & 0x0000FF00) >> 8;
  spi_data[2] = ((addrbsb & 0x000000F8) + 4);
  SPI_DMA_WRITE(spi_data, &data, 1); // 写数据
#endif
}

uint8 IINCHIP_READ(uint32 addrbsb)
{
  uint8 data = 0;
  uint8 spi_data[3];

#if !defined(SPI_DMA)
  // IINCHIP_ISR_DISABLE();                        // Interrupt Service Routine Disable
  IINCHIP_CSoff();                                   // CS=0, SPI start
  IINCHIP_SpiSendData((addrbsb & 0x00FF0000) >> 16); // Address byte 1
  IINCHIP_SpiSendData((addrbsb & 0x0000FF00) >> 8);  // Address byte 2
  IINCHIP_SpiSendData((addrbsb & 0x000000F8));       // Data read command and Read data length 1
  data = IINCHIP_SpiSendData(0x00);                  // Data read (read 1byte data)
  IINCHIP_CSon();                                    // CS=1,  SPI end
                                                     // IINCHIP_ISR_ENABLE();                         // Interrupt Service Routine Enable
#else
  spi_data[0] = (addrbsb & 0x00FF0000) >> 16;
  spi_data[1] = (addrbsb & 0x0000FF00) >> 8;
  spi_data[2] = ((addrbsb & 0x000000F8));
  SPI_DMA_READ(spi_data, &data, 1); // 读数据
#endif
  return data;
}

uint16 wiz_write_buf(uint32 addrbsb, uint8 *buf, uint16 len)
{
  uint16 idx = 0;
  uint8 spi_data[3];
#if !defined(SPI_DMA)
  if (len == 0)
  {
    ;
  }; // printf("Unexpected2 length 0\r\n");

  // IINCHIP_ISR_DISABLE();
  IINCHIP_CSoff();                                   // CS=0, SPI start
  IINCHIP_SpiSendData((addrbsb & 0x00FF0000) >> 16); // Address byte 1
  IINCHIP_SpiSendData((addrbsb & 0x0000FF00) >> 8);  // Address byte 2
  IINCHIP_SpiSendData((addrbsb & 0x000000F8)+4);   // Data write command and Write data length 1
  for (idx = 0; idx < len; idx++)                    // Write data in loop
  {
    IINCHIP_SpiSendData(buf[idx]);
  }
  IINCHIP_CSon(); // CS=1, SPI end
                  // IINCHIP_ISR_ENABLE();                         // Interrupt Service Routine Enable

#else
  spi_data[0] = ((addrbsb & 0x00FF0000) >> 16);
  spi_data[1] = ((addrbsb & 0x0000FF00) >> 8);
  spi_data[2] = ((addrbsb & 0x000000F8)+4);
  SPI_DMA_WRITE(spi_data, buf, len); // 写数据
#endif
  return len;
}

uint16 wiz_read_buf(uint32 addrbsb, uint8 *buf, uint16 len)
{
  uint16 idx = 0;
  uint8 spi_data[3];
  
#if !defined(SPI_DMA)
  if (len == 0)
  {
    ; // printf("Unexpected2 length 0\r\n");
    return 0; // 补充返回，避免空操作
  }
  
  // IINCHIP_ISR_DISABLE();
  IINCHIP_CSoff();                                   // CS=0, SPI开始
  
  // 步骤1：发送地址（3字节）+ 读指令
  // 地址拆分：高8位、中8位、低5位 + 读指令(0x04)
  IINCHIP_SpiSendData((addrbsb & 0x00FF0000) >> 16); // 地址字节1
  IINCHIP_SpiSendData((addrbsb & 0x0000FF00) >> 8);  // 地址字节2
  IINCHIP_SpiSendData( (addrbsb & 0x000000F8) ); // 低5位地址 + 读指令(0x04)
  
  // 步骤2：循环读取数据（而非写入）
  for (idx = 0; idx < len; idx++)                    
  {
    buf[idx] = IINCHIP_SpiSendData(0x00); // 读数据到buf，而非发送
  }
  
  IINCHIP_CSon(); // CS=1, SPI结束
  // IINCHIP_ISR_ENABLE();                            

#else
  // DMA分支：补充读指令，修正地址拼接
  spi_data[0] = (addrbsb & 0x00FF0000) >> 16;        // 地址字节1
  spi_data[1] = (addrbsb & 0x0000FF00) >> 8;         // 地址字节2
  spi_data[2] = ((addrbsb & 0x000000F8));     // 低5位地址 + 读指令(0x04)
  SPI_DMA_READ(spi_data, buf, len); // 读数据
#endif
  
  return len;
}

/**
@brief  This function is for resetting of the iinchip. Initializes the iinchip to work in whether DIRECT or INDIRECT mode
*/
void iinchip_init(void)
{
  setMR(MR_RST);
#ifdef __DEF_IINCHIP_DBG__
  ; // printf("MR value is %02x \r\n",IINCHIP_READ_COMMON(MR));
#endif
}

/**
@brief  This function set the transmit & receive buffer size as per the channels is used
Note for TMSR and RMSR bits are as follows\n
bit 1-0 : memory size of channel #0 \n
bit 3-2 : memory size of channel #1 \n
bit 5-4 : memory size of channel #2 \n
bit 7-6 : memory size of channel #3 \n
bit 9-8 : memory size of channel #4 \n
bit 11-10 : memory size of channel #5 \n
bit 12-12 : memory size of channel #6 \n
bit 15-14 : memory size of channel #7 \n
W5500的Tx, Rx的最大寄存器宽度是16K Bytes,\n
In the range of 16KBytes, the memory size could be allocated dynamically by each channel.\n
Be attentive to sum of memory size shouldn't exceed 8Kbytes\n
and to data transmission and receiption from non-allocated channel may cause some problems.\n
If the 16KBytes memory is already  assigned to centain channel, \n
other 3 channels couldn't be used, for there's no available memory.\n
If two 4KBytes memory are assigned to two each channels, \n
other 2 channels couldn't be used, for there's no available memory.\n
*/
void sysinit(uint8 *tx_size, uint8 *rx_size)
{
  int16 i;
  int16 ssum, rsum;
#ifdef __DEF_IINCHIP_DBG__
  ; // printf("sysinit()\r\n");
#endif
  ssum = 0;
  rsum = 0;

  for (i = 0; i < MAX_SOCK_NUM; i++) // Set the size, masking and base address of Tx & Rx memory by each channel
  {
    IINCHIP_WRITE((Sn_TXMEM_SIZE(i)), tx_size[i]); // MCU记录每个Socket发送缓存的大小到数组tx_size[]对应的元素值
    IINCHIP_WRITE((Sn_RXMEM_SIZE(i)), rx_size[i]); // MCU记录每个Socket接收缓存的大小到数组rx_size[]对应的元素值

#ifdef __DEF_IINCHIP_DBG__
    // printf("tx_size[%d]: %d, Sn_TXMEM_SIZE = %d\r\n",i, tx_size[i], IINCHIP_READ(Sn_TXMEM_SIZE(i)));
    // printf("rx_size[%d]: %d, Sn_RXMEM_SIZE = %d\r\n",i, rx_size[i], IINCHIP_READ(Sn_RXMEM_SIZE(i)));
#endif
    SSIZE[i] = (int16)(0);
    RSIZE[i] = (int16)(0);

    if (ssum <= 16384)
    {
      switch (tx_size[i])
      {
      case 1:
        SSIZE[i] = (int16)(1024);
        break;
      case 2:
        SSIZE[i] = (int16)(2048);
        break;
      case 4:
        SSIZE[i] = (int16)(4096);
        break;
      case 8:
        SSIZE[i] = (int16)(8192);
        break;
      case 16:
        SSIZE[i] = (int16)(16384);
        break;
      default:
        RSIZE[i] = (int16)(2048);
        break;
      }
    }

    if (rsum <= 16384)
    {
      switch (rx_size[i])
      {
      case 1:
        RSIZE[i] = (int16)(1024);
        break;
      case 2:
        RSIZE[i] = (int16)(2048);
        break;
      case 4:
        RSIZE[i] = (int16)(4096);
        break;
      case 8:
        RSIZE[i] = (int16)(8192);
        break;
      case 16:
        RSIZE[i] = (int16)(16384);
        break;
      default:
        RSIZE[i] = (int16)(2048);
        break;
      }
    }
    ssum += SSIZE[i];
    rsum += RSIZE[i];
  }
}

// added

/*.
 */
void setGAR(
    uint8 *addr /**< a pointer to a 4 -byte array responsible to set the Gateway IP address. */
)
{
  wiz_write_buf(GAR0, addr, 4);
}
void getGWIP(uint8 *addr)
{
  wiz_read_buf(GAR0, addr, 4);
}

/**
@brief  It sets up SubnetMask address
*/
void setSUBR(uint8 *addr)
{
  wiz_write_buf(SUBR0, addr, 4);
}
/**
@brief  This function sets up MAC address.
*/
void setSHAR(
    uint8 *addr /**< a pointer to a 6 -byte array responsible to set the MAC address. */
)
{
  wiz_write_buf(SHAR0, addr, 6);
}

/**
@brief  This function sets up Source IP address.
*/
void setSIPR(
    uint8 *addr /**< a pointer to a 4 -byte array responsible to set the Source IP address. */
)
{
  wiz_write_buf(SIPR0, addr, 4);
}

/**
@brief  W5500心跳检测程序，设置Socket在线时间寄存器Sn_KPALVTR，单位为5s
*/
void setkeepalive(SOCKET s)
{
  IINCHIP_WRITE(Sn_KPALVTR(s), 0x02);
}

/**
@brief  This function sets up Source IP address.
*/
void getGAR(uint8 *addr)
{
  wiz_read_buf(GAR0, addr, 4);
}
void getSUBR(uint8 *addr)
{
  wiz_read_buf(SUBR0, addr, 4);
}
void getSHAR(uint8 *addr)
{
  wiz_read_buf(SHAR0, addr, 6);
}
void getSIPR(uint8 *addr)
{
  wiz_read_buf(SIPR0, addr, 4);
}

void setMR(uint8 val)
{
  IINCHIP_WRITE(MR, val);
}

/**
@brief  This function gets Interrupt register in common register.
 */
uint8 getIR(void)
{
  return IINCHIP_READ(IR);
}

/**
@brief  This function sets up Retransmission time.

If there is no response from the peer or delay in response then retransmission
will be there as per RTR (Retry Time-value Register)setting
*/
void setRTR(uint16 timeout)
{
  IINCHIP_WRITE(RTR0, (uint8)((timeout & 0xff00) >> 8));
  IINCHIP_WRITE(RTR1, (uint8)(timeout & 0x00ff));
}

/**
@brief  This function set the number of Retransmission.

If there is no response from the peer or delay in response then recorded time
as per RTR & RCR register seeting then time out will occur.
*/
void setRCR(uint8 retry)
{
  IINCHIP_WRITE(WIZ_RCR, retry);
}

/**
@brief  This function set the interrupt mask Enable/Disable appropriate Interrupt. ('1' : interrupt enable)

If any bit in IMR is set as '0' then there is not interrupt signal though the bit is
set in IR register.
*/
void clearIR(uint8 mask)
{
  IINCHIP_WRITE(IR, ~mask | getIR()); // must be setted 0x10.
}

/**
@brief  This sets the maximum segment size of TCP in Active Mode), while in Passive Mode this is set by peer
*/
void setSn_MSS(SOCKET s, uint16 Sn_MSSR)
{
  IINCHIP_WRITE(Sn_MSSR0(s), (uint8)((Sn_MSSR & 0xff00) >> 8));
  IINCHIP_WRITE(Sn_MSSR1(s), (uint8)(Sn_MSSR & 0x00ff));
}

void setSn_TTL(SOCKET s, uint8 ttl)
{
  IINCHIP_WRITE(Sn_TTL(s), ttl);
}

/**
@brief  get socket interrupt status

These below functions are used to read the Interrupt & Soket Status register
*/
uint8 getSn_IR(SOCKET s)
{
  return IINCHIP_READ(Sn_IR(s));
}

/**
@brief   get socket status
*/
uint8 getSn_SR(SOCKET s)
{
  return IINCHIP_READ(Sn_SR(s)); // MCU读Sn_SR对应地址里面的数值并返回
}

/**
@brief  get socket TX free buf size

This gives free buffer size of transmit buffer. This is the data size that user can transmit.
User shuold check this value first and control the size of transmitting data
*/
uint16 getSn_TX_FSR(SOCKET s)
{
  uint16 val = 0, val1 = 0;
  uint8 buff[2];
  do
  {
    //    val1 = IINCHIP_READ(Sn_TX_FSR0(s));
    //    val1 = (val1 << 8) + IINCHIP_READ(Sn_TX_FSR1(s));
    wiz_read_buf(Sn_TX_FSR0(s), buff, 2);
    val1 = (buff[0] << 8) + buff[1];
    if (val1 != 0)
    {
      //        val = IINCHIP_READ(Sn_TX_FSR0(s));
      //        val = (val << 8) + IINCHIP_READ(Sn_TX_FSR1(s));
      wiz_read_buf(Sn_TX_FSR0(s), buff, 2);
      val = (buff[0] << 8) + buff[1];
    }
  } while (val != val1);
  return val;
}

/**
@brief   get socket RX recv buf size

This gives size of received data in receive buffer.
*/
uint16 getSn_RX_RSR(SOCKET s) // 获取Sn_RX_RSR空闲接收缓存寄存器的值并返回,Sn_RX_RSR中保存的是接收缓存中已接收和保存的数据大小
{
  uint16 val = 0, val1 = 0;
  uint8 buff[2];
  do
  {
    //    val1 = IINCHIP_READ(Sn_RX_RSR0(s));
    //    val1 = (val1 << 8) + IINCHIP_READ(Sn_RX_RSR1(s));		// MCU读出Sn_RX_RSR的值赋给val1
    wiz_read_buf(Sn_RX_RSR0(s), buff, 2);
    val1 = (buff[0] << 8) + buff[1];
    if (val1 != 0) // 如果Sn_RX_RSR的值不为0，说明接收缓存中收到了数据
    {
      //        val = IINCHIP_READ(Sn_RX_RSR0(s));
      //        val = (val << 8) + IINCHIP_READ(Sn_RX_RSR1(s));	// MCU读出Sn_RX_RSR的值赋给val
      wiz_read_buf(Sn_RX_RSR0(s), buff, 2);
      val = (buff[0] << 8) + buff[1];
    }
  } while (val != val1); // 此时应该val=val1,表达式为假，跳出do while循环
  return val; // 返回val
}
/**
@brief   This function is being called by send() and sendto() function also.

This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
register. User should read upper byte first and lower byte later to get proper value.
*/
void send_data_processing(SOCKET s, uint8 *data, uint16 len) // MCU把数据发送给W5500的过程，W5500将数据写入buf，并更新数据的写指针的地址
{
  uint16 ptr = 0;
  uint32 addrbsb = 0;
  uint8 buff[2];

  if (len == 0)
  {
    //    printf("CH: %d Unexpected1 length 0\r\n", s);
    return;
  }

  // Sn_RX_WR是发送写指针寄存器，用来保存发送缓存中将要传输数据的首地址
  //	ptr = IINCHIP_READ( Sn_TX_WR0(s) );
  //  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_TX_WR1(s));			// MCU读取Sn_RX_WR寄存器的值
  //  printf("Sn_TX_WR:0x%4x\r\n",ptr);
  wiz_read_buf(Sn_TX_WR0(s), buff, 2);
  ptr = ((buff[0] & 0x00ff) << 8) + buff[1];

  addrbsb = (uint32)(ptr << 8) + (s << 5) + 0x10; // 将发送缓存中将要传输数据的首地址转成32位

  wiz_write_buf(addrbsb, data, len); // W5500从该首地址开始写入数据，数据长度为len

  ptr += len; // 首地址的值+len变为数据新的首地址

  buff[0] = (uint8)((ptr & 0xff00) >> 8);
  buff[1] = (uint8)(ptr & 0x00ff);
  wiz_write_buf(Sn_TX_WR0(s), buff, 2);
  //  IINCHIP_WRITE( Sn_TX_WR0(s) ,(uint8)((ptr & 0xff00) >> 8));
  //  IINCHIP_WRITE( Sn_TX_WR1(s),(uint8)(ptr & 0x00ff));						// 将新的首地址保存在Sn_RX_WR中
}

/**
@brief  This function is being called by recv() also.

This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
*/
void recv_data_processing(SOCKET s, uint8 *data, uint16 len)
{
  uint16 ptr = 0;
  uint32 addrbsb = 0;
  uint8 buff[2];
  if (len == 0)
  {
    //    printf("CH: %d Unexpected2 length 0\r\n", s);
    return;
  }
  // Sn_RX_RD是接收读指针寄存器
  //  ptr = IINCHIP_READ( Sn_RX_RD0(s) );
  //  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ( Sn_RX_RD1(s) );			// MCU读取Sn_RX_RD寄存器的值

  wiz_read_buf(Sn_RX_RD0(s), buff, 2);
  ptr = ((buff[0] & 0x00ff) << 8) + buff[1];

  addrbsb = (uint32)(ptr << 8) + (s << 5) + 0x18; // 将发送缓存中将要传输数据的首地址转成32位
  wiz_read_buf(addrbsb, data, len);               // W5500从该首地址开始读取数据，数据长度为len
  ptr += len;                                     // 首地址的值+len变为数据新的首地址

  buff[0] = (uint8)((ptr & 0xff00) >> 8);
  buff[1] = (uint8)(ptr & 0x00ff);
  wiz_write_buf(Sn_RX_RD0(s), buff, 2);
  //  IINCHIP_WRITE( Sn_RX_RD0(s), (uint8)((ptr & 0xff00) >> 8));
  //  IINCHIP_WRITE( Sn_RX_RD1(s), (uint8)(ptr & 0x00ff));		// 将新的首地址保存在Sn_RX_RD中
}

void setSn_IR(uint8 s, uint8 val)
{
  IINCHIP_WRITE(Sn_IR(s), val);
}

//void SPI_DMA_WRITE(uint8_t* Addref, uint8_t* pTxBuf, uint16_t tx_len)
//{
//    // 入参合法性检查
////    if (Addref == NULL || pTxBuf == NULL || tx_len == 0 || (tx_len + 3) > SPI_DMA_BUF_MAX_LEN)
////    {
////        return;
////    }

//    uint16_t i;
//    // 1. 拼接地址头+数据到发送缓冲区
//    memset(SPI2_DMA_TxBuff, 0, tx_len + 3);
//    SPI2_DMA_TxBuff[0] = Addref[0];
//    SPI2_DMA_TxBuff[1] = Addref[1];
//    SPI2_DMA_TxBuff[2] = Addref[2];
//    for(i=0; i<tx_len; i++)
//    {
//        SPI2_DMA_TxBuff[3 + i] = pTxBuf[i];
//    }

//    // 2. 定义并初始化新版DMA配置结构体（替换原dma_data_parameter）
//    dma_single_data_parameter_struct dma_init_struct;
//    dma_struct_para_init(&dma_init_struct); // 初始化结构体默认值

//    // 3. 禁用DMA通道（配置前避免误触发）
//    dma_channel_disable(DMA1, DMA_CH1); // TX DMA通道
//    dma_channel_disable(DMA1, DMA_CH0); // RX DMA通道

//    // -------------------------- 通用DMA参数配置 --------------------------
//    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);          // SPI2数据寄存器地址
//    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;        // 外设地址不递增
//    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;         // 内存地址递增
//    dma_init_struct.periph_memory_width = DMA_MEMORY_WIDTH_8BIT;       // 8位数据宽度
//    dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;         // 关闭循环模式
//    dma_init_struct.priority     = DMA_PRIORITY_HIGH;                  // 高优先级
//    dma_init_struct.number       = (uint16_t)(tx_len + 3);             // 总长度：地址头+数据

//    // -------------------------- 配置TX DMA（内存→外设） --------------------------
//    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;                  // 传输方向：内存→外设
//    dma_init_struct.memory0_addr = (uint32_t)SPI2_DMA_TxBuff;          // 发送缓冲区地址
//    dma_single_data_mode_init(DMA1, DMA_CH1, &dma_init_struct);        // 应用TX DMA配置
//    dma_channel_subperipheral_select(DMA1, DMA_CH1, DMA_SUBPERI4);     // 绑定到SPI2（根据硬件调整SUBPERI）

//    // -------------------------- 配置RX DMA（外设→内存） --------------------------
//    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;                  // 传输方向：外设→内存
//    dma_init_struct.memory0_addr = (uint32_t)SPI2_DMA_RxBuff;          // 修复：指向独立接收缓冲区
//    dma_single_data_mode_init(DMA1, DMA_CH0, &dma_init_struct);        // 应用RX DMA配置
//    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI4);     // 绑定到SPI2（根据硬件调整SUBPERI）

//    // 4. 清除DMA中断标志（避免旧标志干扰）
//    dma_interrupt_flag_clear(DMA1, DMA_CH1, DMA_CHXCTL_FTFIF);
//    dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_CHXCTL_FTFIF);

//    // 5. 启动SPI DMA传输
//    IINCHIP_CSoff();                                                  // 拉低CS
//    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);                           // 启用SPI TX DMA请求
//    spi_dma_enable(SPI2, SPI_DMA_RECEIVE);                            // 启用SPI RX DMA请求
//    spi_enable(SPI2);                                                 // 使能SPI2

//    // 6. 启用DMA通道，开始传输
//    dma_channel_enable(DMA1, DMA_CH1);                                // 启动TX DMA
//    dma_channel_enable(DMA1, DMA_CH0);                                // 启动RX DMA

//    // 7. 等待传输完成
//    while(!dma_interrupt_flag_get(DMA1, DMA_CH1, DMA_CHXCTL_FTFIF));   // 等待TX完成
//    while(!dma_interrupt_flag_get(DMA1, DMA_CH0, DMA_CHXCTL_FTFIF));   // 等待RX完成

//    // 8. 传输完成后清理
//    IINCHIP_CSon();                                                   // 拉高CS
//    dma_interrupt_flag_clear(DMA1, DMA_CH1, DMA_CHXCTL_FTFIF);         // 清除TX完成标志
//    dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_CHXCTL_FTFIF);         // 清除RX完成标志
//    dma_channel_disable(DMA1, DMA_CH1);                                // 禁用TX DMA通道
//    dma_channel_disable(DMA1, DMA_CH0);                                // 禁用RX DMA通道
//    spi_dma_disable(SPI2, SPI_DMA_TRANSMIT);                           // 关闭SPI TX DMA请求
//    spi_dma_disable(SPI2, SPI_DMA_RECEIVE);                            // 关闭SPI RX DMA请求
//    spi_disable(SPI2);                                                 // 禁用SPI2
//}

///**
// * @brief SPI DMA读函数（SPI外设→内存）
// * @param Addref: SPI地址头（3字节：地址2字节+指令1字节）
// * @param pRxBuf: 接收数据的缓冲区
// * @param rx_len: 要接收的数据长度
// * @note 保持参数不变，使用dma_init_struct配置DMA
// */
//void SPI_DMA_READ(uint8_t* Addref, uint8_t* pRxBuf, uint16_t rx_len)
//{
//    // 入参合法性检查
//    if (Addref == NULL || pRxBuf == NULL || rx_len == 0 || rx_len > SPI_DMA_BUF_MAX_LEN)
//    {
//        return;
//    }

//    // 1. 拼接3字节地址头到发送缓冲区（读操作仅需发送地址头）
//    memset(SPI2_DMA_TxBuff, 0, 3);
//    SPI2_DMA_TxBuff[0] = Addref[0];
//    SPI2_DMA_TxBuff[1] = Addref[1];
//    SPI2_DMA_TxBuff[2] = Addref[2];
//    memset(SPI2_DMA_RxBuff, 0, rx_len);                               // 清空接收缓冲区

//    // 2. 定义并初始化新版DMA配置结构体
//    dma_single_data_parameter_struct dma_init_struct;
//    dma_struct_para_init(&dma_init_struct);

//    // 3. 禁用DMA通道
//    dma_channel_disable(DMA1, DMA_CH1); // TX DMA通道
//    dma_channel_disable(DMA1, DMA_CH0); // RX DMA通道

//    // -------------------------- 通用DMA参数配置 --------------------------
//    dma_init_struct.periph_addr  = (uint32_t)&SPI_DATA(SPI2);          // SPI2数据寄存器地址
//    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;        // 外设地址不递增
//    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;         // 内存地址递增
//    dma_init_struct.periph_memory_width = DMA_MEMORY_WIDTH_8BIT;       // 8位数据宽度
//    dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;         // 关闭循环模式
//    dma_init_struct.priority     = DMA_PRIORITY_HIGH;                  // 高优先级

//    // -------------------------- 配置TX DMA（仅发送3字节地址头） --------------------------
//    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;                  // 传输方向：内存→外设
//    dma_init_struct.memory0_addr = (uint32_t)SPI2_DMA_TxBuff;          // 发送缓冲区（地址头）
//    dma_init_struct.number       = 3;                                  // 仅发送3字节地址头（核心修复）
//    dma_single_data_mode_init(DMA1, DMA_CH1, &dma_init_struct);        // 应用TX DMA配置
//    dma_channel_subperipheral_select(DMA1, DMA_CH1, DMA_SUBPERI4);     // 绑定到SPI2

//    // -------------------------- 配置RX DMA（接收rx_len字节数据） --------------------------
//    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;                  // 传输方向：外设→内存
//    dma_init_struct.memory0_addr = (uint32_t)SPI2_DMA_RxBuff;          // 接收缓冲区地址
//    dma_init_struct.number       = rx_len;                             // 接收数据长度（核心修复）
//    dma_single_data_mode_init(DMA1, DMA_CH0, &dma_init_struct);        // 应用RX DMA配置
//    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI4);     // 绑定到SPI2

//    // 4. 清除DMA中断标志
//    dma_interrupt_flag_clear(DMA1, DMA_CH1, DMA_CHXCTL_FTFIF);
//    dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_CHXCTL_FTFIF);

//    // 5. 启动SPI DMA传输
//    IINCHIP_CSoff();                                                  // 拉低CS
//    spi_dma_enable(SPI2, SPI_DMA_TRANSMIT);                           // 启用SPI TX DMA请求
//    spi_dma_enable(SPI2, SPI_DMA_RECEIVE);                            // 启用SPI RX DMA请求
//    spi_enable(SPI2);                                                 // 使能SPI2

//    // 6. 启用DMA通道，开始传输
//    dma_channel_enable(DMA1, DMA_CH1);                                // 启动TX DMA（发送地址头）
//    dma_channel_enable(DMA1, DMA_CH0);                                // 启动RX DMA（接收数据）

//    // 7. 等待传输完成
//    while(!dma_interrupt_flag_get(DMA1, DMA_CH1, DMA_CHXCTL_FTFIF));   // 等待TX（地址头）完成
//    while(!dma_interrupt_flag_get(DMA1, DMA_CH0, DMA_CHXCTL_FTFIF));   // 等待RX（数据）完成

//    // 8. 传输完成后清理
//    IINCHIP_CSon();                                                   // 拉高CS
//    dma_interrupt_flag_clear(DMA1, DMA_CH1, DMA_CHXCTL_FTFIF);         // 清除TX完成标志
//    dma_interrupt_flag_clear(DMA1, DMA_CH0, DMA_CHXCTL_FTFIF);         // 清除RX完成标志
//    dma_channel_disable(DMA1, DMA_CH1);                                // 禁用TX DMA通道
//    dma_channel_disable(DMA1, DMA_CH0);                                // 禁用RX DMA通道
//    spi_dma_disable(SPI2, SPI_DMA_TRANSMIT);                           // 关闭SPI TX DMA请求
//    spi_dma_disable(SPI2, SPI_DMA_RECEIVE);                            // 关闭SPI RX DMA请求
//    spi_disable(SPI2);                                                 // 禁用SPI2

//    // 9. 将接收缓冲区数据拷贝到用户缓冲区
//    memcpy(pRxBuf, SPI2_DMA_RxBuff, rx_len);
//}

/*-------------------------------------------------------------------------*/
/*读	PHYCFGR寄存器的值		                                         ------*/
/*------------------------------------------------------------------------*/
uint8 getPHYCFGR(void)
{
  uint8 addr;
  wiz_read_buf(PHYCFGR, &addr, 1);
  // wiz_read_buf(VERSIONR, &addr, 1);
  return addr;
}
