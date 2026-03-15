#include "monitor.h"
#include "globalval.h"
#include "battery.h"


u8 ghadbattflag=1;//测试 暂时认为有电池接入
u8 setPowerLimitFlag;

u8 ghadBattFlagStatus=0;
u8 batttestcount=0;
u16 battdeletectcount=3600;



u8 gChargeStatus=0;
u16 FloatOrEquVoltagLimit;



Monitor gMonitor;
Monitor *pMonitor=&gMonitor;

extern u8 gsynflag;
extern u8 pronlineNum ,onlineNum ;



s8 limitIaddr[30];//记录能够设置单相限流模块的真正地址,初始值应设为-1表示没有有效模块,取值0~29,表示1~30号模块

u8 preModuleCount=0;
u8 limitcount=0;
s16 gSetLimitI=550/*0.1A*/; 
//s16 gSetLimitI2;
u32 powrlimitA;/*A相限流值*/; 
u32 powrlimitB;/*B相限流值**/; 
u32 powrlimitC;/*C相限流值**/; 
u16 gModuleVoltage;
s16 prebattI=0;

u16 gEqudelay=0;
u16 gEquflag = 0;

Monitor::Monitor()
{
	
}

Monitor::~Monitor()
{
	
}

void Monitor::Init()
{
	
	m_workMode=MOD_NONE;//设置无手动转换
	m_waitfordalay=0;//清除浮/ 均充等待标专
	m_Autobootcount=gAutoPeriod*86400;////设置周期均充计时
  m_testPeriodCount=gtestPeriod*86400;//设置周期测试计时
	m_workStatusModeFlag=FLOAT_MOD;
	
	
	
 

	  m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;

		
	 // m_sleepBattTestTimeCount=gsleepBattTestTime*60;//休眠电池测试时长 gsleepBattTestTime单位取1分钟
	 m_sleepBattTestTimeCount=30;
	
	
	m_psysV=(u16 *)&gpSysData[DCVOLTAGE];
	m_pbattI=(s16 *)&gpSysData[TOTAL_BATTI];//使能总电流电流
	m_ploadI=(u16 *)&gpSysData[USER_CURR];

	
//	  m_starEualI=(gSetStartBRI*gSetBattC/I10);//*10 ;//乘10换算出实电流倍数
//	  m_stopEualI=(gSetEndBRI*gSetBattC/I10);//*10 ;//乘10换算出实电流倍数
	
	  m_nolimitIflag=1;
  	m_lmitI=55000; //0.001A
    m_OlmitI=55000;
}


void  Monitor::MonitorOnTick(void *prule)
{
	static u8 isfirstar=0;
  static u8 battdetectMod=0;
	 u32 TaskTick=xTaskGetTickCount();
  // u8  batstatus=3;
	
	if(isfirstar==0)
	{
		 isfirstar=1;
	   ((CanRuleLn *)prule)->SetModuleOnOff(BROADCAST,MODULE_0N);//第一运行时打开所有模块
	}
	
	
	
	
	         SetVPara(prule);
	
	         LimitI(prule);
	         //SendCmd(prule);
	         Sleep(prule); 
	
	
	
	         ChargeConversion();//充电模式转换
	
		if(m_sleepBattTest==0)// 如果不在“电池休眠测试”状态
			{
	       if(m_workStatusModeFlag==FLOAT_MOD)// 当前是浮充模式
					{
						((CanRuleLn * )prule)->m_floatorequal=FLOAT_MOD;
						

						   // 如果是AC模式且交流正常
							if((gWorkShiftMode==1)&&(gAcFlag==1))
							{
								//if(((ACRunMode==PeakMode)||(ACRunMode==SpikeMode))&&(pronlineNum==0))//在高峰，尖峰，平时段都保持最低安全电压
								if(ACRunMode!=RUNNULL)
								{

									  ((CanRuleLn * )prule)->m_floatorequal=PEAK_MOD;//峰值时段调输出电压

							  }
							}
							
					  			
				//		batstatus=3;//浮充
						gcourrentWorkStatus=0;//浮充
							
				
							
							
							 // 如果总电池电流小于-15A（放电状态）且不在特定模式：更新为放电状态
				if(((*((s16 *)&gpSysData[TOTAL_BATTI])<-15)||(totalBattI<-15))&&(	pMonitor->m_workStatusModeFlag!=2))//||(ModuleOnlineMessage[1]==0))
				 { 
   					// batstatus=1;//放电
					    gcourrentWorkStatus=3;//放电
				 }	
				 
				if(battdetectMod==1)//电池在线检测
				{
								((CanRuleLn * )prule)->m_floatorequal=TEST_MOD2;
					  // batstatus=1;//放电
				     gcourrentWorkStatus=2;//测试
				}
							
				///////////////////////////////电池在线检测////////////////////////////////////////////////////	
				if((*((s16 *)&gpSysData[TOTAL_BATTI])<0.1*gSetBattC)&&(*((s16 *)&gpSysData[TOTAL_BATTI])>-1*0.1*gSetBattC)&&(gcheckBatt==1))//充放电电流在1%以认为无电流，要进行电流在线检测
				{
					// 分支1：未检测过 + 时间条件满足   分支2：检测计数为0 + 电池额定容量有效（>0）
				 if(((ghadBattFlagStatus==0)&&(genableTimeFlag==1))||((battdeletectcount==0)&&(gSetBattC>0)))
				 {
					   battdeletectcount=3600;// 设置检测倒计时（如3600ms，控制下次检测间隔）
					   battdetectMod=1;
						 batttestcount=20;
						 ghadBattFlagStatus=1;// 标记为“检测中”
				 }
			 }
				else
				{
					 ghadBattFlagStatus=2;
					 battdeletectcount=3600;
					 ghadbattflag=1;
					 battdetectMod=0;
				}
	
					if(ghadBattFlagStatus==1)// 处于“电池检测中”状态
					{
							if(batttestcount==0)//12
							{
								 if( *((s16 *)&gpSysData[DCVOLTAGE])>5000)
								 {
									 ghadBattFlagStatus=2;//有电池
										//pMonitor->m_workMode=FLOAT_MOD;
										battdetectMod=0;
										ghadbattflag=1;
								 }
								 else
								 {
										ghadBattFlagStatus=3;//没电池
										//pMonitor->m_workMode=FLOAT_MOD;
										 battdetectMod=0;
										ghadbattflag=0;
								 }
								
							}
				   }
							
					//////////////////////////////////////////////////////////////////////////////////////////////	
							
							
							
							
					}
					//均充模式
					else if(m_workStatusModeFlag==EQUAL_MOD)
					{
						
						((CanRuleLn * )prule)->m_floatorequal=EQUAL_MOD;
						
						if((gWorkShiftMode==1)&&(gAcFlag==1))
							{
								//if(((ACRunMode==PeakMode)||(ACRunMode==SpikeMode))&&(pronlineNum==0))//在高峰，尖峰，平时段都保持最低安全电压
							  if(ACRunMode!=RUNNULL)
								{

									  ((CanRuleLn * )prule)->m_floatorequal=PEAK_MOD;//峰值时段调输出电压

							  }
							}
						
					//  batstatus=4;//均充
					  gcourrentWorkStatus=1;//均充
						
					}
					else if(m_workStatusModeFlag==TEST_MOD)
					{
						  ((CanRuleLn * )prule)->m_floatorequal=TEST_MOD;
						
						if((gWorkShiftMode==1)&&(gAcFlag==1))
							{
							  //if(((ACRunMode==PeakMode)||(ACRunMode==SpikeMode))&&(pronlineNum==0))//在高峰，尖峰，平时段都保持最低安全电压
							 if(ACRunMode!=RUNNULL)
								{

									  ((CanRuleLn * )prule)->m_floatorequal=PEAK_MOD;//峰值时段调输出电压

							  }
							}
							//   batstatus=1;//放电
				        gcourrentWorkStatus=2;//测试
					}
			}
			else
			{
				((CanRuleLn * )prule)->m_floatorequal=TEST_MOD2;
				      // batstatus=1;//放电
					    gcourrentWorkStatus=2;//测试放电
			}
	
		if(ghadBattFlagStatus==2)//电池存在,则计算SOC 及放电电量
			{	 
					   getBattDischageSOC(TaskTick);
				
				  
						pbattCap->Update(*((s16 *)&gpSysData[DCVOLTAGE]),*((s16 *)&gpSysData[TOTAL_BATTI]),TaskTick);

					 (*((u16 *)&gpSysData[BATT_SOC]))=pbattCap->SOC;
				 }
			else
			{
					 (*((u16 *)&gpSysData[BATT_SOC]))=0;// 电池不存在，将SOC设为0
				 }	

      
		if(ghadBattFlagStatus==3)//在处于无电池状态，检测到有1.5A电流存在，认为有电池
			{
					if(((*((s16 *)&gpSysData[TOTAL_BATTI])>0.1*gSetBattC))&& (gSetBattC>0))
					{ghadBattFlagStatus=2;
						ghadbattflag=1;
					}
					if(((*((s16 *)&gpSysData[TOTAL_BATTI])<-1*0.1*gSetBattC))&& (gSetBattC>0)) 
					{ghadBattFlagStatus=2;
					 ghadbattflag=1;
					}
				}	
				
	
			

    if(genableTP==1)//防盗功能
		{
		}			

}


