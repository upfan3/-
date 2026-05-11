//告警历史记录
#include"globalval.h"

extern u16 gydtdataflag[2];
extern u32  moduleswitchstatus;
extern u32  modulelimitstatus;
extern u8 battIndex;

int64_t dcdistributionflag=0;
int64_t dcdistributionflag2=0;
int64_t dcdistributionOverLoad=0;
int64_t dcdistributionbroken=0;
uint64_t dcdistributioncommbroken=0;


int32_t dcdistributionflag3=0;
int32_t dcdistributionflag4=0;
int32_t dcdistributionOverLoad1=0;
int32_t dcdistributionbroken1=0;
uint32_t dcdistributioncommbroken1=0;

//uint64_t regWarnbit=0;

//int64_t redcdistributionflag=0;
//int64_t redcdistributionflag2=0;
//int64_t redcdistributionOverLoad=0;
//int64_t redcdistributionbroken=0;
//uint64_t redcdistributioncommbroken=0;

u8 battbroadflag[4]={0}; // bit7:flash, bit6:通信状态,bit1:熔丝2,bit0:熔丝1
u8 battbroadfuse[8]={0};

/*
u32 lastmoduleswitchstatus=0;
u32 lastmodulelimitstatus=0;
u8  lastStatusForMod=0;
u8  lastWorkStatus=0;
u8  lastWorkStatus2=0;
u8  lastACRunMode=0;
u8  lastgSleepOnoff=0;
u8  lastgPeakShaveOnoff=0;
u16 lastDI=0;
*/
	u8 j; 
u16 gDiInPutFlag=0;
Alarmbit * gpAlarmbit=(Alarmbit *)&gWarnbit;
u16 alarmtail[6]={0xffff,0xffff,0xffff,0xffff,0xffff,0};//交流瓶，直流瓶，系统，模块，用户配电，队列总（尾指针）
u16 alarmhead[6]={0xffff,0xffff,0xffff,0xffff,0xffff,0};//交流瓶，直流瓶，系统，模块，用户配电，队列总（头指针）
u16 recordAlarmCount=0;// 第一个表示当显示告警记示位置，第二个表示E2ROM中，告警最大记示数
u8 alarmdisplay[3]={0,0,0};	
u8 alarmCount[]={0,0,0};	
u8 outAlarm[12]={0,0,0,0,0,0,0,0,0,0,0,0};

u8 acAlarmV[8],AlarmVolitage,ACstopflag,mod,galram,downflag,battbreak,locdDownFlag;
u32  moduleBreakFlag=0 ,modulecomm=0,moduleCommBreakFlag=0;

u8 Powerstopflag = 0;

u8 gledtimer;
u8 ation[]={ NO,UNNOMAL,UNNOMAL,BROKEN,BROKEN,BROKEN,BREAK,NO,BREAK,BREAK,BREAK,BREAK};
u8 shift[]={ 28,26,27,20,22,23,21,22,19,29,30,31};

u32 g_tick;

//u32 warn32[3]={0,0,0};
//u16 warn16=0;
//u8  warn8[3]={0,0,0};


extern u8 gAcOnLineFlag;

u8 ctrl_bee=0;

StatusPrioMap statusMap[] = {
		{FLAG_FAULT,        PRIO_FAULT},        // 故障
		{FLAG_ALARM,        PRIO_ALARM},        // 告警
		{FLAG_DISCHARGING,  PRIO_DISCHARGING},  // 放电
		{FLAG_CHARGING,     PRIO_CHARGING},     // 充电
		{FLAG_NORMAL,       PRIO_NORMAL},       // 正常
		{FLAG_OFF,          PRIO_OFF}           // 关机
};

// 全局状态变量（可同时标记多个状态，比如故障+放电）
u8 g_devStatusFlags = FLAG_NORMAL;

void removeModule(u8 nb,u8 type)//从1号开始
{
	u8 i, beh[2]={COMM_BROKEN,BROKEN};
	
	//清除告警显示
	 u32 Altype;
	
	for(u8 j=0;j<2;j++){
		
				((WarnStruct *)&Altype)->wtype=type;
				 ((WarnStruct *)&Altype)->wstatus=0;
				((WarnStruct *)&Altype)->nb=nb;
				((WarnStruct *)&Altype)->behavior=beh[j];//查找模块故障及通信故障
		
		  if(type == USER_DOWN)
			{
			  ((WarnStruct *)&Altype)->ext= nb;
				((WarnStruct *)&Altype)->nb=0;
			}
			 i=0;
				for(i=0;i<alarmCount[0];i++)
			{
						if(palarmtable[i]==Altype)
						
						 break;
			}
	  
	
	     if(i!=alarmCount[0]) //找到模块故障或通信故障
			 {
				for(;i<alarmCount[0];i++)
				{ 
						palarmtable[i]= palarmtable[i+1];
				}
				
					 alarmCount[2]--;
					 alarmCount[0]--;
				   alarmCount[1]=alarmCount[2];
			}
			 
			
		}		
			
			
	if(type==MODULE)
	{
		romveModuleMasklist&=(~((u32)1<<(nb-1)));	//设置屏敝位，忽略该位对应的告警
	}
	else if(type==USER_DOWN)
	{
		nb=nb-1;
		if(nb<64)
		{
				dcdistributioncommbroken&=(~((uint64_t)1<<(nb)));
				SwitchOnlineCount[nb]=0;
		}
		else if(nb>=64)
		{
			 dcdistributioncommbroken1&=(~((uint64_t)1<<(nb-64)));
			 SwitchOnlineCount[nb]=0;
		}
	}
	

   	if(alarmCount[2]==0)
		{
		  ctrl_bee=0;
			g_devStatusFlags &=~FLAG_ALARM;
			g_devStatusFlags |= FLAG_NORMAL;
		}

}

void minusAlarm(AlarmType type,u8 nb,AlarmBehavior behavior,u32 ext)
{ u8 i,writeIndex,removeCount=0,activeCount;
	 u16 count=0;
	
	
	 u32 Altype;
		((WarnStruct *)&Altype)->wtype=type;
	   ((WarnStruct *)&Altype)->wstatus=0;
	  ((WarnStruct *)&Altype)->nb=nb;
	  ((WarnStruct *)&Altype)->behavior=behavior;
	  ((WarnStruct *)&Altype)->ext=ext;
	
	  if(alarmCount[2]>100)
		    activeCount=100;
	  else
		    activeCount=alarmCount[2];
	
	  writeIndex=0;
	  for(i=0;i<activeCount;i++)
	  {
		    if(palarmtable[i]==Altype)
		    {
			      removeCount++;
		    }
		    else
		    {
			      palarmtable[writeIndex]=palarmtable[i];
			      writeIndex++;
		    }
	  }
	  
	  if(removeCount==0) return ;//没有要去掉的告警
	
	  alarmCount[2]-=removeCount;
	  alarmCount[0]=alarmCount[2];
	  alarmCount[1]=alarmCount[2];
	          
	
	            ((WarnStruct *)&Altype)->wstatus=1;//设置告警恢复标志
	
	
	
	     u8 tmpSave[12]={0,0,0,0,0,0,0,0,0,0,0,0};
					    *((uTIME*)&tmpSave[0])=gTimer;//保存时间
							*((u16 *)&tmpSave[6])=Altype;//保存告警信息
							 *((u32 *)&tmpSave[8])=ext;
	        	RecordInfo Record={ &recordAlarmCount,&galarmRecordMax,NULL};	
	
	        count=pgh52c0->SaveRecordInfo(ALARM_DATA_FlAG,&Record,ALARM_RECORD_LEN,1);
					
						
	                pgh52c0->writeI2C(ALARM_DATA_STAR+12*count,tmpSave,12);
	                count++;//指向下一条记录位置（可看作当前记录数）

					        if(count>=ALARM_RECORD_LEN) count=0;
														 
														 
														 

					
															
	             pgh52c0->writeI2C(ALARM_DATA_LEN,(u8 *)&count,2);//写入存储记录数
	
	
	 	if(alarmCount[2]==0)
		{
			g_devStatusFlags &=~FLAG_ALARM;
			g_devStatusFlags |= FLAG_NORMAL;
		}
	   ctrl_bee=0;
	
}

