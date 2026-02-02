#include "globalval.h"
void Monitor_Task(void * pvParameters)
{
	
	
	while(1)
	{  
	       pMonitor->MonitorOnTick(pcanruleln);
         SpeakOnTick();
	 
	    
     vTaskDelay(100/ portTICK_RATE_MS);
		}
}