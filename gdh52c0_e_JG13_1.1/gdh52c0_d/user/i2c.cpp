#include "i2c.h"





#define SCLH       {gpio_bit_set(GPIOB,GPIO_PIN_6);} // GPIOB->BSRR = GPIO_Pin_6
#define SCLL       {gpio_bit_reset(GPIOB,GPIO_PIN_6);} //  GPIOB->BRR  = GPIO_Pin_6 
   
#define SDAH        {gpio_bit_set(GPIOB,GPIO_PIN_7);} //  GPIOB->BSRR = GPIO_Pin_7
#define SDAL        {gpio_bit_reset(GPIOB,GPIO_PIN_7);} // GPIOB->BRR  = GPIO_Pin_7

#define SCLread     gpio_input_bit_get(GPIOB,GPIO_PIN_6) // GPIOB->IDR  & GPIO_Pin_6
#define SDAread     gpio_input_bit_get(GPIOB,GPIO_PIN_7) // GPIOB->IDR  & GPIO_Pin_7


//---------------------------GD32F105 usart硬件相关配置----------------------------------------------








void I2C_gpio_config(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
  gpio_init(GPIOB,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_6|GPIO_PIN_7);
}	
	 




I2C::I2C()
{
	 I2C_gpio_config();
}

void I2C::delay(void)
{ 
  u16 i=m_delay;
   while(i) //这里可以优化速度 ，经测试最低到5还能写入
   { 
     i--; 
   } 
}

bool I2C::Start(void)
{
 SDAH;
 delay();
 SCLH;
 delay();
 if(!SDAread)return FALSE; //SDA线为低电平则总线忙,退出
 SDAL;
 delay();
 if(SDAread) return FALSE; //SDA线为高电平则总线出错,退出
 SDAL;
 delay();
 return TRUE;
}

void I2C::Stop(void)
{
 SCLL;
 delay();
 SDAL;
 delay();
 SCLH;
 delay();
 SDAH;
 delay();
}

void I2C::Ack(void)
{ 
 SCLL;
 delay();
 SDAL;
 delay();
 SCLH;
 delay();
 SCLL;
 delay();
}

void I2C::NoAck(void)
{ 
 SCLL;
 delay();
 SDAH;
 delay();
 SCLH;
 delay();
 SCLL;
 delay();
}

bool I2C::WaitAck(void)   //返回为:=1有ACK,=0无ACK
{
 SCLL;
 delay();
 SDAH;   
 delay();
 SCLH;
 delay();
 if(SDAread)
 {
          SCLL;
          return FALSE;
 }
 SCLL;
 return TRUE;
}

void I2C::SendByte(u8 SendByte) //数据从高位到低位//
{
    u8 i=8;
    while(i--)
    {
        SCLL;
        delay();
        if(SendByte&0x80)
				{ SDAH;  }
        else 
        {SDAL;   }
        SendByte<<=1;
        delay();
        SCLH;
        delay();
    }
    SCLL;
}

u8 I2C::ReceiveByte(void)  //数据从高位到低位//
{ 
    u8 i=8;
    u8 ReceiveByte=0;

    SDAH;    
    while(i--)
    {
      ReceiveByte<<=1;      
      SCLL;
      delay();
      SCLH;
      delay(); 
      if(SDAread)
      {
        ReceiveByte|=0x01;
      }
    }
    SCLL;
    return ReceiveByte;
}