void addAlarm(AlarmType type,u8 nb,AlarmBehavior behavior,u32 ext)
{  u16 count=0;
	 u8 i;
	
	
	  u32 Altype;
		((WarnStruct *)&Altype)->wtype=type;
	   ((WarnStruct *)&Altype)->wstatus=0;
	  ((WarnStruct *)&Altype)->nb=nb;
	  ((WarnStruct *)&Altype)->behavior=behavior;
	   ((WarnStruct *)&Altype)->ext=ext;


	  for(i=0;i<alarmCount[2] && i<100;i++)
	  {
		    if(palarmtable[i]==Altype)
		    {
			      return;
		    }
	  }
	  palarmtable[alarmCount[0]]=Altype;
	 alarmCount[0]++;
	if(alarmCount[0]>99)
	   alarmCount[0]=0;
	 if( alarmCount[2]<100)
	  alarmCount[2]++;
	 alarmCount[1]= alarmCount[2];
	 
	           u8 tmpSave[12]={0,0,0,0,0,0,0,0,0,0,0,0};
					    *((uTIME*)&tmpSave[0])=gTimer;//保存时间
							*((u16 *)&tmpSave[6])=Altype;//保存告警信息
						  *((u32 *)&tmpSave[8])=ext;
	        	RecordInfo Record={ &recordAlarmCount,&galarmRecordMax,NULL};	
	
	               count=pgh52c0->SaveRecordInfo(ALARM_DATA_FlAG,&Record,ALARM_RECORD_LEN,1);
						
						  
						
	                pgh52c0->writeI2C(ALARM_DATA_STAR+12*count,tmpSave,12);
	                count++;//指向下一条记录位置（可看作当前记录数）
	 

					        if(count>=ALARM_RECORD_LEN) count=0;
														

					
															
	             pgh52c0->writeI2C(ALARM_DATA_LEN,(u8 *)&count,2);//写入存储记录数
					    
					      
	 
	
	   ctrl_bee=3;
		 g_devStatusFlags |= FLAG_ALARM;	
	 
}
//void ledbattwarn(void)
//{
//	u8 warn=0;
//	u8 flag=0;
//	switch (flag)
//	{
//		case 0:
//			pgh52c0->m_io.set(WARN_LED,LED_ON);
//		break;
//		
//			case 1:
//			pgh52c0->m_io.set(WARN_LED,LED_ON);
//			
//	}


//}

//void battStatus(void)
//{
//	if()
//}


void ledIndicator(void)
{
    // 独立定时器 + 独立闪烁状态
    static uint32_t last_tick_green = 0;   // 绿灯专用定时器
    static uint32_t last_tick_red   = 0;   // 红灯专用定时器
    static uint8_t  blink_green     = 0;   // 绿灯独立闪烁状态
    static uint8_t  blink_red       = 0;   // 红灯独立闪烁状态

    uint8_t  highestPrio      = 0;
    uint8_t  baseStatePrio    = 0;

    // ===================== 获取最高优先级（红灯） =====================
    for (uint8_t i = 0; i < STATUS_MAP_NUM; i++)
    {
        if (g_devStatusFlags & statusMap[i].flag)
        {
            if (statusMap[i].prio > highestPrio)
                highestPrio = statusMap[i].prio;
        }
    }

    // ===================== 获取基础状态（绿灯） =====================
    if (g_devStatusFlags & FLAG_CHARGING)
        baseStatePrio = PRIO_CHARGING;
    else if (g_devStatusFlags & FLAG_NORMAL)
        baseStatePrio = PRIO_NORMAL;
    else if(g_devStatusFlags & FLAG_OFF)
        baseStatePrio = PRIO_OFF;

    // ===================== 【绿灯独立闪烁】=====================
    if (g_tick - last_tick_green >= BLINK_SLOW_INTERVAL)  // 绿灯固定慢闪
    {
        last_tick_green = g_tick;
        blink_green = !blink_green;
    }

    // ===================== 【红灯独立闪烁】=====================
    uint32_t red_interval = BLINK_SLOW_INTERVAL;
    if (highestPrio == PRIO_ALARM)
        red_interval = BLINK_FAST_INTERVAL;  // 告警 → 红灯快闪
    else
        red_interval = BLINK_SLOW_INTERVAL;  // 放电 → 红灯慢闪

    if (g_tick - last_tick_red >= red_interval)
    {
        last_tick_red = g_tick;
        blink_red = !blink_red;
    }

    // ===================== 绿灯控制 =====================
    switch (baseStatePrio)
    {
        case PRIO_NORMAL:
            pgh52c0->m_io.set(RUN_LED, LED_ON);
            break;
        case PRIO_CHARGING:
            blink_green ? pgh52c0->m_io.set(RUN_LED, LED_ON) : pgh52c0->m_io.set(RUN_LED, LED_OFF);  // 独立慢闪
            break;
        case PRIO_OFF:
            pgh52c0->m_io.set(RUN_LED, LED_OFF);
            break;
    }

    // ===================== 红灯控制 =====================
    switch (highestPrio)
    {
        case PRIO_FAULT:
            pgh52c0->m_io.set(WARN_LED, LED_ON);
						pgh52c0->m_io.set(RUN_LED, LED_OFF);	
            break;

        case PRIO_ALARM:
            blink_red ? pgh52c0->m_io.set(WARN_LED, LED_ON) : pgh52c0->m_io.set(WARN_LED, LED_OFF);  // 独立快闪
            break;

        case PRIO_DISCHARGING:
            blink_red ? pgh52c0->m_io.set(WARN_LED, LED_ON) : pgh52c0->m_io.set(WARN_LED, LED_OFF);  // 独立慢闪
            break;

        default:
            pgh52c0->m_io.set(WARN_LED, LED_OFF);
            break;
    }
}

