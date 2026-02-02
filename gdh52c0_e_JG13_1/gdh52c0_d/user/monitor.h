#ifndef  __MONITOR_H
#define  __MONITOR_H

#include "Tqueue.h"

//#define FLOAT_MOD 0
//#define EQUAL_MOD 1
//#define TEST_MOD  2
//#define HAND_EQUAL_MOD  3  //手动均
//#define TEST_MOD2  4//休眠时，电池检测
//#define PEAK_MOD 5//
//#define MOD_NONE 6
#define I10 10//      10小时放电率




typedef enum
{
   FLOAT_MOD=0,        //浮充
   EQUAL_MOD,      //均充
   TEST_MOD,   //测试
	 MOD_NONE,   //无
	 TEST_MOD2, //电池监测
	 PEAK_MOD,
	// MOD_NONE  //无
} MonitorStatus ;	









class Monitor
{
	
public:	

	

u8 m_workMode;
 MonitorStatus m_workStatusModeFlag;


//  u16 m_starEualI;
//  u16 m_stopEualI;
  u16 m_starEqualI;
  u16 m_stopEqualI;
  u16 m_vailDischarge;
  u16 m_saveDischarge;
 



  u16 * m_psysV;
  s16 * m_pbattI;
  u16 * m_ploadI;

 s32  m_lmitI; //0.001A
 s32  m_OlmitI;

  
s8 m_sleephead;
s8 m_sleeptail;
s8 m_addr_down;





u8 m_limitBattDischargeflag;
//u8 m_testFlag;
u8 m_waitfordalay;	
u8 m_nolimitIflag;

u8 m_sleepLastFlag;

u8 m_sleeponff;

u8 m_sleepBattTest;
u8 m_onoffcount;

u16 m_waitTimeOut;
u16 m_TimeoutCount;

u32 m_Autobootcount;
u32 m_testPeriodCount;
u32 m_sleepTurnPeriodCount; 

u16 m_sleepBattTestTimeCount;
u16 m_sleepPeriodCount;

 
Monitor();
~Monitor();	
void Init();
void MonitorOnTick(void *prule);
void ChargeConversion(void);
void ChargeConversionAndSleepOnSec(void);

void getBattDischageSOC(u32 tick);
void LimitI(void *prule);
void Sleep(void *prule);
void SetVPara(void *prule);
void SendCmd(void *prule);
void LiDiangShiftMode(void *prule);
	
};





extern u8 setPowerLimitFlag;










#endif 

