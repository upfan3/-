#include "powercan.h"


extern u8 prACRunStatus;
extern u8 getEnergyCount;

extern u8 setPoint;
extern s16 setValu;
//extern u8 battBroadOnlineCount;

extern u8 battAddr[2];


SwitchPara gSwitchPara[TOTAL_USER+8];//最后8个空间，第0号，总参数，第7号，为电池，第1~6,分别对应6个用户
u8  SwitchOnlineCount[TOTAL_USER];
u8 gSetDownParaCode=0;
u8 gSetDownParaGroudType=0x51;
u8 gSetDownParaAddr=0;
DateTimeF33 SetTime;
u16 totalUserI=0;

u8 brunchAddr=0;
u8 downAct = 0;


u8 gDcmStatus[10]={1,1,1,1,1,1,1,1,1,1};
u8 gparaFrameType[4]={0,0,0,0};

u8 recvCompleteFlag=0;
u8 gdownbroadinfo[4];//铅酸板版本信息



	u8  DTC;
  u8  CID1;
  u8  ADR;
	u16 CID2;
	u16 c_Uac[3];


u8 gAcmOnLineCount=0;
u8 gAcOnLineFlag=0;
u8 gbattGroud=0;
u8 errCount=0;

u32 energyDiff=0;//电能间隔获取差值
u32 preEnergy=0;//前一次获取电能值
u32 totalEnergy[6]={0,0,0,0,0,0};// 电能差值累计
u32 totaldisplay=0;
u32 powerRate[6]={0,0,0,0,0};//不同时段的电费

Battpara gBattPara[4]= {0};

u8 CompareWith(u8* a,u8 *b,u8 len)
{  u8 result=1;
	    for(int i=0;i<len;i++)
	   {
			   if(a[i]!=b[i])
				 {
					 result=0;
					 break;
				 }
	   }
		 return result;
	
}


u8 GetTimeSec(DateTimeF33 *pTime,u32 * pseccount)
{
	 u16 t;
   u16 syear;
	 u8 smon, sday, hour,min,sec;
	
	syear=pTime->st_year;
	smon=pTime->st_mon;
	sday=pTime->st_day;
	hour=pTime->st_hour;
	min=pTime->st_min;
	sec=pTime->st_sec;
	
       *pseccount=0;

       if(syear<2000||syear>2099)return 1;//syear范围1970-2099，此处设置范围为2000-2099        

       for(t=2000;t<syear;t++) //把所有年份的秒钟相加

       {

              if(Is_Leap_Year(t)) *pseccount+=31622400;//闰年的秒钟数

              else *pseccount+=31536000;                    //平年的秒钟数

       }

       smon-=1;

       for(t=0;t<smon;t++)         //把前面月份的秒钟数相加

       {

              *pseccount+=(u32)mon_table[t]*86400;//月份秒钟数相加

              if(Is_Leap_Year(syear)&&t==1)*pseccount+=86400;//闰年2月份增加一天的秒钟数         

       }

       *pseccount+=(u32)(sday-1)*86400;//把前面日期的秒钟数相加 

       *pseccount+=(u32)hour*3600;//小时秒钟数

    *pseccount+=(u32)min*60;      //分钟秒钟数

       *pseccount+=sec;//最后的秒钟加上去

    
       return 0; 
	
	
}

/*****************************************************
pTime1与pTime2两个时间比较
若果其中有一个日期不在2000~2099，则返回-1
pTime1大于pTime2 返回1
pTime1小于pTime2 返回0
pTime1等于pTime2 返回2
*****************************************************/

s8 CompareTime(DateTimeF33 *pTime1,DateTimeF33 *pTime2)
{ u32 sec1,sec2;
	
	   if(GetTimeSec(pTime1,&sec1)!=0) return -1;
	   if(GetTimeSec(pTime2,&sec2)!=0) return -1;
	    
	    if(sec1>sec2) 
			{
				return 1;
			}
			else if(sec1<sec2)
			{
				 return 0;
			}
			else if(sec1==sec2)
			{
				return 2;
			}
	   
	  return -1;
}








u16 recvCount=0;
u8 gUseToBattBranch[6]={0xff,0xff,0xff,0xff,0xff,0xff};

DateTimeF33 TmpTime;
u32 preDCEnergydata[6]={0,0,0,0,0,0};
u32  cuDCEnergydata[6]={0,0,0,0,0,0};
u32  acDCEnergydata[6]={0,0,0,0,0,0};
extern u8 * pgetE2Databuff;

