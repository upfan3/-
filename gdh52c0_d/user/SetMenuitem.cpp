#include "SetMenuitem.h"
#include "Staggeredpeak.h"
#include "globalval.h"


u8 setPeakKey0=1;
u8 setPeakKey1=1;
u8 setSpecialParaFlag=0;
Timer4 CurrTime={10,12,31,12,1970};//当前时间
u8 setBattParaflag=0;
u8 setPoint=0;
s16 setValu=0;
u16 setBattV = 5700;

// 全局标志：空气参数写命令触发（0=无需发送，1=需发送制冷点，2=需发送高温告警点）
u8 g_AirWriteFlag = 0; 
u8 airAddr;
u8 airIdx;

extern u8 battIndex;

// 门禁 水浸 烟雾 防雷 风机 空调 市电输入 油机输入 防雷开关 电池熔丝 负载熔丝1 负载熔丝2
u8 bitAlarmTypeList[12]={0,0,0,0,0,0,0,0,0,0,0,0};//输入告警DI 分警表 ，表中不同元素表示不同告警（顺序如上）。元素中各位表示同一种告警的序号（1表示该号告安警存在，0表示不存在），同一类告警最多有8个序号
extern u8 LocalInPutNb[2];



extern u8 downModle;
extern u8 timeDownEnable;
extern s8 UserDownSetItem[3];


extern u8 starDownTime[2];
extern u8 stopDownTime[2];

extern u16 passWord;
extern u16 passWord2;

extern u16 battdeletectcount;
extern s16 preAlarmCount;
extern s16 preDayCount;
extern s16 preDayCount2;
extern s16 preBattTestCount;
extern u16 Yout[7];


extern u32 acDCEnergydata[6];
extern u32 preDCEnergydata[6];
extern u32 cuDCEnergydata[6];
extern u32 gTotalMrEnergy;
extern u8 gEnergySle;


extern AIRpara g_astAIRpara[8];
extern AIRpara *pCurrentAir;
extern u8 airOrder[2];
extern s16 mt11Humi;
extern s16 mt11Tmp;

extern u8 battAddr[2];
extern u8 ADR;
extern u8 brunchAddr;
 
s16 RecordOrderNumToGetAddr(RecordInfo * pRecord,u16 ReLen)
{
	s16 getaddr;
	     if(*( pRecord->_pOrder)==0) *( pRecord->_pOrder)=1;//强制指向第一第记录
	     if(*( pRecord->_pOrder)>*( pRecord->_pReMax))*( pRecord->_pOrder)=*( pRecord->_pReMax);
	
	
	      *( pRecord->_pOrder+1)=*( pRecord->_pReMax);
	
	       getaddr=*( pRecord->_pReTotal)-*( pRecord->_pOrder);
	
	        if(getaddr<0)
					{
						if(*( pRecord->_pReMax)<ReLen)
							{
								getaddr=0;	
								*( pRecord->_pOrder)=*( pRecord->_pReTotal);
							}
							else
							{
								getaddr=*( pRecord->_pReMax)+getaddr;
							}
						
					}
	
	
	         return getaddr;
	
}






u8 CheckParaBoundary(u16 *para,s16 l,s16 h)
{
	
	   if(*(s16 *)para>h){ 
		    *(s16 *)para=h;  
		        return 1;
		 }//上限 
	   if(*(s16 *)para<l){ 
		     *(s16 *)para=l;   
		      return 2;
		 }//下限
     return 0;
}


u8 CheckParaBoundary8(u8 *para,u8 l,u8 h)
{
	
	   if(*para>h){ *para=h;  return 1;}//上限 
	   if(*para<l){ *para=l;   return 2;}//下限
     return 0;
}


void  SetDCDMenu(Screen * pCurrentScreen)//直流屏更新
{
	         pdisDisPlayData[0]=gcourrentWorkStatus;
	          if(gWorkShiftMode==1)
				       pdisDisPlayData[1]=ACRunMode; 
						else if(gWorkShiftMode==0)
							 pdisDisPlayData[1]=0; 
				   pdisDisPlayData[2]=gPeakShaveOnoff;//削峰
				   pdisDisPlayData[3]=gChargeStatus;// 充电状态
				   pdisDisPlayData[4]=gsleepSucceedFlag;//休眠
						
							float *paraf=(float *)&pdisDisPlayData[5];
							// 计算电流值（转换为浮点数并除以10）
						   paraf[1]	=(float)(*(u16*)&gpSysData[USER_CURR])/10;
						   // 计算电池可用时间（分钟）
						   paraf[2]=60*(float)(*(u16*)&gpSysData[BATT_SOC])/10000*gSetBattC/paraf[1];

						   // 处理16位整数数据（从索引13开始）
						u16 * buckt= (u16 *)(&pdisDisPlayData[13]);
						// 限制最大显示时间为1440分钟（24小时）
						    if(paraf[2]<1440)
								 buckt[0]=paraf[2]*10;// 扩大10倍存储
							 else
								 buckt[0]=14400; 
}



u8 setTP=0;
void SetModuleTP(Screen * pCurrentScreen)//设置模块防盗解绑
{
	 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[14])&&(genterflag==1))
		{
			                         genterflag=0;
			                         setTP=1;
                          Fun_UbindModule(&pdisDisPlayData[14],gsSN);
			
			                    
		}
		//设置模块锁定延迟时间
		 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[30])&&(genterflag==1))
		{
			                         genterflag=0;
			                         setTP=2;
			                    if((pdisDisPlayData[30]>0)&&(pdisDisPlayData[30]<25))
                               glockDelayTime=pdisDisPlayData[30];
			
			
			                    
		}
		
		
		
		
		
}
void setSMRONOFF(Screen *pCurrentScreen)//设置模块开关机
{
	   //消除由于第一行产生意外产生的确认标志  
	   //通过绝对索引从当前屏幕的项目列表（m_pItem）中，获取用户当前选中的具体菜单项（Item结构体实例）。
		//    pCurrentScreen->m_head+pCurrentScreen->m_coursItem 计算出当前选中项目在整个菜单列表中的绝对索引（首项位置 + 光标偏移量）
	  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&ModuleOnlineMessage[4])&&(genterflag==1))
		{
			                         genterflag=0;
		}
	  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[12])&&(genterflag==1))
		{
			                         genterflag=0;
			
                                pdisDisPlayData[13]=1;//开关机电动标志，由第6条菜单触发
			                     
		}
		
}

void SetBootMenu(Screen * pCurrentScreen)//均充设置
{
	

	 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[9])&&(genterflag==1))
		{    
			   
			   
			        
			                        
									gAutoBootEnable=pdisDisPlayData[9];
										 pgh52c0->savePara(&gAutoBootEnable);
								 
									genterflag=0;
							
		}
	 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])
		{
			             CheckParaBoundary8(& pdisDisPlayData[0],1,185);             
			 
			             if(genterflag==1)
									{
											pMonitor->m_Autobootcount=86400*gAutoPeriod;
												 pgh52c0->savePara(&gAutoPeriod);
												genterflag=0;
									}
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[7])
		{
			        CheckParaBoundary8(&pdisDisPlayData[7],1,18);
                
			       if(genterflag==1)
						 {
							  gequalTimeMax=pdisDisPlayData[7];
								pgh52c0->savePara(&gequalTimeMax);
					 
							  genterflag=0;
						 }
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8 *)&gSetStartBRI)
		{
			        CheckParaBoundary((u16 *)&pdisDisPlayData[3],0,100);	
			       if(genterflag==1)
						 {
			        
			      						
			         pgh52c0->savePara(&gSetStartBRI);
			         genterflag=0;
						 }
		}
			else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8 *)&gSetEndBRI)
		{
			       
	              CheckParaBoundary((u16 *)&pdisDisPlayData[5],0,100);
			          if(genterflag==1)
								 {
										 pgh52c0->savePara(&gSetEndBRI);
										 genterflag=0;
								 }
		}
			else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8 *)&gstartdelay)
		{
			
			           CheckParaBoundary8(&pdisDisPlayData[1],0,180);
			          if(genterflag==1)
								{
			             gstartdelay=pdisDisPlayData[1];
			              pgh52c0->savePara(&gstartdelay);
			 
			             genterflag=0;
							  }
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8 *)&gEndDelay)
		{
			          CheckParaBoundary((u16*)&pdisDisPlayData[12],1,600);
			          if(genterflag==1)
								{
			             gEndDelay=*(u16*)&pdisDisPlayData[12];
			             pgh52c0->savePara(&gEndDelay);
			 
			             genterflag=0;
								}
		}
			else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[10])
		{
			
			       CheckParaBoundary8(&pdisDisPlayData[10],0,99);
			          if(genterflag==1)
								{
									gbootBattCap=pdisDisPlayData[10];
									pgh52c0->savePara(&gbootBattCap);
								}
			 
			    genterflag=0;
		}
		  if(pCurrentScreen->m_EnterStatus!=3)
			{
				  
				  pdisDisPlayData[0]=gAutoPeriod;
				     pdisDisPlayData[1]=gstartdelay;
				     pdisDisPlayData[2]=gEndDelay;
				
				    (*(u16 *)&pdisDisPlayData[3])=gSetStartBRI;
				    (*(u16 *)(&pdisDisPlayData[5]))=gSetEndBRI;
				      pdisDisPlayData[7]=gequalTimeMax;
				     pdisDisPlayData[9]=gAutoBootEnable;
		         pdisDisPlayData[10]=gbootBattCap;
			}
		
}

void SetBattTestMenu(Screen * pCurrentScreen)//电池测试设置
{
	         
	        
	          
	
	   if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[0])&&(genterflag==1))
		{
			                        
			    gtestPeriodEnable=pdisDisPlayData[0];
			
			   pgh52c0->savePara(&gtestPeriodEnable);
			    genterflag=0;
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[1])
		{
			            CheckParaBoundary8(& pdisDisPlayData[1],1,185);               
			      if(genterflag==1)
						{
								gtestPeriod=pdisDisPlayData[1];
								 pMonitor->m_testPeriodCount=gtestPeriod*86400;
								 pgh52c0->savePara(&gtestPeriod);
								genterflag=0;
						}
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[2])
		{
			        CheckParaBoundary8(& pdisDisPlayData[2],1,18);
			   if(genterflag==1)
				 {
							 gtestTimeout=pdisDisPlayData[2];
							pgh52c0->savePara(&gtestTimeout);
							genterflag=0;
				 }
		}
			else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[3])
		{
			    
			   CheckParaBoundary((u16 *)&pdisDisPlayData[3],4800,5000);
			  if(genterflag==1)
				{
			    pgh52c0->savePara(&gtestV);
			    genterflag=0;
				}
		}
	
		if(pCurrentScreen->m_EnterStatus!=3)
			{
				 pdisDisPlayData[0]=gtestPeriodEnable;
				 pdisDisPlayData[1]=gtestPeriod;
				 pdisDisPlayData[2]=gtestTimeout;
				 (*(u16 *)(&pdisDisPlayData[3]))=gtestV;
			}
	
}


/*
void SetBattDownPara(Screen *pCurrentScreen)//电池下电参数设置
{
	
	       
	       
	       
	   if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[0])&&(genterflag==1))
		{
			                        
											gbattDownMod=pdisDisPlayData[0];
											pgh52c0->savePara(&gbattDownMod);
											genterflag=0;
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gdownV)
		{
			                  CheckParaBoundary((u16 *)&pdisDisPlayData[1],3800,recoverbat-50);       
			                  if(genterflag==1)
												{
														pgh52c0->savePara(&gdownV);
														genterflag=0;
												}
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&recoverbat)
		{
			                     CheckParaBoundary((u16 *)&pdisDisPlayData[5],gdownV+50,5400);    
														 if(genterflag==1)
														 {
															pgh52c0->savePara(&recoverbat);
															genterflag=0;
														 }
		}	
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gbattDownTime)
		{
			                         CheckParaBoundary((u16 *)&pdisDisPlayData[3],1,1440);
																if(genterflag==1)
																{
																	pgh52c0->savePara(&gbattDownTime);
																	genterflag=0;
																}
				}
		
		  if(pCurrentScreen->m_EnterStatus!=3)
			{
				  pdisDisPlayData[0]=gbattDownMod;
          *((u16 *)(&pdisDisPlayData[1]))=gdownV;
				  *((u16 *)(&pdisDisPlayData[3]))=gbattDownTime;
				  *((u16 *)(&pdisDisPlayData[5]))=recoverbat;
			}
}

*/
void SetLocalPower(Screen *pCurrentScreen)//设置本地下电
{
		 u16 *precoverV=&recoverV1;
	
	     
	      
				if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[1])&&( genterflag==1))
				{
					                         
					                      	gDcParam0[pdisDisPlayData[0]].st_downModle=pdisDisPlayData[1];  
					
					                        pgh52c0->savePara((u16 *)&gDcParam0[pdisDisPlayData[0]]+0);//指向st_downModle位置

					   genterflag=0;                     
	
				}
				else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[2])
				{
					
									CheckParaBoundary((u16 *)&pdisDisPlayData[2],4000, precoverV[pdisDisPlayData[0]]-50);
					 if( genterflag==1)
					 {
								gDcParam0[pdisDisPlayData[0]].st_LDVoltage=(*(u16 *)(&pdisDisPlayData[2]));
								pgh52c0->savePara((u16 *)&gDcParam0[pdisDisPlayData[0]]+3);		//指st_LDVoltage位置			
							 genterflag=0; 
					 }															 
	
				}
					else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[4])
				{
					
					                      	 CheckParaBoundary((u16 *)&pdisDisPlayData[4],10,14400);
					
					                     if( genterflag==1)
															 {
					                        gDcParam1[pdisDisPlayData[0]].st_LDDelay=(*(u16 *)(&pdisDisPlayData[4]));  
					                         pgh52c0->savePara((u16 *)&gDcParam1[pdisDisPlayData[0]]+2);	//指st_LDDelay位置	
	                                 genterflag=0;
															 }
				}
				
				
					else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[13])
				{
					
					             CheckParaBoundary((u16 *)&pdisDisPlayData[13],gDcParam0[pdisDisPlayData[0]].st_LDVoltage+50,5400);
					              
					                    if( genterflag==1)
															{        
					                                precoverV[pdisDisPlayData[0]]=(*(u16 *)(&pdisDisPlayData[13]));
					                             pgh52c0->savePara(&precoverV[pdisDisPlayData[0]]);

	                                  genterflag=0;
															}
				}
				
				
				
				
           if((pCurrentScreen->m_EnterStatus!=3)||//在非设置参数时可赋值
					(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])&&(pCurrentScreen->m_EnterStatus==3)))//在设置参数,且参数为模块号时可赋值
				{
					              if(pdisDisPlayData[0]>5) pdisDisPlayData[0]=0;	
					           
					            
											pdisDisPlayData[1]=gDcParam0[pdisDisPlayData[0]].st_downModle; 
										 (*(u16 *)(&pdisDisPlayData[2]))=gDcParam0[pdisDisPlayData[0]].st_LDVoltage;
										 (*(u16 *)(&pdisDisPlayData[4]))=gDcParam1[pdisDisPlayData[0]].st_LDDelay;
										 (*(u16 *)(&pdisDisPlayData[13]))=precoverV[pdisDisPlayData[0]];
					           
					
					


             genterflag=0;

				}
				
}