void ledFlash(u8 ledflsah)//告警灯控制
{
	

//       if((ledflsah&0x01)==0x01)
//			 {
//				   if(gledtimer==1)
//					 {
//						 
//						 pgh52c0->m_io.set(WARN_LED,LED_ON);
//					 }
//					 else
//					 {
//						
//						 pgh52c0->m_io.set(WARN_LED,LED_OFF);
//					 }
//			 }
//			 else
//			 {
//				  
//				  pgh52c0->m_io.set(WARN_LED,LED_OFF);
//			 }

			  if((ledflsah&0x02)==0x02)
			 {
				   if(gledtimer==1)
					 {
							if(gbeeEnable==0)
							{ 
									pgh52c0->m_io.set(BEE,BEE_ON );
							}
						
					 }
					 else
					 {
						
						  pgh52c0->m_io.set(BEE,BEE_OFF );
					 }
			 }
			 else
			 {
				   
				   pgh52c0->m_io.set(BEE,BEE_OFF );
			 }

				
}



void OnTickAlarm(void)//告警产生
{ 
	
	
	 if(genableTimeFlag==0)return;
	
		
		{//DI输入告警 包括 门禁 水浸 烟雾 防雷 风机 空调 市电输入 油机输入 防雷开关 电池熔丝 负载熔丝1 负载熔丝2
			   for(u8 i=0;i<12;i++)
			{ 
				u16 diMask=1;
				u8   distatus=0;
	
				 if((pgh52c0->m_di._diData&(diMask<<i))!=0)
				 {
								distatus=1;
				 }
				 

				 
				 if(gInPutAlarm[i].st_AlarmTrigger==2)//不产生告警
				 {
					   if((gDiInPutFlag&(diMask<<i))!=0)//之前已发生该告警
							 {
					        minusAlarm((AlarmType)(gInPutAlarm[i].st_AlarmType+20),gInPutAlarm[i].st_InputNb,(AlarmBehavior)ation[gInPutAlarm[i].st_AlarmType]);
								 
								   
								         gWarnbit&=~((u32)1<<shift[gInPutAlarm[i].st_AlarmType]);
								    gDiInPutFlag&=~(diMask<<i);//清除告警标志		 
							 }
							 continue;
				 
				 }
				 
				 if(distatus==gInPutAlarm[i].st_AlarmTrigger)
				 {  
					 
					    if((gDiInPutFlag&(diMask<<i))==0)//之前未发生该告警
							 {

									
									
									  addAlarm((AlarmType)(gInPutAlarm[i].st_AlarmType+20),gInPutAlarm[i].st_InputNb,(AlarmBehavior)ation[gInPutAlarm[i].st_AlarmType]);
								 
								   
								      gWarnbit|=(u32)1<<shift[gInPutAlarm[i].st_AlarmType];//设置告警标志
								 
								    gDiInPutFlag|=(diMask<<i);//设置告警标志

							 }
				 }
				 else
				 {
					    
					    
					      if((gDiInPutFlag&(diMask<<i))!=0)//之前已发生该告警
							 {
					        minusAlarm((AlarmType)(gInPutAlarm[i].st_AlarmType+20),gInPutAlarm[i].st_InputNb,(AlarmBehavior)ation[gInPutAlarm[i].st_AlarmType]);
								 
								   
								         gWarnbit&=~((u32)1<<shift[gInPutAlarm[i].st_AlarmType]);
								    gDiInPutFlag&=~(diMask<<i);//清除告警标志
								 
										 

										 
										 
							 }
				 }
	
			}


		}		

      

		
		//
		{//直流母线电压（电池电压）    
					 AlarmVolitage=0;
					 if(*((u16 *)&gpSysData[DCVOLTAGE])>gOutOverVLimit)//过压
					 {
						if(gpAlarmbit->bit_OVER_V==0) 
						addAlarm(OUT_V,0,HIGH_);
						 gpAlarmbit->bit_OVER_V=1;
						
						AlarmVolitage=2;
					 }
					 else if(*((u16 *)&gpSysData[DCVOLTAGE])<(gOutOverVLimit-10))//过压
					 {
							if(gpAlarmbit->bit_OVER_V==1) 
						minusAlarm(OUT_V,0,HIGH_);
							gpAlarmbit->bit_OVER_V=0;

					 }
					 
					 
		}			 
	//
		{//市电备电切换
			if(powerBreak)
			{
					if(Powerstopflag == 0)
					{
							Powerstopflag = 1;
							acAlarmV[7]=0xE0;//交流停电
							addAlarm(ACINPUT, 0, STOP_SUPPLY);
					}
			}
			else
			{
					if(Powerstopflag == 1)
					{
							Powerstopflag = 0;
							acAlarmV[7]=0;
							minusAlarm(ACINPUT, 0, STOP_SUPPLY);
					}
			}

			
		}
		{//三相交流输入告警			 
					 
						 acAlarmV[0]=0;
						 acAlarmV[1]=0;
						 acAlarmV[2]=0;
			       acAlarmV[3]=0;
							//A相过压
							if(pgACmointor->st_Ua>gInOverVLimit)
							{
									if(gpAlarmbit->bit_IN_OVER_VA==0) 
//									 addAlarm(IN_VA,0,HIGH_);
									 gpAlarmbit->bit_IN_OVER_VA=1;
										 acAlarmV[0]=2;
							 }
							else  if(pgACmointor->st_Ua<(gInOverVLimit-200))//A相过压恢复
							{
									 if(gpAlarmbit->bit_IN_OVER_VA==1) 
//										 minusAlarm(IN_VA,0,HIGH_);
										gpAlarmbit->bit_IN_OVER_VA=0;
									 
							 }
							 
							 //C相过压
							 if(pgACmointor->st_Uc>gInOverVLimit)
							 {
								if(gpAlarmbit->bit_IN_OVER_VC==0) 
//								 addAlarm(IN_VC,0,HIGH_);
								 gpAlarmbit->bit_IN_OVER_VC=1;
								 acAlarmV[2]=2;
							 }
							 else if(pgACmointor->st_Uc<(gInOverVLimit-200))//C相过压恢复
							 {
								 if(gpAlarmbit->bit_IN_OVER_VC==1) 
//									 minusAlarm(IN_VC,0,HIGH_);
									gpAlarmbit->bit_IN_OVER_VC=0;
							
							 }

             
							                //B相过压
               if(pgACmointor->st_Ub>gInOverVLimit)
							 {
								if(gpAlarmbit->bit_IN_OVER_VB==0) 
//								 addAlarm(IN_VB,0,HIGH_);
								 gpAlarmbit->bit_IN_OVER_VB=1;
									acAlarmV[1]=2;
							 }
							 else if(pgACmointor->st_Ub<(gInOverVLimit-200))//B相过压恢复
							 {
								 if(gpAlarmbit->bit_IN_OVER_VB==1) 
//									 minusAlarm(IN_VB,0,HIGH_);
									gpAlarmbit->bit_IN_OVER_VB=0;
							
								
							 }
							 
						 
							 							//A相过流
							if(pgACmointor->st_Ia>gInOverILimit)
							{
									if(gpAlarmbit->bit_IN_OVER_IA==0) 
									 addAlarm(IN_I,1,HIGH_);
									 gpAlarmbit->bit_IN_OVER_IA=1;
								 }
							else
							{
										if(gpAlarmbit->bit_IN_OVER_IA==1) 
											minusAlarm(IN_I,1,HIGH_);
										gpAlarmbit->bit_IN_OVER_IA=0;
								 }
									
						 
					

							 
								 //B相过流
							if(pgACmointor->st_Ib>gInOverILimit)
							{
								if(gpAlarmbit->bit_IN_OVER_IB==0) 
								 addAlarm(IN_I,2,HIGH_);
								 gpAlarmbit->bit_IN_OVER_IB=1;
							 }
							else
							{
									if(gpAlarmbit->bit_IN_OVER_IB==1) 
										minusAlarm(IN_I,2,HIGH_);
									gpAlarmbit->bit_IN_OVER_IB=0;
							 }
							 
							 
							 //C相过流
							if(pgACmointor->st_Ic>gInOverILimit)
							{
								if(gpAlarmbit->bit_IN_OVER_IC==0) 
								 addAlarm(IN_I,3,HIGH_);
								 gpAlarmbit->bit_IN_OVER_IC=1;
							 }
							else
							{
									 if(gpAlarmbit->bit_IN_OVER_IC==1) 
										minusAlarm(IN_I,3,HIGH_);
									gpAlarmbit->bit_IN_OVER_IC=0;
							 }

 
						
						 if(gAcFlag==0)
						 {//三相交流停电状态
							 
									//三相交流停电告警产生
								 if(ACstopflag==0)
								 {
										ACstopflag=1;
//											acAlarmV[7]=0xE0;//交流停电
											 mod=2;//电池供电
//											addAlarm(ACINPUT,0,STOP_SUPPLY);
									}
								 
								 //停电时产生电池欠压
								 if(*((u16 *)&gpSysData[DCVOLTAGE])<gOutOwrVLimit)
								 {
										 if(gpAlarmbit->bit_OWR_V==0) 
										 addAlarm(BATTSUPPLY_ALARM,0,LOW_VOLT);
										gpAlarmbit->bit_OWR_V=1;
										 AlarmVolitage=1;
								 }
								 else if(*((u16 *)&gpSysData[DCVOLTAGE])>(gOutOwrVLimit+10))
								 {
										 if(gpAlarmbit->bit_OWR_V==1) 
											 minusAlarm(BATTSUPPLY_ALARM,0,LOW_VOLT);
										 gpAlarmbit->bit_OWR_V=0;
									
								 }
								 
								 
								//停电时不产生输出欠压告警，输出欠压恢复
								 {
										 if(gpAlarmbit->bit_OWR_V2==1) 
											 minusAlarm(OUT_V,0,LOW_VOLT);
											 gpAlarmbit->bit_OWR_V2=0;
									
								 }	 
										
								 //停电时，频率低恢复
								 {	 
							   	  if(gpAlarmbit->bit_OWR_FRQ==1) 
													minusAlarm(FRQ,0,LOW_);
													gpAlarmbit->bit_OWR_FRQ=0;
								 }
								 
								 //停电时，频率高恢复
								 {
								     if(gpAlarmbit->bit_OVER_FRQ==1) 
														minusAlarm(FRQ,0,HIGH_);
													gpAlarmbit->bit_OVER_FRQ=0;
								 }
								 
								  //停电时，交流电压A低恢复
								 {
									  if(gpAlarmbit->bit_IN_OWR_VA==1) 
//											minusAlarm(IN_VA,0,LOW_);
											gpAlarmbit->bit_IN_OWR_VA=0;
								 }
								 
								 //停电时，交流电压B低恢复
								 {
									  if(gpAlarmbit->bit_IN_OWR_VB==1) 
//											minusAlarm(IN_VB,0,LOW_);
											gpAlarmbit->bit_IN_OWR_VB=0;
								 }
								 
								 //停电时，交流电压C低恢复
								 {
									  if(gpAlarmbit->bit_IN_OWR_VC==1) 
//											minusAlarm(IN_VC,0,LOW_);
											gpAlarmbit->bit_IN_OWR_VC=0;
								 }
								 
								   //停电时，交流缺相A恢复
								  {
												 if(gpAlarmbit->bit_OWR_PHA==1) 
													 minusAlarm(OWR_PHA,0,NO);
													gpAlarmbit->bit_OWR_PHA=0;

									}
									
									 //停电时，交流缺相B恢复
								  {
												 if(gpAlarmbit->bit_OWR_PHB==1) 
													 minusAlarm(OWR_PHB,0,NO);
													gpAlarmbit->bit_OWR_PHB=0;

									}
									
									//停电时，交流缺相B恢复
								  {
												 if(gpAlarmbit->bit_OWR_PHC==1) 
													 minusAlarm(OWR_PHC,0,NO);
													gpAlarmbit->bit_OWR_PHC=0;

									}
	
								
						 }
						  else
						 { //三相交流非停电状态
								       //三相交流停电取消
											 if(ACstopflag==1) 
											 {  
													ACstopflag=0;
//													minusAlarm(ACINPUT,0,STOP_SUPPLY);
//													acAlarmV[7]=0;
													mod=0;
											}

									    //非停电状态下不产生电池低压告警，电池低压恢复
											 {
													 if(gpAlarmbit->bit_OWR_V==1) 
														 minusAlarm(BATTSUPPLY_ALARM,0,LOW_VOLT);
													 gpAlarmbit->bit_OWR_V=0;
												
											 }
											 
											 
											 
											 //输出欠压
											 if(*((u16 *)&gpSysData[DCVOLTAGE])<goutPutOwrV)
											 {
													 if(gpAlarmbit->bit_OWR_V2==0) 
													 addAlarm(OUT_V,0,LOW_VOLT);
													gpAlarmbit->bit_OWR_V2=1;
													 AlarmVolitage=1;
											 }
											 else if(*((u16 *)&gpSysData[DCVOLTAGE])>(goutPutOwrV+10))
											 {
													 if(gpAlarmbit->bit_OWR_V2==1) 
														 minusAlarm(OUT_V,0,LOW_VOLT);
													 gpAlarmbit->bit_OWR_V2=0;
												
											 }	
									     
									
									
									  	 //A相缺相
											 if((pgACmointor->st_Ua<5000)&&((gAcPhase&0x04)==0x04))
											 {
												if(gpAlarmbit->bit_OWR_PHA==0) 
												 addAlarm(OWR_PHA,0,NO);
												 gpAlarmbit->bit_OWR_PHA=1;
													acAlarmV[0]=3;
											 }
											 else
											 {
												 if(gpAlarmbit->bit_OWR_PHA==1) 
													 minusAlarm(OWR_PHA,0,NO);
													gpAlarmbit->bit_OWR_PHA=0;

											 }
										 
											 
											//A相欠压//在对应相位不缺相时，才能产生欠压告警
											if((pgACmointor->st_Ua<gInOwrVLimit)&& (gpAlarmbit->bit_OWR_PHA==0)&&((gAcPhase&0x04)==0x04))
											{
													if(gpAlarmbit->bit_IN_OWR_VA==0) 
//														 addAlarm(IN_VA,0,LOW_);
														 gpAlarmbit->bit_IN_OWR_VA=1;
														acAlarmV[0]=1;
												 }
											else if(pgACmointor->st_Ua>(gInOwrVLimit+200))
											{
													 if(gpAlarmbit->bit_IN_OWR_VA==1) 
//														minusAlarm(IN_VA,0,LOW_);
														gpAlarmbit->bit_IN_OWR_VA=0;


										 } 
											 
										 
									    //B相缺相
											if((pgACmointor->st_Ub<5000)&&((gAcPhase&0x02)==0x02))
										  {
											if(gpAlarmbit->bit_OWR_PHB==0) 
											 addAlarm(OWR_PHB,0,NO);
											 gpAlarmbit->bit_OWR_PHB=1;
												acAlarmV[1]=3;
										 }
										  else
										  {
											 if(gpAlarmbit->bit_OWR_PHB==1) 
													minusAlarm(OWR_PHB,0,NO);
												gpAlarmbit->bit_OWR_PHB=0;
 										
										 }
										 
										 
										 	//B相欠压，在对应相位不缺相时，才能产生欠压告警
										  if((pgACmointor->st_Ub<gInOwrVLimit)&&(gpAlarmbit->bit_OWR_PHB==0)&&((gAcPhase&0x02)==0x02))
											 {
												if(gpAlarmbit->bit_IN_OWR_VB==0) 
//												 addAlarm(IN_VB,0,LOW_);
												 gpAlarmbit->bit_IN_OWR_VB=1;
													acAlarmV[1]=1;
											 }
											 else if(pgACmointor->st_Ub>(gInOwrVLimit+200))
											 {
												 if(gpAlarmbit->bit_IN_OWR_VB==1) 
//													 minusAlarm(IN_VB,0,LOW_);
													gpAlarmbit->bit_IN_OWR_VB=0;
												 
											 }
											 
										 
										 
										 
										  //C相缺相
											if((pgACmointor->st_Uc<5000)&&((gAcPhase&0x01)==0x01))
										  {

											 if(gpAlarmbit->bit_OWR_PHC==0) 
											 addAlarm(OWR_PHC,0,NO);
											 gpAlarmbit->bit_OWR_PHC=1;
												acAlarmV[2]=3;
										  }
										  else
										  {
												if(gpAlarmbit->bit_OWR_PHC==1) 
													minusAlarm(OWR_PHC,0,NO);		 
												  gpAlarmbit->bit_OWR_PHC=0;										
												
										 }
										 
										  //C相欠压，在对应相位不缺相时，才能产生欠压告警
										 if((pgACmointor->st_Uc<gInOwrVLimit)&&(gpAlarmbit->bit_OWR_PHC==0)&&((gAcPhase&0x01)==0x01))
											{
												if(gpAlarmbit->bit_IN_OWR_VC==0) 
//												 addAlarm(IN_VC,0,LOW_);
												 gpAlarmbit->bit_IN_OWR_VC=1;
													acAlarmV[2]=1;
											}
											else if(pgACmointor->st_Uc>(gInOwrVLimit+200))
										  {
												 if(gpAlarmbit->bit_IN_OWR_VC==1) 
//													 minusAlarm(IN_VC,0,LOW_);
													gpAlarmbit->bit_IN_OWR_VC=0;
													
											}	
									
										 
											acAlarmV[3]=0;
											if((gAcPhase&0x07)!=0)//处于交流模式
											{  //频率过高
														if(pgACmointor->st_Frq>gOverFLimit/10)
													 {
															acAlarmV[3]=2;
														if(gpAlarmbit->bit_OVER_FRQ==0) 
														 addAlarm(FRQ,0,HIGH_);
														 gpAlarmbit->bit_OVER_FRQ=1;
														
													 }
														else if(pgACmointor->st_Frq<(gOverFLimit/10-5))
													 {
															if(gpAlarmbit->bit_OVER_FRQ==1) 
																minusAlarm(FRQ,0,HIGH_);
															gpAlarmbit->bit_OVER_FRQ=0;
														
													 }
				
													 
													 //频率过低
													if(pgACmointor->st_Frq<gOwrFLimit/10)
													 {
														 acAlarmV[3]=1;
														if(gpAlarmbit->bit_OWR_FRQ==0) 
														 addAlarm(FRQ,0,LOW_);
														 gpAlarmbit->bit_OWR_FRQ=1;
														 
													 }
													 else if(pgACmointor->st_Frq>=(gOwrFLimit/10+5))
													 {
															if(gpAlarmbit->bit_OWR_FRQ==1) 
															minusAlarm(FRQ,0,LOW_);
															gpAlarmbit->bit_OWR_FRQ=0;
															
													 }	
										 }
                     else//处于非交流模式
										 {
											 
											    if(gpAlarmbit->bit_OVER_FRQ==1) 
														minusAlarm(FRQ,0,HIGH_);
													  gpAlarmbit->bit_OVER_FRQ=0;
													
													if(gpAlarmbit->bit_OWR_FRQ==1) 
														 minusAlarm(FRQ,0,LOW_);
														gpAlarmbit->bit_OWR_FRQ=0;
											 
										 }											 

									
							 }
	 		
		}			 
		//			 
		{//模块告警							 
					u8 * pModuleWRANNING=(u8 *)pModuledata->GetDataAddr(WRANNING); 
					u8 * moduleph=	(u8 *)pModuledata->GetDataAddr(PHASEPOSITION);				 
												  u32  tmp=1;
			
			   if(ACstopflag==0){//只有不停电时，才产生模块告警
					 for(u8 i=0;i<ModuleOnlineMessage[2];i++)
					    {  
								
								   if((moduleph[i]%3==0x01)&&((gmoduleInputLowV&0x01)==0x01))
									 {
										   tmp<<=1;
										   continue;//跳过相应相位缺相的模块产生告警
									 }
									 
									  if((moduleph[i]%3==0x02)&&((gmoduleInputLowV&0x02)==0x02))
									 {
										   tmp<<=1;
										   continue;//跳过相应相位缺相的模块产生告警
									 }
								
								
								  	  if((moduleph[i]%3==0)&&(moduleph[i]!=0)&&((gmoduleInputLowV&0x04)==0x04))
									 {
										   tmp<<=1;
										   continue;//跳过相应相位缺相的模块产生告警
									 }
								
								
								
								  if((pModuleWRANNING[i]&0x01)==0x01)//模块故障
									{
										 if((moduleBreakFlag&tmp)==0)
											 addAlarm(MODULE,i+1,BROKEN);
										    moduleBreakFlag|=tmp;
									}
									else            
									{
										    if((moduleBreakFlag&tmp)!=0)
												minusAlarm(MODULE,i+1,BROKEN);//模块故障恢复
										 	moduleBreakFlag&=~tmp;
									}
								
								
								  if((pModule->m_ModuleCommStatus&tmp)==0)//通信断开
									{
										   if((moduleCommBreakFlag&tmp)==0)
										    addAlarm(MODULE,i+1,COMM_BROKEN);
									   	moduleCommBreakFlag|=tmp;
										
									}
									else//通信连接上
									{
										  if((moduleCommBreakFlag&tmp)!=0)
												minusAlarm(MODULE,i+1,COMM_BROKEN);//通信故障恢复
										 	moduleCommBreakFlag&=~tmp;
									}
								   tmp<<=1;
							}	
						}							
				}	
		//			
		{//环境温度告警
			 
												 if(*((s16 *)(&gpSysData[ENV_TEMP]))>ghtemp)
												 {
															if(gpAlarmbit->bit_BATT_HTEMP==0)
															{addAlarm(BATT_TEMP,0,HIGH_);
																 galram=0x80;//过高
															}
																	gpAlarmbit->bit_BATT_HTEMP=1;
															  
												 }
												 else if(*((s16 *)(&gpSysData[ENV_TEMP]))<(ghtemp-300))
												 {
															 if(gpAlarmbit->bit_BATT_HTEMP==1)
															 {minusAlarm(BATT_TEMP,0,HIGH_);
																  galram=0;//正常
															 }
													 
																gpAlarmbit->bit_BATT_HTEMP=0;
															
												 }										 
												 
												 if(*((s16 *)(&gpSysData[ENV_TEMP]))<((s16)gltemp)&&(*((s16 *)(&gpSysData[ENV_TEMP]))>-2900))
												 {
															if(gpAlarmbit->bit_BATT_LTEMP==0)
															{
																	addAlarm(BATT_TEMP,0,LOW_);
																  galram=0x80;//过低
															}
																	gpAlarmbit->bit_BATT_LTEMP=1;
															   
												 }
												 else  if(*((s16 *)(&gpSysData[ENV_TEMP]))>((s16)gltemp)+300)
												 {
																 if(gpAlarmbit->bit_BATT_LTEMP==1)
																 {
																	  minusAlarm(BATT_TEMP,0,LOW_);
																	 galram=0;//正常
																 }
																gpAlarmbit->bit_BATT_LTEMP=0;
																 
												 }	
											 


		

		 }
		
		
		//											 
		{//电池电流告警										 
												if(*((s16 *)(&gpSysData[TOTAL_BATTI]))<-15)
												 {
													 if(gpAlarmbit->bit_BATTSUPPLY_ALARM==0)
																	addAlarm(BATTSUPPLY_ALARM,0,NO);
																	gpAlarmbit->bit_BATTSUPPLY_ALARM=1;
												 }
												 else if(*((s16 *)(&gpSysData[TOTAL_BATTI]))>=0)
												 {
													if(gpAlarmbit->bit_BATTSUPPLY_ALARM==1)
																	minusAlarm(BATTSUPPLY_ALARM,0,NO);
													 
																	gpAlarmbit->bit_BATTSUPPLY_ALARM=0;
												 }
												 
												 
		}										 
		//									 																				 
		{//电池下电及告警
											 //电池供电故障
											 if(ghadbattflag==0)	
											 {
														 if(battbreak==0)		
														 {									 
																addAlarm(BATTSUPPLY_ALARM,0,BROKEN);
																battbreak=1;
														 }
																 
											 }
											else
											{
														 if(battbreak==1)		
														 {									 
															 minusAlarm(BATTSUPPLY_ALARM,0,BROKEN);
															 battbreak=0;
														 }
											}	
		}										 
    //
	
		{//配电单无告警
		
			 u8 moduleNum=0,moduleNum2 = 0;
			  for(j=0;j<TOTAL_USER;j++)
			{ 
			
				if(j<64)
				{
						if(SwitchOnlineCount[j]>0)
						{
							// moduleNum++;
								
								//下电告警
								if((gDCdistribution.pst_status[j]&0x01)==1)//&&(gSwitchPara[i].st_downV>*((u16 *)&gpSysData[DCVOLTAGE])))
								{
									 if((dcdistributionflag&(((int64_t)1<<j)))==0)
									 {
										 
										 if(gSwitchPara[j].st_userGroud!=0x7B)
										 {
												
											
													 			 
													 addAlarm(USER_DOWN,moduleNum,DOWNELE,j+1); 
										 }
										 else
										 {
												 
												 addAlarm(BATTDOWN_ALARM,moduleNum,DOWNELE,j+1); 
										 }
									 
											 dcdistributionflag|=((int64_t)1<<j);
									 }
								 }
								else
								{
									
									 if((dcdistributionflag&(((int64_t)1<<j)))!=0)
									 {
										 
										 if(gSwitchPara[j].st_userGroud!=0x7B)
										 {
													
												 minusAlarm(USER_DOWN,moduleNum,DOWNELE,j+1); 
										 }
											else
										 {
													
												 minusAlarm(BATTDOWN_ALARM,moduleNum,DOWNELE,j+1); 
										 }
									 
											 dcdistributionflag&=~((int64_t)1<<j);
									 }
									
								}
							
										
								//熔丝告警
								if((gDCdistribution.pst_status[j]&0x40)==0x40)//&&(gSwitchPara[i].st_downV<=*((u16 *)&gpSysData[DCVOLTAGE])))
								{
												 if((dcdistributionflag2&(((int64_t)1<<j)))==0)
												 {
													 
													 if(gSwitchPara[j].st_userGroud!=0x7B)
													 {
															
														
																addAlarm(USER_DOWN,moduleNum,BREAK,j+1); 				 
														 
													 }
													 else
													 {
															addAlarm(BATTFUSR_BREAK,moduleNum,BREAK,j+1);  
															
													 }
												 
														 dcdistributionflag2|=((int64_t)1<<j);
												 }
											 }
								else
								{
												
												 if((dcdistributionflag2&(((int64_t)1<<j)))!=0)
												 {
													 
													 if(gSwitchPara[j].st_userGroud!=0x7B)
													 {
															 minusAlarm(USER_DOWN,moduleNum,BREAK,j+1);  
															 
													 }
														else
													 {
															 minusAlarm(BATTFUSR_BREAK,moduleNum,BREAK,j+1);  
															
													 }
												 
														 dcdistributionflag2&=~((int64_t)1<<j);
												 }
												
											}
							
								
					
								
							//过载告警
								if((gDCdistribution.pst_status[j]&0x08)==0x08)
								{
									 
										 if((dcdistributionOverLoad&(((int64_t)1<<j)))==0)
										 {
											 addAlarm(USER_DOWN,moduleNum,OVER_LOAD,j+1); 
												dcdistributionOverLoad|=((int64_t)1<<j);
										 }
										
										 
									 
								 }
								else
								{
										 if((dcdistributionOverLoad&(((int64_t)1<<j)))!=0)
										 {
													 minusAlarm(USER_DOWN,moduleNum,OVER_LOAD,j+1); 
													dcdistributionOverLoad&=~((int64_t)1<<j);
										 }
									 
								 }
								
								//开关故障
								if((gDCdistribution.pst_status[j]&0x80)==0x80)
								{
									 
										 if((dcdistributionbroken&(((int64_t)1<<j)))==0)
										 {
											 addAlarm(USER_DOWN,moduleNum,BROKEN,j+1); 
												dcdistributionbroken|=((int64_t)1<<j);
										 }
										
								 }
								else
								{
										 if((dcdistributionbroken&(((int64_t)1<<j)))!=0)
										 {
													 minusAlarm(USER_DOWN,moduleNum,BROKEN,j+1); 
													dcdistributionbroken&=~((int64_t)1<<j);
										 }
									 
								 }
								
								
								
							
							
				
				
				
				
				
				
				
				  

                if(SwitchOnlineCount[j]==1)
							{
									if(((dcdistributioncommbroken)&((uint64_t)1<<j))==0)
									{
											dcdistributioncommbroken|=((uint64_t)1<<j);
										
											addAlarm(USER_DOWN,moduleNum,COMM_BROKEN,j+1); 
									}
							}
							else if(SwitchOnlineCount[j]>1)
							{
								if(((dcdistributioncommbroken)&((uint64_t)1<<j))!=0)
								{
										dcdistributioncommbroken&=(~((uint64_t)1<<j));
										
										minusAlarm(USER_DOWN,moduleNum,COMM_BROKEN,j+1); 
								}
							}

						}
	
		   
							
			
				}
				else if(j>=64)
				{
					  if(SwitchOnlineCount[j]>0)
						{
							// moduleNum++;
								
								//下电告警
								if((gDCdistribution.pst_status[j]&0x01)==1)//&&(gSwitchPara[i].st_downV>*((u16 *)&gpSysData[DCVOLTAGE])))
								{
									 if((dcdistributionflag3&(((int32_t)1<<(j-64))))==0)
									 {
										 
										 if(gSwitchPara[j].st_userGroud!=0x7B)
										 {
												
											
													 			 
													 addAlarm(USER_DOWN,moduleNum,DOWNELE,j+1); 
										 }
										 else
										 {
												 
												 addAlarm(BATTDOWN_ALARM,moduleNum,DOWNELE,j+1); 
										 }
									 
											 dcdistributionflag3|=((int32_t)1<<(j-64));
									 }
								 }
								else
								{
									
									 if((dcdistributionflag3&(((int32_t)1<<(j-64))))!=0)
									 {
										 
										 if(gSwitchPara[j].st_userGroud!=0x7B)
										 {
													
												 minusAlarm(USER_DOWN,moduleNum,DOWNELE,j+1); 
										 }
											else
										 {
													
												 minusAlarm(BATTDOWN_ALARM,moduleNum,DOWNELE,j+1); 
										 }
									 
											 dcdistributionflag3&=~((int32_t)1<<(j-64));
									 }
									
								}
							
										
								//熔丝告警
								if((gDCdistribution.pst_status[j]&0x40)==0x40)//&&(gSwitchPara[i].st_downV<=*((u16 *)&gpSysData[DCVOLTAGE])))
								{
												 if((dcdistributionflag4&(((int32_t)1<<(j-64))))==0)
												 {
													 
													 if(gSwitchPara[j].st_userGroud!=0x7B)
													 {
															
														
																addAlarm(USER_DOWN,moduleNum,BREAK,j+1); 				 
														 
													 }
													 else
													 {
															addAlarm(BATTFUSR_BREAK,moduleNum,BREAK,j+1);  
															
													 }
												 
														 dcdistributionflag4|=((int32_t)1<<(j-64));
												 }
											 }
								else
								{
												
												 if((dcdistributionflag4&(((int32_t)1<<(j-64))))!=0)
												 {
													 
													 if(gSwitchPara[j].st_userGroud!=0x7B)
													 {
															 minusAlarm(USER_DOWN,moduleNum,BREAK,j+1);  
															 
													 }
														else
													 {
															 minusAlarm(BATTFUSR_BREAK,moduleNum,BREAK,j+1);  
															
													 }
												 
														 dcdistributionflag4&=~((int32_t)1<<(j-64));
												 }
												
											}
							
								
					
								
							//过载告警
								if((gDCdistribution.pst_status[j]&0x08)==0x08)
								{
									 
										 if((dcdistributionOverLoad1&(((int32_t)1<<(j-64))))==0)
										 {
											 addAlarm(USER_DOWN,moduleNum,OVER_LOAD,j+1); 
												dcdistributionOverLoad1|=((int32_t)1<<(j-64));
										 }
										
										 
									 
								 }
								else
								{
										 if((dcdistributionOverLoad1&(((int32_t)1<<(j-64))))!=0)
										 {
													 minusAlarm(USER_DOWN,moduleNum,OVER_LOAD,j+1); 
													dcdistributionOverLoad1&=~((int32_t)1<<(j-64));
										 }
									 
								 }
								
								//开关故障
								if((gDCdistribution.pst_status[j]&0x80)==0x80)
								{
									 
										 if((dcdistributionbroken1&(((int32_t)1<<(j-64))))==0)
										 {
											 addAlarm(USER_DOWN,moduleNum,BROKEN,j+1); 
												dcdistributionbroken1|=((int32_t)1<<(j-64));
										 }
										
								 }
								else
								{
										 if((dcdistributionbroken1&(((int32_t)1<<(j-64))))!=0)
										 {
													 minusAlarm(USER_DOWN,moduleNum,BROKEN,j+1); 
													dcdistributionbroken1&=~((int32_t)1<<(j-64));
										 }
									 
								 }
								
								
								
							
							
				         if(SwitchOnlineCount[j]==1)
							{
									if(((dcdistributioncommbroken1)&((uint32_t)1<<(j-64)))==0)
									{
											dcdistributioncommbroken1|=((uint32_t)1<<(j-64));
										
											addAlarm(USER_DOWN,moduleNum,COMM_BROKEN,j+1); 
									}
							}
							else if(SwitchOnlineCount[j]>1)
							{
								if(((dcdistributioncommbroken1)&((uint32_t)1<<(j-64)))!=0)
								{
										dcdistributioncommbroken1&=(~((uint32_t)1<<(j-64)));
										
										minusAlarm(USER_DOWN,moduleNum,COMM_BROKEN,j+1); 
								}
							}
				
				
				
				
				
				
				   }
	
		   
							
				}
			
			}
		}



		
		
		 
		
		

{//电池下电板
			for(u8 i=0;i<4;i++)
			{
					 if( gBattPara[i].battBreak1!=0)//告警
						{
							   if((battbroadflag[i]&0x01)==0)
								 {
									  addAlarm(IN_DV,i+1,BREAK);  
									  battbroadfuse[0]=0x03;
								 }
								 battbroadflag[i]|=0x01;
						}
			      else
						{
							  if((battbroadflag[i]&0x01)!=0)
								 {
									  minusAlarm(IN_DV,i+1,BREAK);  
									 battbroadfuse[0]=0;
								 }
								 battbroadflag[i]&=0xFE;
						}
						
//						 if( gBattPara[i].battBreak2!=0)//告警
//						{
//							   if((battbroadflag[i]&0x02)==0)
//								 {
//									  addAlarm(BATTFUSR_BREAK,2,BREAK);  
//									  battbroadfuse[1]=0x03;
//								 }
//								 battbroadflag[i]|=0x02;
//						}
//			      else
//						{
//							  if((battbroadflag[i]&0x02)!=0)
//								 {
//									  minusAlarm(BATTFUSR_BREAK,2,BREAK);  
//									  battbroadfuse[1]=0;
//								 }
//								 battbroadflag[i]&=0xFD;
//						}
						
						
						  if( gBattPara[i].battFlash!=0)//告警
						  {
								 if((battbroadflag[i]&0x80)==0)
								 {
									   addAlarm(IN_DV,i+1,FLASH_BROKEN);  
								 }
								 
								 battbroadflag[i]|=0x80;
							}
							else
							{
								 if((battbroadflag[i]&0x80)!=0)
								 {
									   minusAlarm(IN_DV,i+1,FLASH_BROKEN);  
								 }
								 
								 battbroadflag[i]&=0x7F;
							}
							
							
								if(gBattPara[i].battBroadOnlineCount==1)
								{
										if((battbroadflag[i]&0x40)==0)
										 {
													addAlarm(IN_DV,i+1,COMM_BROKEN);  
											 
													*((s16 *)&gBattPara[i].battCurr1)=0;
													*((s16 *)&gBattPara[i].battCurr2)=0;								 
										 }
										 
										 battbroadflag[i]|=0x40;
								}
								else
								{
										 if((battbroadflag[i]&0x40)!=0)
										 {
												 minusAlarm(IN_DV,i+1,COMM_BROKEN);  
										 }
										 
										 battbroadflag[i]&=0xBF;
								}
				}
													
			
		}





	   
		{//锂电告警
	       SetBattWarning();
		}




		if((gWarnbit&0x00000000007fffE0)!=0)//检查输入告警
		        outAlarm[0]=1;
				else
					  outAlarm[0]=0;
				
				  if((gWarnbit&0x0000000000000017)!=0)//检查输出告警
		        outAlarm[1]=1;
				else
					  outAlarm[1]=0;
				
			if(((moduleBreakFlag&romveModuleMasklist)|(moduleCommBreakFlag&romveModuleMasklist))!=0)//模块告警
					{
						outAlarm[2]=1;
					}
					else
					{
							outAlarm[2]=0;
					}
					
				 if((gWarnbit&0x00000000E0000000)!=0)//熔丝告警
		        outAlarm[3]=1;
				else
					  outAlarm[3]=0;
				
				  if((gWarnbit&0x000000000f800000)!=0)//环境
		        outAlarm[4]=1;
				else
					  outAlarm[4]=0;
				
				if((outAlarm[0]+outAlarm[1]+outAlarm[2]+outAlarm[3]+outAlarm[4])>0) //共公告警
				{
					outAlarm[5]=1;
				}
				else
				{
					outAlarm[5]=0;
				}
				
				if(locdDownFlag!=0)//负载下电
				{
					outAlarm[10]=1;
				}
				else
				{
					outAlarm[10]=0;
				}
				 if((gWarnbit&0x0000000000000008)!=0)//电池下电
				 {
					outAlarm[11]=1;
				}
				else
				{
					outAlarm[11]=0;
				}
				
							
			  for(u8 i=0;i<9;i++)			
				{
				 if( gInPutAlarm[i].st_behavior>0)
				{
				  if(outAlarm[gInPutAlarm[i].st_behavior-1]!=0)
					{
						//动作继电器
						 pgh52c0->setdo(13-i);//output13对应于实际硬件的 端口1     output8 对应于实际硬件的 端口5 
					}
					else
					{
						//关闭继电器
						 pgh52c0->clrdo(13-i);
					}
				}
				else
				{
					//关闭继电器
						 pgh52c0->clrdo(13-i);
				}
			}
				

					
}

