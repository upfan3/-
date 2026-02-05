#include "globalval.h"


#define  BATT_ONLINE_DELAT 10


//extern u8 senndflag; 
#define GET_BATT_VOLT           0   //  寄存器0000  
#define GET_BATT_CHAR_DATA      1   //  寄存器1031 
#define GET_BATT_CELL_TEMP      2   //  寄存器0012
#define GET_BATT_CELL_VOLT      3   //  寄存器0022
#define GET_BATT_CELL_NUM       4   //  寄存器010F 
#define GET_BATT_SET_VOLT_CURR  5   //  寄存器1010
#define GET_BATT_SET_MOS        6   //  寄存器107D
#define GET_BATT_VERSION        7   //  寄存器0101
#define GET_BATT_DISCAP         8   //  寄存器0142

#define SET_BATT_VOLT      1//设置电池电压
#define SET_BATT_CHR_CURR  2//设置电池充电电流
#define SET_BATT_CHR_MOS   3//设置电池充电MOS
#define SET_BATT_DIS_MOS   4//设置电池放电MOS

u8 BarreryCmd[9][3]={
                      {0x00,0x00,0x07},//总线电压，电池电压，最大温度，最小温度
                      {0x10,0x30,0x0E},//充放电数据
											{0x00,0x12,0x10},//单体温度
											{0x00,0x22,0x10},//单体电池
											{0x01,0x0f,0x01},//电芯数
											{0x10,0x10,0x04},//读取放电电压,电流设定值
											{0x10,0x7D,0x02},//读取充放电回路控制
											{0x01,0x01,0x02},//软件版本，硬件版本
											{0x10,0x42,0x02},//累计放电电量
										};

u8 BattCmdPoing[]={//命令轮询数组
	          GET_BATT_CELL_NUM,GET_BATT_SET_VOLT_CURR,GET_BATT_SET_MOS
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
         };


u8 BattOnlineList[LI_BATTERY_NUM];

u8 Set_BattPara[LI_BATTERY_NUM]={0xD6,0x06,0x00,0x00,0x00,0x00,0x00,0x00};
u8 tmpSend[8]={0xD6,0x03,0,0,0,0,0,0};
u8 battID[2]={0,LI_BATTERY_NUM};//电池模块数列表   0:第n个电池   1:总锂电模块数
u16 Battery_buf[16]={0};//临时存放数据
u8 ErrCode=0;
u8 Next_flag=0;
u8 gCmdCode=0;//判断发送的是查询还是设置指令
u8 set_addr=0;
u8 onlineNum=0,checkflag=1,pronlineNum=0;
u8 SetBattCmd=0,SetBattAddr=0, *pBattData;
u8 recvBattCount[2]={0,0};
u16 currdiscap = 0;

u32 totalBattdisCap = 0;
u32 prtotalBattdisCap = 0;
s16 totalBattI = 0;  //0.1A
u32 totalSOC = 10000;
u32 totalSOH = 0;


SetBattery setbatt;

u16 count_CRC(u8*addr,int num) 
{ 
		u16 crc = 0xFFFF; 
		int i;
		while(num--) 
		{
			crc^=*addr++; 
			for(i=0;i<8;i++) 
			{
				if(crc&1) 
				{
					crc>>=1; 
					crc^=0xA001;
				} 
				else 
				{ 
					crc>>=1;
				}
			} 
		} 
	return crc;
}



u8 GetBatteryCmd(u8 addr,u8 cmd )
{
	u16 tmp_crc=0;


	
	
	
	
	
	
	      tmpSend[0] = 0xD6+addr;
	      tmpSend[2] = BarreryCmd[cmd][0];
	      tmpSend[3] = BarreryCmd[cmd][1];
	      tmpSend[5] = BarreryCmd[cmd][2];
	
	      tmp_crc=count_CRC(tmpSend,6);
				tmpSend[7]=tmp_crc>>8;
				tmpSend[6]=tmp_crc&0x00ff;
				
			pusartBatt->SendDataEx(tmpSend,8);
	
	
	
}
void CheckBattOnline(void)
{
	  
	static u8 addr=0;
	
	    			
				 GetBatteryCmd(addr,GET_BATT_VERSION);
				 addr++;
				 if(addr>=LI_BATTERY_NUM)
				 {
				   addr=0;
					 checkflag=0;
				 }
		  

}