void SetUserDownPara(Screen *pCurrentScreen)//设置用户下电
{
	 u16 *precoverV=&recoverV1;

	       
	
	       if( pdisDisPlayData[13] != pdisDisPlayData[0])
				 {
					   setBattParaflag=3;
					   setPoint =  (pdisDisPlayData[0]+1)/2; //1,2->1\ 3,4->2\ 5,6->3
					   brunchAddr = (pdisDisPlayData[0]+1)%2 + 1;                        
					  pdisDisPlayData[13] = pdisDisPlayData[0];
				 }
	
	        if(pdisDisPlayData[1]>4) pdisDisPlayData[1]=0;
	      
				if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[1])&&( genterflag==1))
				{
					                         
					                      	gDcParam0[pdisDisPlayData[0]+1].st_downModle=pdisDisPlayData[1];  
					
					                        //pgh52c0->savePara((u16 *)&gDcParam0[pdisDisPlayData[0]+1]+0);//指向st_downModle位置
					   setPoint=(pdisDisPlayData[0]+1)/2;//pdisDisPlayData[0];
					   setBattParaflag=2;
					   genterflag=0;                     
	
				}
				else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[2])
				{
					
//									CheckParaBoundary((u16 *)&pdisDisPlayData[2],4000, precoverV[pdisDisPlayData[0]+1]-50);
					 if( genterflag==1)
					 {
								gDcParam0[pdisDisPlayData[0]+1].st_LDVoltage=(*(u16 *)(&pdisDisPlayData[2]));
								//pgh52c0->savePara((u16 *)&gDcParam0[pdisDisPlayData[0]+1]+3);		//指st_LDVoltage位置			
							 setPoint=(pdisDisPlayData[0]+1)/2;//pdisDisPlayData[0];
							 setBattParaflag=2;
							 genterflag=0; 
					 }															 
	
				}
					else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[4])
				{
					
					                      	 CheckParaBoundary((u16 *)&pdisDisPlayData[4],10,14400);
					
					                     if( genterflag==1)
															 {
					                        gDcParam1[pdisDisPlayData[0]+1].st_LDDelay=(*(u16 *)(&pdisDisPlayData[4]));  
					                        // pgh52c0->savePara((u16 *)&gDcParam1[pdisDisPlayData[0]+1]+2);	//指st_LDDelay位置	
                                   setPoint=(pdisDisPlayData[0]+1)/2;															 
					                         setBattParaflag=2;
	                                 genterflag=0;
															 }
				}
				
				
			
				
				
				
				
           if((pCurrentScreen->m_EnterStatus!=3)||//在非设置参数时可赋值
					(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])&&(pCurrentScreen->m_EnterStatus==3)))//在设置参数,且参数为模块号时可赋值
				{
					              if(pdisDisPlayData[0]>=9) pdisDisPlayData[0]=1;	
					              if(pdisDisPlayData[0]==0) pdisDisPlayData[0]=1;
					            
											pdisDisPlayData[1]=gDcParam0[pdisDisPlayData[0]+1].st_downModle; 
										 (*(u16 *)(&pdisDisPlayData[2]))=gDcParam0[pdisDisPlayData[0]+1].st_LDVoltage;
										 (*(u16 *)(&pdisDisPlayData[4]))=gDcParam1[pdisDisPlayData[0]+1].st_LDDelay;
										
					           
					
					


             genterflag=0;

				}
				

}


void SetSleepPara(Screen *pCurrentScreen)//休眠参数
{

	             
						   if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[0]) &&(genterflag==1))
							 {
							     genterflag=0;
								   gSleepOnoff=pdisDisPlayData[0];						 
								    pgh52c0->savePara(&gSleepOnoff);
								 
							 }
							 
							else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[1])&&(genterflag==1)) 
							 {
							      genterflag=0;
								   sleepWait=pdisDisPlayData[1];
								  
								 
							 }
							 	else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gsleepMinCount)
							 {
								 
								       CheckParaBoundary8(&pdisDisPlayData[2],1,30);
								 
												 if(genterflag==1)
												 {
														 genterflag=0;
														 pgh52c0->savePara(&gsleepMinCount);
												 }
								  
								 
							 }
							 	else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gsleepLoadRate)
							 {
							         CheckParaBoundary8(&pdisDisPlayData[3],10,70);
								 
												 if(genterflag==1)
											  {
								            genterflag=0;
								            pgh52c0->savePara(&gsleepLoadRate);
												}
								  
								 
							 }
							 else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gsleepWakeupLoadRate)
							 {       CheckParaBoundary8(&pdisDisPlayData[4],15,90);
								 
												 if(genterflag==1)
											  {
							             genterflag=0;
								           pgh52c0->savePara(&gsleepWakeupLoadRate);
												}
								  
								 
							 }
							 	else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gsleepTurnPeriod)
							 {
								
								 
								       CheckParaBoundary8(&pdisDisPlayData[5],1,72);
								 
												 if(genterflag==1)
											  {
														 genterflag=0;
														
															 pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;
														
														 pgh52c0->savePara(&gsleepTurnPeriod);
											}
							 }
//							 else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gsleepBattTestTime)
//							 {
//								         CheckParaBoundary8(&pdisDisPlayData[6],1,180);
//								 
//												 if(genterflag==1)
//											  {
//							                genterflag=0;
//								               pgh52c0->savePara(&gsleepBattTestTime);
//												}
//								 
//							 }
							 else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gsleepBattTestPeriod) 
							 {
							              CheckParaBoundary((u16*)&pdisDisPlayData[7],60,3000);
								 
												   if(genterflag==1)
											     {
								              genterflag=0;
								              pgh52c0->savePara(&gsleepBattTestPeriod);
													 }
								 
							 }
							 
							 
							 if(pCurrentScreen->m_EnterStatus!=3)
								{
									  pdisDisPlayData[0]=gSleepOnoff;
										pdisDisPlayData[1]=sleepWait;
										pdisDisPlayData[2]=gsleepMinCount;
										pdisDisPlayData[3]=gsleepLoadRate;
										pdisDisPlayData[4]=gsleepWakeupLoadRate;
										pdisDisPlayData[5]=gsleepTurnPeriod;
										//pdisDisPlayData[6]=gsleepBattTestTime;
									 *((u16 *)(&pdisDisPlayData[7]))=gsleepBattTestPeriod;
								}
	
}


void SetPeakShiftPara(Screen *pCurrentScreen)//错峰设置
{
	   if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[0]) &&(genterflag==1))
							 {
							     genterflag=0;
								   gWorkShiftMode=pdisDisPlayData[0];
								  pgh52c0->savePara(&gWorkShiftMode);
							
								 if(gWorkShiftMode == 0){
								  // SetBatteryData(1,0,(u8 *)&setBattV);
								   u8 setmos =0x01;
								   SetBatteryData(3,0,(u8 *)&setmos);
									 }
								 
							 }
		else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gsafedisV) 
							 {
								                CheckParaBoundary((u16 *)&pdisDisPlayData[1],4200,5200); 
								                 if(genterflag==1)
																 {
																		 genterflag=0;
																		
																		pgh52c0->savePara(&gsafedisV);
																 }
							 }
		else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gdisDeep)
							 {
																 CheckParaBoundary((u16 *)&pdisDisPlayData[3],0,10000); 
								               if(genterflag==1)
															 {
                 								 genterflag=0;
																
																pgh52c0->savePara(&gdisDeep);
															 }
							 }		
     else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gBattSafeSoc) 
							 {
								                 CheckParaBoundary((u16 *)&pdisDisPlayData[5],0,10000);
								                 if(genterflag==1)
																 {
																   genterflag=0;
																
																   pgh52c0->savePara(&gBattSafeSoc);
																 }
							 }
							    else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gBattRecovSoc) 
							 {
								                 CheckParaBoundary((u16 *)&pdisDisPlayData[7],gBattSafeSoc,10000);
								                 if(genterflag==1)
																 {
																   genterflag=0;
																
																   pgh52c0->savePara(&gBattRecovSoc);
																 }
							 } 
							  else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gPeakModleDisChargeVol) 
							 {
								               
								                 if(genterflag==1)
																 {
																   genterflag=0;
																
																   pgh52c0->savePara(&gPeakModleDisChargeVol);
																 }
							 } 
							 else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gPeakModleChargeVol) 
							 {
								               
								                 if(genterflag==1)
																 {
																   genterflag=0;
																
																   pgh52c0->savePara(&gPeakModleChargeVol);
																 }
							 } 
							 
							 
							 
//    else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gSetBattLimitPerCPeak)
//							 {
//								                   CheckParaBoundary((u16 *)&pdisDisPlayData[7],-100,100);
//								                    if(genterflag==1)
//																  {
//																	   genterflag=0;
//																	
//																	   pgh52c0->savePara(&gSetBattLimitPerCPeak);
//																		
//																	}
//							 }	
//    else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gSetBattLimitPerCSpike)
//							 {
//								                  CheckParaBoundary((u16 *)&pdisDisPlayData[9],-500,100);
//								                 if(genterflag==1)
//																  {
//																		 genterflag=0;
//																		
//																		pgh52c0->savePara(&gSetBattLimitPerCSpike);
//																	}
//							 }		
//    else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gSetBattLimitPerCNormal) 
//							 {
//								                    CheckParaBoundary((u16 *)&pdisDisPlayData[11],0,100);
//								                    if(genterflag==1)
//																  {
//																				 genterflag=0;
//																				
//																				pgh52c0->savePara(&gSetBattLimitPerCNormal);
//																	}
//							 }	
		  else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&eleRate[TroughMode]) 
							 {
								                  //  CheckParaBoundary((u16 *)&pdisDisPlayData[13],0,65535);
								                    if(genterflag==1)
																  {
																				 genterflag=0;
																				
																				pgh52c0->savePara(&eleRate[TroughMode]);
																		    getEnergyFlag=1;//取得电能值，结算上一个电价的周期的费用
																	}
							 }					 
			 else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&eleRate[PeakMode]) 
							 {
								                   // CheckParaBoundary((u16 *)&pdisDisPlayData[15],0,65535);
								                    if(genterflag==1)
																  {
																				 genterflag=0;
																				
																				pgh52c0->savePara(&eleRate[PeakMode]);
																		     getEnergyFlag=1;//取得电能值，结算上一个电价的周期的费用
																	}
							 }					 
				 else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&eleRate[SpikeMode]) 
							 {
								                  //  CheckParaBoundary((u16 *)&pdisDisPlayData[17],0,65535);
								                    if(genterflag==1)
																  {
																				 genterflag=0;
																				
																				pgh52c0->savePara(&eleRate[SpikeMode]);
																		    getEnergyFlag=1;//取得电能值，结算上一个电价的周期的费用
																	}
							 }					 
					else	if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&eleRate[NormalMode]) 
							 {
								                  //  CheckParaBoundary((u16 *)&pdisDisPlayData[19],0,65535);
								                    if(genterflag==1)
																  {
																				 genterflag=0;
																				
																		    eleRate[RUNNULL]=eleRate[NormalMode];
																		
																		    pgh52c0->savePara(&eleRate[RUNNULL]);//未定义段电价，取平段的电价
																				pgh52c0->savePara(&eleRate[NormalMode]);
																		    getEnergyFlag=1;//取得电能值，结算上一个电价的周期的费用
																	}
							 }				 
							 
							 
							 
							 
							 
							 
							 
							 
                if(pCurrentScreen->m_EnterStatus!=3)
								{
									 pdisDisPlayData[0]=gWorkShiftMode;
									*((u16 *)(&pdisDisPlayData[1]))=gsafedisV;
									*((u16 *)(&pdisDisPlayData[3]))=gdisDeep;
									*((u16 *)(&pdisDisPlayData[5]))=gBattSafeSoc;
									*((u16 *)(&pdisDisPlayData[7]))=gBattRecovSoc;
//									*((u16 *)(&pdisDisPlayData[7]))=gSetBattLimitPerCPeak;
//									*((u16 *)(&pdisDisPlayData[9]))=gSetBattLimitPerCSpike;
//									*((u16 *)(&pdisDisPlayData[11]))=gSetBattLimitPerCNormal;
									
										   *((u16 *)(&pdisDisPlayData[13]))= eleRate[TroughMode];
											 *((u16 *)(&pdisDisPlayData[15]))= eleRate[PeakMode];
											 *((u16 *)(&pdisDisPlayData[17]))= eleRate[SpikeMode];	
											 *((u16 *)(&pdisDisPlayData[19]))= eleRate[NormalMode];
									
									
									
									
									
								}									
							 	
}

void SetPeakPara(Screen *pCurrentScreen)//错峰时段设置
{ 
	
	
							     if(pdisDisPlayData[0]>5)   pdisDisPlayData[0]=1;//取1~5
									 if(pdisDisPlayData[1]>12)  pdisDisPlayData[1]=1;//取1~12
									 if(pdisDisPlayData[0]==0)  pdisDisPlayData[0]=5;//取1~5
									 if(pdisDisPlayData[1]==0)  pdisDisPlayData[1]=12;//取1~12
	
	                   
	
	                 
	
	
	                 setPeakKey0=pdisDisPlayData[0];
                   setPeakKey1=pdisDisPlayData[1];
							 
								 if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[2])
								{
									
									                      if (pdisDisPlayData[2]>12) pdisDisPlayData[2]=12;
																				if (pdisDisPlayData[2]==0) pdisDisPlayData[2]=1;
																		
																				if (pdisDisPlayData[3]>31) pdisDisPlayData[3]=31;
																				if (pdisDisPlayData[3]==0) pdisDisPlayData[3]=1;
//									                       SetSPeakData(pdisDisPlayData[0]-1,pdisDisPlayData[2],pdisDisPlayData[3]);
//									                        u16 tmp16=gsPeakTimeInteval[pdisDisPlayData[0]-1].st_yday;
//							                             YeardayToDate(tmp16,&pdisDisPlayData[2],0);
									                    
									                       if(genterflag==1)
																				 {
																					 
																					 genterflag=0;					                    
																					 SetSPeakData(pdisDisPlayData[0]-1,pdisDisPlayData[2],pdisDisPlayData[3]);							   
																					 pgh52c0->savePara(&gsPeakTimeInteval[pdisDisPlayData[0]-1].st_yday);
																				 }
								}
								
								 if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[6])
								{
									
									                     if ((pdisDisPlayData[6]>24)&&(pdisDisPlayData[6]<128)) pdisDisPlayData[6]=23;
																			 if (pdisDisPlayData[6]>128) pdisDisPlayData[6]=0;
																		
																			 if ((pdisDisPlayData[7]>59)&&(pdisDisPlayData[6]<128)) pdisDisPlayData[7]=59;
																			 if (pdisDisPlayData[7]>128) pdisDisPlayData[7]=0;
									
									                     SetIimeInteval(pdisDisPlayData[0]-1,pdisDisPlayData[1]-1,pdisDisPlayData[6],pdisDisPlayData[7]);
									
									                      if(genterflag==1)
																				 {
																					 genterflag=0;
																				 
																				   pgh52c0->savePara(&gsPeakTimeInteval[pdisDisPlayData[0]-1].st_time[pdisDisPlayData[1]-1]);
																				 }

								}
								
								 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[10])&&(genterflag==1))
								{

								        genterflag=0;
									     SetSpeakFlag(pdisDisPlayData[0]-1,pdisDisPlayData[1]-1,(SpeakFlag)pdisDisPlayData[10]);
									     pgh52c0->savePara(&gSpeakFlag[(pdisDisPlayData[0]-1)*6+(pdisDisPlayData[1]-1)/2]);
									    
								}
											 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[11])&&(genterflag==1))
								{

								        genterflag=0;
									      if(pdisDisPlayData[11]==0){
									        limitCap[pdisDisPlayData[0]-1][pdisDisPlayData[1]-1] =  0xFFFF;
												}else  if(pdisDisPlayData[11]==1){
												  limitCap[pdisDisPlayData[0]-1][pdisDisPlayData[1]-1] =  0;	
												}else  if(pdisDisPlayData[11]==2){
									        limitCap[pdisDisPlayData[0]-1][pdisDisPlayData[1]-1] =  20;	
												}
									      pgh52c0->savePara(& limitCap[pdisDisPlayData[0]-1][pdisDisPlayData[1]-1]);
								}
								
								
								
								
								
								
								
								 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[12])&&(genterflag==1))
								{

									     genterflag=0;
									     if((*(s16 *)&pdisDisPlayData[12])>0) 
											 { 
								        
									        limitCap[pdisDisPlayData[0]-1][pdisDisPlayData[1]-1] =  *(u16 *)&pdisDisPlayData[12];
									        pgh52c0->savePara(& limitCap[pdisDisPlayData[0]-1][pdisDisPlayData[1]-1]);
											 }
									
									     
									    
								}
								 else if((pCurrentScreen->m_EnterStatus!=3)||((pCurrentScreen->m_EnterStatus==3)&&(pCurrentScreen->m_coursItem==0)))
					       {
             
									 u8 tmp8=pdisDisPlayData[0];
									 
									 u16 tmp16=gsPeakTimeInteval[tmp8-1].st_yday;
										 YeardayToDate(tmp16,&pdisDisPlayData[2],0);
							
											if(tmp8==5)tmp8=0;
											tmp16=gsPeakTimeInteval[tmp8].st_yday;//取下一个日期
											 tmp16--;//取前一天
										 if( YeardayToDate(tmp16,&pdisDisPlayData[4],0)!=1)//处理日期为0的问题（若日期为0，则认为是前一年的最后一天）
										 {
											 pdisDisPlayData[4]=12;
											 pdisDisPlayData[5]=31;
										 }
							
											 tmp8=pdisDisPlayData[1];
										 tmp16=gsPeakTimeInteval[pdisDisPlayData[0]-1].st_time[tmp8-1];
											GetTimeInMin(tmp16, &pdisDisPlayData[6],0);
												 if(tmp8==12)tmp8=0; 
											tmp16=gsPeakTimeInteval[pdisDisPlayData[0]-1].st_time[tmp8];//取下一个时间点
													//if(tmp16<1440)
													tmp16--;//取前一分钟 
											 GetTimeInMin(tmp16, &pdisDisPlayData[8],0);
											 pdisDisPlayData[10]=GetSpeakFlag(pdisDisPlayData[0]-1,pdisDisPlayData[1]-1);
							 
							       s16 tmps16 = limitCap[pdisDisPlayData[0]-1][pdisDisPlayData[1]-1];
										 
										 
										 if(tmps16<0){
											 pdisDisPlayData[11]=0;
											  *(u16 *)&pdisDisPlayData[12]= 0;
										 } else if(tmps16==0){
											  pdisDisPlayData[11]=1;
											  *(u16 *)&pdisDisPlayData[12]= tmps16;
										 }else if(tmps16>0){
											  pdisDisPlayData[11]=2;
											  *(u16 *)&pdisDisPlayData[12]= tmps16;
										 }
										 
										 
										 
										
						
						
			         }
								
								
								
}

