#include "globalval.h"
void Net_Task(void * pvParameters)
{ static u8 ch=0;
	u8 ret;
    while(1)
	{ 
	
	  do
		{
			 ret=ptcp->httpOnTick(80,&ptcp->m_rxinfo,ch);
         ch++;
		    if(ch==8)
				{
					 ch=0;
				   ret=0;
				}
		}while(ret==1);
	   vTaskDelay(200 / portTICK_RATE_MS);
	}

}