void Monitor::ChargeConversion(void)
{



	
		m_starEqualI = (gSetStartBRI * gSetBattC / I10); //*10 ;//乘10换算出实电流倍数
	  m_stopEqualI = (gSetEndBRI * gSetBattC / I10);	//*10 ;//乘10换算出实电流倍数
	
	if(m_workStatusModeFlag==FLOAT_MOD)//处于浮充状态
	{
		////////////////////自动均充////////////////////////////////
	
		
		
		 
		       if((pbattCap->TotalDisChagerTime/1000)>=(gbootBattCap*60))//gbootBattCap用作放电时长设置，单位取分钟，按秒进行比较
							 InitSOC=1;
		
				  if(
						  ((gAutoBootEnable==1)&&(ghadbattflag==1)&&(InitSOC==1))
			  		)//处于错峰状态
					{    
                   

					    if(   
							    ((*m_pbattI) > m_starEqualI) && (m_starEqualI > 0)// //处于浮充状态时，系统充电电流大于0.7I10而电池容量大于0
							  )
								{   
									 

									 if(m_waitfordalay==0)//处于非延时等待状，则进入延时状等待状态
									 {
										  m_waitfordalay=1;//设置为延时等待
										  m_waitTimeOut=gstartdelay*60;//设置浮充延时倒计时
									 }	
									 
									 else{
										     ;//延时等待，不做任何事情
									     }
									 	 
								/////////////////////////延时判定执行///////////////////////////////////////////////////////////


										if(m_waitTimeOut==0)//延时到时
								    {
									    m_workStatusModeFlag=EQUAL_MOD;//进入均充状态
											gEquflag = 1;
										  m_waitfordalay=0;
										  m_TimeoutCount=gequalTimeMax*3600;//设置均充计数时长
											InitSOC=0;
										  return;
									  }
								}
								else if( gMaxBattI<m_starEqualI)
								{
											m_waitfordalay=0;//若发现电池电流少于均充启动电流，将等待标志清零，以重新设延时值
								}

	        }
				   else
				  {
					  m_waitfordalay=0;
				  }
					
		/////////////////////周期均充///////////////////////////////		
					if(
						   (gAutoPeriod>0)&&(ghadbattflag==1)&&(gSetBattC>0)
				    )
				  {   
						 
					   if(m_Autobootcount==0)
						 {
							        m_workStatusModeFlag=EQUAL_MOD;//进入均充状态
										  m_waitfordalay=0;
							        m_TimeoutCount=gequalTimeMax*3600;//设置均充计数时长
							        InitSOC=0;
							        return;
						 }
				  }
				 else
				  {
					  m_waitfordalay=0;
				  }
				 
				 
			/////////////////////周期测试///////////////////////////////				
					if(gtestPeriodEnable==1)
					{
						if(m_testPeriodCount==0)//测试周期到期
						{
							m_waitfordalay=0;
				    // m_testFlag=1;
					   m_TimeoutCount=gtestTimeout*3600;//设置测时长
					   m_workStatusModeFlag=TEST_MOD;
					   m_workMode=MOD_NONE;
					   gtestStarTimer=gTimer;
							return;
						}
					}
					
					
					
					if(m_workMode==TEST_MOD)//手动转测试
				 {
					  m_waitfordalay=0;
				    // m_testFlag=1;
					   m_TimeoutCount=gtestTimeout*3600;//设置测时长
					   m_workStatusModeFlag=TEST_MOD;
					   m_workMode=MOD_NONE;
					   gtestStarTimer=gTimer;
					   return;
				 }
				 else if(m_workMode==EQUAL_MOD)//手动转均充
				 {
					   //tmpMod=m_workMode;
				     m_waitfordalay=0;
					   m_TimeoutCount=gequalTimeMax*3600;//设置均充计数时长
				     m_workStatusModeFlag=EQUAL_MOD;
					   m_workMode=MOD_NONE;
					   return;
					 
				 }
				 
	 
	
	}
	else if(m_workStatusModeFlag==EQUAL_MOD)//处于均充状态
	{
	
			
								if(m_TimeoutCount==0)//达到最大均充时长
								{
												 m_workStatusModeFlag=FLOAT_MOD;//结束均充状态进入浮充
												 m_waitfordalay=0;	
												 m_Autobootcount=86400*gAutoPeriod;//均充转浮充时，重置周期均充计数值
												 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
												 gChargeStatus=0;
									       gEquflag = 0;
												 return ;
								}

								  if(m_workMode==FLOAT_MOD)//浮充模式
								{
									    m_workMode=MOD_NONE;
											m_waitfordalay=0;
									    m_workStatusModeFlag=FLOAT_MOD;
									    m_Autobootcount=86400*gAutoPeriod;//均充转浮充时，重置周期均充计数值
									    pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
									    gChargeStatus=0;
									    gEquflag = 0;
									    return ;	
								}
								
						     const u16 exitDelayThreshold = gEndDelay * 60;
								
                  if (((s16)(*m_pbattI) <= (s16)m_stopEqualI )|| (gEqudelay > exitDelayThreshold))							
									{
											m_workStatusModeFlag=FLOAT_MOD;//结束均充状态进入浮充
											m_waitfordalay=0;
											m_Autobootcount=86400*gAutoPeriod;;//均充转浮充时，重置周期均充计数值
											pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
										  gEquflag = 0;
											return ;
									}
				
									
								
				
				          
				
	}
	else if(m_workStatusModeFlag==TEST_MOD)//处于测试状态
	{
		
		    	if((m_TimeoutCount==0)||((* m_psysV)<gtestV)||(vagBatVolt<gtestV))//放电测试到时或系统电压少于测试电压
							{
									//	m_testFlag=0;
										m_workMode=MOD_NONE;
										m_workStatusModeFlag= FLOAT_MOD;
								     gtestStopTimer=gTimer;//记录结束时间
								     gtestSaveFlag=1;//设置保存标志
								      m_saveDischarge=m_vailDischarge;
 
								     m_testPeriodCount=gtestPeriod*86400;//重置周期测试计时
								     return ;
							}
						
						
						 if(m_workMode==FLOAT_MOD)//浮充模式
							{
									    m_workMode=MOD_NONE;
											m_waitfordalay=0;
								    //  m_testFlag=0;
									    m_workStatusModeFlag=FLOAT_MOD;
								      gtestStopTimer=gTimer;//记录结束时间
								      gtestSaveFlag=1;//设置保存标志
								        m_saveDischarge=m_vailDischarge;
								      m_testPeriodCount=gtestPeriod*86400;//重置周期测试计时
								      return ;
								
							}	
	}
	
	
}

