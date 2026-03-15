#ifndef CYDT1363_H
#define CYDT1363_H

#include "globalval.h"

class CYDT1363
{
	

u8 *m_recvdata;
//u8 *m_datapoint;
	
u8 m_cid1;
u8 m_cid2;
u8 m_rtn;
u8 m_addr;
u8 m_ver;
u8 m_dataflag;
u8 m_MyVer;	  // 
u8 m_BigVer;  // 0:F 最新版协议 0x22 ,1:F0 最初版协议 0x22 ,2：TA2.0 铁塔2.0协议 0x10
u16 m_datlen;
u16 m_sdatlen; 
	
u16 m_warnCount[2];//获取告警计数

	
/*	
void set4041F(void);	
void set4141F(void);		
void set4241F(void);		
void set4341F(void);	
void setE141F(void);
void setE241F(void);	


void set4043F(void);
void set4143F(void);
void set4243F(void);
void set4343F(void);
void setE143F(void);

void set4044F(void);	
void set4144F(void);		
void set4244F(void);		
void set4344F(void);	
void setE144F(void);
void setE244F(void);	

void set4046F(void);
void set4146F(void);
void set4246F(void);
void setE146F(void);
void setE246F(void);


void set4048F(void);
void set4148F(void);
void set4248F(void);
void setE148F(void);
void setE248F(void);

void set4145F(void);
void set4345F(void);
void setE145F(void);

void setXXD4F(void);//获取告警事件记录
void set40D6F(void);
void set40D7F(void);
void setE1D5F(void);
void setXXD8F(void);//模拟量校准

void setXX4DF(void);
void setXX4EF(void);
void setXX51F(void);

void setXXDDF(void);

void set40D2F(void);
*/

void set4041(void);
void set4141(void);
void set4145(void);
void set4241(void);
void set4341(void);
void set42A7(void);
void set42A8(void);
void set42A9(void);
void set42AA(void);
void set42AB(void);
void setE241(void);
void set4A41(void);
void setE2A3(void);
void set4043(void);
void set4143(void);
void set4A43(void);
void set4343(void);

void set4A44(void);
void set4044(void);
void set4144(void);
void set4244(void);
void set4248(void);
void set4344(void);
void set4345(void);
void setE244(void);
void setE2A4(void);
void setE144(void);
void set4046(void);
void set4246(void);
void set4280(void);
void setE246(void);
void set4A46(void);
void setE1A0(void);

void setE2A6(void);
void setE2A6_old(void);
void setE2A5(void);
void setE2A5_old(void);
void setE2AC(void);
void setE2AC_old(void);
void setE2AD(void);
void setE2AD_old(void);

void set4048(void);
void set4A48(void);
void setE1A1(void);
void setE248(void);

void setE541(void);
void setE543(void);
void setE544(void);
void setE546(void);
void setE548(void);

void set42AE(void);
void set42AF(void);

void setE141(void);


void setXX4DF(void);
void setXX4EF(void);
void setXX51F(void);
void setXXD4F(void);//获取告警事件记录
void setXXD8F(void);//模拟量校准
void setXXDDF(void);

public:	
u8 *m_senddata;	

CYDT1363(void);
~CYDT1363(void);
void Init(u8 * pdat);
void SetVersion(u8 dat);//设置协议版本

void set1363Data(u8 dat_type,void *pdat);//设置1363数据
u8   Get1363Data(u8 *pdat);//获得1363数据	
void DuleWith1363Data(void);
void Set1363Packet(void);	
void ReturnVailData(u8 rtn);
u8   Getdatalen(u16 *op,u8 *pdat);	
void SetDataLen(u16 datlen);

void  HextoAscii(u8 *op,u8 dat);//16进制码A转为ASCii码		
void  SetOneByteData(u8 dat,u8 def=1);
void  SetTowByeteData(u16 dat,u8 def=1);
void  SetTowByeteDataL(u16 dat,u8 def=1);
void  SetFourByteData(float dat,u8 def=1);	
void  SetFourByteU32Data(u32 dat,u8 def=1);
void  SetOneByteSwitchStatus(u8 dat,u32 logic,u8 def=1);

u8  AsciitoHexOneByte(u8 *op,u8* pdat);	
u8  AsciitoHex16(u16 *op,u8* pdat);//16位ASCii码转为16进制码	
u8  AsciitoHex(u8 *op,u8* pdat);	
u8  AsciitoHex32L(u32 *op32 ,u8* pdat);//ASCII码转为32位 小端模式（浮点数用）
	
};




extern CYDT1363 com1363;







#endif