void SetPeakShavePara(Screen *pCurrentScreen)//削峰设置
{
	   if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[0])&&(genterflag==1))
		{
			                        
			    gPeakShaveOnoff=pdisDisPlayData[0];
			      pgh52c0->savePara(&gPeakShaveOnoff);
			    genterflag=0;
		}
//			else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gSinglePhasePower)
//		{
//			             CheckParaBoundary((u16 *)&pdisDisPlayData[1],1,10000);               
//			         if(genterflag==1)
//							 {
//									pgh52c0->savePara(&gSinglePhasePower);
//									genterflag=0;//
//							 }
//		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gAPhasePower)
		{
			               CheckParaBoundary((u16 *)&pdisDisPlayData[3],1,10000);              
			            if(genterflag==1)
									{
										pgh52c0->savePara(&gAPhasePower);
										preModuleCount=0;
										genterflag=0;//
									}
		}
			else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gBPhasePower)
		{
			            CheckParaBoundary((u16 *)&pdisDisPlayData[5],1,10000);              
			           if(genterflag==1)
								 {
									pgh52c0->savePara(&gBPhasePower);
									preModuleCount=0;
									genterflag=0;//
								 }
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gCPhasePower)
		{
			               CheckParaBoundary((u16 *)&pdisDisPlayData[7],1,10000);           
			             if(genterflag==1)
									 {
										pgh52c0->savePara(&gCPhasePower);
										preModuleCount=0;
										genterflag=0;//
									 }
		}
//		else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[11])&&(genterflag==1))
//		{
//			                        
//			    gAcPhase=pdisDisPlayData[11];
//			      pgh52c0->savePara(&gAcPhase);
//			    genterflag=0;//
//		}
//		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gAcRateP)
//		{
//			                        
//			           CheckParaBoundary((u16 *)&pdisDisPlayData[9],0,2000); 
//                  if(genterflag==1)		
//                  {										
//											pgh52c0->savePara(&gAcRateP);
//											genterflag=0;//
//									}
//		}
		
		           if(pCurrentScreen->m_EnterStatus!=3)
								{
									pdisDisPlayData[0]=gPeakShaveOnoff;
						//		 	*((u16 *)(&pdisDisPlayData[1]))=gSinglePhasePower;
									*((u16 *)(&pdisDisPlayData[3]))=gAPhasePower;
									*((u16 *)(&pdisDisPlayData[5]))=gBPhasePower;
									*((u16 *)(&pdisDisPlayData[7]))=gCPhasePower;
								//  *((u16 *)(&pdisDisPlayData[9]))=gAcRateP;
									pdisDisPlayData[11]=gAcPhase;
								}
		
}

void SetCalibrationBattI(Screen *pCurrentScreen)//电流电流传感器校准
{ 

u8 ch=pdisDisPlayData[7];

	 
   if(pdisDisPlayData[3]==0)
	 { 
			*((u16 *)(&pdisDisPlayData[4]))=*((s16 *)&gpSysData[BATT_CURR1]);
	 }
	 else if(pdisDisPlayData[3]==1)
	 {
		 
			*((u16 *)(&pdisDisPlayData[4]))=*((s16 *)&gpSysData[BATT_CURR2]);
   }
	 
	 if(ch!=pdisDisPlayData[3])
	 {
		 
		      pdisDisPlayData[7]=pdisDisPlayData[3];
		 
				 if(pdisDisPlayData[3]==0)
				 { 
					 *((u16 *)(&pdisDisPlayData[8]))=psysPara[BATT1_ZERO];//零点
					 pdisDisPlayData[0]=psysPara[BATT1_RANGE];//量程
				 }
				 else if(pdisDisPlayData[3]==1)
				 {
					 *((u16 *)(&pdisDisPlayData[8]))=psysPara[BATT2_ZERO];//零点
					 pdisDisPlayData[0]=psysPara[BATT2_RANGE];//量程
				 } 
		 
		 
		 
		} 
	 
 
   if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[0])&&(genterflag==1)) //当确认输入时将值写入
   {
		    genterflag=0;
		 


					 if(pdisDisPlayData[3]==0)
					 { 
						 psysPara[BATT1_RANGE]=pdisDisPlayData[0];//量程
					 }
					 else if(pdisDisPlayData[3]==1)
					 {
						 psysPara[BATT2_RANGE]=pdisDisPlayData[0];//量程
					 } 
	 }
	 
/*	 
//	 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[1])&&(genterflag==1)) //当确认输入时将值写入
//   {
//		    genterflag=0;
//		 
//		     gk[pdisDisPlayData[3]+2]=*((u16 *)(&pdisDisPlayData[1]));//增益
//		     pgh52c0->savePara(&gk[pdisDisPlayData[3]+2]);
//	 }
 
*/	 
	  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[8])&&(genterflag==1)) //当确认输入时将值写入
   {
							genterflag=0;

					 
						if(pdisDisPlayData[3]==0)
					 { 
						 psysPara[BATT1_ZERO]=*((u16 *)(&pdisDisPlayData[8]));//零点
					 }
					 else if(pdisDisPlayData[3]==1)
					 {
						 psysPara[BATT2_ZERO]=*((u16 *)(&pdisDisPlayData[8]));//零点
					 } 
		 
	 }
 

}


void SetBttCalibration(Screen *pCurrentScreen)//分流器器校准
{
	  
		 if(pdisDisPlayData[3]==0) 
		{
			*((u16 *)(&pdisDisPlayData[5]))=*((u16*)(&gpSysData[BATT_CURR3]));
			
		    	*((u16 *)(&pdisDisPlayData[7]))=Yout[4]*0.805;//((u16)(AD_channel[2]>>6))*0.805;
			  	*((u16 *)(&pdisDisPlayData[9]))=psysPara[BATT3_ZERO]*0.805;// (u16)((*(u32 *)&gxl[2])>>6)*0.805;
			
			
			
		}
		else if(pdisDisPlayData[3]==1) 
		{
			*((u16 *)(&pdisDisPlayData[5]))=*((u16*)(&gpSysData[BATT_CURR4]));
			
				*((u16 *)(&pdisDisPlayData[7]))=Yout[5]*0.805;//((u16)(AD_channel[4]>>6))*0.805;
			  *((u16 *)(&pdisDisPlayData[9]))= psysPara[BATT4_ZERO]*0.805;//(u16)((*(u32 *)&gxl[4])>>6)*0.805;
		}
		
		
		
	
	  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[0])//设置零点
		{
			        if(genterflag==1)
									{
										if(pdisDisPlayData[0]==1)
										{
										  if(pdisDisPlayData[3]==0) 
											{
										    psysPara[BATT3_ZERO]=Yout[4];//AD_channel[2];
											
											}
											else if (pdisDisPlayData[3]==1) 
											{
										    psysPara[BATT4_ZERO]=Yout[5];//AD_channel[4];
											
											}
											pdisDisPlayData[0]=0;
										}
											
										 	genterflag=0;
									}
		}
/*
//		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[1])//增益设置
//		{
//			            if(genterflag==1)
//									{
//										
//										gk[pdisDisPlayData[3]]=pdisDisPlayData[1];
//										
//										pgh52c0->savePara(&gk[pdisDisPlayData[3]]);
//											genterflag=0;
//									}
//			
//			
//			
//			
//			
//		}
		*/
	  else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[4])//量程设置
		{
							                 
									if(genterflag==1)
									{
										 
										 if(pdisDisPlayData[3]==0) 
		                {
										 psysPara[BATT3_RANGE]=pdisDisPlayData[4];
											
										}
										else if(pdisDisPlayData[3]==1) 
										{
											psysPara[BATT4_RANGE]=pdisDisPlayData[4];
											
										}
										
										genterflag=0;//
									}
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[11])//实测电流
		{
							                 
									if(genterflag==1)
									{
										  
										if(pdisDisPlayData[3]==0)
										{
											 psysPara[BATT3_ZERO]=pgh52c0->setIvalue(*(s16 *)&pdisDisPlayData[11],0);
											
										}
										else if(pdisDisPlayData[3]==1)
										{
											psysPara[BATT4_ZERO]=pgh52c0->setIvalue(*(s16 *)&pdisDisPlayData[11],1);
										  
										}
										
										genterflag=0;//
									}
		}
		else
		{
			 //pdisDisPlayData[4]=range[pdisDisPlayData[3]];
			
			  //pdisDisPlayData[1]=gk[pdisDisPlayData[3]];
							if(pdisDisPlayData[3]==0) 
							{
							 pdisDisPlayData[4]=psysPara[BATT3_RANGE];
								
							}
							else if(pdisDisPlayData[3]==1) 
							{
								pdisDisPlayData[4]=psysPara[BATT4_RANGE];
								
							}
		}
	
	
	
	
}


void SetCalibrationBattV(Screen *pCurrentScreen)//校准电池电压
{
	

	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[1])&&(genterflag==1)) //当确认输入时将值写入
	{ genterflag=0;
						

														 psysPara[VY]=*((u16 *)(&pdisDisPlayData[1]));
													   psysPara[VX]=Yout[2];
														 
	}	

}


void  ShowALARM(Screen *pCurrentScreen)//告警显示
{
			//u8 *para8=(u8 *)tmpPara;
								
								
									
									 if(alarmCount[1]==101 )alarmCount[1]=100;
	                 if(alarmCount[1]>alarmCount[2]) alarmCount[1]=1;
	                  if(alarmCount[1]==0) alarmCount[1]=alarmCount[2];
	
								if(alarmCount[1]!=0)
								{
									 alarmdisplay[0]= ((WarnStruct*)&palarmtable[ alarmCount[1]-1])->nb;
									
									 alarmdisplay[1]= ((WarnStruct*)&palarmtable[ alarmCount[1]-1])->wtype;
								   
								   alarmdisplay[2]= ((WarnStruct*)&palarmtable[ alarmCount[1]-1])->behavior;
									
									
									
									if(alarmdisplay[1]==USER_DOWN)
									{
										 alarmdisplay[0]= ((WarnStruct*)&palarmtable[ alarmCount[1]-1])->ext;
									}
									
									if((alarmdisplay[1]==MODULE)||((alarmdisplay[1]==USER_DOWN)&&(alarmdisplay[2]==COMM_BROKEN)))
									{
										  pdisDisPlayData[0]=1;
									}
									else
									{
											pdisDisPlayData[0]=0;
									}
									
									
									
								}
								else
								{
									alarmdisplay[0]=0;
								   alarmdisplay[1]=0;
								   alarmdisplay[2]=0;
								}
				
      if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==pdisDisPlayData)&&(genterflag==1)) //光标在“清除行"，可进行清除操作
				{
								 								
								if(genterflag==1)
		          { genterflag=0;
								    if(pdisDisPlayData[0]==1)
										{
											 pdisDisPlayData[0]=0;
											if(alarmdisplay[1]==MODULE)
											{
											 removeModule(alarmdisplay[0],MODULE);
											}
											else if((alarmdisplay[1]==USER_DOWN)&&(alarmdisplay[2]==COMM_BROKEN))
											{
												removeModule(alarmdisplay[0],USER_DOWN);
											}
										}
								
							}
				}

								
								
}



void setDiPara(Screen *pCurrentScreen)//DI设置
{
	  if(LocalInPutNb[0]>12) LocalInPutNb[0]=1;
	      if(LocalInPutNb[0]<1) LocalInPutNb[0]=12;
	      if(pdisDisPlayData[2]>5) pdisDisPlayData[2]=1;
	
	      u16 diMask=1;
				if((gDiInPutFlag&(diMask<<(LocalInPutNb[0]-1)))==0)//之前未发生该告警,才充许修改
				{  LocalInPutNb[1]=0;}
				 else
				{	 LocalInPutNb[1]=1;} 
			
			
		
		 
		 
		 if(genterflag==1)//修改状态
		 {  
			       
				     gInPutAlarm[LocalInPutNb[0]-1].st_AlarmTrigger=pdisDisPlayData[1];
			      if(LocalInPutNb[1]==0)//之前未发生该告警,才充许修改
						{
							 
							
						
							
							if((bitAlarmTypeList[pdisDisPlayData[0]]==0xFF)||pdisDisPlayData[0]>11)//该告警类型数据已达最大值，不能设置
							{
								pdisDisPlayData[0]=0x0C;
								genterflag=0;
								return ;//返回不进行操作
							}
							
							
							bitAlarmTypeList[gInPutAlarm[LocalInPutNb[0]-1].st_AlarmType]&=(~((u8)1<< (gInPutAlarm[LocalInPutNb[0]-1].st_InputNb%8)));//将原对应告警位置清零
						
							
							
							
							
							 gInPutAlarm[LocalInPutNb[0]-1].st_AlarmType=pdisDisPlayData[0];
							
							u8 i;
							for( i=0;i<8;i++)
							{
								if(((1<<i)&bitAlarmTypeList[pdisDisPlayData[0]])==0)
								{
									break;
								}
							}
							
							
                gInPutAlarm[LocalInPutNb[0]-1].st_InputNb=i;
							  gInPutAlarm[LocalInPutNb[0]-1].st_behavior=pdisDisPlayData[3];
							  bitAlarmTypeList[gInPutAlarm[LocalInPutNb[0]-1].st_AlarmType]|=1<<(gInPutAlarm[LocalInPutNb[0]-1].st_InputNb%8);
							
						
							    pgh52c0->savePara(&gInPutAlarm[LocalInPutNb[0]-1]);
               pdisDisPlayData[2]=gInPutAlarm[LocalInPutNb[0]-1].st_InputNb;
							
						}
         genterflag=0;   
		 }
		 
		  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==LocalInPutNb)||(pCurrentScreen->m_EnterStatus!=3))
			 {
								pdisDisPlayData[0]=gInPutAlarm[LocalInPutNb[0]-1].st_AlarmType;
								pdisDisPlayData[1]=gInPutAlarm[LocalInPutNb[0]-1].st_AlarmTrigger;
								pdisDisPlayData[2]=gInPutAlarm[LocalInPutNb[0]-1].st_InputNb;
								pdisDisPlayData[3]=gInPutAlarm[LocalInPutNb[0]-1].st_behavior;				
			 }
}

void setDoPara(Screen *pCurrentScreen)//DO设置
{
	
	          if(pdisDisPlayData[0]==0)pdisDisPlayData[0]=1;
	           if(pdisDisPlayData[0]>9)pdisDisPlayData[0]=9;
	
	        
	             
	         if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[1]) //光标在“清除行"，可进行清除操作
					 {
						 
						 if(genterflag==1)
						 {
	            gInPutAlarm[pdisDisPlayData[0]-1].st_behavior=pdisDisPlayData[1];
						 
						  pgh52c0->savePara(&gInPutAlarm[pdisDisPlayData[0]-1]);
	            genterflag=0;
						 }
					 }
//					 else
//	         {
//					 
//					    pdisDisPlayData[1]=gInPutAlarm[pdisDisPlayData[0]-1].st_behavior;	
//					 }
					 
					   if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])||(pCurrentScreen->m_EnterStatus!=3))
						{
							pdisDisPlayData[1]=gInPutAlarm[pdisDisPlayData[0]-1].st_behavior;	
						}
}





void SetTimePara(Screen *pCurrentScreen)//时间设置
{
	 
		       

									 if(genterflag==0)
										 {
											 if(pCurrentScreen->m_EnterStatus!=3)//处于非编辑状态时
											{						
												*((u16 *)(&pdisDisPlayData[0]))=gTimer.year;
												*((u16 *)(&pdisDisPlayData[2]))=gTimer.mon;
												*((u16 *)(&pdisDisPlayData[4]))=gTimer.day;
											
												*((u16 *)(&pdisDisPlayData[6]))=gTimer.hour;
												*((u16 *)(&pdisDisPlayData[8]))=gTimer.min;
												*((u16 *)(&pdisDisPlayData[10]))=gTimer.sec;
											}
											else
											{
										    if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])
												{
												     CheckParaBoundary((u16 *)&pdisDisPlayData[0],2021,2098);
												     CheckParaBoundary((u16 *)&pdisDisPlayData[2],1,12);
												     CheckParaBoundary((u16 *)&pdisDisPlayData[4],1,31);
												}
												else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[6])
												{
												     CheckParaBoundary((u16 *)&pdisDisPlayData[6],0,23);
												     CheckParaBoundary((u16 *)&pdisDisPlayData[8],0,59);
												     CheckParaBoundary((u16 *)&pdisDisPlayData[10],0,59);
												}
												
												     
												
											}
											
											
											
											
											
											
									  }
								else{
								
								   RTC_Set( *((u16 *)(&pdisDisPlayData[0])),
														*((u16 *)(&pdisDisPlayData[2])),
														*((u16 *)(&pdisDisPlayData[4])),
														*((u16 *)(&pdisDisPlayData[6])), 
														*((u16 *)(&pdisDisPlayData[8])), 
														*((u16 *)(&pdisDisPlayData[10]))
									         );
									 genterflag=0;
								  }
}