void Monitor::ChargeConversionAndSleepOnSec(void)//每秒设用本函数一次
{
	
	if(m_workStatusModeFlag==TEST_MOD)
	{
		if(m_TimeoutCount>0)
		 m_TimeoutCount--;//测试时长，以秒为单位,最长计数为65536s
	}
	
	
	if(m_waitfordalay==1)//浮充转均充启动延时，均充转浮充结束延时
	{
		 if(m_waitTimeOut>0)
		  m_waitTimeOut--;//将分钟转秒为计算
	}
	
	
	
	
	if(m_workStatusModeFlag==EQUAL_MOD)
	{
		  if(m_TimeoutCount>0)
			 m_TimeoutCount--;//最大均充延时计数，以s钟为单位
		
		
	}
	
	
	
	if(m_workStatusModeFlag==FLOAT_MOD)
	{
		 if(gAutoPeriod>0)//周期均充使能状态下加秒
		 {
			 
			 if(m_Autobootcount>0)
			  m_Autobootcount--;//自动均充周期计数，以秒为单位

		 }

			 
		
	
		 if(gtestPeriodEnable==1)//周期测试使能状态下加秒
		 {
    	   if(m_testPeriodCount>0)
				 m_testPeriodCount--;//测试周期计数，以秒为单位
			 
		 }

			 
		
	}
	
		if((gSleepOnoff==1)&&(ghadbattflag==1))
		{
						if(m_sleepBattTestTimeCount>0)//拉低电压，测试电池是否存计数
							m_sleepBattTestTimeCount--;
			
			     if(m_sleepPeriodCount>0)//休眠测试周期
							{
								m_sleepPeriodCount--;
							}
			
							
			     if(m_sleepLastFlag==1)//达到休眠最优状态
							{
								if(m_sleepTurnPeriodCount>0)
								{
									m_sleepTurnPeriodCount--;//模块休眠周期轮换计数
								}
							}	
							
							
			
			
		}
	
	
	
	
}

void Monitor::SetVPara(void *prule)
{
	// static s16 disVtmp=0; 
	 static u16 saveBattV;
	 //对转换后的s16*指针进行 “解引用”（*操作符），读取该地址开始的 2 个字节数据，得到一个s16类型的值。
	 s16 distmp=*((s16 *)(&gpSysData[ENV_TEMP]));

	
	              ((CanRuleLn *)prule)->m_vfloat=(u32)(gSetfloatV*10);
	             
	
								 if(gtmpCompEn==1)//温度补偿使能,补偿浮充电压
								 {u16 tmpV;
									s16 tmpC;
									 

									        tmpC=distmp-2500;
									       if(tmpC>2000) tmpC=2000;   //最高保偿不超45度，超过45度按45度
									       if(tmpC<-2000) tmpC=-2000; //最低保偿不低于5度，不低于5度按5度
									 
									    tmpV=gSetfloatV*10-tmpC*gtmpFactor16/100;
									   ((CanRuleLn *)prule)->m_vfloat=(u32)(tmpV);
									 

								 }
								 
								 
								 	((CanRuleLn *)prule)->m_vequal = (u32)(gSetequalV * 10);

									if (gtmpCompEn == 1) // 温度补偿使能,补偿均充电压
									{
										u16 tmpV;
										s16 tmpC;

										tmpC = distmp - 2500;
										if (tmpC > 2000)
											tmpC = 2000; // 最高保偿不超45度，超过45度按45度
										if (tmpC < -2000)
											tmpC = -2000; // 最低保偿不低于5度，不低于5度按5度

										tmpV = gSetequalV * 10 - tmpC * gtmpFactor16 / 100;
										((CanRuleLn *)prule)->m_vequal = (u32)(tmpV);
										
										
									}
								

                    // 整流电压未达均充电压时，延时清零
        if (*((s16 *)&gpSysData[DCVOLTAGE])<gSetequalV-10)
        {
            gEqudelay = 0;
        }

               
	  
							
								
							  	if((gWorkShiftMode==1)&&(gAcFlag==1))//错峰状态
								  {  
										
										if((pronlineNum > 0)||(onlineNum > 0))//智能锂电时的错峰
										 {
											 
											    
													
											 
											 
											   LiDiangShiftMode(prule);
											 
											 
											 
										 }
										 else//非智能锂电时的错峰
										 {
									      u16 disbattC=gdisDeep*gSetBattC/1000;	 
										   if(m_limitBattDischargeflag==0)
												{
															((CanRuleLn *)prule)->m_vshift=gsafedisV*10;
															
														 if(m_vailDischarge>=disbattC)//当统计的放电电量大于电池放电深度时，将输出电压值设到电池电压值
														 {
																		m_limitBattDischargeflag=1;
																	//	disVtmp=0;
																	 saveBattV=(* m_psysV)*10;	
																		 
														 }
												}
											 else if(m_limitBattDischargeflag==1){//达到放电深度后，跟踪电池电压，使充放电电流为0
														
												    if(gsynflag>5)
														{

															     if((*m_pbattI)<-5)
																	 {
																		 if(saveBattV<55000)
																			 saveBattV+=5;
																	 }	
                                   else if((*m_pbattI)>5)														 
																	 {
																		 if(saveBattV>gsafedisV*10)
																			 saveBattV-=2;
																	 }
															
															     ((CanRuleLn *)prule)->m_vshift=saveBattV;
															
															
																		gsynflag=0;
														 }
													
												 }
											 }




									}
									else//非错峰状态
									{
												m_limitBattDischargeflag=0;
									}

							
							
							
							
							
	  
}