void BeeAndLed(void)
{
	/*
						 if((gWarnbit!=regWarnbit)||((moduleBreakFlag&romveModuleMasklist)!=warn32[1])||
							        ((moduleCommBreakFlag&romveModuleMasklist)!=warn32[2])||
						                                       (gDiInPutFlag!=warn16)||
						                                        (ACstopflag!=warn8[0])||
						                                        (locdDownFlag!=warn8[1])||
						                                        (battbreak!=warn8[2])||
						                    (redcdistributionflag!=dcdistributionflag)||
						                    (redcdistributionflag2!=dcdistributionflag2)||
						                  (redcdistributionOverLoad!=dcdistributionOverLoad)||
						                  (redcdistributionbroken!=dcdistributionbroken)||
						                  (redcdistributioncommbroken!=dcdistributioncommbroken)
						                                     
						 )	
						 
							  {

									     
										
									
									
									 if((gWarnbit!=0)||((moduleBreakFlag&romveModuleMasklist)!=0)||
							        ((moduleCommBreakFlag&romveModuleMasklist)!=0)||
						                                       (gDiInPutFlag!=0)||
						                                        (ACstopflag!=0)||
						                                        (locdDownFlag!=0)||
						                                        (battbreak!=0)||
									                            (dcdistributionflag!=0)||
									                            (dcdistributionflag2!=0)||
									                      (dcdistributionOverLoad!=0)||
									                      (dcdistributionbroken!=0)||
									                        (dcdistributioncommbroken!=0)
									 )
									 
									
									 {
										  ctrl_bee=3;
									 }
									 else
							     {

									    ctrl_bee=0;
								   }		

							 
									 

                     regWarnbit=gWarnbit;
										 warn32[1]=(moduleBreakFlag&romveModuleMasklist);
										 warn32[2]=(moduleCommBreakFlag&romveModuleMasklist);
										 warn16=gDiInPutFlag;
									
                     warn8[0]=ACstopflag;
									   warn8[1]=locdDownFlag;
									   warn8[2]=battbreak;

                    redcdistributionflag=dcdistributionflag;
									  redcdistributionflag2=dcdistributionflag2;
									  redcdistributionOverLoad=dcdistributionOverLoad;
									  redcdistributionbroken=dcdistributionbroken;
										redcdistributioncommbroken=dcdistributioncommbroken;
								}		
						*/	  
                  ledFlash(ctrl_bee);		


}