void SaveDownUser()
{
	
	
		for(u8 userid=1;userid<8;userid++)//总用户参数设置，同步用户参数
		{
			
			for(u8 i=0;i<TOTAL_USER;i++)
			{
				if((((gSwitchPara[i].st_userGroud&0xf0)>>4)==userid))
				{
					gSwitchPara[TOTAL_USER+userid]=gSwitchPara[i];
					break;
				}
				
			}
			
		
	  }
		
		
//		for(u8 i=0;i<34;i++)//同步电池分路信息
//		{
//		}
	
	
}

void GetEnergy(void)
{
	      u32          watt[4]={0};
	      u32     totalWatt[4]={0};
        u32     totalWattSec[4]={0};
static	u16 remainWatt[4]={0};
static	u16 wattSec[4]={0};
static	u16 remainWattSec[4]={0};

		for(u8 i=0;i<4;i++)
		{
				watt[i] = gBattPara[i].battVol*gBattPara[i].battCurr1;
			
				totalWatt[i]= watt[i]+remainWatt[i];
				
				 wattSec[i] = totalWatt[i]/1000;
			
				 remainWatt[i]=totalWatt[i]%1000;
			 
				
				totalWattSec[i] = wattSec[i]+ remainWattSec[i];
			
				gBattPara[i].battEnergy+=totalWattSec[i]/3600;
			
				remainWattSec[i]=totalWattSec[i]%3600;
			
			
		   bkp_data_write((bkp_data_register_enum)(2*i+1),*((u16 *)&gBattPara[i].battEnergy));//本地支路电流1
       bkp_data_write((bkp_data_register_enum)(2*i+2),*((u16 *)&gBattPara[i].battEnergy+1));
			
			
			
			
		}
		
}






void SetParaByCmd(u8 cmd,CanBuf *prbuf,u8 *pdat)
{
	   	u8 tmpCmd[][2]={{0,0},{23,1},{0,4},{4,2},{6,2},{8,2},{10,2},{12,2},{14,2},{16,2},{18,2},{20,2},{22,1}};//参数命令码所对应的偏移量及长度
						     //命令码      1      2     3     4     5      6      7      8      9     10     11     12
				         //参数     用户号 截止日期 电量  电压  时间  恢复电压 定时始 定时终 免责始 免责终  过载  下电模式
				   if(cmd==0) return;//无效命令，直接返回，不操作
				  prbuf->data[0]=cmd;
					for(u8 i=0;i<tmpCmd[cmd][1];i++)		//根据数据长度设置 1单字节 2双字节 4四字节
					prbuf->data[2+i]= *(&pdat[tmpCmd[cmd][0]]+i);		
}