void SetAlarmPara(Screen *pCurrentScreen)//告警设置
{
	
	   if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gInOwrVLimit)
		{
			         //CheckParaBoundary((u16 *)&pdisDisPlayData[0],14000,20000);                
			         if(genterflag==1)
							 {
								pgh52c0->savePara(&gInOwrVLimit);
								genterflag=0;//
							 }
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gInOverVLimit)
		{
			         // CheckParaBoundary((u16 *)&pdisDisPlayData[2],26000,31000);               
			          if(genterflag==1)
							 {
									pgh52c0->savePara(&gInOverVLimit);
									genterflag=0;//
							 }
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gInOverILimit)
		{
			                        
			           CheckParaBoundary((u16 *)&pdisDisPlayData[4],0,30000);
			           if(genterflag==1)
								 {
										pgh52c0->savePara(&gInOverILimit);
										genterflag=0;//
								 }
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gOutOwrVLimit)//电池低压
		{
			            CheckParaBoundary((u16 *)&pdisDisPlayData[6],4200,5300);            
			             if(genterflag==1)
								 {   
										pgh52c0->savePara(&gOutOwrVLimit);
										genterflag=0;//
								 }
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&goutPutOwrV)//输出欠压
		{
			            CheckParaBoundary((u16 *)&pdisDisPlayData[18],4200,5300);            
			             if(genterflag==1)
								 {   
										pgh52c0->savePara(&goutPutOwrV);
										genterflag=0;//
								 }
		}
			else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gOutOverVLimit)//输出过压
		{
			              CheckParaBoundary((u16 *)&pdisDisPlayData[8],5600,6100);           
			              if(genterflag==1)
								   {  
												pgh52c0->savePara(&gOutOverVLimit);
												genterflag=0;//
									 }
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gOverFLimit)
		{
			               CheckParaBoundary((u16 *)&pdisDisPlayData[10],4000,7000);          
											if(genterflag==1)
										 {      
											pgh52c0->savePara(&gOverFLimit);
											genterflag=0;//
										 }
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gOwrFLimit)
		{
			                    CheckParaBoundary((u16 *)&pdisDisPlayData[12],4000,5000);     
			                   	if(genterflag==1)
										 { 
												pgh52c0->savePara(&gOwrFLimit);
												genterflag=0;//
										 }
		}
			else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&ghtemp)
		{
			                    // CheckParaBoundary((u16 *)&pdisDisPlayData[14],3000,9000);     
												if(genterflag==1)
												{ 
													pgh52c0->savePara(&ghtemp);
													genterflag=0;//
												 }
		}
		else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gltemp)
		{
			                    // CheckParaBoundary((u16 *)&pdisDisPlayData[16],-3000,2000);    
													if(genterflag==1)
														{ 
															pgh52c0->savePara(&gltemp);
															genterflag=0;//
														}
		}
		           if(pCurrentScreen->m_EnterStatus!=3)
								{
										(*(u16 *)&pdisDisPlayData[0])=gInOwrVLimit;
										(*(u16 *)&pdisDisPlayData[2])=gInOverVLimit;      
										(*(u16 *)&pdisDisPlayData[4])=gInOverILimit;		 
										(*(u16 *)&pdisDisPlayData[6])=gOutOwrVLimit;
										(*(u16 *)&pdisDisPlayData[8])=gOutOverVLimit;
										(*(u16 *)&pdisDisPlayData[10])=gOverFLimit;
										(*(u16 *)&pdisDisPlayData[12])=gOwrFLimit;
										(*(u16 *)&pdisDisPlayData[14])=ghtemp;
										(*(u16 *)&pdisDisPlayData[16])=gltemp;
										(*(u16 *)&pdisDisPlayData[18])=goutPutOwrV;
								}
}	

void SetOter(Screen *pCurrentScreen)//工厂设置
{
	
	 if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])
		{
								CheckParaBoundary8(&pdisDisPlayData[0],1,28);                   
									if(genterflag==1)
									{
										 gGetEnergyDay=pdisDisPlayData[0];
										pgh52c0->savePara(&gGetEnergyDay);
										genterflag=0;//
									}
		}
	   else  if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[1])
		{
			           CheckParaBoundary8(&pdisDisPlayData[1],1,128);                
			           if(genterflag==1)
								 {
									 monitor_addr=pdisDisPlayData[1];
									pgh52c0->savePara(&monitor_addr);
									genterflag=0;//
								 }
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[2])
    {
			          CheckParaBoundary((u16*)&pdisDisPlayData[2],10 ,200);//超限时回写显存
			     if(genterflag==1)
					 {
						    gRateCurr=pdisDisPlayData[2];
           			pgh52c0->savePara(&gRateCurr);
			           genterflag=0;
					 }
		}
	  else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[4])
    {
			       
			         if(genterflag==1)
							 {
									 sysPara[SWITCH_FACT] = pdisDisPlayData[4];
								
									 genterflag=0;
							 }
		}
		 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[5])
    {
			       
			         if(genterflag==1)
							 {
									sysPara[PROL_OLD_FLAG] = pdisDisPlayData[5];
								
									 genterflag=0;
							 }
		}
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[6])&&(genterflag==1))
		{
			                       
			                  gbeeEnable=pdisDisPlayData[6];
												pgh52c0->savePara(&gbeeEnable);
												genterflag=0;//
		}

		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[7])&&(genterflag==1))
		{
			                        
										 gStopTest=pdisDisPlayData[7];
											pgh52c0->savePara(&gStopTest);
											genterflag=0;//
		}
		else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[20])&&(genterflag==1))
		{
			                        
			    gAcPhase=pdisDisPlayData[20];
			      pgh52c0->savePara(&gAcPhase);
			    genterflag=0;//
		}
		
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[8])&&(genterflag==1))
		{
			                        
			       gcheckBatt=pdisDisPlayData[8];
			      pgh52c0->savePara(&gcheckBatt);
			      battdeletectcount=0;
			      genterflag=0;//
		}
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[17])&&(genterflag==1))
		{
			          if(genableTP==0)    
							 {
								 genableTP=pdisDisPlayData[17];
								 pgh52c0->savePara(&genableTP);
								}	
								genterflag=0;//
														
			     
		}
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[18])&&(genterflag==1))
		{
			         
								 battComType=pdisDisPlayData[18];
								 pgh52c0->savePara(&battComType);
								
								genterflag=0;
														
			     
		}
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[19])&&(genterflag==1))
		{
			         
								 gEnergySle=pdisDisPlayData[19];
								 pgh52c0->savePara(&gEnergySle);
								
								genterflag=0;
														
			     
		}
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[21])&&(genterflag==1))
		{
			         
								 sysPara[BATT_MOS_OFF]=pdisDisPlayData[21];
								 
								
								genterflag=0;
														
			     
		}
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[22])&&(genterflag==1))
		{
			         
								 sysPara[DELAY_VOLT_UP]=pdisDisPlayData[22];
								 
								
								genterflag=0;
														
			     
		}
				else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[23])&&(genterflag==1))
		{
			         
								 sysPara[SELECT_FACT]=pdisDisPlayData[23];
								 
								
								genterflag=0;
														
			     
		}

		
		
		
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[9])&&(genterflag==1))
		{
			           
                              pgh52c0->writeI2C(SYS_INFO_START+40,gIP,4);
			                        //  gnetwork.OsRest();
			
			                        genterflag=0;
		}
		else  if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[13])&&(genterflag==1))
		{
			           
                              pgh52c0->writeI2C(SYS_INFO_START+44,gGW,4);
			                        //  gnetwork.OsRest();
			
			                        genterflag=0;
		}
		
		
		
		
		
		
		
	   if(pCurrentScreen->m_EnterStatus!=3)
		{
			     pdisDisPlayData[0]=gGetEnergyDay;
				   pdisDisPlayData[1]=monitor_addr;
				   (*(u16*)&pdisDisPlayData[2])=gRateCurr;
			
				   pdisDisPlayData[4]= sysPara[SWITCH_FACT];
				   pdisDisPlayData[6]=gbeeEnable;
				   pdisDisPlayData[7]=gStopTest;
				   pdisDisPlayData[8]=gcheckBatt;
			     pdisDisPlayData[21] = sysPara[BATT_MOS_OFF];
			     pdisDisPlayData[22] = sysPara[DELAY_VOLT_UP];
			     pdisDisPlayData[23] = sysPara[SELECT_FACT];
			
			    for(u8 i=0;i<4;i++)
			  {
				    pdisDisPlayData[9+i]=gIP[i];
					  pdisDisPlayData[13+i]=gGW[i];
				}
			  
		}
		
}

void SetBattParaSetMenu(Screen *pCurrentScreen)//电池参数设置
{
	    u16 *para16=(u16 *)pdisDisPlayData;

		 u8 *para8=(u8 *)&para16[5];

	   if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&para8[0])&&(genterflag==1))
		{
			       
			                        pMonitor->m_workMode=para8[0];
			                         genterflag=0;
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8 *)&gSetfloatV)
		{
			                      CheckParaBoundary(&para16[0],4000 ,gSetequalV-10);  //浮充电压限制
			
			                 if(genterflag==1)
											 {
												 
			                      pgh52c0->savePara(&gSetfloatV);
			                         genterflag=0;
											 }
		}
			else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8 *)&gSetequalV)
		{
			                       CheckParaBoundary(&para16[1],gSetfloatV+10 ,5900); //均充电压限制   
			                    if(genterflag==1)
													 {
																pgh52c0->savePara(&gSetequalV);
																	 genterflag=0;
													 }
		}
		else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&para8[1])&&(genterflag==1))
		{
			                   
			                        gtmpCompEn=para8[1];
			                      pgh52c0->savePara(&gtmpCompEn);
			                         genterflag=0;
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8 *)&gtmpFactor16)
    {
			         
			                                  CheckParaBoundary(&para16[2],0,500);//温度系数限制
			                                 if(genterflag==1)
													            {
																				pgh52c0->savePara(&gtmpFactor16);
																				 genterflag=0;
																			}
		}
	  else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gSetBattC)
		{
			                                  CheckParaBoundary(&para16[3],0 ,30000);//电池容量限制
			                                 if(genterflag==1)
													            {
																				pgh52c0->savePara(&gSetBattC);
																				//pbattCap->Init();
																				  pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																				
																				for(u8 i=0;i<battGroud;i++)
																				{
																					  gbranchBatt[i]=gSetBattC/battGroud;
																					  
																					  pgh52c0->savePara(&gbranchBatt[i]);
																				}
																				

																				genterflag=0;//
																			}
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gSetBattLimitPerC)
		{
			                                CheckParaBoundary(&para16[4],0 ,100);//限流系数限制
			                               if(genterflag==1)
													          {
																			pgh52c0->savePara(&gSetBattLimitPerC);
																			genterflag=0;//
																		}
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&battGroud)
		{
			                         
			                      CheckParaBoundary8(&para8[2],1 ,6);//限流系数限制
			                               if(genterflag==1)
																		 {
																			 
																				pgh52c0->savePara(&battGroud);
																				genterflag=0;//
																			 gSetBattC=0;
																			 for(u8 i=0;i<battGroud;i++)
																				{
																					 gSetBattC+= gbranchBatt[i];
																					  
																				}
																			  pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																			 
																		 }
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gbranchBatt[0])
		{
			                                CheckParaBoundary(&para16[7],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			pgh52c0->savePara(&gbranchBatt[0]);
																			gSetBattC=0;
																			for(u8 i=0;i<battGroud;i++)
																			{
																				 gSetBattC+=gbranchBatt[i];
																			}
																			
																			pgh52c0->savePara(&gSetBattC);
																			 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																			genterflag=0;//
																		}
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gbranchBatt[1])
		{
			                                CheckParaBoundary(&para16[8],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			pgh52c0->savePara(&gbranchBatt[1]);
																			gSetBattC=0;
																			for(u8 i=0;i<battGroud;i++)
																			{
																				 gSetBattC+=gbranchBatt[i];
																			}
																			
																			pgh52c0->savePara(&gSetBattC);
																			 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																			genterflag=0;//
																		}
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gbranchBatt[2])
		{
			                                CheckParaBoundary(&para16[9],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			pgh52c0->savePara(&gbranchBatt[2]);
																			gSetBattC=0;
																			for(u8 i=0;i<battGroud;i++)
																			{
																				 gSetBattC+=gbranchBatt[i];
																			}
																			
																			pgh52c0->savePara(&gSetBattC);
																			 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																			genterflag=0;//
																		}
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gbranchBatt[3])
		{
			                                CheckParaBoundary(&para16[10],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			pgh52c0->savePara(&gbranchBatt[3]);
																			gSetBattC=0;
																			for(u8 i=0;i<battGroud;i++)
																			{
																				 gSetBattC+=gbranchBatt[i];
																			}
																			
																			pgh52c0->savePara(&gSetBattC);
																			 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																			genterflag=0;//
																		}
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gbranchBatt[4])
		{
			                                CheckParaBoundary(&para16[11],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			pgh52c0->savePara(&gbranchBatt[4]);
																			gSetBattC=0;
																			for(u8 i=0;i<battGroud;i++)
																			{
																				 gSetBattC+=gbranchBatt[i];
																			}
																			
																			pgh52c0->savePara(&gSetBattC);
																			 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																			genterflag=0;//
																		}
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gbranchBatt[5])
		{
			                                CheckParaBoundary(&para16[12],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			pgh52c0->savePara(&gbranchBatt[5]);
																			gSetBattC=0;
																			for(u8 i=0;i<battGroud;i++)
																			{
																				 gSetBattC+=gbranchBatt[i];
																			}
																			
																			pgh52c0->savePara(&gSetBattC);
																			 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																			genterflag=0;//
																		}
		}
		
		
		
		
		
		
		
		
		
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==(u8*)&gOcrdiffV)
		{
			             CheckParaBoundary(&para16[13],0 ,8000);//光伏电压差
			      if(genterflag==1)
						{
							  pgh52c0->savePara(&gOcrdiffV);
								genterflag=0;									
																		
						}
		}
		if(pCurrentScreen->m_EnterStatus!=3)
			{
				
				
				 para16[0]=gSetfloatV;
				 para16[1]=gSetequalV;
				 para16[2]=gtmpFactor16;
			   para16[3]=gSetBattC;
         para16[4]=gSetBattLimitPerC;
       
				
				 para8[0]=pMonitor->m_workMode;
				 para8[1]=gtmpCompEn;
				 para8[2]=battGroud;
				
				for(u8 i=0;i<6;i++)
				{
					 para16[7+i]=gbranchBatt[i];
				}
				
				 para16[13]=gOcrdiffV;
				
				
			}
		
		
		
		
		
		
}