void SetBatteryCmd(u8 _cmd,u8 _addr,u8 *pdat)
{
	
	u16 crc_set=0;
	
	if( _addr == 0)
	{	
	  Set_BattPara[0]=0;	
	}
	else
	{
	  Set_BattPara[0]=0xD5+_addr;//锂电地址
	}
	
	if(_cmd==SET_BATT_VOLT)//锂电放电电压 40-60V
	{
		Set_BattPara[2]=0x10;
		Set_BattPara[3]=0x10;
		setbatt.setflag[0]++;
	}
	else if(_cmd==SET_BATT_CHR_CURR)	//设定充电电流大小
	{
		Set_BattPara[2]=0x10;
		Set_BattPara[3]=0x13;
		setbatt.setflag[1]++;
	}
	else if(_cmd==SET_BATT_CHR_MOS)//充电回路控制
	{
		Set_BattPara[2]=0x10;
		Set_BattPara[3]=0x7D;
		setbatt.setflag[2]++;
	}
	else if(_cmd==SET_BATT_DIS_MOS)//放电回路控制
	{
		Set_BattPara[2]=0x10;
		Set_BattPara[3]=0x7E;
		setbatt.setflag[3]++;
	}

	
	Set_BattPara[4]=pdat[1];//数据
	Set_BattPara[5]=pdat[0];

	
	
	crc_set=count_CRC(Set_BattPara,6);
	Set_BattPara[6]=crc_set&0x00ff;
	Set_BattPara[7]=crc_set>>8;
	
	pusartBatt->SendDataEx(Set_BattPara,8);
}

/**************SetBatteryData********************
电池设置命令
 _cmd 命令码 
 _addr 0 广播 非0 电池地址
 pdat  数据地址
************************************************/
void SetBatteryData(u8 _cmd,u8 _addr,u8  *pdat)
{
	
	 SetBattCmd=_cmd;
	 SetBattAddr=_addr;
	// pBattData = pdat;
	
	if(_cmd == SET_BATT_VOLT)
	{
		  setbatt.Set_BattV =  *(u16 *)pdat;
		  pBattData= (u8 *)&setbatt.Set_BattV;
	}
	else if(_cmd == SET_BATT_CHR_CURR)
	{
		   setbatt.Set_Batt_ChargA = *(u16 *)pdat;
		   pBattData= (u8 *)&setbatt.Set_Batt_ChargA;
	}
	else if(_cmd == SET_BATT_CHR_MOS)
	{
		   setbatt.MOS_Charg = *(u16 *)pdat;
		   pBattData= (u8 *)&setbatt.MOS_Charg;
	}
	else if(_cmd == SET_BATT_DIS_MOS)
	{
		   setbatt.MOS_DisCharg =*(u16 *)pdat;
		   pBattData= (u8 *)&setbatt.MOS_DisCharg;
	}
	   
	 setbatt.setflag[_cmd-1] = 0;
}


void SetBatteryDataIn(u8 _cmd,u8 _addr,u8  *pdat)
{
	
	 SetBattCmd=_cmd;
	 SetBattAddr=_addr;
	 pBattData = pdat;
	
}



void BattCmdPolling(void)
{
	static s8 addr=0,cmd=0;
	 
      
	   
	  if( SetBattCmd == 0)
		{
	   if(checkflag == 0)
		 {
			 
			 if(onlineNum>0)
	     GetBatteryCmd(BattOnlineList[addr],BattCmdPoing[cmd]);
	     Next_flag=BattCmdPoing[cmd];
			 if(pronlineNum != onlineNum)
			 {
				    for(u8 i=0;i<4;i++)
				    setbatt.setflag[i]=0;
				 
			 }
				
				addr++;
			
		  
	    if(addr>=onlineNum)
			   {
					  
					  addr=0; 
						cmd++;
						if(cmd>=sizeof(BattCmdPoing)) cmd=0;
					 
				 }
				 
				 
				
			 }
       else if( checkflag == 1 )
       {
				  CheckBattOnline();
				 
				 Next_flag=GET_BATT_VERSION;
			 }	
		 }
     else
		 {
			  SetBatteryCmd( SetBattCmd, SetBattAddr, pBattData);
			  SetBattCmd = 0;
		 }			 

	    
	    
	
}