void PowrCan:: DuleWiteData(CanBuf *prbuf)
{
	PCid *pPcid;
	u16 *pDCddata=NULL;
	u8 *pDC8data=NULL;
	
	     pPcid=(PCid *)(&prbuf->id);
	       DTC=pPcid->st_DTC;
				 CID1=pPcid->st_CID1;
				 CID2=pPcid->st_CID2;
	  
	         if(pPcid->st_ADR==0) return ;//0地址不合法
				 ADR=pPcid->st_ADR-1;
	
	
	       if(CID1==UPDATA){   //升级
					   ;//
				 }
				 else if(CID1==DEVICE_MESSAGE){//设备信息
					       	if(DTC==BATTERY_DOWN){//电池下电板
										   for(u8 i=0;i<4;i++)
										    gdownbroadinfo[i]=prbuf->data[i]; 
										}
					 
					    ;//
				 }
	       else if(CID1==REMOTE_CTRL){//遥控
					    if(DTC==MAIN_CTRL){//主控
							 ;//
						 }
							else if(DTC==DCM){//直流配电
								
							}
					 
					 
			
				  }
				 else if(CID1==DEVICE_PARA){//设备参数
					     if(DTC==MAIN_CTRL){}//主控						 
					else if(DTC==DCM){
									 
					}
			 
					else if(DTC==JGSW){//同步协议板用户参数到监控
						/*
						 if((CID2&0xfff)==0x080)
						 {
							    memcpy(&gSwitchPara[ADR],prbuf->data,8);
							    //ua++;
						 }
						 else if((CID2&0xfff)==0x081)
						 {
							    memcpy(&gSwitchPara[ADR].st_downT,prbuf->data,8);
							    //ub++;
						 }
						 else if((CID2&0xfff)==0x082)
						 {
							   memcpy(&gSwitchPara[ADR].startTime1,prbuf->data,8);
							   if(gbattGroud<6)
								 {
									   if(gSwitchPara[ADR].st_userGroud==0x7B)//电池
										 {
											 u8 isOnly=1;
											    for(u8 i=0;i<6;i++)//检查电路分路对应下电分路的唯一性
												 {
													 if(gUseToBattBranch[i]==ADR)//下电分路在电池分路中已存在
														 isOnly=0;//不唯一
												 }
											  if(isOnly==1)
												{
													
											    gUseToBattBranch[gbattGroud]=ADR;//设置电池分路
											    pgBattBranch[gbattGroud]=&gDCdistribution.pst_I[ADR];//绑定对应电流分路地址
											     gbattGroud++;
												}
										 }
								 }
							 
						 }
						
						 recvCount++;
						 if(recvCount==450)//当收到同一参数大于4次时，统计用户参数
						 {
							 SaveDownUser();
							 recvCount=0;
						 }
						*/ 
					}
          else if(DTC==BATTERY_DOWN){//获取下电板参数   
						//  ADR的值为0-3，prbuf->data[0]值为1-2  ADR*2+prbuf->data[0]+1 映射到2-9地址
//							gDcParam0[ADR*2+prbuf->data[0]+1].st_downModle=prbuf->data[1]; //下电模式
//						  gDcParam0[ADR*2+prbuf->data[0]+1].st_LDVoltage=(*(u16 *)(&prbuf->data[2]));//下电电压
//						  gDcParam1[ADR*2+prbuf->data[0]+1].st_LDDelay=(*(u16 *)(&prbuf->data[4]))*10; //下电时间
					}						

				 }
	        else if(CID1==BASIC_DATA){//基本数据
					    if(DTC==MAIN_CTRL){//主控
						 }
						  else if(DTC==ACM){//交流配电单元


										 if((CID2&0xfff)==0x100){
											   gAcmOnLineCount=0;
											   gAcOnLineFlag=1;
											    int i=prbuf->data[0]&0x3f;
											    if(i==0)//a
													{
														 pgACmointor->st_Ia=*(u16 *)&prbuf->data[2];
														 pgACmointor->st_Ia=(float) pgACmointor->st_Ia*0.975;
														 pgACmointor->st_Pa=(*(u16 *)&prbuf->data[4]);
														 pgACmointor->st_Pa=(float) pgACmointor->st_Pa*0.975;
													}
													else if(i==1)//b
													{
														 pgACmointor->st_Ib=*(u16 *)&prbuf->data[2];
														 pgACmointor->st_Ib=(float) pgACmointor->st_Ib*0.975;
														 pgACmointor->st_Pb=(*(u16 *)&prbuf->data[4]);
														 pgACmointor->st_Pb=(float) pgACmointor->st_Pb*0.975;
													}
													else if(i==2)//c
													{
														 pgACmointor->st_Ic=*(u16 *)&prbuf->data[2];
														 pgACmointor->st_Ic=(float) pgACmointor->st_Ic*0.975;
														 pgACmointor->st_Pc=(*(u16 *)&prbuf->data[4]);
														 pgACmointor->st_Pc=(float) pgACmointor->st_Pc*0.975;
													}
										}
											else if((CID2&0xfff)==0x110){
												  gAcmOnLineCount=0;
												   gAcOnLineFlag=1;
											    int i=prbuf->data[0]&0x3f;
												
//												if(((prbuf->data[1])&0x20)!=0)
//												{
//													gSPD=0x20;
//												}
//												else
//												{
//													gSPD=0;
//												}
												
													if(i==0)//a
													{
														 pgACmointor->st_Ua=*(u16 *)&prbuf->data[2];
														 if( pgACmointor->st_Ua>8000)
														 pgACmointor->st_Frq=*(u16 *)&prbuf->data[4];
													}
													else if(i==1)//b
													{
														 pgACmointor->st_Ub=*(u16 *)&prbuf->data[2];
														 if( pgACmointor->st_Ub>8000)
														 pgACmointor->st_Frq=*(u16 *)&prbuf->data[4];
													}
													else if(i==2)//c
													{
														 pgACmointor->st_Uc=*(u16 *)&prbuf->data[2];
														 if( pgACmointor->st_Uc>8000)
														 pgACmointor->st_Frq=*(u16 *)&prbuf->data[4];
													}
											
//													if( (pgACmointor->st_Ua<80)&&(pgACmointor->st_Ub)<80&&(pgACmointor->st_Uc<80))
//													{
//														 pgACmointor->st_Frq=0;
//													}
										}
											
										
							 ;//
						 }
							else if(DTC==DCM){//直流配电单元
								}
							else if(DTC==MRM){//整流模块
							 ;//
									}
							else if(DTC==WR){//风机模块
							 ;//
								}
							else if(DTC==SC){//光伏模块
							 ;//
								}
							
							else if(DTC==JGSW){//智能开关
								  if((CID2&0xff0)==0x100)
									{
										    pDCddata=gDCdistribution.pst_I;
									      pDC8data=gDCdistribution.pst_status;					
										    SwitchOnlineCount[ADR]=DELAY_COUNT;
										
										
										
										
										    u32 *pDC32data=gDCdistribution.pst_enerqy;
										
										pDC8data[ADR]=0;//开关状态
										pDCddata[ADR]=0;//开关电流
										if(*(u32*)&gTimerNow<=*(u32*)&gSwitchPara[ADR].st_stopTime)//开关授权后才接收数据,以防止地直误报
										{
											
														//开关数据
														pDC8data[ADR]=prbuf->data[1];//开关状态
														pDCddata[ADR]=*(u16 *)&prbuf->data[2];//开关电流
														pDC32data[ADR]=*(u32 *)&prbuf->data[4];//开关电量
													//	pDC32data[ADR]/=10;

											
										}
										
										
										   
										
										
										//开关参数
										    pDC8data=gDCdistribution.pst_user;
										    pDC8data[ADR]= ((pDC8data[ADR]&0xf0)|(0x000f&CID2));//用户类型
										    pDC8data=gDCdistribution.pst_switchtype;
										
										    pDC8data[ADR]=1;//单向
										    if((prbuf->data[0]&0x40)!=0)
												{
													pDC8data[ADR]=2;//双向
												}
										    
										
										
										
									}
							}
						  else if(DTC==BATTERY_DOWN){//电池下电板
																		
											gBattPara[ADR].battBroadOnlineCount = 30;
								
											*((s16 *)&gBattPara[ADR].battVol) =*(s16 *)&prbuf->data[0];
								
								       if(*(s16 *)&prbuf->data[2]>0){
											 *((s16 *)&gBattPara[ADR].battCurr1) =*(s16 *)&prbuf->data[2];}
											 else
											 {
												  *((s16 *)&gBattPara[ADR].battCurr1) = 0;
											 }
											 if(*(s16 *)&prbuf->data[4]>0){
										  	*((s16 *)&gBattPara[ADR].battCurr2) =*(s16 *)&prbuf->data[4];}
											 else
											 {
												 *((s16 *)&gBattPara[ADR].battCurr2) = 0;
											 }
											gBattPara[ADR].battBranch1 = (prbuf->data[6]&0x02)>>1;
											gBattPara[ADR].battBranch2 = prbuf->data[6]&0x01;
											gBattPara[ADR].battFlash = (prbuf->data[6]>>7)&0x01;	
								
								      gBattPara[ADR].battBreak1 = prbuf->data[7]&0x01;
								      gBattPara[ADR].battBreak2 = (prbuf->data[7]&0x02)>>1;
								   
						
							}
					}
						 
	
	        else if(CID1==STATISTICS_DATA){//统计参数
						       if(DTC==DCM){//直流配电模块
										 
					
									 }
									else if(DTC==ACM){//交流瓶
										   u8 i=prbuf->data[0]&0x3f;
									     if(i==0)
											 { 
												  recvCompleteFlag|=0x01;
												   memcpy((void *)&pgACmointor->st_energy1,&prbuf->data[1],4);
											 }
											 else if(i==1)
											 {
												    recvCompleteFlag|=0x02;
												   memcpy((void *)&pgACmointor->st_energy2,&prbuf->data[1],4);
											 }
											 else if(i==2)
											 {
												   recvCompleteFlag|=0x04;
												   memcpy((void *)&pgACmointor->st_energy3,&prbuf->data[1],4);
											 }
										
											 
											
											 
											if(recvCompleteFlag==0x07)//收齐ABC三帧电能后，才进行后继处理
											{ 
												recvCompleteFlag=0;//清除电能接收标志，以便以重新统计
												pgACmointor->st_totalenergy=pgACmointor->st_energy1+pgACmointor->st_energy2+pgACmointor->st_energy3;
											
													 /*											
															 UpDataACEnergy();//错峰电能及费用统计
													 */
										 }
											   
											 
											 
									}										

										 
									 
					    ;//
				 }
	        else if(CID1==DEBUG_DATA){//调试数据
					    ;//
				 }
	
}
extern u32 gtimeCount1s;
void PowrCan::OnMinTick(void)
{
	         if(gtimeCount1s>=60)
						 {
							   gtimeCount1s=0;
							   AccUserEnergy();//电流统计
							   UpDataACEnergy();
						 }
}







