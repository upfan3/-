#include "globalval.h"


u8 gscreensleep=180;	
u8 gled2onoff=1;		
void SCREEN_Task(void * pvParameters)
{
	while(1)
	{
		 
		 
		  RTC_Get();
		
		
		  
		 
		  pModule->AssignAndGetData(pcanruleln,pModuledata);
		  pScreen->HMI();
		   
		   if(gscreensleep==0)
				{
					
					pScreen->ClearScreen();
					if(gled2onoff==0)
					{
						gled2onoff=1;
						
						gIO.setOnOff(RUN_LED);
					}
					
				}
				else
				{
					
					gIO.set(RUN_LED,LED_ON);
				}
		
		
		
	    
		  pScreen->FreshGram();
		  OnTickSetPara(pScreen);
		 vTaskDelay(200 / portTICK_RATE_MS);	
	}
}
