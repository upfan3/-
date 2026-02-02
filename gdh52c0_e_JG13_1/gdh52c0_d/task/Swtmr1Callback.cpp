#include "globalval.h"
//extern u32 testEenergy;
extern u8 batttestcount;
extern u16 battdeletectcount;
extern u8 gscreensleep;
extern u8 gled2onoff;
extern u8 ghadBattFlagStatus;
extern s8 gmoduleTPdeay;
extern s8 gmoduleTPdeay2;
extern u8 setSpecialParaFlag;

u32 dischargeTimer=0;
u32 acTimer=0;
u8 getEnergyCount=GET_ENERGY_DELAY;
//u8 getDCEnergyCount=30;
u8 sendACflag=0;


u8 battBroadOnlineCount=0;
//u8 limitIcount=0;
void Swtmr1_Callback(void *parameter)
{  static s8 startdelay=90;
	   if(startdelay>0)
		 {
	      startdelay--;
			 // limitIcount++;
		 }
		   if(startdelay==0)
			 {
				 
				 genableTimeFlag=1;
				 setBattParaflag=3;
				 startdelay=-1;
				 
			 }
			 else if(startdelay==5)
			 {
				 gtimersyc=1;
			 }

	
	  if(genableTimeCount>0)
			genableTimeCount--;//开机十秒内不读取AD值 
		
/*		

			if((gbattDownMod==2)&&(ModuleOnlineMessage[1]==0))//模块数在线数为0
			{
				if(gbattDownCount>0)
					gbattDownCount--;
			}
			 else
		  {
			     gbattDownCount=gbattDownTime*60;
		  }
			
			
			if((gbattDownMod==1)&&(gpAlarmbit->bit_BATTDOWN_ALARM==1))
			{
				if(gbattDownDelay>0)
					gbattDownDelay--;
			}
*/
		
 	  pMonitor->ChargeConversionAndSleepOnSec();
	  SpeakOnTick();
			
				if(gledtimer==1)
			 {gledtimer=0;}
				 else
			 {gledtimer=1;}
			 
		if(ghadBattFlagStatus==1)
		{				
			 if(batttestcount>0)
			 {
				   batttestcount--;
			 }
	  }
			 
	 if(battdeletectcount>0)
		{
				   battdeletectcount--;
		}
	  
		//gIO.setOnOff(WARN_LED);
		
		if(isUpDataTimeOut>0)
		{
			isUpDataTimeOut--;
		}
		if(isUpDataTimeOut==0)
		{
			if(UpdateFlag==1)
			{
				ErrEscUpData();//清除标志位，释放分配的内存空间
			}
		}
		
		if(gscreensleep>0)
		{
			gscreensleep--;
		}
		
		if(gled2onoff>0)
		{
			gled2onoff--;
		}
		
		DownElectronicOnTick();
		

		
		if(delayPassWord>0)
			delayPassWord--;
		
		if(delayPassWord==0)
		{
			 setSpecialParaFlag=0;
		}
		
		
		
		if((gcourrentWorkStatus==2)||(gcourrentWorkStatus==3))
		{
			      dischargeTimer++;
		}
		if(((gcourrentWorkStatus==0)||(gcourrentWorkStatus==1)))//&&ModuleOnlineMessage[1]>0)
		{
				    acTimer++;
		}
		
		if(gmoduleTPdeay>0)
		{
			  gmoduleTPdeay--;
		}
		
		if(gmoduleTPdeay2>0)
		{
			 gmoduleTPdeay2--;
		}	

		
		//定时每隔1钟从电能表获取一次电能数据
		//testEenergy+=5;
		if(getEnergyCount>0)//读电能延迟计数
					getEnergyCount--;
    if(getEnergyCount==0)
		{
			  getEnergyCount=GET_ENERGY_DELAY;
			 getEnergyFlag=1;
		}
		else if(getEnergyCount==30)
		{
			sendACflag=1;
		}

		
		
detectTimer1=0;
 gcounter32++;
		
		
		if(battBroadOnlineCount>1)
		{
			  battBroadOnlineCount--;
		}

		
}
