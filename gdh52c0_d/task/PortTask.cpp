#include "globalval.h"

extern u8 resetCount;
extern u8  specilCount;
extern u8  specilCount2;

u8 delayalarm=0;

CtrlState flag = MOS_OFF;
void PORT_Task(void * pvParameters)
{
	  while(1)
	{
		
		DownElectronicCtrl();
	  pgh52c0->portOntick();
		pgh52c0->SaveEnergyOnTick();
		DownElectronicDetectAC();
		DownElectricty(); 
		battCutCmd(LI_BATTERY_NUM);
//		SetBatteryflag(flag);
//		
		 delayalarm++;
		if(delayalarm>50)
		{
			  delayalarm=0;
			  OnTickAlarm();
		}
			
		
		BeeAndLed();
		ledIndicator();
		

		if((detectTimer1<250)&&(detectTimer2<250))
		{
			
			detectTimer1++;
			detectTimer2++;
		
			
		  fwdgt_counter_reload();
			
		}
		
		if(resetCount>5)
		{
			 resetCount=0;
			SoftReset();
		}
		
		if(specilCount>5)
		{
			showSpecilMenu=1;
		}
		
		
		if(specilCount2>10)
		{
			 specilCount2=0;
			 gOled.initSPI();
			 gOled.Init();//OLED≥ı ºªØ
		}
		
		
		
		 vTaskDelay(40 / portTICK_RATE_MS);	
	}
}

