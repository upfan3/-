

#include "powercan.h"
#include "globalval.h"

u32 gAcStopTime[2]={5,5};//,5,5,5};
u8 ticktimeDelay[4]={20,20,30,30};//前两个为下电计时，后两个为上电计时
u8 ticktimeAC[2]={0,0};//,0,0,0};
extern u16 gVac[7];//交流电信息
extern u32 preEnergy;
extern PowrCan PowCanDevice;	
extern u32 gTotalMrEnergy;
extern u16 gpower[3];
extern u8 stopPowrSupply;
u8 usart3ConnetTimeOut=0;
u8 acdelay=140;
u16 downloadstatus[2]={0,0};


u32 startEnergy[2]={0,0};
void DownElectronicOnTick(void) //下电延时计数，每秒调用1次
{
	for(u8 i=0;i<2;i++)
	{
		
		  if((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0))
			{
				if(acdelay>0)
					acdelay--;
			}
			else
			{
				 acdelay=140;
			}
		
		
		
		
		   if((gAcFlag==0)&&(downloadstatus[i]==0))//停电交流&非下电状态倒计时
			 {

				 
				     if(gAcStopTime[i]>0)
							gAcStopTime[i]--;
				 
			}
		
			 
		
			if((*((u16 *)&gpSysData[DCVOLTAGE])<gDcParam0[i].st_LDVoltage))
			{
				 if(ticktimeDelay[i]>0)
				  ticktimeDelay[i]--;
			}
			else
			{
				  ticktimeDelay[i]=20;//重置延时计数
			}
			
			if(*((u16 *)&gpSysData[DCVOLTAGE])>5000)
			{
				 if(ticktimeDelay[i+2]>0)
				  ticktimeDelay[i+2]--;
			}
			else
			{
				  ticktimeDelay[i+2]=30;//重置延时计数
			}
						
			
	}
					
}


void ADMonlineDetect(void)//三相模块在线检测
{
	if(gAcmOnLineCount<250)
				   gAcmOnLineCount++;
           if(gAcmOnLineCount==249)
					 {//交流模块掉失

						 
						 gAcOnLineFlag=0;//
						 preEnergy=0;
					 
					  }

		for(int i=0;i<3;i++)
	{
			if(acOnlineTimer[i]>0)
			{
				acOnlineTimer[i]--;
			}
			else if(acOnlineTimer[i]==0)//计数超时,模块不在线
			{
				gVac[i] = 0;  //将对应相位电压清零
			}
	
	}

}




void DownElectronicDetectAC(void)//交流在线检测
{
	

	
  if((gAcOnLineFlag==0)&&(usart3ConnetTimeOut==0)) //交流配电单元及电能表不在线，取整流模块输入电压
	{
			  


				 pgACmointor->st_Ua=gVac[1];//A相
				 pgACmointor->st_Ub=gVac[2];//B相
				 pgACmointor->st_Uc=gVac[0];//C相




							
					pgACmointor->st_Frq=gVac[3];//用模频率替代交流单元

			
			
			    pgACmointor->st_Ia=*(u16 *)&gpSysData[INA_CURR];
			     
			    pgACmointor->st_Ib=*(u16 *)&gpSysData[INB_CURR];
				
					pgACmointor->st_Ic=*(u16 *)&gpSysData[INC_CURR];
					
					
					
					pgACmointor->st_Pa=gpower[1]/10;	
					pgACmointor->st_Pb=gpower[2]/10;	
					pgACmointor->st_Pc=gpower[0]/10;	
					
					pgACmointor->st_totalenergy=gTotalMrEnergy/100;	
					

					if(
							((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0)&&((*(u16*)(&gpSysData[DCVOLTAGE])<(5305)))&&(acdelay==0))||
							((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0)&&((*(s16*)(&gpSysData[TOTAL_BATTI])<-50)))||
							((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0)&&(gStopTest==1))//停电延迟关
						)
					{
						 if(gAcFlag==1){
								gAcStopTime[0]=gDcParam1[0].st_LDDelay*6;//gDcParam1[n].st_LDDelay单位为0.1min 为6秒
								gAcStopTime[1]=gDcParam1[1].st_LDDelay*6;
									
								 gAcFlag=0;
								 stopPowrSupply=1;
						}

					}
					else
					{
							if(gAcFlag==0)
							{
								gAcFlag=1;	
								stopPowrSupply=0;									
							}
					}
		
		
		
		
		
	}
  else//交流检测板及交流电表存在时按下面方法处理
	{		
		
	  //pgACmointor->st_Frq=gVac[3];
		
		if(pgACmointor->st_Ua==0) 	//当A相电压为0时，以模块代替
       {
				 if(gVac[1]>0)
				 pgACmointor->st_Ua=gVac[1];
    
			 }
		if(pgACmointor->st_Ub==0)   //当B相电压为0时，以模块代替
      {
					
          if(gVac[2]>0)
					 pgACmointor->st_Ub=gVac[2];			
					
			}
		if(pgACmointor->st_Uc==0)   //当C相电压为0时，以模块代替
      {
				 
            if(gVac[0]>0)
					 pgACmointor->st_Uc=gVac[0];			
						
			}
			
			
			if( pgACmointor->st_Ia==0){pgACmointor->st_Ia=*(u16 *)&gpSysData[INA_CURR];}
			
			if( pgACmointor->st_Ib==0){pgACmointor->st_Ib=*(u16 *)&gpSysData[INB_CURR];}
			
			if( pgACmointor->st_Ic==0){pgACmointor->st_Ic=*(u16 *)&gpSysData[INC_CURR];}
			
			
			
			if(pgACmointor->st_Pa==0){  pgACmointor->st_Pa=gpower[1]/10;	}
			
			if(pgACmointor->st_Pb==0){  pgACmointor->st_Pb=gpower[2]/10;	}
			
			if(pgACmointor->st_Pc==0){  pgACmointor->st_Pc=gpower[0]/10;	}
			
			
		
				if(
						(pgACmointor->st_Ua<5000)&& (pgACmointor->st_Ub<5000)&&(pgACmointor->st_Uc<5000)	
					)
					{
							
						 if(gAcFlag==1){
									gAcStopTime[0]=gDcParam1[0].st_LDDelay*6;//gDcParam1[n].st_LDDelay单位为0.1min 为6秒
									gAcStopTime[1]=gDcParam1[1].st_LDDelay*6;
										
                   gAcFlag=0;
							     stopPowrSupply=1;
							}
						 
					}
					else//交流有电
					{
						
						if(gAcFlag==0)
						
						gAcFlag=1;
						 stopPowrSupply=0;
						
					}
		}

	   
	
}