void SendPhaseLimitI(CanPort *pcan, u32 * _phaseLimitI, u8 * _phaddrcount){
                u8 *phasedata=(u8 *)pModuledata->GetDataAddr(PHASEPOSITION);// 各模块的相位位置（1=A相，2=B相，3=C相）
        
    for(u8 ph=0; ph<3 ;ph++ ){
        if(_phaddrcount[ph]>0 && _phaseLimitI[ph]>0){
            for(u8 idx=0; idx<30; idx++){
                if(phasedata[idx]%3 == ph && phasedata[idx]>0){
                    SetLimitI(pcan, idx+1, _phaseLimitI[ph]/_phaddrcount[ph]);
                }
            }
        }
    }
}
u16 battLimitI = 0; 
extern u8 limitIcount;
void Monitor::LimitI(void *prule)
{
	u8 msmrCount=0;//整流模块数量
  u8 mscrCount=0;//光伏模块数量
	u8 phaddrcount[3]={0,0,0};//记录各相位模块的数目
	static s16 preMaxBattI=0;
	static u8 preMaxCount=0;
	static u8 limitICountms = 0;
	
	u32 oklist=((CanRuleLn *)prule)->m_ModuleAddrlistOK;
	u16 * currdata=(u16 *)pModuledata->GetDataAddr(CURRENT);
	u16 * limitdata=(u16 *)pModuledata->GetDataAddr(LIMITCURRENT);
	u8 *onoffdata=(u8 *)pModuledata->GetDataAddr(ONOFF);// 各模块开关机状态（0=开机，非0=关机）
	u8 *typedata=(u8 *)pModuledata->GetDataAddr(MODULETYPE);// 各模块类型（ACDC=整流，DCDC=光伏
	u8 *phasedata=(u8 *)pModuledata->GetDataAddr(PHASEPOSITION);// 各模块的相位位置（1=A相，2=B相，3=C相）
	 
	*((s16*)&gpSysData[LOADCURR])=0;// 负载电流（初始化为0）
	*((s16*)&gpSysData[RECT_CURR])=0;// 整流模块总电流（初始化为0
	*((s16*)&gpSysData[SOLAR_CURR])=0;// 光伏模块总电流（初始化为0）
/////////////////////////////////模块电流统计，整流，光伏/////////////////////////////////////////////////////	 
	 for(u8 addr=0;addr<30;addr++)//统计模块类型及数量
	 { 
		 if((((u32)1<<addr)&oklist)!=0)//查找有效的模块
		 {
				 if(onoffdata[addr]==0)//查找开机的模块数
				 {
							 *((s16*)&gpSysData[LOADCURR])+=(currdata[addr]/10);//总模块电流 总电流单位取 0.1A
					 
						 if(typedata[addr]==ACDC)//整流模块数量统计
						{
							 msmrCount++;
						 *((s16*)&gpSysData[RECT_CURR])+=(currdata[addr]/10);
							

								if(phasedata[addr]%3==1) // 相位值%3=1 → A相
								{
									phaddrcount[1]++;    //统计A相模块个数
								}
								if(phasedata[addr]%3==2)// 相位值%3=2 → B相
								{
									phaddrcount[2]++;   //统计B相模块个数
								}
								if((phasedata[addr]%3==0)&&(phasedata[addr]>0))
								{
									phaddrcount[0]++;  //统计C相模块个数
								}

																									
						}
					 
						if(typedata[addr]==DCDC)//统计光伏数量统计
						{
							
							mscrCount++;
							// 累加光伏模块总电流（单位转换：0.01A → 0.1A）
							 *((s16*)&gpSysData[SOLAR_CURR])+=(currdata[addr]/10);
						}
				 }
		 }
	 }
					

	 

	 if(*((u16*)&gpSysData[LOADCURR])>(( * m_pbattI)+totalBattI))
	 {
		// 负载电流 = 总负载电流 - 电池电流（电池放电时，部分负载由电池承担）
		 ( *m_ploadI)=*((u16*)&gpSysData[LOADCURR]) - ( * m_pbattI)-totalBattI;
	 }
	else
	( *m_ploadI)=0;	
			
/*		
			//启动初期模块 / 电池状态不稳定，通过 “降额限流” 或 “按负载动态分配限流”，避免大电流冲击硬件（如模块过流、电池损伤）
			if((genableTimeFlag == 0)&&( sysPara[DELAY_VOLT_UP] == 1))  //监控起动90s内进行限流操
			{
				//电池当前电流（单位：0.1A），判断电池是否大电流放电
				  if(( * m_pbattI) > 50)
					{
						u32 limitI=300+( *m_ploadI); // 基础限流值=30A（300×0.1A）+ 实际负载电流
						// 1. 计算光伏模块过流保护值（OCIlimit）：按光伏模块数均分
						    u32 tmp= (limitI*100)/mscrCount;  // ×100适配系统单位（0.01A）
						
						     if(tmp>2000)// 上限20A（2000×0.01A），避免超模块硬件
						     ((CanRuleLn *)prule)->m_OCIlimit=tmp;        //设置模块限流最大输出
								 // 2. 计算整流模块限流值（Ilimit）：按整流模块数均分
								 tmp= (limitI*100)/msmrCount; 
								  if(tmp>2000)
								 ((CanRuleLn *)prule)->m_Ilimit=tmp;          //设置模块限流最大输出
						
					}
					else
					{
						 u32 tmp = gMaxModuleCurr/2;//+(gMaxModuleCurr/180)*limitIcount;
						          tmp = tmp*100; // 单位转换为0.01A（适配系统数据格式）
						
						      if(tmp>2000)
									{
										 ((CanRuleLn *)prule)->m_OCIlimit=tmp;        //设置模块限流最大输出
										 
										
										 ((CanRuleLn *)prule)->m_Ilimit=tmp;          //设置模块限流最大输出
									}
						 
					}
				
				
			}
			else
			{
			
			   if((msmrCount>=0)||(mscrCount>0))//模块数量大于0，即最小存在一个模块
				 {
					
					
								 if(gPeakShaveOnoff==1)
								 {
					
											if(preModuleCount!=msmrCount)//当整流模块数目发生变化时，对模块进行一次限功率设置
											{
													 powrlimitA=gAPhasePower*100;//totalpowr*10/3;
													 powrlimitB=gBPhasePower*100;//totalpowr*10/3;
													 powrlimitC=gCPhasePower*100;//totalpowr*10/3;
												
												 if(phaddrcount[1]>0)//A相存在模块
												 {
														
														 powrlimitA=powrlimitA*9*10;//系统效率接90%计算  放大100倍
														 powrlimitA=1000*powrlimitA/(*m_psysV)/phaddrcount[1]; //A相单模块平均限流
													
													}
													else
													{
														powrlimitA=0;
													}
													
													if(phaddrcount[2]>0)//B相存在模块
													{
														
														powrlimitB=powrlimitB*9*10;//系统效率接90%计算  放大100倍
														powrlimitB=1000*powrlimitB/(*m_psysV)/phaddrcount[2];//B相单模块平均限流
													
													}
													else
													{
														powrlimitB=0;
													}
													
													if(phaddrcount[0]>0)//C相存在模块
													{
														
														powrlimitC=powrlimitC*9*10;//系统效率接90%计算  放大100倍
														powrlimitC=1000*powrlimitC/(*m_psysV)/phaddrcount[0];//C相单模块平均限流
														
													}
													else
													{
														powrlimitC=0;
													}
													

													
													for(u8 i=0;i<30;i++)
													{
														
															 limitIaddr[i]=-1;
														
															if(typedata[i]==ACDC)
															{
																 if(phasedata[i]%3==1)
																	{
																		 limitdata[i]=powrlimitA;    //设置A相模块限流
																			limitIaddr[i]=i;
																	}
																	else if(phasedata[i]%3==2)
																	{
																		 limitdata[i]=powrlimitB;    //设置B相模块限流
																		 limitIaddr[i]=i;
																	}
																	else if((phasedata[i]%3==0)&&(phasedata[i]>0))
																	{
																		 limitdata[i]=powrlimitC;  //设置C相模块限流	
																		 limitIaddr[i]=i;
																	}
															}
														}
													
													preModuleCount=msmrCount;
													setPowerLimitFlag=1;
													limitcount=0;
												}		
													
													
									}
								 else//非限功率情况
								 {
										 ((CanRuleLn *)prule)->m_OCIlimit=gMaxModuleCurr*100;        //设置模块限流最大输出
										 ((CanRuleLn *)prule)->m_Ilimit=gMaxModuleCurr*100;          //设置模块限流最大输出
											 setPowerLimitFlag=0;
											 preModuleCount=0;
								 }

									
								
				 }
				
			}
			
*/
	limitICountms++;
	
	if(limitICountms < 100)
		return;
	
	limitICountms = 0;
										
	if(gMaxModuleCurr > 825)  // 75 * 1.1
		gMaxModuleCurr = 825;

	if((msmrCount > 0) && (msmrCount < 31))//模块数量大于0，即最小存在一个模块
	{
			u32 totalI = gMaxModuleCurr * 100 *  msmrCount;
			u32 phaseLimitI[3] = {0,0,0}; 
		
			for(u8 i=0 ;i<3 ;i++)
			{
				phaseLimitI[i] = phaddrcount[i] > 0 ? totalI / phaddrcount[i] : 0;
			}
																																													 
			if(gPeakShaveOnoff==1)
			{
					//限功率（削峰）使能）
					u32 setPower[3] = {gCPhasePower,gAPhasePower,gBPhasePower};                                                                   
					u32 outPutPower,inPutPower;

					for(u8 i = 0; i < 3; i++)
					{
							if((phaddrcount[i] > 0)&&((*m_psysV) > 0))
							{						
								if(setPower[i] > 300) 
									setPower[i] = 300;//输入功率最大限定为30KW
								 
								 inPutPower = setPower[i] * 100;
								 outPutPower = inPutPower * 9 * 10;//系统效率接90%计算  放大100倍
								 phaseLimitI[i] = 1000 * outPutPower / (*m_psysV); //模块各相输出电流
										
							}
							else
							{
									phaseLimitI[i] = 0;
							}
					 }
																										
			}
                                                
        
			if((( * m_pbattI) > 50) && (gSetBattC > 0) && *((u16*)&gpSysData[RECT_CURR]) > 0)
			{
				u16 loadI = 0;
				
				if(*((u16*)&gpSysData[RECT_CURR])>( * m_pbattI))
				{							
					loadI = *((u16*)&gpSysData[RECT_CURR]) - (*m_pbattI);										
				}
								
				gSetLimitI = gSetBattC  * ((s16)gSetBattLimitPerC) / 10;
				
				battLimitI = gSetLimitI + loadI ;//设限流值算出模块总限流
								
				if(gPeakShaveOnoff == 0)
				{											
					SetLimitI(pcan1,0,battLimitI * 100/msmrCount); //非削峰情况下，直接均分下发限流值进行电池限流 
					return;
				}
				else if(gPeakShaveOnoff == 1)
				{
					float allocateRateI[3];
											
					u32 totalPhaseI = phaseLimitI[0]+phaseLimitI[1]+phaseLimitI[2];
											
					if(battLimitI * 100 < totalPhaseI)
					{
							//电池限流值少于总的三相削峰限流
						for(u8 i=0; i<3 ;i++)
						{
							allocateRateI[i] = (float)phaseLimitI[i]/totalPhaseI;//计算各相电流占比																			
						}
									
						for(u8 i=0; i<3 ;i++)
						{
							phaseLimitI[i] = allocateRateI[i]*battLimitI*100;
						}
					}
					
					SendPhaseLimitI(pcan1, phaseLimitI, phaddrcount);
					return ;
			  	}
				}
					
				
				
			
			if(gPeakShaveOnoff == 1)
			{
				 SendPhaseLimitI(pcan1, phaseLimitI, phaddrcount);
			}
			else
			{
				SetLimitI(pcan1,0,totalI/msmrCount);
			}
   }
}


