#ifndef  __E2PROM_H
#define  __E2PROM_H

#include "i2c.h"

#define ADDR_24CXX        0xA0
#define ADDR_24CX1        0xA2



class E2PROM{
	

private:	
u8 m_device;	
I2C m_i2c;
bool WriteByte(u8 SendByte, u16 WriteAddress);
u8  ReadByte( u16 ReadAddress);

public:
void WriteString(u16 nAddr, u8* pDat, u16 nLen);	
void ReadString(u16 nAddr, u8* pDat, u16 nLen);

E2PROM(u8 dev,u16 i2c_delay)
{
	  m_i2c.Init(i2c_delay);
	  m_device=dev;
}



	
	
	
};



#endif