void GetAlarmRecordMenu(Screen *pCurrentScreen)//获取告警记录
{
	
	uTIME tTimer;
	 u16 *para16=(u16 *)pdisDisPlayData;
					 u16 *recordTime=&para16[2];
					 u8 *alarmRecordType=(u8 *)&recordTime[6];
		s16 getaddr=0;			
					
				RecordInfo Record={ &recordAlarmCount,&galarmRecordMax,&para16[0]};	
					
					getaddr=RecordOrderNumToGetAddr(&Record,ALARM_RECORD_LEN);	

	         if(recordAlarmCount==0) //记录为空
						{
							   alarmRecordType[0]=0;
							   alarmRecordType[1]=0;
						     alarmRecordType[2]=0;
							   alarmRecordType[3]=0;
						    return;//记录为零时不作任何操作

						}
						
						
						
				
						
						 
						    if(preAlarmCount!=getaddr)
								{
							    pgh52c0->readI2C(ALARM_DATA_STAR+12*getaddr,(u8 *)&tTimer,6);//非第一次记录，读取存储记录数
							     recordTime[0]=tTimer.year;
							     recordTime[1]=tTimer.mon;
							     recordTime[2]=tTimer.day;
							     recordTime[3]=tTimer.hour;
							     recordTime[4]=tTimer.min;
							     recordTime[5]=tTimer.sec;
							    
							    u16 tmpalarmRecord=0;
							   pgh52c0->readI2C(ALARM_DATA_STAR+6+12*getaddr,(u8 *)&tmpalarmRecord,2);//非第一次记录，读取存储记录数

							    preAlarmCount=getaddr;
								 
									   alarmRecordType[0]=((WarnStruct *)&tmpalarmRecord)->nb;
								     alarmRecordType[1]=((WarnStruct *)&tmpalarmRecord)->wtype;
									
									
								     if( alarmRecordType[1] == USER_DOWN)
										 {
											   // alarmRecordType[0]=((WarnStruct *)&tmpalarmRecord)->ext;
											  u32 tmp32;
											  pgh52c0->readI2C(ALARM_DATA_STAR+2+6+12*getaddr,(u8 *)&tmp32,4);
											  alarmRecordType[0]=tmp32;
										 }
									  
									   alarmRecordType[2]=((WarnStruct *)&tmpalarmRecord)->behavior;
								     alarmRecordType[3]=((WarnStruct *)&tmpalarmRecord)->wstatus;
								 
								     if(alarmRecordType[1]>=MAX_TYPE)  alarmRecordType[0]=0;
								     if(alarmRecordType[0]>94)  alarmRecordType[1]=0;
								     if(alarmRecordType[2]>=MAX_BEH)  alarmRecordType[2]=0;
								     if(alarmRecordType[3]>1)  alarmRecordType[3]=0;
						 }
						
						
	
					
					
}

void GetEenergyRecordMenu(Screen *pCurrentScreen)//获取电量记录
{ 
		

	  u16 *para16=(u16 *)pdisDisPlayData;
		s16 getaddr=0;	
				
					
				  RecordInfo Record={ &EnergyDataWriteConunt,&genergyDateRecordMax,&para16[0]};	
					
					getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_RECORD_LEN);
	
					if(EnergyDataWriteConunt==0)
					{
						for(u8 i=0;i<28;i++)
						{
							pdisDisPlayData[4+i]=0;
						}
						return;
					}
				
				
				
	
	
			   if(preDayCount!=getaddr)
				 {
					 uTIME tTimer;
					    pgh52c0->readI2C(ENERGY_DATA_STAR+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);
					 
					   *((u16 *)&pdisDisPlayData[4])=tTimer.year;
					   pdisDisPlayData[6]=tTimer.mon;
					   pdisDisPlayData[7]=tTimer.day;
					   pdisDisPlayData[8]=tTimer.hour;
					   pdisDisPlayData[9]=tTimer.min;
					   pdisDisPlayData[10]=tTimer.sec;
					   
					   pgh52c0->readI2C(ENERGY_DATA_STAR+getaddr*TOTAL_DATA_LEN+TIME_LEN+USER_ENERGY_DATA_LEN,(u8*)&pdisDisPlayData[11],6*4);//读取交流电能记录
					 
					 preDayCount=getaddr;
					 
					 
				 }

	  
	
	
}


void GetEenergyRateRecordMenu(Screen *pCurrentScreen)
{
	     u16 *para16=(u16 *)pdisDisPlayData;
		s16 getaddr=0;	
				
					
				  RecordInfo Record={ &EnergyDataWriteConunt,&genergyDateRecordMax,&para16[0]};	
					
					getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_RECORD_LEN);
	
					if(EnergyDataWriteConunt==0)
					{
						for(u8 i=0;i<28;i++)
						{
							pdisDisPlayData[4+i]=0;
						}
						return;
					}
				
	    if(preDayCount!=getaddr)
				 {
					 uTIME tTimer;
					    pgh52c0->readI2C(ENERGY_DATA_STAR+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);
					 
					   *((u16 *)&pdisDisPlayData[4])=tTimer.year;
					   pdisDisPlayData[6]=tTimer.mon;
					   pdisDisPlayData[7]=tTimer.day;
					   pdisDisPlayData[8]=tTimer.hour;
					   pdisDisPlayData[9]=tTimer.min;
					   pdisDisPlayData[10]=tTimer.sec;
					   
					 u32 tmpdat;
					 for(u8 i=0;i<6;i++)
					 {
					   pgh52c0->readI2C(ENERGY_DATA_STAR+getaddr*TOTAL_DATA_LEN+TIME_LEN+USER_ENERGY_DATA_LEN+24+4*i,(u8*)&tmpdat,4);//读取交流电能记录
						 
						   *((u32 *)&pdisDisPlayData[11+i*4])=tmpdat/1000;
					 }
					 
					 preDayCount=getaddr;
					 
					 
				 }
	
	
	
	
}

void GetEenergy90dayRecordMenu(Screen *pCurrentScreen)//获取配电电量记录
{
	  u16 *para16=(u16 *)pdisDisPlayData;
		s16 getaddr=0;	
				
				 if(para16[2]>para16[3])	para16[2]=1;
	       if(para16[2]<1)	para16[2]=para16[3];
	
				  RecordInfo Record={ &EnergyDataWriteConunt90,&genergyDateRecord90Max,&para16[0]};	
					
					getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_90_RECORD_LEN);
					
						if(EnergyDataWriteConunt90==0)
					{
						for(u8 i=0;i<11;i++)
						{
							pdisDisPlayData[8+i]=0;
						}
						return;
					}	
					
					 if(preDayCount!=getaddr)
				 {
					 uTIME tTimer;
					    pgh52c0->readI2C2(ENERGY_90DAY_DATA_START+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);
					 
					   *((u16 *)&pdisDisPlayData[8])=tTimer.year;
					   pdisDisPlayData[10]=tTimer.mon;
					   pdisDisPlayData[11]=tTimer.day;
					   pdisDisPlayData[12]=tTimer.hour;
					   pdisDisPlayData[13]=tTimer.min;
					   pdisDisPlayData[14]=tTimer.sec;
					
					 
					 
					}
				 
					if((preDayCount!=getaddr)||(preDayCount2!=para16[2]))
					{
					  // pgh52c0->readI2C2(ENERGY_90DAY_DATA_START+6+TOTAL_DATA_LEN*getaddr+(para16[2]-1)*4,(u8 *)&pdisDisPlayData[15],4);//获取指定配电用户
						pgh52c0->readI2C2(ENERGY_90DAY_DATA_START+6+TOTAL_DATA_LEN*getaddr+(para16[2])*4,(u8 *)&pdisDisPlayData[15],4);//获取指定配电用户2022-12-2
						
					}
					
					  preDayCount=getaddr;
					  preDayCount2=para16[2];
				 						
					
}


void GetBattTestRecordMenu(Screen *pCurrentScreen)//获取电池测试记录
{
	 

	  u16 *para16=(u16 *)pdisDisPlayData;
		s16 getaddr=0;			
					
	
	         
	
	
	
	
	
	
				RecordInfo Record={ &gbattTestCount,&gdisBattRecordMax,&para16[0]};	
	
	     	getaddr=RecordOrderNumToGetAddr(&Record,DIS_BATT_RECORD_LEN);
				
				  if(gbattTestCount==0)
						 {
							 for(u8 i=0;i<16;i++)
							 {
								 pdisDisPlayData[4+i]=0;
							 }
							  return;
						 }
				
				
	          if(preBattTestCount!=getaddr)
						{
						    uTIME tmpTime;
	              u16 flag=0;        
//	                 pgh52c0->readI2C(BATT_TEST_DATA_FLAG,(u8 *)&flag,2);
//	                 if(flag!=0xA5A5) return;
						
						       pgh52c0->readI2C(BATT_TEST_DATA_STAR+0 +getaddr*14,(u8 *)&tmpTime,6);
							
										*((u16 *)&pdisDisPlayData[4])=tmpTime.year;
															pdisDisPlayData[6]=tmpTime.mon;
															pdisDisPlayData[7]=tmpTime.day;
															pdisDisPlayData[8]=tmpTime.hour;
															pdisDisPlayData[9]=tmpTime.min;
															 pdisDisPlayData[10]=tmpTime.sec;
							
							    pgh52c0->readI2C(BATT_TEST_DATA_STAR +6+getaddr*14,(u8 *)&tmpTime,6);
							
										*((u16 *)&pdisDisPlayData[11])=tmpTime.year;
															pdisDisPlayData[13]=tmpTime.mon;
															pdisDisPlayData[14]=tmpTime.day;
															pdisDisPlayData[15]=tmpTime.hour;
															pdisDisPlayData[16]=tmpTime.min;
															pdisDisPlayData[17]=tmpTime.sec;
															
															
									pgh52c0->readI2C(BATT_TEST_DATA_STAR +12+getaddr*14,(u8 *)&	pdisDisPlayData[18],2);				


						   preBattTestCount=getaddr;
						}
						
	
	
	
	
	
	
	
	
}


 

void ComparaPassWord(Screen * pCurrentScreen)
{
	    if(passwordshow[0]>9&&passwordshow[0]<11) passwordshow[0]=0;
	    if(passwordshow[1]>9&&passwordshow[1]<11) passwordshow[1]=0;
	    if(passwordshow[2]>9&&passwordshow[2]<11) passwordshow[2]=0;
	    if(passwordshow[3]>9&&passwordshow[3]<11) passwordshow[3]=0;
	
	    if(genterflag==1)//修改状态
		 { 
			 u16 PassWord=0;
			     PassWord=password[0]*1000+password[1]*100+password[2]*10+password[3];
			     if((PassWord==passWord)||(PassWord==passWord2))
					 {
						 delayPassWord=900;
						 if(PassWord==passWord2)
						 {
							   setSpecialParaFlag=2;
						 }
						 if(passWordToChange!=NULL)
						 {
							 if( setMenu(passWordToChange)!=0)
							 {
								 pCurrentScreen->m_layer[ pCurrentScreen->m_LayerCount].st_ppItem=passWordToChange;//保存本次更新屏的指针地址

								 pCurrentScreen->reSet(*passWordToChange,0);//切换到下一层指针
							 }
						 }
					 }
			 
			 genterflag=0;
		 }
}
void  ComparapFactoryPassWord(Screen * pCurrentScreen)
{
	    if(passwordshow[0]>9&&passwordshow[0]<11) passwordshow[0]=0;
	    if(passwordshow[1]>9&&passwordshow[1]<11) passwordshow[1]=0;
	    if(passwordshow[2]>9&&passwordshow[2]<11) passwordshow[2]=0;
	    if(passwordshow[3]>9&&passwordshow[3]<11) passwordshow[3]=0;
	
	     if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==passwordshow)
			 {
									     if(genterflag==1)//修改状态
										 { 
											 u16 PassWord=0;
													 PassWord=password[0]*1000+password[1]*100+password[2]*10+password[3];
											    if(PassWord==1010)
					               {
													   setSpecialParaFlag=2;
											   }
											 
											}
				}
	
	
}




void FactorySettingPara(Screen *pCurrentScreen)//恢复工厂设置（恢复默认参数，删除告警记录）
{
	
	
	 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[0])&&(genterflag==1))
	 {
		 //恢复默认值
		 if(pdisDisPlayData[0]==1)
		 {
			 u16 _flag=0;
			  pgh52c0->writeI2C(SYS_PARA_FLAG, (u8 *)&_flag, 2);
			 
			  pgh52c0->writeI2C(AD_PARA, (u8 *)&_flag, 2);
		 }
		 pdisDisPlayData[0]=0;
		 genterflag=0;
	 }
	 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[1])&&(genterflag==1))
	 {
		 //清告警记录
		 if(pdisDisPlayData[1]==1)
		 {   
			   u16 _flag=0;
			  pgh52c0->writeI2C(ALARM_DATA_FlAG,(u8 *)&_flag,2);//读取记录位置
				recordAlarmCount=0;
			  galarmRecordMax=0; 
			  pgh52c0->savePara(&galarmRecordMax);
			 for(u8 i=0;i<6;i++)
			 {
				   alarmhead[i]=0xffff;
				   alarmtail[i]=0xffff;
			 }
			    alarmhead[5]=0;
				  alarmtail[5]=0;
			   	pgh52c0->writeI2C(ALARM_DATA_HEAD,(u8 *)&alarmhead[0],12);
					pgh52c0->writeI2C(ALARM_DATA_TAIL,(u8 *)&alarmtail[0],12);
		 }
		 pdisDisPlayData[1]=0;
		 genterflag=0;
	 }
//	 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[2])&&(genterflag==1))
//	 {
		 
//		 if(pdisDisPlayData[2]==1)
//		 {  
//			  gpEnergy_100Wh[0]=0;
//			  gpEnergy_100Wh[1]=0;
//			  u16 *ptmp= (u16*)(&gpEnergy_100Wh[0]); 
//	         bkp_data_write(BKP_DATA_11,ptmp[0]);
//	         bkp_data_write(BKP_DATA_12,ptmp[1]);
//	 
//	    
//    	      ptmp= (u16*)(&gpEnergy_100Wh[1]);
//	           bkp_data_write(BKP_DATA_13,ptmp[0]);
//	           bkp_data_write(BKP_DATA_14,ptmp[1]);
//		 }
//		 pdisDisPlayData[2]=0;
//		 genterflag=0;
//	 }
	 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[3])&&(genterflag==1))
	 {
		 
		 if(pdisDisPlayData[3]==1)
		 {
			  
			   u16 _flag=0;
			  pgh52c0->writeI2C(ENERGY_DATA_FLAG,(u8 *)&_flag,2);//读取记录位置
				EnergyDataWriteConunt=0;
			  genergyDateRecordMax=0; 
			  pgh52c0->savePara(&genergyDateRecordMax);
			 
			   gTotalMrEnergy=0;
			   bkp_data_write(BKP_DATA_12, *(u16 *)&gTotalMrEnergy);
				 bkp_data_write(BKP_DATA_13,*((u16 *)&gTotalMrEnergy+1));
			 
			 
			 
			 
		 }
		 pdisDisPlayData[3]=0;
		 genterflag=0;
	 }
	 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[4])&&(genterflag==1))
	 {
		 
		 if(pdisDisPlayData[4]==1)
		 {
			    u16 _flag=0;
			  pgh52c0->writeI2C2(ENERGY_90DAY_DATA_FLAG,(u8 *)&_flag,2);//读取记录位置
//			  pgh52c0->writeI2C2(ENERGY_TMP,(u8 *)&_flag,2);//读取记录位置
				EnergyDataWriteConunt90=0;
			  genergyDateRecord90Max=0; 
			  pgh52c0->savePara(&genergyDateRecord90Max);
			  for(u8 i=0;i<8;i++)
			  gDCdistribution.pst_userEnerqy[i]=0;
			 
			 for(u8 i=0;i<4;i++)
			 {
				 
						preDCEnergydata[i]=0;
						cuDCEnergydata[i]=0;
						acDCEnergydata[i]=0;
				    gBattPara[i].battEnergy=0;
				 
				     bkp_data_write((bkp_data_register_enum)(2*i+1),*((u16 *)&gBattPara[i].battEnergy));
             bkp_data_write((bkp_data_register_enum)(2*i+2),*((u16 *)&gBattPara[i].battEnergy+1));
				 
			 }
			 
			 
			 
			 
			 
			 
		 }
		 pdisDisPlayData[4]=0;
		 genterflag=0;
	 }
	 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[5])&&(genterflag==1))
	 {
		 
		 if(pdisDisPlayData[5]==1)
		 {
			  u16 _flag=0;
			  pgh52c0->writeI2C(BATT_TEST_DATA_FLAG,(u8 *)&_flag,2);//读取记录位置
			 
				gbattTestCount=0;
			  gdisBattRecordMax=0; 
			  pgh52c0->savePara(&gdisBattRecordMax);
		 }
		 pdisDisPlayData[5]=0;
		 genterflag=0;
	 }
	 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[6])&&(genterflag==1))
	 {
		 
		 if(pdisDisPlayData[6]==1)
		 {
			 // gtimersyc=1;
			  u16 _flag=0x0000;
				pgh52c0->writeI2C2(SWITCH_PARA,(u8 *)&_flag,2);
		 }
		 pdisDisPlayData[6]=0;
		 genterflag=0;
	 }
	 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[7])&&(genterflag==1))
	 {
		 
		 if(pdisDisPlayData[7]==1)
		 {
			  gsysParaflag=1;
		 }
		 pdisDisPlayData[7]=0;
		 genterflag=0;
	 }
	 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[8])&&(genterflag==1))
	 {
		 
		     if(pdisDisPlayData[8]==1)
		 {
			 
			 
			     for(u8 i=0;i<6;i++)
			 {
				 
						
				    preDCEnergydata[i]=0;
						cuDCEnergydata[i]=0;
						acDCEnergydata[i]=gDCdistribution.pst_userEnerqy[i+1];
				 
				     bkp_data_write((bkp_data_register_enum)(2*i+1),*((u16 *)&acDCEnergydata[i]));//本地支路电流1
						 bkp_data_write((bkp_data_register_enum)(2*i+2),*((u16 *)&acDCEnergydata[i]+1));
				 
			 }
			 
			 
		 }
		 pdisDisPlayData[8]=0;
		 genterflag=0;
	 }
	 
	 
	

}





