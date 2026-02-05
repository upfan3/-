#include "globalval.h"
#include "powercan.h"
extern s8 gmoduleTPdeay;
extern s8 gmoduleTPdeay2;
extern u8 setTP;
//u8 startCmdTP=0;
PowrCan PowCanDevice;	

// u8 test123=0;
// u32 test789=50;
//u8 taskcount=0;
//u8 taskcount2=0;
void CAN_Task(void * pvParameters)
{
   CanBuf rbuf ;
	 CanBuf rbuf2 ;
	 u8 canRutern=0;
	static u8 send2delay=0;
	
	while(1)
	{   /*-----------------------雷能协义------------------------------*/
	     while(pcan1->onRecvFrame(0,&rbuf)==true)//历遍所有缓冲数据帧 
		   {                                       //并将模块数据传到pModuledata中
          
			       canRutern=pcanruleln->DuleWithData(&rbuf,pModuledata);
			       if((canRutern==SET_ADDR)||canRutern==SET_MODULE_TP)
							    break;
								
				     
				}		 
							if(canRutern==SET_ADDR)//地址分配设置
						 {
							 pcan1->SendData(&rbuf);
						 }
             else if((gmoduleTPdeay==0)&&(genableTP==1))//使能防盗功能并且计时为0时，才触请求命令激活与绑定
						 {   
							  gmoduleTPdeay=-1;

							 for(u8 i=0;i<11;i++)
							 {

							   pcanruleln->ModudleTP(&rbuf,i);
							   pcan1->SendData(&rbuf);

								
							 }
							// startCmdTP=0;
							  
						 }	
              else if(setTP==1)//使能防盗功能并且计时为0时，才触请求命令//解绑
						 {   
							   setTP=0;

							 for(u8 i=0;i<6;i++)
							 {

							   pcanruleln->ModudleTP2(&rbuf,i);
							   pcan1->SendData(&rbuf);
								 
								
							 }
							gmoduleTPdeay=60;//解绑60秒后,重新发激活命令
							
							  
						 }
             else if(setTP==2)
						 {
							   setTP=0;
							   pcanruleln->ModudleTP3(&rbuf,5);//设置锁定延时
							   pcan1->SendData(&rbuf);
						 }	
              else if((gmoduleTPdeay2==0)&&(genableTP==1))
						 {
							    gmoduleTPdeay2=-1;
							   pcanruleln->ModudleTP3(&rbuf,0);//查询模块序列号
							   pcan1->SendData(&rbuf);
							    pcanruleln->ModudleTP3(&rbuf,3);//查询锁定延时时间
							   pcan1->SendData(&rbuf);
							    pcanruleln->ModudleTP3(&rbuf,4);//查询模块绑定状态
							   pcan1->SendData(&rbuf);
							 
							 
							 
						 }						 
						 else if(pcanruleln->SendCmd(&rbuf,pModuledata)==true)//突发命令
						 {  
							pcan1->SendData(&rbuf);
							
						 }
						 else if(pcanruleln->PollingModuleCmd(&rbuf)==true)//周期轮询协议中的命令
						 { 
									pcan1->SendData(&rbuf); 
							   
						 }
					 
					
					 
						 
						 
						 pcanruleln->ModuleOnLineDetect(pModuledata);
						
						 
						 /*-------------------------PowerCan--------------------------------------*/
						 
						while(pcan2->onRecvFrame(0,&rbuf2)==true) 
						{
							    PowCanDevice.DuleWiteData(&rbuf2);
						}
						while(pcan2->onRecvFrame(1,&rbuf2)==true)
						{
							    PowCanDevice.DuleWiteData(&rbuf2);
						}

				
			 
				 
      
						  u8 n;//是否连续发送 1继续发送，0结束发送
						 send2delay++;
						if(send2delay>4)
						{				send2delay=0;			

							 do{ 
										n=PowCanDevice.CanPowerOnTick(&rbuf2,pScreen);
										if(powerCanSendEnable==1)
										{
										 pcan2->SendData(&rbuf2);
											powerCanSendEnable=0;
										}
								 } while(n==1);
							 

						
					  }
						 PowCanDevice.UpDataUserEnergy();
						
						  PowCanDevice.OnMinTick();//约1分钟执行一次
						
						
						
						 ADMonlineDetect();

				     vTaskDelay(40 / portTICK_RATE_MS);		 
		 }
						 
		 
	
	
	
	

}
