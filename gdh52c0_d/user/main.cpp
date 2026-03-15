#include "globalval.h"




void Net_Task(void * pvParameters);
void AD_Task(void * pvParameters);
void Uart_Task(void * pvParameters);
void AC_PEAK_Task(void * pvParameters);


#ifdef __cplusplus
 extern "C" {
#endif

void HardFault_Handler(void)
{
	
	
	 SoftReset();//重启CPU
}


#ifdef __cplusplus
}
#endif


/*内存监测*/
void track_os(void * pvParameters)
{
	u8 *pbuf;

	  while(1)
	 { 
		   pbuf=(u8 *)pvPortMalloc (sizeof(u8)*512);
		   memset(pbuf, 0, 512);
		 sprintf((char *)pbuf,"--------------------------Last Memory:%d---------------------------\r\n",xPortGetMinimumEverFreeHeapSize());
		 strcat((char *)pbuf+strlen((const char *)pbuf),"Name          \tRun\tPrio\tLastStack\tNo.\r\n");
		  vTaskList((char *)pbuf+strlen((const char *)pbuf));
		 
#ifdef USE_RUN_TIME_STATS 
		 strcat((char *)pbuf+strlen((const char *)pbuf),"Name          \tRunCount      \tUseRate\r\n");
		  vTaskGetRunTimeStats((char *)(pbuf + strlen((const char *)pbuf)));
#endif
		  strcat((char *)pbuf,"------------------------------------------------------------------------\r\n");
		  pusartBatt->SendDataEx(pbuf,strlen((const char *)pbuf));
		   
		
		 vPortFree(pbuf);
		 
		 vTaskDelay(3000 / portTICK_RATE_MS);
		 
		
	 }
}


u8 t1=0;

int main(void)
{ 



	 nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);//抢占优先级4个，响应优先级4个
	 Rtc_configuration();
	
	
	
	
	
  
	
   InitData();
	 
	 
	dbg_periph_enable(DBG_FWDGT_HOLD);
  fwdgt_config(0xfff, FWDGT_PSC_DIV32);
  fwdgt_enable();

	
	       xTaskCreate(SCREEN_Task, (const char *)"OLED", 256, NULL, tskIDLE_PRIORITY + 6, NULL);
         xTaskCreate(CAN_Task, (const char *)"CAN", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY +5, NULL);
   		   xTaskCreate(Monitor_Task, (const char *)"MONITOR", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);
		     xTaskCreate(PORT_Task, (const char *)"PORT", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);//
				 xTaskCreate(Uart_Task, (const char *)"Uart", 256, NULL, tskIDLE_PRIORITY + 4, NULL);
			  // xTaskCreate(track_os, (const char *)"Track_os", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
	      
			  // xTaskCreate(Net_Task, (const char *)"Net", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 6, NULL);
        
		
		     vTaskStartScheduler();
	
     return 0;
}









