#ifndef  __I2C_H
#define  __I2C_H

#include "Tqueue.h"

class I2C{
	
private:		
u16 m_delay;	


public:
I2C();
void Init(u16 delay)
{
	 m_delay=delay;	
}
void delay(void);
bool Start(void);
void Stop(void);
void Ack(void);	
void NoAck(void);
bool WaitAck(void);  //返回为:=1有ACK,=0无ACK
void SendByte(u8 SendByte); //数据从高位到低位//
u8   ReceiveByte(void);  //数据从高位到低位//
};

#endif