//计算不同错峰时段所用的电能及电费,每分钟执行一次
void PowrCan::UpDataACEnergy(void)
{
	         if(preEnergy>0)// 第一次获取得的电能不进行累计电价操作,因为第一次从电能表取得的值可能为非0值，且值较大
					 {
								if(pgACmointor->st_totalenergy>=preEnergy)
										 energyDiff=pgACmointor->st_totalenergy-preEnergy;//正常情况 （只有新值大于旧值，才处理 ） 
										else
										energyDiff=0xffffffff-preEnergy+pgACmointor->st_totalenergy;//处理电能溢出情况
							 
										if(energyDiff>7)//energyDiff=7;//差值超0.7度时，按0.7度累加
										{
											errCount++;
											if(errCount>5)
											{
												 errCount=0;
												preEnergy=0;
											}
											return ;
										}

										powerRate[prACRunStatus]+=energyDiff*eleRate[prACRunStatus];//错锋时段电价
										
										 pgh52c0->SaveACDataToBuckUpReg(prACRunStatus,powerRate);// 将累计电费写入备份寄存器
										
										totalEnergy[prACRunStatus]+=energyDiff;//各时段用电量
										totaldisplay=(float)totalEnergy[0]*1.01;//显示值
										
										 pgh52c0->SaveACDataToBuckUpReg(prACRunStatus,totalEnergy);// 将累计电量写入备份寄存器
								
										if(prACRunStatus>0)
										{
											 powerRate[5]+=energyDiff*eleRate[prACRunStatus];//错锋时段电价
											 totalEnergy[5]+=energyDiff;//峰谷平总用电量
										}
									 prACRunStatus=ACRunMode;
					 }
					 else
					 {
									prACRunStatus=ACRunMode;
					 }
						 getEnergyCount=GET_ENERGY_DELAY;//每收到一次电能数据，重置一次电能获取计数
						 preEnergy=pgACmointor->st_totalenergy;
}