void SetBranchParaMenu(Screen *pCurrentScreen)//智能空开分路参数配置页
{   
	
		u8 branchCount=0,SearchCount=0;
	
	
	
	                   if(pdisDisPlayData[0]>pdisDisPlayData[1]) 
										 {	 
											  pdisDisPlayData[0]=pdisDisPlayData[1];
										    return;
										 }
                
	
	                	for( branchCount=0;branchCount<TOTAL_USER;branchCount++)
										 {
													if(SwitchOnlineCount[branchCount]>0)//查找已出现过的用户分路
													{
														 SearchCount++;
														if(SearchCount==pdisDisPlayData[0])
															break;
													}
										 }
										 
										if(branchCount==TOTAL_USER)
										{
											 
											pdisDisPlayData[0]=0;
												 return;
										}
	
	
	
	              
									pdisDisPlayData[2]=branchCount+1;//物理通道
									
	                pdisDisPlayData[32]=gDCdistribution.pst_switchtype[branchCount];//分路类型
	
	
	
											if(( pdisDisPlayData[31]!= pdisDisPlayData[0])||pdisDisPlayData[34]==1)//当前分路不等于（上一次执行）分路时，刷新屏幕参数
											 {
														pdisDisPlayData[34]=0;
																
																pdisDisPlayData[3]=gSwitchPara[ branchCount].st_downMode&0x03;
												        
																pdisDisPlayData[4]=gSwitchPara[ branchCount].st_userGroud&0x0f;
																*(u16 *)&pdisDisPlayData[5]=gSwitchPara[ branchCount].st_maxoverLoadI;
																*(u16 *)&pdisDisPlayData[7]=gSwitchPara[ branchCount].st_downV;
																*(u16 *)&pdisDisPlayData[9]=gSwitchPara[ branchCount].st_recoverV;

												 
												      if(gSwitchData[ branchCount].authorizeflag==0)
															{
																 														
															    pdisDisPlayData[11]=0;
																
															}
															else if(gSwitchData[ branchCount].authorizeflag==1)
															{
																  pdisDisPlayData[11]=1;

															}
															else if(gSwitchData[ branchCount].authorizeflag==0xff)
															{
																 
                                  pdisDisPlayData[11]=0;
															}
												 
												 
												 
																*(u16 *)&pdisDisPlayData[17]=gSwitchPara[ branchCount].st_downT;
																*(u16 *)&pdisDisPlayData[19]=gSwitchPara[ branchCount].st_downEnery;
																pdisDisPlayData[22]=gSwitchPara[ branchCount].startTime0/60;
																pdisDisPlayData[23]=gSwitchPara[ branchCount].startTime0%60;
												
																pdisDisPlayData[24]=gSwitchPara[ branchCount].stopTime0/60;
																pdisDisPlayData[25]=gSwitchPara[ branchCount].stopTime0%60;
																
																
																pdisDisPlayData[27]=gSwitchPara[ branchCount].startTime1/60;
																pdisDisPlayData[28]=gSwitchPara[ branchCount].startTime1%60;
												
																pdisDisPlayData[29]=gSwitchPara[ branchCount].stopTime1/60;
																pdisDisPlayData[30]=gSwitchPara[ branchCount].stopTime1%60;
																
															  pdisDisPlayData[33]=(gSwitchPara[ branchCount].st_downMode&0x0C)>>2;
																	
																  if((gSwitchPara[ branchCount].st_downMode&0x10)!=0)
																	 {
																			 pdisDisPlayData[21]=1;
																	 }
																	 else
																		{
																			 pdisDisPlayData[21]=0;
																	 } 
																		
																	  if((gSwitchPara[ branchCount].st_downMode&0x20)!=0)
																	 {
																			 pdisDisPlayData[26]=1;
																	 }
																	 else
																		{
																			 pdisDisPlayData[26]=0;
																	 } 
																		
																	 pdisDisPlayData[35]=gSwitchData[ branchCount].rateI;
																	 
																	 *((u32*)&pdisDisPlayData[36])=gDCdistribution.pst_enerqy[branchCount];
																	 
																	 gSwitchData[ branchCount].CmdStatus=1;
																	 
											 }
											 gSetDownParaAddr= branchCount+1;//pdisDisPlayData[0];
											 pdisDisPlayData[31]= pdisDisPlayData[0];
	
                 
	 u8 usegroud[12]={0,1,1,2,2,3,3,4,4,6,5,7};//0无, 1移动 ，2联通，3电信，4广电，6铁塔，5行业外，7电池

       if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[3])&&(genterflag==1))//下电模式
			 {
				    genterflag=0;
				                     gSwitchPara[ branchCount].st_downMode&=0xF0;
				                     gSwitchPara[ branchCount].st_downMode|=pdisDisPlayData[3];
				                        gSetDownParaCode=12;//下电模式命令码
				                        gSetDownParaGroudType=0x51;
				                        pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downMode,1);
			 }
			 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[21])&&(genterflag==1))//定时下电使能
			 {
				    genterflag=0;
				                     if(pdisDisPlayData[21]==1)
				                     gSwitchPara[ branchCount].st_downMode|=0x10;
														 else
															gSwitchPara[ branchCount].st_downMode&=0xEF;
														 
				                        gSetDownParaCode=12;//下电模式命令码
				                        gSetDownParaGroudType=0x51;
														 
														    pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downMode,1);
														  
			 }
			 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[26])&&(genterflag==1))//免责下电使能
			 {
				    genterflag=0;
				                      if(pdisDisPlayData[26]==1)
				                     gSwitchPara[ branchCount].st_downMode|=0x20;
														 else
															gSwitchPara[ branchCount].st_downMode&=0xDF;
				                 
				                        gSetDownParaCode=12;//下电模式命令码
				                        gSetDownParaGroudType=0x51;
														 
														   pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downMode,1);
			 }
			 
			 
			 
			 
			 
			 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[4])&&(genterflag==1))//分路用户
			 {
				    genterflag=0;
				 
				 
				                     if((gSwitchPara[ branchCount].st_userGroud==0x7B)&&(pdisDisPlayData[4]!=0x0B))//若设置前为电池分路并且将设为非电池分路，则移除分路电池绑定
														 {
															  for(u8 i=0;i<6;i++)
															 {
																  if(gUseToBattBranch[i]==( branchCount))
																	{
																		
																		   pgBattBranch[i]=NULL;//移除电池绑定
																		   gUseToBattBranch[i]=0xff;//设置电池分路设为未定义
																		   gbattGroud--;//电池分路数减一
																		   
																		   break;
																	
																	}
															 }
															 pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入

														 }
														 else if((gSwitchPara[ branchCount].st_userGroud!=0x7B)&&(pdisDisPlayData[4]==0x0B))//若设置前为非电池分路并且将设为电池分路，则分路绑定电池
														 {
															 
																 if(gDCdistribution.pst_switchtype[ branchCount]!=2)//若当前分路开关不是双向开关
																 {
																		 pdisDisPlayData[4]=gSwitchPara[ branchCount].st_userGroud&0x0f;//则将分路类型设为原值
																		 return ;//返回
																 }
																 u8 i;
															   for( i=0;i<6;i++)
																 {
																	 
																		if(gUseToBattBranch[i]==0xff)
																		{
																			
																				 pgBattBranch[i]=&gDCdistribution.pst_I[ branchCount];//电池绑定
																				 gUseToBattBranch[i]= branchCount;//设置电池分路
																				 gbattGroud++;//电池分路数加一
																			   
																			   break;
																		
																		}
																 }
																 if(i==6)//电池绑定失败
																 {
																	  pdisDisPlayData[4]=gSwitchPara[ branchCount].st_userGroud&0x0f;//则将分路类型设为原值
																		 return ;//返回
																 }
																 
																 pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入

														 }
															 
														 
														 
				                       gSwitchPara[ branchCount].st_userGroud=pdisDisPlayData[4]|(usegroud[pdisDisPlayData[4]]<<4);
				                       gSetDownParaCode=1;//分组号命令码
				                       gSetDownParaGroudType=0x51;
														
														  
														 if(gSwitchPara[ branchCount].st_userGroud!=0x7B)//用户类型只存非电池的用户
														 {
														   pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_userGroud,1);
														 
														 
														 
														  *(((u16 *)&gsetTmpData)+1)=gSwitchPara[ branchCount].st_downV;
														  *(((u8 *)&gsetTmpData)+0)=gSwitchPara[ branchCount].st_userGroud;
														  gcmdAddr = branchCount+1;
														  gSwitchData[branchCount].CmdStatus=8;
														 }
			 }
			 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[5])//过载电流
			 {
				                              CheckParaBoundary((u16 *)&pdisDisPlayData[5],0 ,2000);
																			if(genterflag==1)
																			{
																						genterflag=0;
																					gSwitchPara[ branchCount].st_maxoverLoadI=*(u16 *)&pdisDisPlayData[5];
																				  gSetDownParaCode=11;//最大过载电流命令码
																				  gSetDownParaGroudType=0x51;
																				   pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_maxoverLoadI);
																				
																				   gSwitchData[branchCount].CmdStatus=2;
																				   gcmdAddr=branchCount+1;
																			}
																			
				 
			 }
			 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[7])//下电电压
			 {
				                             CheckParaBoundary((u16 *)&pdisDisPlayData[7],4000 ,5300);
				                         if(genterflag==1)
																 {
																    genterflag=0;
															      gSwitchPara[ branchCount].st_downV= *(u16 *)&pdisDisPlayData[7];
																	  gSetDownParaCode=4;//下电电压命令码
																	  gSetDownParaGroudType=0x51;
																	   pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downV);
																	 
																	   *(((u16 *)&gsetTmpData)+1)=gSwitchPara[ branchCount].st_downV;
																	    *(((u8 *)&gsetTmpData)+0)=gSwitchPara[ branchCount].st_userGroud;
																		gcmdAddr = branchCount+1;
																		gSwitchData[branchCount].CmdStatus=8;
																	 
																 }
				 
			 }
			 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[9])&&(genterflag==1))//恢复电压
			 {
				                           CheckParaBoundary((u16 *)&pdisDisPlayData[9],4000 ,5765);
				                          if(genterflag==1)
																 {
																		genterflag=0;
																		gSwitchPara[ branchCount].st_recoverV= *(u16 *)&pdisDisPlayData[9];
																	   gSetDownParaCode=6;//恢复电压命令码
																	   gSetDownParaGroudType=0x51;
																	   pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_recoverV);
																 }
			 }
			 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[11])//截止日期
			 {
				                         
				 
				                         if(genterflag==1)
																 {
				                                genterflag=0;
																			 
																				 if(pdisDisPlayData[11]==0)
																	       {
																					   gSwitchPara[ branchCount].st_stopTime.st_year= 1900;
																						 gSwitchPara[ branchCount].st_stopTime.st_mon=1;
																						 gSwitchPara[ branchCount].st_stopTime.st_day=1;
																					 
																					   gSwitchData[branchCount].setAuthorize=0;
																					   gSwitchData[branchCount].CmdStatus=7;
																					   gcmdAddr=branchCount+1;
																			   }
																				 else if(pdisDisPlayData[11]==1)
																				 {
																					   gSwitchPara[ branchCount].st_stopTime.st_year= 2100;
																						 gSwitchPara[ branchCount].st_stopTime.st_mon=1;
																						 gSwitchPara[ branchCount].st_stopTime.st_day=1;
																					 
																					   gSwitchData[branchCount].setAuthorize=1;
																					   gSwitchData[branchCount].CmdStatus=7;
																					   gcmdAddr=branchCount+1;
																				 }
																	       
																				gSetDownParaCode=2;//截止时间 命令码
																	      gSetDownParaGroudType=0x51;
																	      pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_stopTime,4);
																	 
																	 
																	 
																	 
																 }
			 }
			 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[17])//下电时间
			 {
															CheckParaBoundary((u16 *)&pdisDisPlayData[17],1 ,1440);
				                   if(genterflag==1)
													 {
				                      genterflag=0;
															gSwitchPara[ branchCount].st_downT = *(u16 *)&pdisDisPlayData[17];
														  gSetDownParaCode=5;//下电时间 命令码
														  gSetDownParaGroudType=0x51;
														 
														  pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downT);
													 }
			 }
			 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[19])&&(genterflag==1))//下电电量
			 {
				                               if(genterflag==1)
													            {         
				                               genterflag=0;
				                               gSwitchPara[ branchCount].st_downEnery=*(u16 *)&pdisDisPlayData[19];
																			 gSetDownParaCode=3;//下电电能 命令码
																			 gSetDownParaGroudType=0x51;	
																			  pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downEnery);
													            }
			 }
			 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[22])//开始定时间
			 {
												CheckParaBoundary8(&pdisDisPlayData[22],0 ,23);
				                CheckParaBoundary8(&pdisDisPlayData[23],0 ,59);
                				 if(genterflag==1)
													{
														genterflag=0;
														gSwitchPara[ branchCount].startTime0=pdisDisPlayData[22]*60+pdisDisPlayData[23];
														gSetDownParaCode=7;//定时下电始 命令码
														gSetDownParaGroudType=0x51;
														 pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].startTime0);
															gSwitchPara[ branchCount].st_downMode&=0xEF;
														  pdisDisPlayData[31]=0;
													}
													
			 }
			 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[24])&&(genterflag==1))//定时时长
			 {
				                 CheckParaBoundary8(&pdisDisPlayData[24],0 ,23);
				                 CheckParaBoundary8(&pdisDisPlayData[25],0 ,59);
                				 if(genterflag==1)
													{
														genterflag=0;
														gSwitchPara[ branchCount].stopTime0=pdisDisPlayData[24]*60+pdisDisPlayData[25];
														gSetDownParaCode=8;//定时下电终 命令码
														gSetDownParaGroudType=0x51;
														pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].stopTime0);
														
															gSwitchPara[ branchCount].st_downMode&=0xEF;
														  pdisDisPlayData[31]=0;
													}
			 }
			  else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[27])//免责开始
			 {
				                  CheckParaBoundary8(&pdisDisPlayData[27],0 ,23);
				                  CheckParaBoundary8(&pdisDisPlayData[28],0 ,59);
                				 if(genterflag==1)
													{
														genterflag=0;
														gSwitchPara[ branchCount].startTime1=pdisDisPlayData[27]*60+pdisDisPlayData[28];
														gSetDownParaCode=9;//免责下电始 命令码
														gSetDownParaGroudType=0x51;
														pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].startTime1);
														
															gSwitchPara[ branchCount].st_downMode&=0xDF;
														  pdisDisPlayData[31]=0;
													}
			 }
			 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[29])//免责时长
			 {
				                  CheckParaBoundary8(&pdisDisPlayData[29],0 ,23);
				                  CheckParaBoundary8(&pdisDisPlayData[30],0 ,59);
                				 if(genterflag==1)
													{
														genterflag=0;
														gSwitchPara[ branchCount].stopTime1=pdisDisPlayData[29]*60+pdisDisPlayData[30];
														gSetDownParaCode=10;//免责下电终 命令码
														gSetDownParaGroudType=0x51;
														pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].stopTime1);
															gSwitchPara[ branchCount].st_downMode&=0xDF;
														  pdisDisPlayData[31]=0;
														
													}  
			 }
										 

       else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[36])//电量设置
			 {
				        if(genterflag==1)
								{
									
									   genterflag=0;
									   gsetTmpData=*((u32*)&pdisDisPlayData[36]);
									   gSwitchData[branchCount].CmdStatus=5;
									   gcmdAddr=branchCount+1;
									
								}
			 }
			 else if(pCurrentScreen->m_EnterStatus!=3)
			 {
				 *(u16 *)&pdisDisPlayData[5]=gSwitchPara[ branchCount].st_maxoverLoadI;
				 *((u32*)&pdisDisPlayData[36])=gDCdistribution.pst_enerqy[branchCount];
			 }

			 
			 
}