void Monitor::Sleep(void *prule)
{
	u8 mdNum=0;
	
static u16 count=0;
static u8 sleepStatus=0;
static u8 minCount=0;

static u8 sleepAddModuleDalay=0;
static u8 sleepReduceModuleDalay=0;
static u8 sleepdlay;
static u8 sleepfalse=0;	
//u16 sleepLoadRate,wakeupLoadRate;
u16  loadRate;
	



	u16 loadI=*((u16 *)&gpSysData[LOADCURR]);//模块总输出电流

	
	if(ModuleOnlineMessage[2]==0)
	{
		
		m_sleepBattTestTimeCount=0;
		return;
		
	}
	if(genableTimeFlag==0)
	{
		m_sleephead=ModuleOnlineMessage[2]-1;
		return;
	}
		

	
	if(((s16)( * m_pbattI))<0)//加上电池放电电流，为系统所需负载电流
      loadI-=( * m_pbattI);
	    
	    loadI-=*((s16 *)&gpSysData[SOLAR_CURR]);//扣除光伏模块电流，只计算整流模块带载率
	
	
	
	//sleepLoadRate=gsleepLoadRate*10;
	//wakeupLoadRate=gwakeupLoadRate*10;
	   
	if((gSleepOnoff==1)&&(ghadbattflag==1))
	{     
		  //if(gsleepstartcount>0) return;//启动延时未到，直接退出
		
		

		if(m_sleepPeriodCount==0){//电池检测周期到时，可以进行休眠操作
		               u32 oklist=((CanRuleLn *)prule)->m_ModuleAddrlistOK;
						       u8 * pv8dat=(u8 *)pModuledata->GetDataAddr(ONOFF);
			             u8 *v8data=(u8 *)pModuledata->GetDataAddr(MODULETYPE);
			
 /////////////////////////////////////////休眠退出逻辑/////////////////////////////////////////////////////////////																	
													
				if((gWarnbit&0x200707EE)!=0)//电池熔丝,缺相,输入输出过欠压	                             
				  { 
						 sleepStatus=5;  //退出休眠		
						  pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;							
	          //休眠轮换赋值,提前进入非轮换状态，以进入sleepStatus5
					}						
 
					if(sleepWait==1)
					{
						 sleepWait=0;
						 sleepStatus=5;  //退出休眠
						pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;
							 //休眠轮换赋值,提前进入非轮换状态，以进入sleepStatus5;//休眠轮换赋值,提前进入非轮换状态，以进入sleepStatus5
					}					
					
																	 

         if(
					  (((s16)( * m_pbattI)<-50)&&(gsleepSucceedFlag==1))//||//电池放电
					  //((moduleBreakFlag&romveModuleMasklist)!=0)||//模块故障
				    //((moduleCommBreakFlag&romveModuleMasklist)!=0)//模块通信故障
					 )//检查休眠期间电池是否为放电
				  {  
						  sleepfalse++;//在启用休眠时刻，由于拉低电压，电池放电，因此延后退出休眠，等待电压上升恢复，另插放模块时，延时等模块告警清除
             if(sleepfalse>150)						
						 { 
							 sleepfalse=0;
							 sleepStatus=5;  //退出休眠
							 pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;
	                //休眠轮换赋值,提前进入非轮换状态，以进入sleepStatus5;//休眠轮换赋值,提前进入非轮换状态，以进入sleepStatus5
						 }
						  
					}
					else
					{
						sleepfalse=0;
					}
			
			
			
			
			
		      if(m_sleepTurnPeriodCount!=0)//正在休眠，未轮换
				 {
		               
		
		                         for(u8 addr=0;addr<ModuleOnlineMessage[2];addr++)
						                      {   if((((u32)1<<addr)&oklist)!=0)//查找有效的模块
																		 {
																			   if(pv8dat[addr]==0)//查找开关的模块数
																				 {
																					 
																					   if(v8data[addr]==ACDC)//只统计整流模块的数量
																					          mdNum++;
																				 }
																		 }
																	 }

					
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

             loadRate= loadI*1000/(mdNum*ModuleCap*10);//计算模块当前带载率
				 if(sleepStatus==0){

       

          
					 
				
				    
					   if(loadRate<gsleepLoadRate*10)
						 {
							   minCount=mdNum-1;
						 }
						 else if(loadRate>gsleepWakeupLoadRate*10)
						 {
							    minCount=mdNum+1;
						 }
						 else
						 {
							  minCount=mdNum;//当前模块数为最优
								if( ModuleOnlineMessage[1]>1)
								 m_sleepLastFlag=1;
								 return;
						 }
									
									
									
									if(minCount>mdNum)
									{
										//增加一个模块
										
                      
									  	sleepStatus=1;
								      pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;
	
										  m_sleepLastFlag=0;
										  m_sleeponff=0;
										  count=50;
  										sleepAddModuleDalay=150;

										
											
									}
									else if(minCount<mdNum)//当前模块数为最优)//minCount小于当前模块数
									{
									
										
                      if(gsleepMinCount<=minCount)
											{  //关一个模块
                        count=0;
										  	sleepStatus=2;
								        pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;
                        m_sleepLastFlag=0;												
												m_sleeponff=0;
												if(mdNum>1)
												{
													 m_lmitI=((CanRuleLn *)prule)->m_Ilimit;
													m_lmitI=m_lmitI*mdNum/(mdNum-1);//重新调整模块的限流值	
														 if(m_lmitI<50000)		
                          					 
													((CanRuleLn *)prule)->m_Ilimit=m_lmitI;//发送限流值 		
												}
											}
                      else//少于基数时不作任何操作
											{
												m_sleepLastFlag=1;
												return;
											}												
                    									 
										//减小一个模块
									}
					}
				   else if(sleepStatus==1)//模块增加为最优
				   {
                                       
                                   u8 whileCycle=0;
																	
																	 s8 addr=m_sleephead;
																	  while(1)
						                      {  
																		    addr++;
																		if(addr==ModuleOnlineMessage[2]) 
																			{
																					  addr=0;//翻转到队列头部
																			      whileCycle++;
																				   if(whileCycle>1)//翻译转队列次数不超过两次，超过2次(0,1)结束查找	
                                           break;       																					
																			 } 
																		        

																		if((((u32)1<<addr)&oklist)!=0)//查找有效的模块
																		 {
																			   if((pv8dat[addr]==1)&&(v8data[addr]==ACDC))//找到一个已关闭的模块,且该模块为整流模块
																				 {
																					   ((CanRuleLn *)prule)->m_setAddr=(addr+1);
																					   ((CanRuleLn *)prule)->m_setCmd=SET_ONOFF;
																					   ((CanRuleLn *)prule)->m_isCmd=1;
																	           ((CanRuleLn *)prule)->m_setData=0;//开机
																					 
																					     
    																				   gsleepSucceedFlag=1;//设置成功进入休眠标志
																					     sleepStatus=6;//跳到模检测开机状态
																					     m_sleephead=addr;
																					 
																					 
											                          return;
																				 }
																		 }
																		 else
																		 {
																			  if(ModuleOnlineMessage[1]==0)//模块在线数目为0
																					     break; //结束查找
																		 }
																	 }
																	
																	 
																	 
																	 
																	 
																	 
																	 
																	 
																	 
																	sleepStatus=0;//无合适模块增加休眠失败
																	return ;
						 
					 }
					 else if(sleepStatus==2)//模块减小为最优,因些查到合适变机的模块
					 {
						                     
						             if(loadRate>gsleepLoadRate*10)//进入休眠减模块过程中，出现负载率大于唤醒负载率，
													{
														  sleepStatus=0;  //中止模块减小过程
														   minCount=mdNum;//修正当前模块数
													}		

													count++;
													if(count<50) return;//延迟一段时间，
													
													     u8 whileCycle=0;
                               m_addr_down=m_sleephead;
													
													
													
						                     while(1)
																 {  

 																		if((((u32)1<<m_addr_down)&oklist)!=0)//查找有效的模块
																		 {
																			   if((pv8dat[m_addr_down]==0)&&(v8data[m_addr_down]==ACDC))//找到一个正在开机的模块 ,且该模块为整流模块
																				 {
																					 
																					 if(gsleepSucceedFlag==0)//第一次启动进入休眠时，进行电池存在测试
																					 { 
																						 
																						   if(((s16)( * m_pbattI)<-10))//电池放电大于1A时，不执行休眠操作
																							 {
																								   sleepStatus=5;//退出休眠
																                   	return ;
																								 
																							 }
																							 else if(((s16)( * m_pbattI)>10))//有电池充电电流，充电电流大于1A时直接接执行休眠操作
																							 {
																								   gsleepSucceedFlag=1;
																							 }
																							 else//充放电在1A以内时，进行一次电池测试
																							 {
																								 if((ACRunMode==PeakMode)||(ACRunMode==SpikeMode))//若处于尖峰或高峰，追综电池电压状态，直接进入休眠
																										{
																											 gsleepSucceedFlag=1;
																										}
																									 else
																									 {
																										m_sleepBattTest=1;//设置休眠电流测试
																										m_sleepBattTestTimeCount=30;
																									 }
																							 }
																						  
																					 }
			 
																					 sleepReduceModuleDalay=200;
																					 sleepStatus=3;//进入休眠预备状态
																					 count=0;
																					 return;
																				 }
																		 }
																		 else
																		 {
																			   if(ModuleOnlineMessage[1]==0)
																					   break;
																		 }
																		       m_addr_down--;
																		 
																		     if(m_addr_down<0)
																				{
																							 m_addr_down=ModuleOnlineMessage[2]-1;//转到队尾
																		           whileCycle++;
																				      if(whileCycle>1)//翻译转队列次数不超过两次，超过2次(0,1)结束查找	
                                              break;  
																				}
 
																	 }

																	sleepStatus=5;//无合适模块减少休眠失败,而
																	return ;

					 }	
           else if(sleepStatus==3)
					 {
						 if(mdNum>1)
						 {        
							  m_lmitI=((CanRuleLn *)prule)->m_Ilimit;
						   m_lmitI=m_lmitI*mdNum/(mdNum-1);//重新调整模块的限流值		
							 	
                  ((CanRuleLn *)prule)->m_Ilimit=m_lmitI;//发送限流值 
						 }							 
						    count++;
                	
						 
						    if(((m_sleepBattTestTimeCount==0)&&(gsleepSucceedFlag==0))||((count==50)&&(gsleepSucceedFlag==1)))
							  {
									
						

								   if(( * m_psysV)>=4850)// >4850     系统电压大于48.5V可以关机
									 {
										  m_sleepBattTest=0;
										  gsleepSucceedFlag=1;
										 
																					
										   ((CanRuleLn *)prule)->m_setAddr=m_addr_down+1;
											 ((CanRuleLn *)prule)->m_setCmd=SET_ONOFF;
											 ((CanRuleLn *)prule)->m_isCmd=1;
											 ((CanRuleLn *)prule)->m_setData=1;//关机
										
									      m_sleephead=m_addr_down-1;//更新头指针
										    if(m_sleephead<0)
													m_sleephead=ModuleOnlineMessage[2]-1;

										   sleepStatus=7;//将当前设为休眠模式
											 count=0;//预备计数清零

											 return;
									 }
									 else
									 {
										  m_sleepBattTest=0;
										   	sleepStatus=5;
										   return;
										 
									 }
							 }
							 else
							 {
								  
								   if(( * m_psysV)<4850)//若发现在延时时间内，低于48.5V恢愎系统电流，结束当前测试
									 {
										  m_sleepBattTest=0;
										  m_sleepBattTestTimeCount=0;
										   	sleepStatus=5;
									 }
										   return;
							 }
					}						 
					 else if(sleepStatus==4)//等待开关机，计算限流稳定模块输出  100*40ms 等待4s  
				   {
						     sleepdlay--;
						    if(sleepdlay==0)
						     sleepStatus=0;
								 return;
					 }
 					 else if(sleepStatus==5)//休眠条件不满足，不成功，重置测试周期
				   {
						    // gSecondSleepTimer=0;
						    m_sleepPeriodCount=gsleepBattTestPeriod;//周期检测电池赋值，以再次进入休成				
							  pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600; 
						    sleepStatus=0;
						    m_sleepLastFlag=0;
							  count=0;
						  	minCount=0;
				         m_sleepBattTest=0;//取消测试电压
						 if(gsleepSucceedFlag==1){
							 ((CanRuleLn *)prule)->m_setAddr=BROADCAST;//将所有模块全部打开
							 ((CanRuleLn *)prule)->m_setCmd=SET_ONOFF;
							 ((CanRuleLn *)prule)->m_isCmd=1;
							 ((CanRuleLn *)prule)->m_setData=0;//开机
						   gsleepSucceedFlag=0;
						 }
					 }
					 else if(sleepStatus==6)//检测模块是否成功开启
					 {
							
							
																			   if(pv8dat[m_sleephead]==0)//找到开启的模块
																				 {
																					      sleepdlay=100;
																					      sleepStatus=4;//跳到模检测开机状态
											                          return;
																				 }
																				 else
																				 {
																					      //gSecondSleepTimer=0;
																					      sleepAddModuleDalay--;
																					      if(sleepAddModuleDalay==0)																						
																								{sleepStatus=0;}	
																					      return;
																				 }
							
							
						}
					 else if(sleepStatus==7)//检测模块是否成功关闭
					 {
							
							
																			   if(pv8dat[m_sleephead+1]==1)//找到一个已关闭的模块
																				 {
																					      sleepdlay=100;
																					      sleepStatus=4;//将当前设为休眠模式								                        
											                          return;
																				 }
																				 else
																				 {      
																					      //gSecondSleepTimer=0;
																					      sleepReduceModuleDalay--;
																					      if(sleepReduceModuleDalay==0)
																								{sleepStatus=0;}	
																					      return;
																				 }
							
							
						}
					 
					 
					 
					 
					 
					 
			 	 }
          else if(m_sleepTurnPeriodCount==0)//轮换
				 {
												
											 if(m_sleeponff==0)
											 {
		 
																	 									 	 
																	 m_sleeponff=1;
																	return;
													}
                       else if(m_sleeponff==1)	
											 {														
										
	
																	             m_sleephead++;//开它的下一个
														                if(m_sleephead==ModuleOnlineMessage[2]) m_sleephead=0;
														                 
														                 if(v8data[m_sleephead]==ACDC)   
																						{
																							 ((CanRuleLn *)prule)->m_setAddr=m_sleephead+1;
																							 ((CanRuleLn *)prule)->m_setCmd=SET_ONOFF;
																							 ((CanRuleLn *)prule)->m_isCmd=1;
																							 ((CanRuleLn *)prule)->m_setData=0;//开机
																							 m_sleeponff=3;//进入延迟，等待模块开机稳定输出
																               m_onoffcount=600;//等待周期
																						 }
																						 else
																						 {
																							  m_sleeponff=3;//进入延迟，等待模块开机稳定输出
																                m_onoffcount=1;//等待周期
																						 }
														
														
														
																				
												
																

                               return ;																	 
													}
											 else if(m_sleeponff==2)
											 {														
														                       if(m_sleeptail==m_sleephead)//关机时头不能等于尾,结束本次休眠
																									 {
																										    m_sleepBattTest=0;
										                                    m_sleepBattTestTimeCount=0;
										                                  	sleepStatus=5;
																												pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;
						                                                //休眠轮换赋值,提前进入非轮换状态，以进入sleepStatus5
															                       return ;	
																									 }
                                            if(v8data[m_sleeptail]==ACDC)//整流模块才执行操作,否则查找下一个
														                 {
																								 ((CanRuleLn *)prule)->m_setAddr=m_sleeptail+1;
																								 ((CanRuleLn *)prule)->m_setCmd=SET_ONOFF;
																								 ((CanRuleLn *)prule)->m_isCmd=1;
																								 ((CanRuleLn *)prule)->m_setData=1;//关机				
																							 
												                           m_sleeponff=4;
															                     m_onoffcount=600;
																						 }
																						 else
																						 {
																							   m_sleeponff=4;
															                    m_onoffcount=1;
																						 }
															
                               return ;	 
													}
											 else if(m_sleeponff==3)
											 {

												     if(pv8dat[m_sleephead]==0)//
															{
																  
//																    m_onoffcount--;
//																 if(m_onoffcount==0)  
//																 { 
//																	 	
//																 }	
                                 m_sleeponff=2;//等待开启成功后，将要进入的状态																 
																										 
											        }
															else //开启失败，返回1继续开下一下
															{
																    m_onoffcount--;
																 if(m_onoffcount==0)  
											           {
																	   m_sleeponff=1;
																 }
																
															}
												 										 
												   return;
											 }
                        else if(m_sleeponff==4)
											 {
												  if(pv8dat[m_sleeptail]==1)//
															{
																	m_sleeponff=5;//等待结束后，将要进入的状态
																  sleepdlay=150;
															    
                                          if(m_sleeptail==ModuleOnlineMessage[2]-1) 
																						       m_sleeptail=-1;
																       u8 i;
																 
														              for(i=m_sleeptail+1;i<ModuleOnlineMessage[2];i++)//查找有效的模块
                                           {		

																						 
																							 if((((u32)1<<i)&oklist)!=0)
																							 {
																									 if(pv8dat[i]==0)//找到下一个正在关机的模块
																									 {
																										  m_sleeptail=i;
																											break;
																										
																									 }
																							 }
																						}	
																					  if(i==ModuleOnlineMessage[2]) 
																						{
																							for(i=0;i<ModuleOnlineMessage[2];i++)//从队列头开始查找
																								 {		

																									 
																										 if((((u32)1<<i)&oklist)!=0)
																										 {
																												 if(pv8dat[i]==0)//找到下一个正在关机的模块
																												 {
																														m_sleeptail=i;
																														break;
																													
																												 }
																										 }
																									}	
																						}
																										 
											        }
															else
															{
																    m_onoffcount--;
																 if(m_onoffcount==0) //关闭失败，超时，返开2继续关下一个 
											           {
																	   m_sleeponff=2;
																	   if(m_sleeptail==ModuleOnlineMessage[2]-1) 
																			          m_sleeptail=-1;
																	          u8 i;
														              for(i=m_sleeptail+1;i<ModuleOnlineMessage[2];i++)//查找有效的模块
                                           {														
																							 if((((u32)1<<i)&oklist)!=0)
																							 {
																									 if(pv8dat[i]==0)//找到下一个正在关机的模块
																									 {
																										  m_sleeptail=i;
																											break;
																										
																									 }
																							 }
																						}	
                                           if(i==ModuleOnlineMessage[2]) 
																						{
																							for(i=0;i<ModuleOnlineMessage[2];i++)//从队列头开始查找
																								 {		

																									 
																										 if((((u32)1<<i)&oklist)!=0)
																										 {
																												 if(pv8dat[i]==0)//找到下一个正在关机的模块
																												 {
																														m_sleeptail=i;
																														break;
																													
																												 }
																										 }
																									}	
																						}																					 
																	 
																	 
																	 
																	
																 }
																
															}
															return ;	
											 }
                       else if(m_sleeponff==5)//关闭模块后，等待计算限流，稳定输出
											 {
												     sleepdlay--;
						                if(sleepdlay==0)
														{
															m_sleeponff=1;
															pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;
	
												        //休眠轮换赋值,提前进入非轮换状态，以进入sleepStatus5//更新轮换周期
														}
														return ;	
											 }												 
												 
											
					
				 
				 }	
		    
				
				 
		   
			}//m_sleepPeriodCount==0
	}
	else//gSleepOnoff==0//禁止休眠
	{      // gSecondSleepTimer=0;	
		      m_sleepPeriodCount=0;
		      m_sleephead=ModuleOnlineMessage[2]-1;
		      m_sleeptail=0;
           m_sleepBattTest=0;
		     // gsleepstartcount=gsleepStartDelay;
		    if(gsleepSucceedFlag==1)//休眠成功时
			 {
						 sleepStatus=0;
						 gsleepSucceedFlag=0;
				     m_sleepLastFlag=0;
						 count=0;
						minCount=0;
	
						 ((CanRuleLn *)prule)->m_setAddr=BROADCAST;//将所有模块全部打开
						 ((CanRuleLn *)prule)->m_setCmd=SET_ONOFF;
						 ((CanRuleLn *)prule)->m_isCmd=1;
						 ((CanRuleLn *)prule)->m_setData=0;//开机
						
					}
	  }
}