void PowrCan::UpDataUserEnergy(void)
{
			
										 memset( gDCdistribution.pst_userI,0,16);//先重置
										
										 memset( gDCdistribution.pst_userStatus,0,8);//先重置
											
                     memset( gDCdistribution.pst_userEnerqy,0,32);//先重置
											
				
										
											   for(u8 i=0;i<TOTAL_USER;i++)
												{
													
											    
												   
													 u8 tmpID= gSwitchPara[i].st_userGroud>>4; 
													
													if(tmpID<8) {
													 
												   gDCdistribution.pst_userI[tmpID]+=  gDCdistribution.pst_I[i];
													 
													 gDCdistribution.pst_userEnerqy[tmpID]+=  gDCdistribution.pst_enerqy[i];
													 
													   if((gDCdistribution.pst_status[i]&0x07)!=0)
														 {
															    gDCdistribution.pst_userStatus[tmpID]|=0x01;//设置 下电位

														 }
														 
													  if((gDCdistribution.pst_status[i]&0x40)==0x40)
														 {
															    gDCdistribution.pst_userStatus[tmpID]|=0x40;//设置 熔丝位
														 }	
                           
													 }

														 
														 
												
												}
												
												totalUserI=0;
												for(u8 i=0;i<6;i++)
												{
													cuDCEnergydata[i]=gDCdistribution.pst_userEnerqy[i+1];
													
													totalUserI=totalUserI+gDCdistribution.pst_userI[i+1];//所有用户分路总电流
												}
												
										
							


}



void PowrCan::AccUserEnergy(void)
{   s8 tmp32;
	
	  for(u8 i=0;i<6;i++)
	{
		  if(preDCEnergydata[i]!=0)
			{
				     tmp32=cuDCEnergydata[i]-preDCEnergydata[i];
				
				
				    if((tmp32>0)&&(tmp32<=60))//系统电能1分钟内，不大于0.6度电
						{
							  acDCEnergydata[i]+=tmp32;
							  preDCEnergydata[i]=cuDCEnergydata[i];
							
							
							
//								 bkp_data_write((bkp_data_register_enum)(2*i+1),*((u16 *)&acDCEnergydata[i]));//本地支路电流1
//								 bkp_data_write((bkp_data_register_enum)(2*i+2),*((u16 *)&acDCEnergydata[i]+1));
							
							
						}
						else
						{
							   preDCEnergydata[i]=cuDCEnergydata[i];
						}
				
				
				
			}
			else
			{
				
				  preDCEnergydata[i]=cuDCEnergydata[i];
				
			}
		
		
		
		
	}
	
	
	
	
	
	
}