void *DuleWithDataBattery(u8 *pdat,u16 *plen)
{
	
  u8 j=3,k=4;
	u8 Data_i=0;
	u8 DataID=0;

	
	
     DataID=pdat[0]-0xD6;
	   if( DataID >= LI_BATTERY_NUM ) return NULL;
	  batt[DataID].Nexttime = BATT_ONLINE_DELAT;//超时计数重置


	 if(pdat[1]==0x03)	//读单个或连续多个寄存器   解析
	 { 
		  //数据存放
		 for(u8 i=0;i<(pdat[2]/2);i++)
		 {
			 
			 Battery_buf[Data_i]=pdat[j+(i*2)]<<8|pdat[k+(i*2)]; 
			 Data_i ++;
		 }
			Data_i=0;
		 
						 
			if(Next_flag==GET_BATT_VOLT)  //寄存器 0000
			{
				batt[DataID].Vbus=Battery_buf[0];								//总线电压
				batt[DataID].Vbat=Battery_buf[1];								//电池电压
				batt[DataID].CellTempMax=Battery_buf[5];								//电池温度
				batt[DataID].CellTempMin=Battery_buf[6];								//电池温度
			}
			else if(Next_flag==GET_BATT_CELL_NUM) //寄存器 010F
			{

				batt[DataID].Cell_Num=Battery_buf[0];						//电池芯数量
			}
			else if(Next_flag==GET_BATT_CHAR_DATA) //寄存器 1030
			{
				 batt[DataID].Ibus=(s16)Battery_buf[0];								//总线电流
				 batt[DataID].Ibat=(s16)Battery_buf[1];               //电池电流
				 batt[DataID].B_capacity=Battery_buf[2];					//电池额定容量(电池满充容量)
				 batt[DataID].B_SOC=Battery_buf[4];								//SOC
				 batt[DataID].B_SOH=Battery_buf[5];								//SOH

				 batt[DataID].B_Status=Battery_buf[13]&0xff;			//电池充放电状态
				 recvBattCount[0]++;
				
				 batt[DataID].BattWarning=Battery_buf[11];
				
				
				if((Battery_buf[8]&0x04) == 0x04) //读取短路告警
				{
					batt[DataID].BattWarning |= 0x8000; 
				}
				else
				{
					batt[DataID].BattWarning &= 0x7FFF;
				}

				
				
				
				   if(recvBattCount[0] >= onlineNum)  //轮询完所有模块后，计算总值
					 {
				     totalBattI=0;
					   totalSOC = 0;
             totalSOH = 0;
					   
							   for(u8 i=0;i<LI_BATTERY_NUM;i++)
							 {
							   totalBattI  = totalBattI + batt[i].Ibus/10;
								 
								 totalSOC = totalSOC + ((batt[i].B_SOC*batt[i].B_SOH)/100)*(batt[i].B_capacity/100);
								 
								 totalSOH  = totalSOH + ((batt[i].B_SOH*batt[i].B_capacity)/10000);
								 
							
								 
							 }
							 
							 
							  totalSOC=totalSOC/totalSOH;
							 
							  recvBattCount[0]=0;
							 
					  }
				
				
				
				
			}
			else if(Next_flag==GET_BATT_SET_VOLT_CURR)  //寄存器 1010
			{
				batt[DataID].Set_BattV=Battery_buf[0];					//放电电池电压设定值
				batt[DataID].Set_BattA=Battery_buf[1];					//读取放电电池电流设定值
				batt[DataID].Set_Batt_ChargA=Battery_buf[3];			//充电电池电流设定值
				
				

				
			
				
				
				if((batt[DataID].Set_Batt_ChargA != setbatt.Set_Batt_ChargA)&&(setbatt.setflag[1]<5))
				{
					  SetBatteryDataIn(2 ,0, (u8 *)&setbatt.Set_Batt_ChargA);//重新设置
				}
				
					if((batt[DataID].Set_BattV != setbatt.Set_BattV)&&(setbatt.setflag[0]<5))
				{
					   SetBatteryDataIn(1 ,0, (u8 *)&setbatt.Set_BattV);//重新设置
				}
				
				
				
				
			}
			else if(Next_flag==GET_BATT_SET_MOS) //寄存器 107D
			{
				batt[DataID].MOS_Charg=Battery_buf[0];					//充电回路控制
				batt[DataID].MOS_DisCharg=Battery_buf[1];					//放电回路控制
		   // batt[DataID].Last_Time=Battery_buf[3];
				
					if((batt[DataID].MOS_Charg != setbatt.MOS_Charg)&&(setbatt.setflag[2]<5))
				{
					  SetBatteryDataIn(3 ,0, (u8 *)&setbatt.MOS_Charg);//重新设置
				}
				
					if((batt[DataID].MOS_DisCharg != setbatt.MOS_DisCharg)&&(setbatt.setflag[3]<5))
				{
					   SetBatteryDataIn(4 ,0, (u8 *)&setbatt.MOS_DisCharg);//重新设置
				}
				
				
			}
			else if(Next_flag==GET_BATT_CELL_TEMP)
			{
				 for(u8 i=0 ;i<16 ;i++)
				 batt[DataID].CellTemp[i] = Battery_buf[i];
			}
			else if(Next_flag==GET_BATT_CELL_VOLT)
			{
				  batt[DataID].CellVMax = Battery_buf[0];
				  batt[DataID].CellVMin = Battery_buf[0];
				
				u8 CellNum=15;
				
			  	if(batt[DataID].Cell_Num!=0)
					CellNum = batt[DataID].Cell_Num;
				
				 for(u8 i=0 ;i< CellNum ;i++)
				{
				  batt[DataID].CellV[i] = Battery_buf[i];
					
					if ( batt[DataID].CellV[i]> batt[DataID].CellVMax )
					{
						 batt[DataID].CellVMax= batt[DataID].CellV[i];
					}
					
					if ( batt[DataID].CellV[i]< batt[DataID].CellVMin )
					{
						 batt[DataID].CellVMin= batt[DataID].CellV[i];
					}
					
					
					
				}
				
				
			}
	    else if(Next_flag==GET_BATT_DISCAP)
			{
          batt[DataID].DisCap = (Battery_buf[0]<<16)|Battery_buf[1];
				
				    totalBattdisCap = 0;
						 for(u8 i=0;i<onlineNum;i++)
						 {
							 totalBattdisCap =  totalBattdisCap + batt[BattOnlineList[i]].DisCap ;//累计所有电池放电电量
						 }
						 

			}
			else if(Next_flag==GET_BATT_VERSION)
			{
				
				 
				 BattOnlineList[onlineNum] = DataID;
				if(onlineNum<LI_BATTERY_NUM)
				  onlineNum++;
			}
	 
		
	 
	 }
	
	 	 
	 
	 
		return NULL;

}
void detectBatteryOnline(void)
{
	static u8 checkcount=60;
	    for(u8 i=0;i<LI_BATTERY_NUM;i++)
			{
				
				   if(batt[i].Nexttime>1)
				     batt[i].Nexttime--;  
           else if(batt[i].Nexttime == 1)
					 {
						 memset( &batt[i].Vbus ,0, sizeof(Battery));
					 }


					 
			}
			
			
			
			  checkcount--;
			  if(checkcount == 0)
				{
					  checkcount = 60;
					  checkflag=1;
					
					 pronlineNum = onlineNum;
					  onlineNum=0;
				}
					
				
				
				
				
				
				if((totalBattdisCap>prtotalBattdisCap)&&(prtotalBattdisCap>0)) //若最新一次放电电量大于前一次放电电量，且前一次放电电量大于0
						{
							u8 tmpdiscap=0;
							
								tmpdiscap = totalBattdisCap - prtotalBattdisCap ;
								 
								 if(tmpdiscap<10)
								 {currdiscap  =currdiscap + tmpdiscap;}  //累计本次放电电量
								 prtotalBattdisCap =  totalBattdisCap ;//更新上一次放电电量
							
						}
						else
						{ 
								 prtotalBattdisCap =  totalBattdisCap ;//更新上一次放电电量
						}
			
			
}


