

#include "powercan.h"
#include "globalval.h"

u32 gAcStopTime[6]={5,5,5,5,5,5};//,5,5,5};
u8 ticktimeDelay[12]={20,20,20,25,30,35,30,30,30,35,40,45};//前两个为下电计时，后两个为上电计时
u8 ticktimeAC[2]={0,0};//,0,0,0};
extern u16 gVac[7];//交流电信息
extern u32 preEnergy;
extern PowrCan PowCanDevice;	
extern u32 gTotalMrEnergy;
extern u16 gpower[3];
extern u8 stopPowrSupply;
u8 usart3ConnetTimeOut=0;
u8 acdelay=140;
u16 downloadstatus[6]={0,0,0,0,0,0};



u8 setBattParaAddr = 0;
TestLogicFlag testFlag = {0,1,1};
u8 setMospara = MOS_OFF;

u32 startEnergy[2]={0,0};
void DownElectronicOnTick(void) //下电延时计数，每秒调用1次
{
	
	u16 *precoverV=&recoverV1;
	
	
	
	 if((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0))
			{
				if(acdelay>0)
					acdelay--;
			}
			else
			{
				 acdelay=140;
			}
	
	
	
	if(genableTimeFlag==0)return;
	
	
	for(u8 i=0;i<6;i++)
	{
	
		
		 if((gAcFlag==0)&&(downloadstatus[i]==0))//停电交流&非下电状态倒计时
		 {

				 
				     if(gAcStopTime[i]>0)
							gAcStopTime[i]--;
				 
			}
		
			 
		
			if(((*((u16 *)&gpSysData[DCVOLTAGE]))<gDcParam0[i].st_LDVoltage)||(vagBatVolt<gDcParam0[i].st_LDVoltage))
			{
				 if(ticktimeDelay[i]>0)
				  ticktimeDelay[i]--;
			}
			else
			{
				  ticktimeDelay[i]=20;//重置延时计数
				if(i==2){ticktimeDelay[i]=20;}
				else if(i==3){ticktimeDelay[i]=25;}
				else if(i==4){ticktimeDelay[i]=30;}
				else if(i==5){ticktimeDelay[i]=35;}
			}
			
			if((*((u16 *)&gpSysData[DCVOLTAGE])>precoverV[i])&&(vagBatVolt>precoverV[i]))
			{
				 if(ticktimeDelay[i+6]>0)
				  ticktimeDelay[i+6]--;
			}
			else
			{
				  ticktimeDelay[i+6]=30;//重置延时计数
				 if(i==2){ticktimeDelay[i+6]=30;}
				 else if(i==3){ticktimeDelay[i+6]=35;}
				 else if(i==4){ticktimeDelay[i+6]=40;}
				 else if(i==5){ticktimeDelay[i+6]=45;}
				
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
				
					

//					if(
//							((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0)&&((*(u16*)(&gpSysData[DCVOLTAGE])<(5305)))&&(acdelay==0))||
//							((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0)&&((*(s16*)(&gpSysData[TOTAL_BATTI])<-50)))||
//							((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0)&&(gStopTest==1))//停电延迟关
//						)
          if((gVac[0]==0)&&(gVac[1]==0)&&(gVac[2]==0))
					{
						 if(gAcFlag==1){
								gAcStopTime[0]=gDcParam1[0].st_LDDelay*6;//gDcParam1[n].st_LDDelay单位为0.1min 为6秒
								gAcStopTime[1]=gDcParam1[1].st_LDDelay*6;
							  gAcStopTime[2]=gDcParam1[2].st_LDDelay*6;//gDcParam1[n].st_LDDelay单位为0.1min 为6秒
								gAcStopTime[3]=gDcParam1[3].st_LDDelay*6;
							  gAcStopTime[4]=gDcParam1[4].st_LDDelay*6;//gDcParam1[n].st_LDDelay单位为0.1min 为6秒
								gAcStopTime[5]=gDcParam1[5].st_LDDelay*6;
									
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
							  gAcStopTime[2]=gDcParam1[2].st_LDDelay*6;//gDcParam1[n].st_LDDelay单位为0.1min 为6秒
								gAcStopTime[3]=gDcParam1[3].st_LDDelay*6;
							  gAcStopTime[4]=gDcParam1[4].st_LDDelay*6;//gDcParam1[n].st_LDDelay单位为0.1min 为6秒
								gAcStopTime[5]=gDcParam1[5].st_LDDelay*6;
										
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





extern u8 downAct;
extern u8	setPoint;





void   setRelay(u8 ch,u8 act)//下电动作
{

	
	       if(ch==1)
	        pgh52c0->m_do._doBit.output2=act;//   
				 else if(ch==2)
					pgh52c0->m_do._doBit.output0=act;// 
         else if((ch>2)&&(ch<7))//3,4,5,6	
				 {
					 downAct = (act==0)?3:0;
					 setPoint = ch - 2;
					 setBattParaflag=2;
				 }					 
   				
}




u8 actflag[6]={0,0,0,0,0,0};
void DownElectronicCtrl(void)//下电逻辑
{u8 i;
	u16 *precoverV=&recoverV1;
	

  if(genableTimeFlag==0)return;
		u8 tmp=1;
	  for(i=0;i<6;i++)
	{
		
			 switch( gDcParam0[i].st_downModle)
			 {
				 
				 case LD_NONE://0
				 {
				       if( (locdDownFlag&tmp)!=0)
							 {
							   if(i<2){
								   minusAlarm(USER_BRANCH,i+1,DOWNELE);
								 }
								 else
								 {
									 minusAlarm(IN_DV,i-1,DOWNELE);
								 }
								
								 locdDownFlag&=~tmp; // 清除下电状态标志
				        
							 }
							 // 若之前执行过下电动作（actflag[i]=1），则恢复供电
							 if(actflag[i]==1)
								{
									actflag[i]=0;
									setRelay(i+1,0);// 接上接触器，恢复供电
								}
				     
				     break;
				 }

				 case LD_VOL://1
				 case LD_STOP_V_TIM://2  定时下电模式
         {
					 					  
					// 分支1：交流断电（gAcFlag=0）时的定时下电逻辑（仅LD_STOP_V_TIM模式生效			 
            if(gAcFlag==0)//输入电压为0或停电
						{	
											 if(( gAcStopTime[i]==0)&&(gDcParam0[i].st_downModle==LD_STOP_V_TIM))//时长下电
													{
                                     if((locdDownFlag&tmp)==0)//下电操作  
                                     {
																			   locdDownFlag|=tmp;	
          															 //addAlarm(USER_BRANCH,i+1,DOWNELE);
																			   if(i<2){
																					 addAlarm(USER_BRANCH,i+1,DOWNELE);
																				 }
																				 else
																				 {
																					 addAlarm(IN_DV,i-1,DOWNELE);
																				 }
																		 }																			 
																			downloadstatus[i]|=0x0002;
													}  
 
							} 
										 
										   //电压下电
										   // 若直流电压 < 预设下电阈值，且延迟计时结束（ticktimeDelay[i]=0）
						if( (
							    (
						        (*((u16 *)&gpSysData[DCVOLTAGE])<gDcParam0[i].st_LDVoltage)||    //母线电线少于设定值
						        ((vagBatVolt<gDcParam0[i].st_LDVoltage)&&(totalBattI<0))        //电芯电压少于设定值且处于放电状态
						      )
						       &&(ticktimeDelay[i]==0)
						    ))
						 {
                                  if((locdDownFlag&tmp)==0)//下电操作
                                     {
																			   locdDownFlag|=tmp;	
          															 if(i<2){
																					 addAlarm(USER_BRANCH,i+1,DOWNELE);
																				 }
																				 else
																				 {
																					 addAlarm(IN_DV,i-1,DOWNELE);
																				 }
																		 }	
																  downloadstatus[i]|=0x0001;// 记录“电压触发下电”
																 
						 }
											 
											 
						
											 
										 
									 
											 
											    // 恢复供电判断
    											// 若直流电压 > 恢复阈值，且恢复延迟计时结束（ticktimeDelay[i+6]=0）
											 if((*((u16 *)&gpSysData[DCVOLTAGE])>precoverV[i])&&(vagBatVolt>gDcParam0[i].st_LDVoltage)&&(ticktimeDelay[i+6]==0))
											 { 

												        downloadstatus[i]&=0xFFFE;// //电压下电表示恢复供电
											          if(gAcFlag==1)//时长或电量
													      downloadstatus[i]&=0xFFFD;// 表示恢复供电
												}
												//执行下电/恢复动作（核心）
											 if(downloadstatus[i]>0)
												{
													 
													  if(actflag[i]==0)//下电操作
                      				  { 
															 actflag[i]=1;
															 setRelay(i+1,1);// 断开接触器，下电操作 
                              
													  }
												}
												else
												{
													if(actflag[i]==1)
													{
														actflag[i]=0;
													  setRelay(i+1,0);// 接上接触器，恢复供电
													}
													 if( (locdDownFlag&tmp)!=0)
														{ 													 
															 locdDownFlag&=~tmp;
															// minusAlarm(USER_BRANCH,i+1,DOWNELE);
															  if(i<2){
																	 minusAlarm(USER_BRANCH,i+1,DOWNELE);
																 }
																 else
																 {
																	 minusAlarm(IN_DV,i-1,DOWNELE);
																 }
														}
												}
											 
	
					 break;
				 }	
			 // 模式LD_STOP_POWER（3：强制下电模式）
				 case LD_STOP_POWER: //3
				 {
				               if((locdDownFlag&tmp)==0)
														 {
																
															 //addAlarm(USER_BRANCH,i+1,DOWNELE);
															  if(i<2){
																 addAlarm(USER_BRANCH,i+1,DOWNELE);
															 }
															 else
															 {
																 addAlarm(IN_DV,i-1,DOWNELE);
															 }
															 locdDownFlag|=tmp;
														 }
														 
														 
														  if(actflag[i]==0)//下电操作
                            { 
															 actflag[i]=1;
															 setRelay(i+1,1);// 断开接触器，下电操作 

													  }
														 
				       break;
					}
				 default:break;
														 
			 }
			 tmp<<=1;
 
	}
	
}


void battCut(u8 addr)
{
	
			u8 lastStatus = g_devStatusFlags;
			u8 index = addr - 1;        

			if((*((u16 *)&gpSysData[DCVOLTAGE])) <=Volcut &&(*((u16 *)&gpSysData[DCVOLTAGE])) > 4200 && batt[index].Vbat > 0)
			{
	//			pgh52c0->setdo(0);
				setMospara = MOS_ON;
				testFlag.status = 1;
				g_devStatusFlags &= ~FLAG_CHARGING;
				g_devStatusFlags |= FLAG_DISCHARGING;
				
				
			}
			
			if((*((u16 *)&gpSysData[DCVOLTAGE])) > recoverVol)
			{


            if(totalSOC == 10000)
            {              
                g_devStatusFlags &= ~FLAG_CHARGING;
                g_devStatusFlags |= FLAG_NORMAL;
            }
            else
            {
                if(totalBattI > 0 && batt[index].Vbat > 0 && recvBattEnd == 1)
                {
										g_devStatusFlags &= ~FLAG_DISCHARGING;
                    recvBattEnd = 0;
		//							pgh52c0->clrdo(0);
//										testFlag.status = 0;
                    setMospara = MOS_OFF;
                    g_devStatusFlags |= FLAG_CHARGING;

									
                }
            }
						

						
			 }
			 if( !(lastStatus & FLAG_DISCHARGING) && (g_devStatusFlags & FLAG_DISCHARGING) )
			{
					disChargeCount[index]++;
					pgh52c0->savePara(&disChargeCount[index]);
			}	
			
			if( !(lastStatus & FLAG_CHARGING) && (g_devStatusFlags & FLAG_CHARGING)  )
			{
							chargeCount[index]++;
							pgh52c0->savePara(&chargeCount[index]);
			}
		
			

}


u8 sw =0;
void SetBatteryflag( CtrlState state)
{
	if(sw)
	{
		sw = 0;
		setDisChargeMos(1,state);
	}
}

void battCutCmd(u8 num)
{
	 for(u8 i=0;i<num;i++)
	{
		battCut(i+1);
	}
}