extern u8 sendACflag;

u8 powerCanSendEnable=0;

u8 gtimersyc=0;
u8 gsysParaflag=0;
u8 setParaAdress=1;



#ifdef test_energy	//跑电能测试，以0.3度每分钟跑	

u32 testenerggy=0xfffffff0;
u8  PowrCan::CanGetEnergy()
{
	recvCompleteFlag=0;//清除电能接收标志，以便以重新统计
												//pgACmointor->st_totalenergy=pgACmointor->st_energy1+pgACmointor->st_energy2+pgACmointor->st_energy3;
	                           testenerggy+=3;
												   pgACmointor->st_totalenergy=testenerggy;//每分钟加0.3度
											 if(preEnergy>0)// 第一次获取得的电能不进行累计电价操作,因为第一次从电能表取得的值可能为非0值，且值较大
											 {
												    if(pgACmointor->st_totalenergy>=preEnergy)
												         energyDiff=pgACmointor->st_totalenergy-preEnergy;//正常情况 （只有新值大于旧值，才处理 ） 
                               else
																energyDiff=0xffffffff-preEnergy+pgACmointor->st_totalenergy;//处理电能溢出情况
												   
																if(energyDiff>7)energyDiff=7;//差值超0.7度时，按0.7度累加
												   

												        powerRate[prACRunStatus]+=energyDiff*eleRate[prACRunStatus];//错锋时段电价
																
																 pgh52c0->SaveACDataToBuckUpReg(prACRunStatus,powerRate);// 将累计电费写入备份寄存器
																
												        totalEnergy[prACRunStatus]+=energyDiff;//各时段用电量
																
																 pgh52c0->SaveACDataToBuckUpReg(prACRunStatus,totalEnergy);// 将累计电量写入备份寄存器
												    
																if(prACRunStatus>0)
																{
																	 powerRate[5]+=energyDiff*eleRate[prACRunStatus];//错锋时段电价
																	 totalEnergy[5]+=energyDiff;//峰谷平总用电量
																}
												       prACRunStatus=ACRunMode;
											 }
											 else
											 {
												      prACRunStatus=ACRunMode;
											 }
											   getEnergyCount=GET_ENERGY_DELAY;//每收到一次电能数据，重置一次电能获取计数
											   preEnergy=pgACmointor->st_totalenergy;
}
#endif