void Monitor::SendCmd(void *prule)
{
	  if (setPowerLimitFlag==1)//设置单相限流
	 {  
		 if(limitIaddr[limitcount]!=-1)
		 {     u16 * vp16data=(u16 *)pModuledata->GetDataAddr(LIMITCURRENT);
			      ((CanRuleLn *)prule)->m_setData=vp16data[limitIaddr[limitcount]];
				  // m_setAddr：目标模块的地址（+1是因为limitIaddr存的是0~29，实际模块号是1~30）
			      ((CanRuleLn *)prule)->m_setAddr=limitIaddr[limitcount]+1;
            ((CanRuleLn *)prule)->m_isCmd=1;
			      ((CanRuleLn *)prule)->m_setCmd=SET_CURRENT_LIMIT;
			       limitcount++;
			  
		 }
		 else if(limitIaddr[limitcount]==-1)
		 {
						limitcount++;
						if(limitcount>=30)
						{
							setPowerLimitFlag=2;
							limitcount=0;
						}
			 
		 }
		 
	  
	 }
}










//普通锂电找有效vbat电压
uint16_t findValiVbat()
{
	uint16 tmpValue = 0;
	for(uint8_t i = 0; i < onlineNum; i++)
	{
		tmpValue = batt[i].Vbat;
		
		if(tmpValue != 0)
			break;
	}
	
	if(tmpValue == 0)
		tmpValue = (*((s16 *)&gpSysData[DCVOLTAGE])+20);
	
	return tmpValue;
}

