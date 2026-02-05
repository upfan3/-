//#include "globalval.h"
#include "user.h"
u8 modelbusreq[]={0x6E,0x03,0x00,0x00,0x00,0x11,0x85,0xC6};//从 为免地址与空开冲突，电能表地址取110（0x6E）


void Uart_Task(void * pvParameters)
{
	 u8 static Inquire_Nubm=1;
		while (1)
	{
		
//上位机接口及测试用
	    pusart4->onRecvFrame((PTRFUN1)DuleWithData);
  
		
		
		
		 pusartSwitch->onRecvModulebus((PTRFUN1)DealWithJinGuiData);
		
		 u8 sendbuf[15],datalen=0;
		        Inquire_Nubm=JinGuiOnTick();
             
		          if(Inquire_Nubm<=TOTAL_USER)//智能空开数据
							{
									
							 datalen=JinGuiSendCmdData( Inquire_Nubm,sendbuf);
								if(datalen>0)
								{
									 pusartSwitch->CRCcheck(sendbuf,datalen);
									 pusartSwitch->SendData(sendbuf,datalen);
								}
							}
							else//请求电能表数据
							{
								   
									 pusartSwitch->SendData(modelbusreq,8);
							}
							
							
							
			 pusartBatt->onRecvModulebus((PTRFUN1)DuleWithDataBattery);		
            
                  BattCmdPolling();							
							
			
		vTaskDelay(300 / portTICK_RATE_MS);
	}
}