/*---------------------------------------------
type:DI类型
output：告警输出标识
nb:同一告警类型不同输入通道，默认为0通道
----------------------------------------------*/
u8 GetWarnInPut(u8 type,u8 output,u8 nb)
{ 
	u8 result=0;
	 for(u8 i=0;i<12;i++)
	{
		 if ((gInPutAlarm[i].st_AlarmType==(type-20))&&(gInPutAlarm[i].st_InputNb==nb))
		 {
	  
					if((gDiInPutFlag&((u16)1<<i))!=0)
					{
						  result=output;
					}

		 }
	}
	
	return result;
}

u8 GetDiInput(u8 n,u8 output)
{
	 u8 result=0;
				if((gDiInPutFlag&((u16)1<<n))!=0)
				{
						result=output;
				}
					
	return result;
}



/*-----------------------------------
禁止或启用电池分路 1或2熔丝告警
参数：n 1或2 指定电池熔丝分路
   act 0/1/2 0：常用 1：常闭 2：禁用
-----------------------------------*/
void DisableBattInPut(u8 n,u8 act)
{
	
	   if((n<1)||(n>2))return ;
	   if(act>2) return ;
	
	
	  u8 count=0;
	   for(u8 i=0;i<12;i++)
	  {
			   if (gInPutAlarm[i].st_AlarmType==9)
		 {
			 
			  if(count<2)
				{
			    
					if((count+1)==n)
					 gInPutAlarm[i].st_AlarmTrigger=act;
			 
			   count++;
				}
	  	}
		}
	
	
	
	
}