// 智能锂电错峰控制函数（精准修复：回充完成后确保停止充电）
void Monitor::LiDiangShiftMode(void *prule)
{
    // 静态变量：保持状态（仅初始化一次）
    static u8 battReChager = 0;        // 停电来电回充标记（1=需强制回充，0=正常）
    static s16 prCurrLimitCap = 0;     // 上一次充放电限制值（用于检测变化）
    static SpeakFlag prACRunMode = RUNNULL;  // 上一次电网运行模式（与SpeakFlag对应）
		static u8 stopDischargeFlag = 0;
	  static bool needStopDischarge = false;

    // 1. 参数合法性校验（避免空指针访问，防止崩溃）
    if (prule == NULL) {
        return;
    }
    CanRuleLn *pCanRule = static_cast<CanRuleLn*>(prule);  // 类型转换（提高可读性）

    // 核心判断：是否处于「强制回充状态」（优先级最高）
    bool isForceRecharge = (battReChager == 1) && (totalSOC < gBattRecovSoc);

    // 2. 强制回充逻辑（优先级最高，忽略普通模式切换）
    if (isForceRecharge) {
         // 无需重复执行SetBatteryData和电压偏移：
        // - 硬件开启（充电）和电压偏移已在「深度欠压保护」中首次配置
        // - 函数周期性调用，只要处于该状态，就跳过普通模式，保持充电状态
        return;  // 锁定回充状态，忽略所有普通模式（放电/不充不放）逻辑
    }

    // 3. 回充完成后的强制恢复逻辑（关键修复：每次调用都校验，确保充电停止）
    if (battReChager == 1 && totalSOC >= gBattRecovSoc) {
        battReChager = 0;  // 彻底清除强制回充标记
        prCurrLimitCap = -1;  // 强制触发后续普通模式的模式切换逻辑（重新配置硬件）
        prACRunMode = static_cast<SpeakFlag>(-1);  // 同上，确保模式切换逻辑必执行
    }

    // 4. 普通模式逻辑（非强制回充时执行）
    // 触发条件：电网运行模式切换 或 充放电限制值变化 或 回充完成后强制触发
    if ((prACRunMode != ACRunMode) || (prCurrLimitCap != currLimitCap)) {
        // 更新历史状态（用于下次对比，检测变化）
        prACRunMode = ACRunMode;
        prCurrLimitCap = currLimitCap;
        currdiscap = 0;                  // 重置实际放电量计数器
				preRemainCapSum = 0;	//重置起点
				stopDischargeFlag = 0;
				needStopDischarge = false;

        // 根据充放电限制值 + 电网模式，设置锂电状态和电压偏移
        if (currLimitCap > 0) {
            // 场景1：放电模式（尖峰/高峰）—— 强制关闭充电，确保充电停止
            u16 setmos = 0x0055;
            SetBatteryData(3, 0, (u8 *)&setmos);  // 关闭锂电（终止充电+准备放电）—— 取消setModOFFflag限制！
            
						pCanRule->m_vshift = gPeakModleDisChargeVol*10; //(setbatt.Set_BattV - 50) * 10;//gPeakModleDisChargeVol
        }
        else if (currLimitCap == 0) {
            // 场景2：不充不放模式（正常）—— 关闭锂电，终止充电
            u16 setmos = 0x0055;
            SetBatteryData(3, 0, (u8 *)&setmos);  // 取消setModOFFflag限制，确保执行
						if(battComType == MODBUS_COM)
						{
							pCanRule->m_vshift = (*((s16 *)&gpSysData[DCVOLTAGE])+20)*10;//(setbatt.Set_BattV + 60) * 10;//gPeakModleChargeVol
						
						}
						else
						{
							uint16_t VbatTmp = 0;
							VbatTmp = findValiVbat();
							pCanRule->m_vshift = VbatTmp * 10;
						}
						needStopDischarge = true;
				}
        else {  // currLimitCap < 0（充电模式，低谷）
            // 场景3：允许充电—— 正常开启锂电
                u16 setmos = 1;
                SetBatteryData(3, 0, (u8 *)&setmos);  // 打开锂电（允许充电） 
								 pCanRule->m_vshift = gPeakModleChargeVol*10;//(setbatt.Set_BattV + 60) * 10;//gPeakModleChargeVol
        }
    }

    // 5. 放电停止保护（周期性检查，满足任一条件则停止放电）

    needStopDischarge |= (currLimitCap > 0) && (currdiscap >= currLimitCap);  // 放电量达标
    needStopDischarge |= (totalSOC <= gBattSafeSoc);                          // SOC过低
    needStopDischarge |= (vagBatVolt > 100) && (vagBatVolt < gsafedisV);      // 电压过低

    if (needStopDischarge) 
		{
			
			static u32 tmpV = 0;
			static u16 _delay = 0;
			
			if(stopDischargeFlag == 0)
			{
				stopDischargeFlag = 1;
				
				if(battComType == MODBUS_COM)
				{
					pCanRule->m_vshift = (*((s16 *)&gpSysData[DCVOLTAGE])+20)*10;//pCanRule->m_vshift = (setbatt.Set_BattV + 60) * 10;  // 抬高电压，停止放电
				}
				else	
				{
					uint16_t VbatTmp = 0;
					VbatTmp = findValiVbat();
					pCanRule->m_vshift = VbatTmp * 10;
				}
				
				tmpV = pCanRule->m_vshift;
			}
			
			if(stopDischargeFlag == 1)
			{  
				if(_delay > 0) 
					_delay--;
				
				if(_delay == 0)
				{
					_delay = 30;
					
					if(totalBattI > 5)
					{
						tmpV = tmpV - 50;
						
						if(tmpV >= (gsetbatt.Set_BattV + 50) * 10)
							pCanRule->m_vshift = tmpV;
					}
					else if(totalBattI < -5)
					{
					 tmpV = tmpV + 10;
						
					 if(tmpV < gPeakModleChargeVol * 10)
						pCanRule->m_vshift = tmpV;		
					}
				}					
			}	
    }

    // 6. 深度欠压紧急保护（触发强制回充）
    if (totalSOC <= (gBattSafeSoc - 300)) {
       
            u16 setmos = 1;
            SetBatteryData(3, 0, (u8 *)&setmos);  // 紧急开启锂电（充电）
            pCanRule->m_vshift = gPeakModleChargeVol*10;//pCanRule->m_vshift = (setbatt.Set_BattV + 60) * 10;  // 抬高电压，优先充电
            battReChager = 1;  // 标记为强制回充状态
    }
}