void   setRelay(u8 ch,u8 act)//下电动作
{

	
	       if(ch==1)
	        pgh52c0->m_do._doBit.output2=act;//   
				 else if(ch==2)
					pgh52c0->m_do._doBit.output0=act;//   
   				
}




u8 actflag[2]={0,0};
void DownElectronicCtrl(void)//下电逻辑
{u8 i;
	u16 *precoverV=&recoverV1;
	

   if(genableTimeFlag==0)return;
		u8 tmp=1;
	  for(i=0;i<2;i++)
	{
		
			 switch( gDcParam0[i].st_downModle)
			 {
				 
				 case LD_NONE://0
				 {
				       if( (locdDownFlag&tmp)!=0)
							 {
							   
								 minusAlarm(BATTDOWN_ALARM,i+1,DOWNELE);
								
								 locdDownFlag&=~tmp;
				        
							 }
							 
							 if(actflag[i]==1)
								{
									actflag[i]=0;
										DisableBattInPut(i+1,0);//启用电池熔丝告警并设为常开
									setRelay(i+1,0);// 接上接触器，恢复供电
								}
				     
				     break;
				 }

				 case LD_VOL://1
				 case LD_STOP_V_TIM://2
         {
					 					  
											 
                     if(gAcFlag==0)//输入电压为0或停电
										 {
												

											 if(( gAcStopTime[i]==0)&&(gDcParam0[i].st_downModle==LD_STOP_V_TIM))//时长下电
													{
                                     if((locdDownFlag&tmp)==0)//下电操作
                                     {
																			   locdDownFlag|=tmp;	
          															 addAlarm(BATTDOWN_ALARM,i+1,DOWNELE);
																		 }																			 
																			downloadstatus[i]|=0x0002;
													}  
 
										 } 
										 
										   //电压下电
											if( (((*((u16 *)&gpSysData[DCVOLTAGE])<gDcParam0[i].st_LDVoltage))&&(ticktimeDelay[i]==0)))
											 {
                                  if((locdDownFlag&tmp)==0)//下电操作
                                     {
																			   locdDownFlag|=tmp;	
          															 addAlarm(BATTDOWN_ALARM,i+1,DOWNELE);
																		 }	
																  downloadstatus[i]|=0x0001;
																 
											 }
											 
											 
						
											 
										 
									 
											 
											
											 if((*((u16 *)&gpSysData[DCVOLTAGE])>precoverV[i])&&(ticktimeDelay[i+2]==0))
											 { 

												        downloadstatus[i]&=0xFFFE;// //电压下电表示恢复供电
											          if(gAcFlag==1)//时长或电量
													      downloadstatus[i]&=0xFFFD;// 表示恢复供电
												}

											 if(downloadstatus[i]>0)
												{
													 
													  if(actflag[i]==0)//下电操作
                            { 
															 actflag[i]=1;
															 DisableBattInPut(i+1,2);//禁用电池熔丝告警
															 setRelay(i+1,1);// 断开接触器，下电操作 
                              
													  }
												}
												else
												{
													if(actflag[i]==1)
													{
														actflag[i]=0;
														DisableBattInPut(i+1,0);//启用电池熔丝告警并设为常开
													  setRelay(i+1,0);// 接上接触器，恢复供电
													}
													 if( (locdDownFlag&tmp)!=0)
														{ 													 
															 locdDownFlag&=~tmp;
															 minusAlarm(BATTDOWN_ALARM,i+1,DOWNELE);
														}
												}
											 
	
					 break;
				 }	
			 
				 case LD_STOP_POWER: //3
				 {
				               if((locdDownFlag&tmp)==0)
														 {
																
															 addAlarm(BATTDOWN_ALARM,i+1,DOWNELE);
															 locdDownFlag|=tmp;
														 }
														 
														 
														  if(actflag[i]==0)//下电操作
                            { 
															 actflag[i]=1;
															 DisableBattInPut(i+1,2);//禁用电池熔丝告警
															 setRelay(i+1,1);// 断开接触器，下电操作 

													  }
														 
				       break;
					}
				 default:break;
														 
			 }
			 tmp<<=1;
 
	}
	
}
