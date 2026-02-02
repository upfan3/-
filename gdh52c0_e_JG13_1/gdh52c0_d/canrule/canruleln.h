#ifndef  __CANRULE_LN_H
#define  __CANRULE_LN_H

#include "can.h"
#include "database.h"
#include "monitor.h"

//------AccessObject---------
#define MONITOR_S   0xF0
#define BROADCAST   0

//------DuleWithObject-------
#define _NONE_ 0
#define SET_ADDR 1
#define SET_MODULE_TP 2
#define MENERROR 0x0F
#define RULEERROR 0x0E
#define ONLINE_TIME 500
#define FIRST_ONLINE_TIME 500
#define WAIT_TIME 200
#define WAIT_REC_TIME 5
#define RECV_SN_RROR 0x0C

#define SMR 0
#define SCM 0x80

//--------Protocol----------
#define  CURRENT_SHARE 0
#define  NORMAL 1
#define  SOFTWARE_UPLOAD 2
#define  LOOP_MEASUREMENT 3

//--------SystemType---------
#define BROADCAST 0
#define MONITOR   1
#define ACDC      2
#define DCDC      3
#define OVER_DET  4

//---------MessageType-------
#define SET_DATA 0
#define SET_DATA_RESPONSE 1
#define READ_DATA 2
#define READ_DATA_RESPONSE 3
#define READ_SERIAL_RESPONSE 7
#define SET_MODULE_ADDR 10

//-----------CMD-------------

#define CMD_SET_LED_FLASH  98//模块指示灯
#define CMD_SET_ONOFF 33 //模块开关机
#define CMD_SET_ADDR 13     //设置模块地址
#define CMD_SET_OUT_VOLTAGE 8 //设置输出电压
#define CMD_SET_CURRENT_LIMIT 20  //设置输出限流

#define CMD_GET_MODULE_STATUS 7    //读取模块状态字
#define CMD_GET_PS 77   //读模块设置的物理地址
#define CMD_GET_AC_V 2  //读交流输入电压
#define CMD_GET_AC_C 24  //读交流输入电流
#define CMD_GET_AC_F 30  //读交流频率
#define CMD_GET_OUT_V 0  //读输出电压
#define CMD_GET_OUT_I 1  // 读输出电流
#define CMD_GET_TMP 4    //读取模块温度
#define CMD_GET_SN 12    //读取模块序列号
#define CMD_GET_INFO 99   //读取模块特征值

#define CMD_GET_POWER 97  //读取功率
#define CMD_GET_ENERGY 96  //读取电能



//----------USER_CMD--------------

#define GET_OUT_V 0
#define GET_OUT_I 1
#define GET_STATUS 2
#define GET_AC_V   3
#define GET_AC_I   4
#define GET_AC_F   5
#define SET_OUT_VOLTAGE 6
#define SET_CURRENT_LIMIT 7

#define CMD_END 15
#define SET_ONOFF  15
#define SET__LED_FLASH 16
//----------ErroType----------
#define NO_ERROR 0
#define OTHER_ERROR 1
#define CMD_ERROR 2
#define DATA_ERROR 3
#define ADDR_ERROR 4

#define MODULE_0N 0
#define MODULE_0FF 1

#define MODULE_NUM 30


typedef struct _LeiNengSN
{
u32   st_snL;
u16		st_snH;
u16    st_onlinetimer;
_LeiNengSN  * st_pNext;
_LeiNengSN  * st_pPre;
u8    st_addr;		
}LeiNengSN;


typedef struct
{
	u32 st_Snl:6;
	u32 st_sAddr:8;
	u32 st_dAddr:8;
	u32 st_systemType:5;
	u32 st_protocol:2;
	u32 st_rev:3;
	
}Lnid;


typedef struct
{
	u32 st_mType: 4;
	u32 st_eType:4;
	u32 st_mAddrandFrame:8;
	u32 st_cmdType:16;
	u32 st_cmdData;
		
}Lndata;




class CanRuleLn
{
	
	  private:
	 	public:
	LeiNengSN *m_psn,*m_ptotalBuf;
		
    u32 m_ModuleAddrlist;
	  u32 m_ModuleAddrlistOK;
		u32 m_setData;
		u32 m_vfloat;
		u32 m_vequal;
		u32 m_vshift;
		u32 m_vtest;
		u32 m_Ilimit;
    u32 m_OCIlimit;	
		u16 m_WaitTimer;
		
    u8 m_MoudleCount;
		u8 m_MoudleAccCount;
   	u8 m_WaitResponse;
		
		u8 m_cmdlist[17];
		s8 m_isCmd;
		
			
		MonitorStatus m_floatorequal;
		
		u8 m_setAddr;
		s8 m_setCmd;
		
		u8 m_prol;
		u8 m_cmd;
    //u8 m_setPowerLimitFlag;
		
CanRuleLn();
~CanRuleLn();
		
LeiNengSN * DeletSNList( LeiNengSN *psn);
LeiNengSN *ApplySNList(void);
		
u8 DuleWithData(PCanBuf pCanbuf,DATABASE *pdatabase);	
void ModuleOnLineDetect(DATABASE *pdatabase);
bool PollingModuleCmd(PCanBuf pCanbuf);
bool SendCmd(PCanBuf pCanbuf,DATABASE *pdatabase);	
void Init(u8 prol);		
void OnTick(PCanBuf pCanbuf,DATABASE *pdatabase);		
void SetModuleOnOff(u8 sn,u8 act);	
u8 ModudleTP(PCanBuf pCanbuf,u8 n);//激活
u8 ModudleTP2(PCanBuf pCanbuf,u8 n);//解绑
u8 ModudleTP3(PCanBuf pCanbuf,u8 n);//查询模块序列号

};







#endif
