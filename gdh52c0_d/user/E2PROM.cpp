#include "E2PROM.h"

//*************初始化CAN合局变量**********************************/
E2PROM gEprom(ADDR_24CXX,50), gEprom1(ADDR_24CX1,50);

//写入1字节数据       待写入数据    待写入地址       
bool E2PROM::WriteByte(u8 SendByte, u16 WriteAddress)   //E2P
{  
    u32 j;
    if(!m_i2c.Start())return FALSE;
    m_i2c.SendByte( m_device & 0xFE);//写器件地址 
    if(!m_i2c.WaitAck()){m_i2c.Stop(); return FALSE;}
    m_i2c.SendByte((u8)((WriteAddress>>8) & 0xFF));   //设置高起始地址      
    m_i2c.WaitAck(); 
    m_i2c.SendByte((u8)((WriteAddress) & 0xFF));   //设置低起始地址      
    m_i2c.WaitAck(); 
    m_i2c.SendByte(SendByte);           //写数据
    m_i2c.WaitAck();   
    m_i2c.Stop(); 
 //注意：因为这里要等待EEPROM写完，可以采用查询或延时方式(10ms)
    for(j=0;j<1500;j++)
      m_i2c.delay();
    return TRUE;
}


//读出1字节数据         存放读出数据  待读出长度      待读出地址       器件类型(24c16或SD2403) 
u8  E2PROM::ReadByte( u16 ReadAddress)	 //E2P
{  
    u8 temp;
    if(! m_i2c.Start())return FALSE;
    m_i2c.SendByte((m_device & 0xFE));//写器件地址 
    if(! m_i2c.WaitAck()){ m_i2c.Stop(); return FALSE;}
    m_i2c.SendByte((u8)((ReadAddress>>8) & 0xFF));   //设置高起始地址  
     m_i2c.WaitAck();
     m_i2c.SendByte((u8)((ReadAddress) & 0xFF));   //设置低起始地址      
     m_i2c.WaitAck();
     m_i2c.Start();
     m_i2c.SendByte((m_device & 0xFE)|0x01);    //读器件地址
     m_i2c.WaitAck();
   
    
    temp =  m_i2c.ReceiveByte();
    
     m_i2c.NoAck();
     
     m_i2c.Stop();
    return temp;
}

/************************************************************
E2PROM写入n个字节
nAddr：写入地址
nLen： 写入长度
*************************************************************/

void E2PROM::WriteString(u16 nAddr, u8* pDat, u16 nLen)	 //E2P
{
  u16 i;
  for(i=0;i<nLen;i++)
  {
    WriteByte(*(pDat+i), nAddr+i);
   
  }
}


/************************************************************
E2PROM读出n个字节
nAddr：读出地址
nLen： 读出长度
*************************************************************/
void E2PROM::ReadString(u16 nAddr, u8* pDat, u16 nLen)	  //E2P
{
    u16 i;
    for(i=0;i<nLen;i++)
      *(pDat+i)=ReadByte(nAddr+i);
}


//对I2C写8位数据

