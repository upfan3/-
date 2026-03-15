//#include "globalval.h"
#include "user.h"

u8 modelbusreq[]={0x80,0x03,0x00,0x00,0x00,0x11,0x9B,0xD7};//从 为免地址与空开冲突，电能表地址取128（0x80）
extern u8 battComType;


void Uart_Task(void * pvParameters)
{
	 u8 static Inquire_Nubm=1;
		while (1)
	{
		
//上位机接口及测试用
	    pusart4->onRecvFrame((PTRFUN1)DuleWithData);
  
		 pusartSwitch->onRecvModulebus((PTRFUN1)DealWithJinGuiData);
		 u8 sendbuf[15],datalen=0;
		 u8 airbuf[8];
		 u8 mt11buf[8];
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
							/*
							else if(Inquire_Nubm == 9)//温湿度数据
							{
								datalen = MT11SendCmdData(Inquire_Nubm,mt11buf);
								if(datalen >0)
								{
									pusartSwitch->SendData(mt11buf,datalen);
								}

							}
							else if(Inquire_Nubm>9 && Inquire_Nubm<18)//空调数据
							{
								datalen = AirSendCmdData(Inquire_Nubm,airbuf);
								if(datalen>0)
								{	
									pusartSwitch->SendData(airbuf,datalen);
									
								}
								
							}
							else if(Inquire_Nubm ==18)//请求电能表数据
							{		
								 pusartSwitch->SendData(modelbusreq,8);
								
							}
							*/
							
			if(battComType == MODBUS_COM)
			{
				pusartBatt->onRecvModulebus((PTRFUN1)DuleWithDataBattery);
				BattCmdPolling();						
			}
			else
			{
				bool dataParseDoneFlag = pusartBatt->onRecvCommon((PTRFUN1)DuleWithDataBatteryCommon);
				
				BattCmdPollingCommon(dataParseDoneFlag);
				calculateTotalData();
			}
												
							
			
		vTaskDelay(200 / portTICK_RATE_MS);
	}
}