void SetBattWarning(void)
{
	   AlarmType WarnType[11]={ 
			                        ENVTEMP         ,ENVTEMP           ,BATT_CELL_VOLT   ,
		                          BATT_CELL_VOLT  ,BATTSUPPLY_ALARM  ,BATTSUPPLY_ALARM ,
		                          BATTDOWN_ALARM  ,BATTDOWN_ALARM    ,BATTDOWN_ALARM   ,  
		                          BATTSOC         ,BATTDOWN_ALARM
		                        };
 AlarmBehavior Behavior[11]={ HIGH_    ,LOW_      ,HIGH_    , 
	                            LOW_     ,HIGH_     ,LOW_     ,
	                            SHORT    ,OVER_DHG  ,OVER_CHG , 
	                            LOW_     ,VOLT_DIFF_OVER
                             };
		       u16 WarnCode[11]={ 
						                  ENVTEMP_TEMP_HIGH  ,ENVTEMP_TEMP_LOW ,BATT_CELL_VOLT_HIGH ,
					                    BATT_CELL_VOLT_LOW ,BATT_VOLT_HIGH   ,BATT_VOLT_LOW       ,
					                    BATT_SHORT         ,BATT_OVER_DHG    ,BATT_OVER_CHG       ,
					                    BATT_SOC_LOW       ,BATT_VOLT_DIFF_OVER
					                  };
			
			 for(u8 i=0;i<LI_BATTERY_NUM;i++)
			{
				     for(u8 j=0;j<11;j++)  //环境温度，单体电压，总体电压，短路
						{
							   if((batt[i].prBattWarning & WarnCode[j] ) == 0)
							{
							
								 if((batt[i].BattWarning & WarnCode[j] ) == WarnCode[j])
								 {
									 
									   addAlarm(WarnType[j],i+1,Behavior[j]);
									   batt[i].prBattWarning|= WarnCode[j] ;
									 
									
								 }
							} 
							else if((batt[i].prBattWarning & WarnCode[j]) == WarnCode[j])
							{
								  if((batt[i].BattWarning & WarnCode[j]) == 0)
								 {
									   minusAlarm(WarnType[j],i+1,Behavior[j]);
									   batt[i].prBattWarning &= ~(WarnCode[j]);
									 
									
								 }
							}
							
						}
						
						
						 //电池温度高
							if((batt[i].prBattWarning & BATT_TEMP_HIGH ) == 0)
							{
							
								 if(
//									   ((batt[i].BattWarning & BATT_DHG_TEMP_HIGH ) == BATT_DHG_TEMP_HIGH) || //电池放电温度高
//							       ((batt[i].BattWarning & BATT_CHG_TEMP_HIGH ) == BATT_CHG_TEMP_HIGH)  //电池充电温度高
								       (batt[i].CellTempMax*100>ghtemp)&&(batt[i].Nexttime>1)
							     )
								 
								 {
									 
									   addAlarm(BATT_TEMP,i+1,HIGH_);
									   batt[i].prBattWarning|= BATT_TEMP_HIGH ;
									 
									
								 }
							} 
							else if((batt[i].prBattWarning & BATT_TEMP_HIGH) == BATT_TEMP_HIGH)
							{
								  if(
//										     ((batt[i].BattWarning & BATT_DHG_TEMP_HIGH) == 0) &&//环境温度高取消
//									       ((batt[i].BattWarning & BATT_CHG_TEMP_HIGH) == 0) 
									        (batt[i].CellTempMax*100)<(ghtemp-300)
								    )
								 {
									   minusAlarm(BATT_TEMP,i+1,HIGH_);
									   batt[i].prBattWarning &= ~(BATT_TEMP_HIGH);
									 
									
								 }
							}
							
							
							 //电池温度低
							if((batt[i].prBattWarning & BATT_TEMP_LOW ) == 0)
							{
							
								 if(
//									   ((batt[i].BattWarning & BATT_DHG_TEMP_LOW ) == BATT_DHG_TEMP_LOW) || //电池放电温度低
//							       ((batt[i].BattWarning & BATT_CHG_TEMP_LOW ) == BATT_CHG_TEMP_LOW)  //电池充电温度低
								        ((batt[i].CellTempMin*100)<gltemp)&&(batt[i].Nexttime>1)
							     ) 
								 
								 {
									 
									   addAlarm(BATT_TEMP,i+1,LOW_);
									   batt[i].prBattWarning|= BATT_TEMP_LOW ;
									 
									
								 }
							} 
							else if((batt[i].prBattWarning & BATT_TEMP_LOW) == BATT_TEMP_LOW)
							{
								  if(
//										     ((batt[i].BattWarning & BATT_DHG_TEMP_LOW) == 0) &&//环境温度低取消
//									       ((batt[i].BattWarning & BATT_CHG_TEMP_LOW) == 0) 
									          (batt[i].CellTempMin*100)>(gltemp+300)
								    )
								 {
									   minusAlarm(BATT_TEMP,i+1,LOW_);
									   batt[i].prBattWarning &= ~(BATT_TEMP_LOW);
									 
									
								 }
							}
			}
		
		
}
