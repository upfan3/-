#ifndef  _H52C0_H_
#define  _H52C0_H_
#include "gpio.h"










typedef struct
{
  u8 output0:1; 
  u8 output1:1;
  u8 output2:1;
  u8 output3:1;
  u8 output4:1;
  u8 output5:1;
  u8 output6:1;
  u8 output7:1;
  
  u8 output8:1;
  u8 output9 :1;
  u8 output10:1;
  u8 output11:1;
  u8 output12:1;
  u8 output13:1;
  u8 output14:1;
  u8 output15:1;
  

}doBit;

typedef union {
doBit _doBit; 
u16 _doData;
} h52c0_DO;





typedef struct
{
  u8 input0:1;
  u8 input1:1;
  u8 input2:1;
  u8 input3:1;
  u8 input4:1;
  u8 input5:1;
  u8 input6:1;
  u8 input7:1;
  
  u8 input8:1;
  u8 input9:1;
  u8 input10:1;
  u8 input11:1;
  u8 input12:1;
  u8 input13:1;
  u8 input14:1;
  u8 input15:1;
  

}diBit;

typedef union {
diBit _diBit; 
u16 _diData;
} h52c0_DI;

class h52c0
{

u8  * m_pdata8head;
u16 * m_pdata16head;
u8 * m_pdatatail;	
u16 m_lenth8;
u16 m_lenth16;
//u16 m_paraAddr;
public:
	h52c0(void);
	~h52c0(void);
	h52c0_DO m_do;
	h52c0_DI m_di;
  gpio m_io;
//  u16 m_V1;//系统电压1
//  u16 m_V2;//系统电压2
//  u16 m_I1;//传感器1
//  u16 m_I2;//传感器2
//  u16 m_I3;//分流器1
//  u16 m_I4;//分流器2
  
void Init(void);
  s16 m_result[6];

	void setdo(u8 _No);
	void clrdo(u8 _No);
  void portOntick(void);
  u32 setIvalue(s16 valuI,u8 ch);
  void GetAdc(void);
	void writeI2C(u16 nAddr, u8* pDat, u16 nLen);
	void readI2C(u16 nAddr, u8* pDat, u16 nLen);
	void writeI2C2(u16 nAddr, u8* pDat, u16 nLen);
	void readI2C2(u16 nAddr, u8* pDat, u16 nLen);
 void ad_calibration(u8 ch);
 void SaveEnergyOnTick(void);
 void GetEnergyOnBackupRegister(void);
 void savePara(void *paddr);
 void InitPara(void);
 s16 GetOffSet(void * paddr);
 void SaverUserEenergy(void);
 void SaveACDataToBuckUpReg(u8 n, void * pdat);
 void GetACDataFromBuckUpReg(u8 n, void * pdat);
 KeyStats ScanKey(void);
u16 SaveRecordInfo(u16 BaseAddr,void * pRecord,u16 ReLen,u8 ch);
void SaveSwitchPara(void *pdat,u16 _type=2);
s8 SaveFlashData16(u8 *pdat,u16 datlen);
s8 GetFlashData16(u8 *pdat,u16 datlen);
s8 SaveCRCData(u16 addr,u8 *pdat,u8 len);
s8 GetCRCData(u16 addr,u8 *pdat,u8 len);
};


#endif 