void Monitor::getBattDischageSOC(u32 tick)
{//电池放电电量计算
	static u16 Ams=0;
	static u32 preTick=0;
	static u16 Ah=0;
	static u8 startdischargeflag=1;

	u32 tickInteral;
	   //指向电池当前电流值（负数表示放电，正数表示充电）
	if((*m_pbattI)>-20)//不处于放电状态
	{
		
		 
	    Ams=0;//清零统计值
		  Ah=0;//清零统计值
		  preTick=0;//清零前一Tick计数量
		  startdischargeflag=1;//重置进入放电标志位
			
			
	}
	else//放电状态
 {
	 
	  if(startdischargeflag==1)//进入放电时，将累计放电量清零
			{
		    m_vailDischarge=0;
				//m_vailDischarge2=0;
				startdischargeflag=0;
			
			}
			
		
	  tickInteral=tick-preTick;
	   if(tick<preTick)
    tickInteral=tickInteral+0xFFFFFFFFFF+1;
		 //累计放电电流 × 时间的临时值
		   Ams+=((*m_pbattI)*-1)/10*tickInteral;//(Ams)
		// 累计放电电量
		   Ah+=Ams/36000;   //0.01AH
		   Ams=Ams%36000;   
		 

				m_vailDischarge=Ah/10;    //m_vailDischarge单位取0.1AH
				//m_vailDischarge2=Ah/10;    //m_vailDischarge单位取0.1AH

		
		 preTick=tick;
	 }
		  
}


