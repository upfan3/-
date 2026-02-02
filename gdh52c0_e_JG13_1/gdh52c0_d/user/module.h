#ifndef MODULE_H
#define MODULE_H

#include "Tqueue.h"
#include "canruleln.h"

extern u8 ModuleOnlineMessage[6];// 0: 1：模志实际在线数 2：已发现模块数 3：开关机状态 4：遍历模块序号 5：整流或光伏数目




class Module{
	

public:	
	
u16 *m_pcurr; //0：整流总电流  1：光伏总电流 2：C相 3：A相 4：B相   倍数关系   0/1:0.1A  2/3/4 :0.01A
u8  * m_pworkstatus;	
u8  *m_pmoduleaddr;
u8  *m_psetflag;
u8  *m_prfreshdata;
	
u32 m_ModuleCommStatus;//模块通信状态


u8  m_smrCount;
u8  m_scrCount;	
	

	
Module(u8 *paddr);	

void SetPara(u16 *_pcurr,u8 * _pworkstatus,u8 *_psetflag,u8  *_prfreshdata);
	
~Module();	
	
void AssignAndGetData(CanRuleLn *pruleCtrl,DATABASE *pdata);	
	
	
	
	
	
};




#endif