u8 PowrCan:: CanPowerOnTick(CanBuf *prbuf,void *pScreen)
{static u8 sendstatus=0;
	   Screen * pCurrentScreen=(Screen *)pScreen;
      PCid *pPcid;
	
	     pPcid=(PCid *)(&prbuf->id);
	       DTC=pPcid->st_DTC;
	    
	  if(pPcid->st_ADR==0)return 0;//收到地址无效直接返口
			 
	       ADR=pPcid->st_ADR-1;
	
	
 
			         if(gtimersyc==1)
							 {
			            pPcid->st_DTC=MAIN_CTRL;
									 pPcid->st_CID1=REMOTE_CTRL;
									 pPcid->st_CID2=0x080;
									//pPcid->st_ADR=ADR+1;
									 pPcid->st_ADR=0;
									 prbuf->rtr=0;//数据帧
								   prbuf->dlc=8;//长度

									 SetTime.st_year=gTimer.year;
									 SetTime.st_mon=gTimer.mon;
									 SetTime.st_day=gTimer.day;
									 SetTime.st_hour=gTimer.hour;
									 SetTime.st_min=gTimer.min;
									 SetTime.st_sec=gTimer.sec;

									 memcpy(prbuf->data,&SetTime,5);
									 powerCanSendEnable=1;
								  gtimersyc=0;
								 // gbattGroud=0;//同步参数时，将电池分路数量重置
								  recvCount=0;//接收计重置
							 }
							 else if(gSetDownParaCode>0)
							 {
								 
								  pPcid->st_DTC=JGSW;//知能开关设备
									 pPcid->st_CID1=DEVICE_PARA;//参数
									 pPcid->st_CID2=gSetDownParaGroudType;//0x51：设置单个工作参数，0x52：设置工作组参数
									 pPcid->st_ADR=gSetDownParaAddr;//开关地址
									 prbuf->rtr=0;//数据帧
								   prbuf->dlc=8;//长度
								 
								 memset(prbuf->data,0,8);
								 if(gSetDownParaGroudType==0x51)
								 SetParaByCmd(gSetDownParaCode,prbuf,(u8 *)&gSwitchPara[gSetDownParaAddr-1]);
								 else if(gSetDownParaGroudType==0x52)
								 {
									 if(gSetDownParaAddr==0xf) gSetDownParaAddr=0;//广播所有用户
									 SetParaByCmd(gSetDownParaCode,prbuf,(u8 *)&gSwitchPara[gSetDownParaAddr+TOTAL_USER]); 
								 }
								 gSetDownParaCode=0;
								 powerCanSendEnable=1;
							 }
							 else if(gsysParaflag>0)
							 {
								 
								 
											pPcid->st_DTC=JGSW;//知能开关设备
											 pPcid->st_CID1=DEVICE_PARA;//参数
											
											 pPcid->st_ADR=gsysParaflag;//开关地址
											 prbuf->rtr=0;//数据帧
											 prbuf->dlc=8;//长度
										 
										 if(sendstatus==0)
										 {  
												pPcid->st_CID2=0x080;
												memcpy(prbuf->data,(u8 *)&gSwitchPara[gsysParaflag-1].st_stopTime,8);
											 sendstatus=1;
											  powerCanSendEnable=1;
											 return 1;
										 }
										 else if(sendstatus==1)
										 {
												 pPcid->st_CID2=0x081;
												memcpy(prbuf->data,(u8 *)&gSwitchPara[gsysParaflag-1].st_downT,8);
											 sendstatus=2;
											  powerCanSendEnable=1;
											 return 1;
										 }
										 else if(sendstatus==2)
										 {
												 pPcid->st_CID2=0x082;
												memcpy(prbuf->data,(u8 *)&gSwitchPara[gsysParaflag-1].startTime1,8);
											 sendstatus=0;
											  powerCanSendEnable=1;
											 gsysParaflag++;
											 if(gsysParaflag>TOTAL_USER)
												 gsysParaflag=0;
											 return 0;
										 }
										 

							 }
							 else if(sendACflag==1)
							 {
								 
								   	   pPcid->st_DTC=ACM;//知能开关设备
											 pPcid->st_CID1=BASIC_DATA;//参数
											 pPcid->st_CID2=0x110;//参数
											 pPcid->st_ADR=0;//开关地址
											 prbuf->rtr=0;//数据帧
											 prbuf->dlc=8;//长度
												 for(u8 i=0;i<8;i++)
											 {
													 prbuf->data[i]=0;
											 }
											 
											   prbuf->data[0]=0;
											 
											    if(gAcFlag==1)
													{
														  *((u16 *)&prbuf->data[2])= 23000;
													}
													else
													{
											      *((u16 *)&prbuf->data[2])= pgACmointor->st_Ua;
													}
											 
											sendACflag=2;
											  powerCanSendEnable=1;
											 return 1;
											 
							 }
							 else if(sendACflag==2)
							 {
								 
								   	   pPcid->st_DTC=ACM;//知能开关设备
											 pPcid->st_CID1=BASIC_DATA;//参数
											 pPcid->st_CID2=0x110;//参数
											 pPcid->st_ADR=0;//开关地址
											 prbuf->rtr=0;//数据帧
											 prbuf->dlc=8;//长度
												 for(u8 i=0;i<8;i++)
											 {
													 prbuf->data[i]=0;
											 }
											 
											   prbuf->data[0]=1;
											 
											 
											     if(gAcFlag==1)
													{
														  *((u16 *)&prbuf->data[2])= 23000;
													}
													else
													{
											      *((u16 *)&prbuf->data[2])= pgACmointor->st_Ub;
													}
											 
											sendACflag=3;
											 
											  powerCanSendEnable=1;
											 return 1;
											 
							 }
							 else if(sendACflag==3)
							 {
								       	sendACflag=0;
								   	   pPcid->st_DTC=ACM;//知能开关设备
											 pPcid->st_CID1=BASIC_DATA;//参数
											 pPcid->st_CID2=0x110;//参数
											 pPcid->st_ADR=0;//开关地址
											 prbuf->rtr=0;//数据帧
											 prbuf->dlc=8;//长度
												 for(u8 i=0;i<8;i++)
											 {
													 prbuf->data[i]=0;
											 }
											 
											   prbuf->data[0]=2;
											 
											     if(gAcFlag==1)
													{
														  *((u16 *)&prbuf->data[2])= 23000;
													}
													else
													{
											     *((u16 *)&prbuf->data[2])= pgACmointor->st_Uc;
													}
											 
											  powerCanSendEnable=1;
											 return 0;
											 
							 }
 
	

			
			     

							 if(getEnergyFlag==1){ //查询交流信息 电量记录 每隔5分钟取一次，错锋状态改变取一次，电价参数改变取一次
							  getEnergyCount=GET_ENERGY_DELAY;//重置延时计数
				        getEnergyFlag=0;
						    pPcid->st_DTC=ACM;
								pPcid->st_CID1=TOTAL;//统计电量远程帧
								pPcid->st_CID2=0x100;
								pPcid->st_ADR=1;
				 
								prbuf->rtr=1;//远程帧
								memset(prbuf->data,0,8);
						  powerCanSendEnable=1;
						 
					 }


	         
							 
					     if(setBattParaflag==1)
							 {
									setBattParaflag=0;
									pPcid->st_DTC=BATTERY_DOWN;
										pPcid->st_CID1=DEVICE_PARA;
										pPcid->st_CID2=0;
										pPcid->st_ADR=battAddr[0];
										prbuf->rtr=0;//数据帧
										prbuf->dlc=8;//长度
								 
								 
								 prbuf->data[0]=3;
								 prbuf->data[1]=setPoint;
								 *(s16 *)&prbuf->data[2]=setValu;
								 prbuf->data[4]=0;
								 prbuf->data[5]=0;
								 prbuf->data[6]=0;
								 prbuf->data[7]=0;
								 powerCanSendEnable=1;
								 
							 }
							 else if(setBattParaflag==2)
							 {
									 setBattParaflag=0;
									pPcid->st_DTC=BATTERY_DOWN;
									pPcid->st_CID1=DEVICE_PARA;
									pPcid->st_CID2=0;
									pPcid->st_ADR=setPoint;//battAddr[0];
									prbuf->rtr=0;//数据帧
									prbuf->dlc=8;//长度
						 
						 
								 prbuf->data[0]=1;//brunchAddr;
								 
								// if(setPoint == 4) setPoint = 1; //后台设置
								 prbuf->data[1]= downAct;//gDcParam0[(setPoint-1)*2+brunchAddr+1].st_downModle;
								 *(u16 *)&prbuf->data[2]=0;//gDcParam0[(setPoint-1)*2+brunchAddr+1].st_LDVoltage;
								 *(u16 *)&prbuf->data[4]=0;//gDcParam1[(setPoint-1)*2+brunchAddr+1].st_LDDelay/10;
								 *(u16 *)&prbuf->data[6]=0;//gDcParam1[(setPoint-1)*2+brunchAddr+1].st_LDEnergy;
								
								 powerCanSendEnable=1;
					    }
							else if (setBattParaflag==3)//获取分路1
							{
								 setBattParaflag=0;
								 
								 pPcid->st_DTC=BATTERY_DOWN;
									pPcid->st_CID1=DEVICE_PARA;
									pPcid->st_CID2=0;
									pPcid->st_ADR=setPoint;
									prbuf->rtr=0;//数据帧
									prbuf->dlc=1;//长度
								  prbuf->data[0]=brunchAddr;
								  powerCanSendEnable=1;
								  return 1;
							}
//		          else if (setBattParaflag==4)//获取分路2
//							{
//								 setBattParaflag=0;
//								 
//								 pPcid->st_DTC=BATTERY_DOWN;
//									pPcid->st_CID1=DEVICE_PARA;
//									pPcid->st_CID2=0;
//									pPcid->st_ADR=battAddr[0];
//									prbuf->rtr=0;//数据帧
//									prbuf->dlc=1;//长度
//								  prbuf->data[0]=2;
//								  powerCanSendEnable=1;
//							}
              else if (setBattParaflag==5)//获取铅酸单元信息
							{
								 setBattParaflag=0;
								 pPcid->st_DTC=BATTERY_DOWN;
									pPcid->st_CID1=DEVICE_INFO;
								  pPcid->st_CID2=0;
									pPcid->st_ADR=battAddr[0];
									prbuf->rtr=0;//数据帧
									prbuf->dlc=1;//长度
								  prbuf->data[0]=0;
								  powerCanSendEnable=1;
							}
		
		
		
		
		
		return 0;
}