u32 Power[TOTAL_USER]={0};
s16 userCurr = 0; // 0.1A
void SetUSRPara(Screen *pCurrentScreen)
{
	
 
	u32 totalEng=0;
	     if (pdisDisPlayData[2]==0) pdisDisPlayData[2]=pdisDisPlayData[3];//通道号
	     if (pdisDisPlayData[2]>pdisDisPlayData[3]) pdisDisPlayData[2]=1;
	
	     u8 tmpbranch=0,i;
	     
	                   for( i=0;i<TOTAL_USER;i++)
										 {
													if(SwitchOnlineCount[i]>0)//查找已出现过的用户分路
													{
														  tmpbranch++;
														
														    if(tmpbranch==pdisDisPlayData[2])
																{
																	break;
																}
														
													}
										 }
										 
										 if(i==TOTAL_USER) return;
	

	

		 	*((u16 *)&pdisDisPlayData[4])=gDCdistribution.pst_I[i];
			*((u32 *)&pdisDisPlayData[6])=gDCdistribution.pst_enerqy[i];
			*((u16 *)&pdisDisPlayData[10])=(*((u16 *)&gpSysData[DCVOLTAGE]));
			 Power[i]=*((u16 *)&pdisDisPlayData[10])*(*((u16 *)&pdisDisPlayData[4]))/10;//*(*((u16 *)&pdisDisPlayData[10]));
		  *((u32 *)&pdisDisPlayData[12])=Power[i];
	
}

void SetUSR2Menu(Screen *pCurrentScreen)
{
	 int64_t tatoatEnergy=0;
   int64_t tmpEnergy=0;
	    if (pdisDisPlayData[2]<1) pdisDisPlayData[2]=6;//通道号
	     if (pdisDisPlayData[2]>6) pdisDisPlayData[2]=1;
	
	
	   		 	*((u16 *)&pdisDisPlayData[4])=gDCdistribution.pst_userI[pdisDisPlayData[2]];
			    *((u32 *)&pdisDisPlayData[6])=gDCdistribution.pst_userEnerqy[pdisDisPlayData[2]];
	
	        *((u32 *)&pdisDisPlayData[10])=acDCEnergydata[pdisDisPlayData[2]-1];
	
	
	        tatoatEnergy=0;
	        tmpEnergy=gDCdistribution.pst_userEnerqy[pdisDisPlayData[2]]*100;
	       
	        for(u8 i=1;i<7;i++)
					{
						  tatoatEnergy+=gDCdistribution.pst_userEnerqy[i];
					}
					
					 if(tatoatEnergy>0)
					 {
						 tmpEnergy=tmpEnergy/tatoatEnergy;
							*((u16 *)&pdisDisPlayData[0])=(u16)tmpEnergy;
					 }
					 else
					 *((u16 *)&pdisDisPlayData[0])=0;
	
	
}

void SetUSR3Menu(Screen *pCurrentScreen)
{

	     if (pdisDisPlayData[0]<1) pdisDisPlayData[0]=4;//通道号
	     if (pdisDisPlayData[0]>4) pdisDisPlayData[0]=1;
	
			 battIndex = pdisDisPlayData[0] -1;
	     
				if(pCurrentScreen->m_EnterStatus!=3)
				{
						*(s16 *)&pdisDisPlayData[2]=(*((s16 *)&gBattPara[battIndex].battVol))/10;
						*(s16 *)&pdisDisPlayData[4]=*((s16 *)&gBattPara[battIndex].battCurr1);
					 	*(u32 *)&pdisDisPlayData[6]=*((u32 *)&gBattPara[battIndex].battEnergy);
				}

}


void SetUserDownMenu(Screen *pCurrentScreen)
{ 
	        
	
	
	     
			 
			 if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[3])&&(genterflag==1))//下是模式
			 {
				    genterflag=0;
				                     gSwitchPara[7+TOTAL_USER].st_downMode&=0xF0;
				                     gSwitchPara[7+TOTAL_USER].st_downMode|=pdisDisPlayData[3];
				                      pgh52c0->SaveSwitchPara(&gSwitchPara[7+TOTAL_USER].st_downMode,1);
				                        gSetDownParaCode=12;//下电模式命令码
				                       
				 
				             //同步相同用户的下电参数
				                for(u8 i=0;i<TOTAL_USER;i++)
											 {
												  if(gSwitchPara[i].st_userGroud == 0x7B)
													{
														       gSwitchPara[i].st_downMode=gSwitchPara[pdisDisPlayData[0]+TOTAL_USER].st_downMode;
														       pgh52c0->SaveSwitchPara(&gSwitchPara[i].st_downV,1);
													}
													
											 }
				 
				                gSetDownParaGroudType=0x52;
				 
			 }
			 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[7])
			 {
				                             CheckParaBoundary((u16 *)&pdisDisPlayData[7],4000 ,5300);
				                         if(genterflag==1)
																 {
																    genterflag=0;
															      gSwitchPara[7+TOTAL_USER].st_downV= *(u16 *)&pdisDisPlayData[7];
																	   pgh52c0->SaveSwitchPara(&gSwitchPara[7+TOTAL_USER].st_downV);
																	  gSetDownParaCode=4;//下电电压命令码
																	 
										
																	     	//同步电池分路
																				for(u8 i=0;i<TOTAL_USER;i++)
																			 {
																					if(gSwitchPara[i].st_userGroud==0x7B)
																					{
																									 gSwitchPara[i].st_downV=gSwitchPara[7+TOTAL_USER].st_downV;
																										pgh52c0->SaveSwitchPara(&gSwitchPara[i].st_downV);
																					}
																				
																			 }
																	  gSetDownParaGroudType=0x52;
																 }
				 
			 }
			 else if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[9])&&(genterflag==1))
			 {
				                           CheckParaBoundary((u16 *)&pdisDisPlayData[9],4000 ,5300);
				                          if(genterflag==1)
																 {
																		genterflag=0;
																		gSwitchPara[7+TOTAL_USER].st_recoverV= *(u16 *)&pdisDisPlayData[9];
																	   pgh52c0->SaveSwitchPara(&gSwitchPara[7+TOTAL_USER].st_recoverV);
																	   gSetDownParaCode=6;//恢复电压命令码
																	 
																
																	  //同步电池分路
																		for(u8 i=0;i<TOTAL_USER;i++)
																	 {
																			if(gSwitchPara[i].st_userGroud == 0x7B)
																			{
																							 gSwitchPara[i].st_recoverV=gSwitchPara[7+TOTAL_USER].st_recoverV;
																				       pgh52c0->SaveSwitchPara(&gSwitchPara[i].st_recoverV);
																			}
																			
																	 }
																	 
																	   gSetDownParaGroudType=0x52;
																 }
			 }
			 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[17])
			 {
															CheckParaBoundary((u16 *)&pdisDisPlayData[17],1 ,1440);
				                   if(genterflag==1)
													 {
																		genterflag=0;
																		gSwitchPara[7+TOTAL_USER].st_downT = *(u16 *)&pdisDisPlayData[17];
														        pgh52c0->SaveSwitchPara(&gSwitchPara[7+TOTAL_USER].st_downT);
																		gSetDownParaCode=5;//下电时间 命令码
																	 
																	//同步电池分路
																	 for(u8 i=0;i<TOTAL_USER;i++)
																 {
																		if(gSwitchPara[i].st_userGroud == 0x7B)
																		{
																						 
																			
																			 gSwitchPara[i].st_downT=gSwitchPara[7+TOTAL_USER].st_downT;
																			 pgh52c0->SaveSwitchPara(&gSwitchPara[i].st_downT);
																				 
																			
																		}
										
																 }
														  gSetDownParaGroudType=0x52;
													 }
			 }
			 
			  if(pCurrentScreen->m_EnterStatus!=3)//处于非编辑状态时
			 {
				    
				       
				        pdisDisPlayData[3]=gSwitchPara[TOTAL_USER+7].st_downMode&0x03;
				        *(u16 *)&pdisDisPlayData[7]=gSwitchPara[TOTAL_USER+7].st_downV;
				        *(u16 *)&pdisDisPlayData[9]=gSwitchPara[TOTAL_USER+7].st_recoverV;
				       *(u16 *)&pdisDisPlayData[17]=gSwitchPara[TOTAL_USER+7].st_downT;
				 
				      genterflag=0;			
			 }
			 


}




void SetPOWERRATEMenu(Screen *pCurrentScreen)
{
	        *((u32 *)&pdisDisPlayData[0])=powerRate[0]/1000;
			    *((u32 *)&pdisDisPlayData[4])=powerRate[1]/1000;
			    *((u32 *)&pdisDisPlayData[8])=powerRate[2]/1000;
		    	*((u32 *)&pdisDisPlayData[12])=powerRate[3]/1000;
			    *((u32 *)&pdisDisPlayData[16])=powerRate[4]/1000;
	        *((u32 *)&pdisDisPlayData[20])=powerRate[5]/1000;
}


void  SetBattBranchMenu(Screen *pCurrentScreen)//扩展分路电池分路配置
{
	
	
      if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])&&(genterflag==1))
				 {
					 genterflag=0;
					   
					    if( gUseToBattBranch[0] != 0xff)
							{
								 gSwitchPara[gUseToBattBranch[0]].st_userGroud=0;
								 gUseToBattBranch[0]=0xff;
								 pgBattBranch[0]=NULL;
								 if(gbattGroud>0)
								 {				    
									   gbattGroud--;//电池分路数减一
	 	 
								 }
								 pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
							   pdisDisPlayData[0]=0;
								 
								 
								 
							}
							
				 }
			else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[1])&&(genterflag==1))
				 {
					 genterflag=0;

					       if( gUseToBattBranch[1] != 0xff)
							{
					       gSwitchPara[gUseToBattBranch[1]].st_userGroud=0;
								 gUseToBattBranch[1]=0xff;
								 pgBattBranch[1]=NULL;	
                 if(gbattGroud>0)
								 {
								    
									   gbattGroud--;//电池分路数减一

								 }
								 pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
								  pdisDisPlayData[1]=0;
							}

							  

				 }
			else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[2])&&(genterflag==1))
				 {
					 genterflag=0;

					      if( gUseToBattBranch[2] != 0xff)
							{
					       gSwitchPara[gUseToBattBranch[2]].st_userGroud=0;
								 gUseToBattBranch[2]=0xff;
								 pgBattBranch[2]=NULL;
								 if(gbattGroud>0)
								 {
								    
									   gbattGroud--;//电池分路数减一

									 
								 }		
								 pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
								  pdisDisPlayData[2]=0;
							}

							  
				 }
			else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[3])&&(genterflag==1))
				 {
					 genterflag=0;

					      if( gUseToBattBranch[3] != 0xff)
							{
					       gSwitchPara[gUseToBattBranch[3]].st_userGroud=0;
								 gUseToBattBranch[3]=0xff;
								 pgBattBranch[3]=NULL;
								if(gbattGroud>0)
								 {
								    
									   gbattGroud--;//电池分路数减一
									 
									 
								 }			 
								 pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
								 pdisDisPlayData[3]=0;
							}

				 }
			else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[4])&&(genterflag==1))
				 {
					 genterflag=0;
               
					      if( gUseToBattBranch[4] != 0xff)
							{
					       gSwitchPara[ gUseToBattBranch[4]].st_userGroud=0;
								 gUseToBattBranch[4]=0xff;
								 pgBattBranch[4]=NULL;	
                 if(gbattGroud>0)
								 {
								    
									   gbattGroud--;//电池分路数减一

									 
								 }
								 pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
				          pdisDisPlayData[4]=0;  
							}								

				 }
			else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[5])&&(genterflag==1))
				 {
					 genterflag=0;
 
					      if( gUseToBattBranch[5] != 0xff)
							{
                 gSwitchPara[gUseToBattBranch[5]].st_userGroud=0;
								 gUseToBattBranch[5]=0xff;
								 pgBattBranch[5]=NULL;
								 if(gbattGroud>0)
								 {
								    
									   gbattGroud--;//电池分路数减一
    								 
									 
								 }
								 pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
                 pdisDisPlayData[5]=0;
							}
				 }
			else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[6])&&(genterflag==1))
			 {
				 //genterflag=0;
				 
				   CheckParaBoundary((u16 *)&pdisDisPlayData[6],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			
																			if(	gbattGroud > 0)
																			{
																				gbranchBatt[0] = *(u16 *)&pdisDisPlayData[6];
																				
																				pgh52c0->savePara(&gbranchBatt[0]);
																				gSetBattC=0;
																				 for(u8 i=0;i<6;i++)
																				{
																						 if(gUseToBattBranch[i]<TOTAL_USER)
																						 {
																								
																								 gSetBattC+=gbranchBatt[i];
																						 }
																							
																				}
																				 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																				
																			}	
																			
																			genterflag=0;
																		}

							
			 }
			 else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[8])&&(genterflag==1))
			 {
				 //genterflag=0;
				 
				   CheckParaBoundary((u16 *)&pdisDisPlayData[8],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			if(gbattGroud > 0)
																			{
																						gbranchBatt[1] = *(u16 *)&pdisDisPlayData[8];
																					
																					pgh52c0->savePara(&gbranchBatt[1]);
																					gSetBattC=0;
																					 for(u8 i=0;i<6;i++)
																					{
																							 if(gUseToBattBranch[i]<TOTAL_USER)
																							 {
																									
																									 gSetBattC+=gbranchBatt[i];
																							 }
																								
																					}
																					 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																		 }
																			genterflag=0;//
																		}

							
			 }
			  else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[10])&&(genterflag==1))
			 {
				 //genterflag=0;
				 
				   CheckParaBoundary((u16 *)&pdisDisPlayData[10],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			
																		  if( gbattGroud > 0)
																			{
																					gbranchBatt[2] = *(u16 *)&pdisDisPlayData[10];	
																					pgh52c0->savePara(&gbranchBatt[2]);
																					gSetBattC=0;
																					 for(u8 i=0;i<6;i++)
																					{
																							 if(gUseToBattBranch[i]<TOTAL_USER)
																							 {
																									
																									 gSetBattC+=gbranchBatt[i];
																							 }
																								
																					}
																					 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																		 }
																			genterflag=0;//
																		}

							
			 }
			  else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[12])&&(genterflag==1))
			 {
				 //genterflag=0;
				 
				   CheckParaBoundary((u16 *)&pdisDisPlayData[12],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			
																			if( gbattGroud > 0)
																			{
																					gbranchBatt[3] = *(u16 *)&pdisDisPlayData[12];
																					
																					pgh52c0->savePara(&gbranchBatt[3]);
																					gSetBattC=0;
																					 for(u8 i=0;i<6;i++)
																					{
																							 if(gUseToBattBranch[i]<TOTAL_USER)
																							 {
																									
																									 gSetBattC+=gbranchBatt[i];
																							 }
																								
																					}
																					 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																		 }
																			genterflag=0;//
																		}

							
			 }
			 else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[14])&&(genterflag==1))
			 {
				 //genterflag=0;
				 
				   CheckParaBoundary((u16 *)&pdisDisPlayData[14],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			  if( gbattGroud > 0)
																				{
																					gbranchBatt[4] = *(u16 *)&pdisDisPlayData[14];
																					
																					pgh52c0->savePara(&gbranchBatt[4]);
																					gSetBattC=0;
																					 for(u8 i=0;i<6;i++)
																					{
																							 if(gUseToBattBranch[i]<TOTAL_USER)
																							 {
																									
																									 gSetBattC+=gbranchBatt[i];
																							 }
																								
																					}
																					 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																				}
																			genterflag=0;//
																		}

							
			 }
				 
				 else	if(((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[16])&&(genterflag==1))
			 {
				 //genterflag=0;
				 
				   CheckParaBoundary((u16 *)&pdisDisPlayData[16],0 ,8000);//限流系数限制
			                               if(genterflag==1)
													          {
																			if( gbattGroud > 0)
																			{
																			  gbranchBatt[5] = *(u16 *)&pdisDisPlayData[16];									
																				pgh52c0->savePara(&gbranchBatt[5]);
																				gSetBattC=0;
																				 for(u8 i=0;i<6;i++)
																				{
																						 if(gUseToBattBranch[i]<TOTAL_USER)
																						 {
																								
																								 gSetBattC+=gbranchBatt[i];
																						 }
																							
																				}
																				 pbattCap->ReSetCap();//重置SOC为百分一百,清空放电总时长
																			}
																			genterflag=0;//
																		}

							
			 } 
				 
}



void GetUpDataLogMenu(Screen *pCurrentScreen)
{    
	    u16 _flag=0;
	    u16 *para16=(u16 *)pdisDisPlayData;
			if(para16[0]!=para16[1])
			{
	
	      pgh52c0->readI2C2(UPDATA_LOG,(u8 *)&_flag,2);
				if(_flag==0x5a5a)
				{   u8 tmpdat[18];
					 
				
				
					
			u16 len=0;
	      pgh52c0->readI2C2(UPDATA_LOG+2,(u8 *)&len,2);
		//	  if(len>=5)len=0;
					
			 // if(para16[0]<=0) return;
				if(para16[0]>len)para16[0]=len;
				if(((s16)para16[0])<(len-4))para16[0]=len-4;
				//if(para16[0]==0) para16[0]=1;
					
					if(para16[0]>0)
					{

					
							pgh52c0->readI2C2(UPDATA_LOG_START+14*((para16[0]-1)%5),tmpdat,14);
							
							(*(u16 *)&pdisDisPlayData[4])=(*(DateTimeF33 *)&tmpdat[0]).st_year;
									pdisDisPlayData[6]=(*(UPDATA_LOG_ST *)&tmpdat[0]).stime.st_mon;
									pdisDisPlayData[7]=(*(UPDATA_LOG_ST *)&tmpdat[0]).stime.st_day;
									pdisDisPlayData[8]=(*(UPDATA_LOG_ST *)&tmpdat[0]).stime.st_hour;
									pdisDisPlayData[9]=(*(UPDATA_LOG_ST *)&tmpdat[0]).stime.st_min;
									pdisDisPlayData[10]=(*(UPDATA_LOG_ST *)&tmpdat[0]).stime.st_sec;
							
							
								(*(u16 *)&pdisDisPlayData[11])=(*(UPDATA_LOG_ST *)&tmpdat[0]).stime.st_year;
									pdisDisPlayData[13]=(*(UPDATA_LOG_ST *)&tmpdat[0]).etime.st_mon;
									pdisDisPlayData[14]=(*(UPDATA_LOG_ST *)&tmpdat[0]).etime.st_day;
									pdisDisPlayData[15]=(*(UPDATA_LOG_ST *)&tmpdat[0]).etime.st_hour;
									pdisDisPlayData[16]=(*(UPDATA_LOG_ST *)&tmpdat[0]).etime.st_min;
									pdisDisPlayData[17]=(*(UPDATA_LOG_ST *)&tmpdat[0]).etime.st_sec;
							para16[1]=para16[0];
							
							
							if((*(UPDATA_LOG_ST *)&tmpdat[0]).verOle==0xE000)
							{
									sprintf((char *)&pdisDisPlayData[18],( const char *)"升级失败");
							}
							else
							{
								if(para16[0]==len)
								{
									sprintf((char *)&pdisDisPlayData[18],( const char *)"旧:%4.2f 新:%4.2f",(float)(*(UPDATA_LOG_ST *)&tmpdat[0]).verOle/100,(float)VerNB/100);
								}
								else if(para16[0]<len)
								{
									u8 tmpdatNext[18];
									pgh52c0->readI2C2(UPDATA_LOG_START+14*(para16[0]%5),tmpdatNext,14);//取其下一条记录，
									sprintf((char *)&pdisDisPlayData[18],( const char *)"旧:%4.2f 新:%4.2f",(float)(*(UPDATA_LOG_ST *)&tmpdat[0]).verOle/100,(float)(*(UPDATA_LOG_ST *)&tmpdatNext[0]).verOle/100);
								}
							}
							
							
						}
					
					}
					
					
					
				}
	
}

void SetCalibrationMenu2(Screen *pCurrentScreen)
{
	
				if(battAddr[0] > 4 || battAddr[0] == 0)
			{
					battAddr[0] = 1;  // 只有当索引无效时才重置为1
			}			
			battIndex = battAddr[0]-1;
	
		if(pCurrentScreen->m_EnterStatus!=3)
		{
				if(pdisDisPlayData[0]==0)
				{
				 *(s16 *)&pdisDisPlayData[2] = *((s16 *)&gBattPara[battIndex].battCurr1);
								 
					
				}
				else if(pdisDisPlayData[0]==1)
				{
				 *(s16 *)&pdisDisPlayData[2] = *((s16 *)&gBattPara[battIndex].battCurr2);  
								
				}
				else if(pdisDisPlayData[0]==2)
				{
				 *(s16 *)&pdisDisPlayData[2] = *((s16 *)&gBattPara[battIndex].battVol)/10;    
								
				}
				
//				*(u32 *)&pdisDisPlayData[10]	=	*((u32 *)&gBattPara[battIndex].battEnergy);
		}
		
			
	    if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[4])//电量设置
			{
				
				      if(genterflag==1)
							{
								 genterflag=0;
								
								 setPoint=pdisDisPlayData[0]*2+pdisDisPlayData[1]+1;//
								 setValu=*(s16 *)&pdisDisPlayData[4];
								if(pdisDisPlayData[0]==2)  setValu*=10;
								 setBattParaflag=1;
							}
				 
			}
			if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[6])
			{
				  if(genterflag==1)
							{
								 genterflag=0;
								
								  if(pdisDisPlayData[0]==0)
								   sysPara[BATT5_EN]=pdisDisPlayData[6];
									else if(pdisDisPlayData[0]==1)
								   sysPara[BATT6_EN]=pdisDisPlayData[6];
								
							}
				
				
				
				
			}
			else
			{
				  genterflag=0;
			}
	    

}
void Update_BattData(Screen *pCurrentScreen)
{
	if(battID[0]>LI_BATTERY_NUM||battID[0]==0)
	{
		battID[0]=1;
	}
	
	if(batt[battID[0]-1].MOS_Charg==0x55)
	{
			(*(u16 *)&pdisDisPlayData[18])=0;	//MOS充状态
	}
	else if(batt[battID[0]-1].MOS_Charg==1)
	{
		(*(u16 *)&pdisDisPlayData[18])=1;	//MOS充状态
	}
	
	if(batt[battID[0]-1].MOS_DisCharg==0x55)
	{
			(*(u16 *)&pdisDisPlayData[20])=0;	//MOS充状态
	}
	else if(batt[battID[0]-1].MOS_DisCharg==1)
	{
		(*(u16 *)&pdisDisPlayData[20])=1;	//MOS充状态
	}
	

    pdisDisPlayData[26]=batt[battID[0]-1].B_Status;
    if(batt[battID[0]-1].B_Status>14) pdisDisPlayData[26]=15;
		

	
	//锂电信息更新
	(*(u16 *)&pdisDisPlayData[0])=batt[battID[0]-1].Vbus;		//总线电压
	(*(u16 *)&pdisDisPlayData[2])=batt[battID[0]-1].Vbat;		//电池电压
	(*(u16 *)&pdisDisPlayData[4])=batt[battID[0]-1].Ibus;		//充放电流
	(*(u16 *)&pdisDisPlayData[6])=batt[battID[0]-1].B_Status;	//充放状态
	(*(u16 *)&pdisDisPlayData[8])=batt[battID[0]-1].CellTempMax;		//电池温度 
	(*(u16 *)&pdisDisPlayData[10])=batt[battID[0]-1].Cell_Num;	//电池芯数
	(*(u16 *)&pdisDisPlayData[12])=batt[battID[0]-1].B_SOC;		//SOC
	(*(u16 *)&pdisDisPlayData[14])=batt[battID[0]-1].B_SOH;		//SOH
	(*(u16 *)&pdisDisPlayData[16])=batt[battID[0]-1].B_capacity;	//额定容量
	(*(u16 *)&pdisDisPlayData[22])=batt[battID[0]-1].Set_BattA;//放电电流
	(*(u16 *)&pdisDisPlayData[24])=batt[battID[0]-1].Last_Time;//剩余时间
	(*(u16 *)&pdisDisPlayData[28])=batt[battID[0]-1].CellVMin;//最小单体电压
	(*(u16 *)&pdisDisPlayData[30])=batt[battID[0]-1].CellVMax;//最大单体电压
	(*(u16 *)&pdisDisPlayData[32])=batt[battID[0]-1].Ibat;		//充放电流
	(*(u16 *)&pdisDisPlayData[34])=	batt[battID[0]-1].batteryCycleCount;//电池循环次数

	
}


void SetBrainBatt(Screen *pCurrentScreen)
{

	if(battID[0]>LI_BATTERY_NUM||battID[0]==0)
	{
		battID[0]=1;
	}
	
	 if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])
	 {
		 CheckParaBoundary((u16 *) &pdisDisPlayData[0],4000,6000);//锂电放电电压 40-60V
		 if(genterflag==1)
				{

					SetBatteryData(1,0,&pdisDisPlayData[0]);
					gsetbatt.Set_BattV = (*(u16 *)&pdisDisPlayData[0]);
		     
					 pgh52c0->savePara(&gsetbatt.Set_BattV );
					genterflag=0;
					
					setBattV = gsetbatt.Set_BattV;
					
				}
	 }
	 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[2])
	 {
		 CheckParaBoundary((u16 *) &pdisDisPlayData[2],0,12000);//锂电充电电流  0-120A
		 if(genterflag==1)
				{

					SetBatteryData(2,0,&pdisDisPlayData[2]);
					gsetbatt.Set_Batt_ChargA = (*(u16 *)&pdisDisPlayData[2]);
					
					 pgh52c0->savePara(&gsetbatt.Set_Batt_ChargA );
					genterflag=0;
					
				}
	 }

	 else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[4])
	 {
		//MOS充电状态控制
		 
		 
		 if(genterflag==1)
		{
			
			
			if(pdisDisPlayData[4]==0)
			{
				pdisDisPlayData[4]=MOS_OFF;
			}
			else if(pdisDisPlayData[4]==1)
			{
				pdisDisPlayData[4]=MOS_ON;
			}

			  SetBatteryData(3,0,&pdisDisPlayData[4]);
			  gsetbatt.MOS_Charg = (*(u16 *)&pdisDisPlayData[4]);
			  
			   pgh52c0->savePara(&gsetbatt.MOS_Charg );
				genterflag=0;
			
		}
	 }
	  else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[6])
	 {
		 //MOS放电状态控制
		if(genterflag==1)
		{
			
			if(pdisDisPlayData[6]==0)
			{
				pdisDisPlayData[6]=0x55;
			}
			else if(pdisDisPlayData[6]==1)
			{
				pdisDisPlayData[6]=0x01;
			}

			SetBatteryData(4,0,&pdisDisPlayData[6]);
			gsetbatt.MOS_DisCharg = (*(u16 *)&pdisDisPlayData[6]);
			
			pgh52c0->savePara(&gsetbatt.MOS_DisCharg );
			genterflag=0;
			
		}
	 }
	  else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_pParalist==&pdisDisPlayData[8])
		{
			 if(genterflag==1)
				{
						if(pdisDisPlayData[8] == 0)
						{
								u16 tmp = 0x0101;//设为恒压模式
								SetBatteryData(5,0,(u8 *)&tmp);
								gsetbatt.WORK_MODE = tmp;
								pgh52c0->savePara(&gsetbatt.WORK_MODE );
						}
						else if(pdisDisPlayData[8] == 1)
						{
								u16 tmp = 0x0303;//设为电池特性模式
								SetBatteryData(5,0,(u8 *)&tmp);
								gsetbatt.WORK_MODE = tmp;
								pgh52c0->savePara(&gsetbatt.WORK_MODE );
						}
						else if((pdisDisPlayData[8] == 2)||(pdisDisPlayData[8] == 3))
						{
								u16 tmp = 0x0707;//设为自管理特性模式
								SetBatteryData(5,0,(u8 *)&tmp);
								gsetbatt.WORK_MODE = tmp;
								pgh52c0->savePara(&gsetbatt.WORK_MODE );
						}
						
						genterflag=0;
												
				}
		}

		
		if(pCurrentScreen->m_EnterStatus!=3)//处于非编辑状态时
		 {
				genterflag=0;

										 
				(*(u16 *)&pdisDisPlayData[0])=batt[battID[0]-1].Set_BattV;
				(*(u16 *)&pdisDisPlayData[2])=batt[battID[0]-1].Set_Batt_ChargA;
				(*(u16 *)&pdisDisPlayData[4])=batt[battID[0]-1].MOS_Charg;
				(*(u16 *)&pdisDisPlayData[6])=batt[battID[0]-1].MOS_DisCharg;
				pdisDisPlayData[8] = batt[battID[0]-1].WorkMode-1;
				(*(u16 *)&pdisDisPlayData[10])=Volcut;
				(*(u16 *)&pdisDisPlayData[12])=recoverVol;	 
			 
	 
				if(gsetbatt.WORK_MODE == 0x0101)
				{
					pdisDisPlayData[9] = 0;
				}
			  else  if(gsetbatt.WORK_MODE == 0x0303)
			  {
					 pdisDisPlayData[9] = 1;
			  }
			  else  if(gsetbatt.WORK_MODE == 0x0707)
			  {
					 pdisDisPlayData[9] = 2;
			  }
		 }
}

void SetAirData(Screen *pCurrentScreen) 
{
		if(airOrder[0]>8||airOrder[0]==0)
		{
			airOrder[0] = 1;
				
		}
		airAddr = airOrder[0]+9;
		airIdx = airOrder[0] - 1;

		pCurrentAir = &g_astAIRpara[airIdx];
		

		
		if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==airOrder)
		{
			if(genterflag == 1) 
			{
				genterflag=0;
			}
		}
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[8]) 
		{
			
			CheckParaBoundary((u16 *)&pdisDisPlayData[8],200,500);
			if(genterflag == 1) 
			{
        
				g_AirWriteFlag = 1;
				g_AirInterfaceData[airIdx].g_AirColdData = (*(s16 *)&pdisDisPlayData[8]);
				pgh52c0->savePara(&g_AirInterfaceData[airIdx].g_AirColdData);//掉电保存
				genterflag = 0;
				
			}
		}

		else if ((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[10]) 
		{
			
			 CheckParaBoundary((u16 *)&pdisDisPlayData[10],200,800);
			if(genterflag == 1) 
			{
       
				g_AirWriteFlag =2;
				g_AirInterfaceData[airIdx].g_AirTempData	 = (*(s16 *)&pdisDisPlayData[10]);
				pgh52c0->savePara(&g_AirInterfaceData[airIdx].g_AirTempData);
				genterflag = 0; 
			}
		}
		
		if(pCurrentScreen->m_EnterStatus!=3)//处于非编辑状态时
		{
				pdisDisPlayData[0] = pCurrentAir->ast_workStatus;
				pdisDisPlayData[1] = pCurrentAir->ast_compressorstatus;
				
				(*(s16 *)&pdisDisPlayData[2]) = pCurrentAir->ast_cabinetTemp;// 柜内回风温度
				(*(u16 *)&pdisDisPlayData[4]) = pCurrentAir->ast_infanSpeed;// 内风机转速
				(*(u16 *)&pdisDisPlayData[6]) = pCurrentAir->ast_exfanSpeed;//外风机转速
				(*(s16 *)&pdisDisPlayData[17]) = pCurrentAir->ast_humi;//湿度
				(*(u16 *)&pdisDisPlayData[8]) = pCurrentAir->ast_airColdData;//g_AirInterfaceData[airIdx].g_AirColdData;//设置制冷点
				(*(u16 *)&pdisDisPlayData[10]) = pCurrentAir->ast_airTempData;//g_AirInterfaceData[airIdx].g_AirTempData;//设置高温告警点
			  (*(u16 *)&pdisDisPlayData[13]) = mt11Humi;
	     	(*(u16 *)&pdisDisPlayData[15]) = mt11Tmp;
				
		}
		

}

void GetBattData(Screen *pCurrentScreen) 
{
		if(pdisDisPlayData[0]>4||pdisDisPlayData[0]==1)
		{
			pdisDisPlayData[0] = 1;
				
		}
		
		(*(s16 *)&pdisDisPlayData[2])=*((s16 *)&gBattPara[pdisDisPlayData[0]-1].battVol);
		(*(s16 *)&pdisDisPlayData[4])=*((s16 *)&gBattPara[pdisDisPlayData[0]-1].battCurr1);
		*(u32 *)&pdisDisPlayData[6]	=	*((u32 *)&gBattPara[pdisDisPlayData[0]-1].battEnergy);
}


void SetBattCut(Screen *pCurrentScreen)
{
		if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[0])
		{
			 CheckParaBoundary((u16 *) &pdisDisPlayData[10],4200,5500);//备电切换阈值
				if(genterflag==1)
				{
					Volcut=(*(u16 *)&pdisDisPlayData[0]);
					
					pgh52c0->savePara(&Volcut);
					genterflag=0;
					
				}
		}
		
		else if((pCurrentScreen->m_pItem[pCurrentScreen->m_head+pCurrentScreen->m_coursItem]).st_plink==&pdisDisPlayData[2])
		{
			 CheckParaBoundary((u16 *) &pdisDisPlayData[2],4200,5500);//市电恢复电压
				if(genterflag==1)
				{
					recoverVol=(*(u16 *)&pdisDisPlayData[2]);
					
				 if(recoverVol<Volcut)
				 {
						recoverVol=Volcut;
				 }
				 pgh52c0->savePara(&recoverVol );
					genterflag=0;
				}
			
		}
		
				if(pCurrentScreen->m_EnterStatus!=3)//处于非编辑状态时
		 {
				genterflag=0;
				(*(u16 *)&pdisDisPlayData[0])=Volcut;
				(*(u16 *)&pdisDisPlayData[2])=recoverVol;	 
		 }
		
		
}

	



