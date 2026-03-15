#include "globalval.h"


#define  BATT_ONLINE_DELAT 90


//extern u8 senndflag; 
#define GET_BATT_VOLT           0   //  寄存器0000  
#define GET_BATT_CHAR_DATA      1   //  寄存器1031 
#define GET_BATT_CELL_TEMP      2   //  寄存器0012
#define GET_BATT_CELL_VOLT      3   //  寄存器0022
#define GET_BATT_CELL_NUM       4   //  寄存器010F 
#define GET_BATT_SET_VOLT_CURR  5   //  寄存器1010
#define GET_BATT_SET_MOS        6   //  寄存器107D
#define GET_BATT_VERSION        7   //  寄存器0101
#define GET_BATT_DISCAP         8   //  寄存器1042

#define SET_BATT_VOLT      1//设置电池电压
#define SET_BATT_CHR_CURR  2//设置电池充电电流
#define SET_BATT_CHR_MOS   3//设置电池充电MOS
#define SET_BATT_DIS_MOS   4//设置电池放电MOS
#define SET_BATT_WORK_MOD  5//设置电池工作模式

#define BATTCOMM_SOI 0x7E
#define BATTCOMM_EOI 0x0D
#define BATTCOMM_VER 0x20
#define BATT_MAX_NUM LI_BATTERY_NUM
#define BATTERY_START_ADDR 1
#define SINGLE_HEX_FRAME_LENGTH 250   //原来64

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
//					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
//					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
//					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
//					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
//					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
//					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
//					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_CELL_TEMP ,GET_BATT_DISCAP
//					 ,GET_BATT_VOLT,GET_BATT_CHAR_DATA,GET_BATT_DISCAP ,GET_BATT_CELL_VOLT
         };


u8 BattOnlineList[LI_BATTERY_NUM];
u8 recvBattEnd = 1;

u8 Set_BattPara[8]={0xD6,0x06,0x00,0x00,0x00,0x00,0x00,0x00};
u8 tmpSend[8]={0xD6,0x03,0,0,0,0,0,0};
u16 Battery_buf[16]={0};//临时存放数据
u8 ErrCode=0;
u8 Next_flag=0;
u8 gCmdCode=0;//判断发送的是查询还是设置指令
u8 set_addr=0;
u8 onlineNum=8,checkflag=1,pronlineNum=4;
u8 battID[2]={0,onlineNum};//电池模块数列表   0:第n个电池   1:总锂电模块数
u8 SetBattCmd=0,SetBattAddr=0, *pBattData;
u8 recvBattCount[2]={0,0};
u16 currdiscap = 0;

u32 totalBattdisCap = 0;
u32 prtotalBattdisCap = 0;
s16 totalBattI = 0;  //0.1A
u32 totalSOC = 10000;
u32 totalSOH = 0;
u16 vagBatVolt = 5300;


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
	else if(_cmd==SET_BATT_WORK_MOD)//模式控制
	{
		 Set_BattPara[2]=0x10;
		 Set_BattPara[3]=0x16;
	  	setbatt.setflag[4]++;
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
	else  if(_cmd == SET_BATT_WORK_MOD)//模式控制
	{
		  setbatt.WORK_MODE =*(u16 *)pdat;
		   pBattData= (u8 *)&setbatt.WORK_MODE;
	}
	   
	 setbatt.setflag[_cmd-1] = 0;
}


void SetBatteryDataIn(u8 _cmd,u8 _addr,u8  *pdat)
{
	
	 SetBattCmd=_cmd;
	 SetBattAddr=_addr;
	 pBattData = pdat;
	
}





void calculateTotalData(void)
{ //static u8 vailbat = 0;
	  		   if(recvBattCount[0] >= onlineNum)  //轮询完所有模块后，计算总值
					 {
				     totalBattI=0;
					   totalSOC = 0;
             totalSOH = 0;
					  // vagBatVolt = 0;
						 u8 vailbatCount = 0;
						 u32 temVolt = 0;
							  for(u8 i=0;i<onlineNum;i++)
							 {
							   totalBattI  = totalBattI + batt[i].Ibus/10;
								 
								 totalSOC = totalSOC + ((batt[i].B_SOC*batt[i].B_SOH)/100)*(batt[i].B_capacity/100);
								 
								 totalSOH  = totalSOH + ((batt[i].B_SOH*batt[i].B_capacity)/10000);
								 
							   if(batt[i].Vbat>0){
								 temVolt = temVolt + batt[i].Vbat;
							    vailbatCount++;}
								 
							 }
							 
							 
							  totalSOC=totalSOC/totalSOH;
							 
							  if(vailbatCount>0)
								{
									  vagBatVolt =  temVolt/vailbatCount;
								}
							 
							 
							  recvBattCount[0]=0;
							 
					  }
				
}


void BattCmdPolling(void)
{
	static s8 addr=0,cmd=0;
	 
      
	   
	  if( SetBattCmd == 0)
		{
			
			
			
       calculateTotalData();
			 GetBatteryCmd(addr,BattCmdPoing[cmd]);
	     Next_flag=BattCmdPoing[cmd];
			if(Next_flag == GET_BATT_CHAR_DATA)
			{
				 recvBattCount[0]++;
			}
			 addr++;
			
		  
	    if(addr>=onlineNum)
			   {
					  
					  addr=0; 
						cmd++;
						if(cmd>=sizeof(BattCmdPoing)) cmd=0;
					 
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
	   if( DataID >= onlineNum ) return NULL;
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
				 batt[DataID].WorkMode=Battery_buf[13]>>8;
				
				 u16 workMode;
				 if( batt[DataID].WorkMode == 1)
				 {        
						workMode = 0x0101;
				 }
				 else if(batt[DataID].WorkMode == 2)
				 {
						workMode = 0x0303;
				 }
				 else if(batt[DataID].WorkMode == 3)
				 {
						workMode = 0x0707;
				 }
				 else if(batt[DataID].WorkMode == 4)
				 {
						workMode = 0x0707;
				 }
				
				 if((workMode != setbatt.WORK_MODE)&&(setbatt.setflag[4]<5))
				 {
					 SetBatteryDataIn(SET_BATT_WORK_MOD ,0, (u8 *)&setbatt.WORK_MODE);//重新设置
				 }
				
			
				
				 batt[DataID].BattWarning=Battery_buf[11];
				
				
				if((Battery_buf[8]&0x04) == 0x04) //读取短路告警
				{
					batt[DataID].BattWarning |= 0x8000; 
				}
				else
				{
					batt[DataID].BattWarning &= 0x7FFF;
				}

				
				
				
		
				
				
				
			}
			else if(Next_flag==GET_BATT_SET_VOLT_CURR)  //寄存器 1010
			{
				batt[DataID].Set_BattV=Battery_buf[0];					//放电电池电压设定值
				batt[DataID].Set_BattA=Battery_buf[1];					//读取放电电池电流设定值
				batt[DataID].Set_Batt_ChargA=Battery_buf[3];			//充电电池电流设定值
				
				

				
			
				
				
				if((batt[DataID].Set_Batt_ChargA != setbatt.Set_Batt_ChargA)&&(setbatt.setflag[1]<5))
				{
					  SetBatteryDataIn(SET_BATT_CHR_CURR ,0, (u8 *)&setbatt.Set_Batt_ChargA);//重新设置
				}
				
					if((batt[DataID].Set_BattV != setbatt.Set_BattV)&&(setbatt.setflag[0]<5))
				{
					   SetBatteryDataIn(SET_BATT_VOLT ,0, (u8 *)&setbatt.Set_BattV);//重新设置
				}
				
				
				
				
			}
			else if(Next_flag==GET_BATT_SET_MOS) //寄存器 107D
			{
				batt[DataID].MOS_Charg=Battery_buf[0];					//充电回路控制
				batt[DataID].MOS_DisCharg=Battery_buf[1];					//放电回路控制
		   // batt[DataID].Last_Time=Battery_buf[3];
				
					if((batt[DataID].MOS_Charg != setbatt.MOS_Charg)&&(setbatt.setflag[2]<5))
				{
					  SetBatteryDataIn(SET_BATT_CHR_MOS ,0, (u8 *)&setbatt.MOS_Charg);//重新设置
				}
				
					if((batt[DataID].MOS_DisCharg != setbatt.MOS_DisCharg)&&(setbatt.setflag[3]<5))
				{
					   SetBatteryDataIn(SET_BATT_DIS_MOS ,0, (u8 *)&setbatt.MOS_DisCharg);//重新设置
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
							 totalBattdisCap =  totalBattdisCap + batt[i].DisCap ;//累计所有电池放电电量
						 }
			}

		
	 
	 }
	
	 	 
	 
	 
		return NULL;

}

void detectBatteryOnline(void)
{
	static u8 checkcount=60;
	    for(u8 i=0;i<onlineNum;i++)
			{
				
				   if(batt[i].Nexttime>1)
				     batt[i].Nexttime--;  
           else if(batt[i].Nexttime == 1)
					 {
							memset( &batt[i].Vbus ,0, sizeof(Battery));
					 }


					 
			}
			
				
				
				if((totalBattdisCap>prtotalBattdisCap)&&(prtotalBattdisCap>0)) //若最新一次放电电量大于前一次放电电量，且前一次放电电量大于0
						{
							u8 tmpdiscap=0;
							
								tmpdiscap = totalBattdisCap - prtotalBattdisCap ;
								 
								 currdiscap  =currdiscap + tmpdiscap;  //累计本次放电电量
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
			
			 for(u8 i=0;i<onlineNum;i++)
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
									   ((batt[i].BattWarning & BATT_DHG_TEMP_HIGH ) == BATT_DHG_TEMP_HIGH) || //电池放电温度高
							       ((batt[i].BattWarning & BATT_CHG_TEMP_HIGH ) == BATT_CHG_TEMP_HIGH)  //电池充电温度高
							     )
								 
								 {
									 
									   addAlarm(BATT_TEMP,i+1,HIGH_);
									   batt[i].prBattWarning|= BATT_TEMP_HIGH ;
									 
									
								 }
							} 
							else if((batt[i].prBattWarning & BATT_TEMP_HIGH) == BATT_TEMP_HIGH)
							{
								  if(
										     ((batt[i].BattWarning & BATT_DHG_TEMP_HIGH) == 0) &&//环境温度高取消
									       ((batt[i].BattWarning & BATT_CHG_TEMP_HIGH) == 0) 
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
									   ((batt[i].BattWarning & BATT_DHG_TEMP_LOW ) == BATT_DHG_TEMP_LOW) || //电池放电温度高
							       ((batt[i].BattWarning & BATT_CHG_TEMP_LOW ) == BATT_CHG_TEMP_LOW)  //电池充电温度高
							     )
								 
								 {
									 
									   addAlarm(BATT_TEMP,i+1,LOW_);
									   batt[i].prBattWarning|= BATT_TEMP_LOW ;
									 
									
								 }
							} 
							else if((batt[i].prBattWarning & BATT_TEMP_LOW) == BATT_TEMP_LOW)
							{
								  if(
										     ((batt[i].BattWarning & BATT_DHG_TEMP_LOW) == 0) &&//环境温度高取消
									       ((batt[i].BattWarning & BATT_CHG_TEMP_LOW) == 0) 
								    )
								 {
									   minusAlarm(BATT_TEMP,i+1,LOW_);
									   batt[i].prBattWarning &= ~(BATT_TEMP_LOW);
									 
									
								 }
							}
			}
		
		
}

//计算ydt1363校验码
u16 CalChecksum(u8* asciiData, u16 dataLength) 
{
    u16 sum = 0;
    
    // 遍历每个十六进制字节 跳过SOI
    for (u16 i = 0; i < dataLength; i++) 
		{        
        sum += asciiData[i];	// ASCII字符累加
    }
    // 取反加1
    sum = ~sum + 1;
    
    return sum;
}

u8 AsciiToHex(u8 c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

static bool IsValidHexAscii(u8 c)
{
    return ((c >= '0' && c <= '9') || 
            (c >= 'A' && c <= 'F') || 
            (c >= 'a' && c <= 'f'));
}

u8 AsciiToByte(u8 high, u8 low)
{
    return (AsciiToHex(high) << 4) | AsciiToHex(low);
}

//将两字节数据交换高低字节
u16 swapTwoBytes(u16 value) 
{
    // 将低字节移到高字节位置，将高字节移到低字节位置
    return ((value & 0xFF) << 8) | ((value & 0xFF00) >> 8);
}

//计算ydt1363长度校验
u16 CalLength(u16 lenId)
{
    // 初始化总和为 0
    u8 sum = 0;
    // 提取 LENID 的每 4 位并相加
    for (u8 i = 0; i < 16; i += 4) {
        sum += (lenId >> i) & 0x0F;
    }
    // 计算模 16 的余数
    u8 remainder = sum % 16;
    // 对余数取反加 1
    u8 result = ~remainder + 1;
    // 组合 LCHKSUM 和 LENID 得到 LENGTH
    u16 length = (result << 12) | lenId;

    return length;
}

//HEX数据帧转为ASCII
void HexFrameToAscii(u8 *frame, u8 hexValue, u8* startPos) 
{
    u8 i = 2;
    u8 tmp;
		u8* output = &frame[*startPos];
    while (i--) {
        tmp = (hexValue >> (4 * i)) & 0x0F; // 提取高4位或低4位
        if (tmp < 10) {
            *output = tmp + '0'; // ASCII码0-9
        } else {
            *output = tmp + 'A' - 10; // ASCII码A-F
        }
        output++; // 移动指针到下一个位置
    }
		
		*startPos += 2;
}

u8 ydtSendData[30] = {0};
u16 cycleFrameNumber = 0;
//打包轮询指令 infoLength HEX字节数  info 16进制数据
s8 PackBattCmdPolling(u8 addr, u8 cid1, u8 cid2, u16 infoLength, const u8* info)
{
	#define SINGLE_ASCII_FRAME_LENGTH 128
	// 计算实际需要的帧长度
    u8 cidCount = ((cid1 == 0) ? 0 : 1) + ((cid2 == 0) ? 0 : 1);
    u16 requiredSize = 1 + 1 + 1 + cidCount + 2 + infoLength + 2 + 1;
    
		//最好保证满足一帧的最大长度
    if (requiredSize > SINGLE_ASCII_FRAME_LENGTH)
		{
        return -1; // 缓冲区过小
    }

    u8 pos = 0;
		
    // 起始字节SOI
		ydtSendData[pos++] = BATTCOMM_SOI;
    // 协议版本VER
		HexFrameToAscii(ydtSendData,BATTCOMM_VER,&pos);
		
    // 目标地址ADDR
		HexFrameToAscii(ydtSendData,addr,&pos);
    
    // 添加有效CID
    if (cid1 != 0) 
			HexFrameToAscii(ydtSendData,cid1,&pos);
		
    if (cid2 != 0) 
			HexFrameToAscii(ydtSendData,cid2,&pos);
		
		cycleFrameNumber = cid1 << 8 | cid2;
    
    // 数据长度LENGTH
		if(infoLength == NULL || info == NULL)
		{
			HexFrameToAscii(ydtSendData,0,&pos);
			HexFrameToAscii(ydtSendData,0,&pos);
		}
		else
		{
			u16 resLength = CalLength(infoLength * 2);
			u8 highByteLength = (u8)(resLength >> 8);
			
			HexFrameToAscii(ydtSendData,highByteLength,&pos);
			
			HexFrameToAscii(ydtSendData,(u8)resLength,&pos);
			
			for(u8 i = 0; i < infoLength; i++)
			{
				HexFrameToAscii(ydtSendData,info[i],&pos);
			}
			
			
			// 数据字段INFO
			//memcpy(&ydtSendData[pos], info, infoLength);
			//pos += infoLength;
		}
    
    // 计算校验和CHKSUM (包含INFO的所有字节)
    u16 checksum = CalChecksum(&ydtSendData[1], pos - 1);
		u8 highByteChecksum = (u8)(checksum >> 8);
		HexFrameToAscii(ydtSendData,highByteChecksum,&pos);
		HexFrameToAscii(ydtSendData,(u8)checksum,&pos);
    
    // 结束字节EOI
    ydtSendData[pos++] = BATTCOMM_EOI;
		return pos;
}

void AsciiFrameToHex(u8* asciiData, u16 asciiDataLength, u8* hexData, u16 hexDataMaxLength, u16* vailHexDataNum)
{
    // 初始化有效计数为0，避免未定义值
    *vailHexDataNum = 0;
    u16 index = 0;

    // 指针合法性检查：避免空指针访问崩溃
    if (asciiData == NULL || hexData == NULL || vailHexDataNum == NULL)
        return;

    // 遍历ASCII数据，每2个字符转换为1个HEX字节
    for (u16 i = 0; i < asciiDataLength; i += 2)
    {
        // 关键修复1：判断是否有完整的2个字符，避免数组越界（处理奇数长度输入）
        if (i + 1 >= asciiDataLength)
            break;

        // 关键修复2：判断输出缓冲区是否已满，避免溢出
        if (index >= hexDataMaxLength)
            break;

        // 关键修复3：AsciiToByte参数传值错误（原传索引i/i+1，应传对应ASCII字符）
        u8 highByte = asciiData[i];    // 高4位对应的ASCII字符
        u8 lowByte = asciiData[i + 1]; // 低4位对应的ASCII字符

        // 新增：ASCII字符合法性检查（仅允许0-9、A-F、a-f）
        if (!IsValidHexAscii(highByte) || !IsValidHexAscii(lowByte))
            continue; // 跳过非法字符组，不生成错误HEX

        // 转换并存储HEX字节
        hexData[index] = AsciiToByte(highByte, lowByte);
        index++; // 仅转换成功后，索引才递增
    }

    // 返回实际有效转换的HEX字节数
    *vailHexDataNum = index;
}

void setChargeMos(u8 addr,CtrlState state)
{
	u8 byteNum = 0;
	u8 cmdData[2] = {0x01,state};
	byteNum = PackBattCmdPolling(addr,0x4A,0x45,2,cmdData);
	pusartBatt->SendDataEx(ydtSendData,byteNum);
}

void setDisChargeMos(u8 addr,CtrlState state)
{
	u8 byteNum = 0;
	u8 cmdData[2] = {0x00,state};
	byteNum = PackBattCmdPolling(addr,0x4A,0x45,2,cmdData);
	pusartBatt->SendDataEx(ydtSendData,byteNum);
}

//设定充电限流值 setData 单位mA
void setChargeLimitI(u8 addr,u16 setData)
{
	u8 arrayTmp[3] = {0};
	
	arrayTmp[0] = 0x3E; //62对应充电限流
	
	u8 ctrsSate = setData >> 8;
	arrayTmp[1] = ctrsSate;
	
	ctrsSate = setData;
	arrayTmp[2] = ctrsSate;

	u8 byteNum = 0;
	byteNum = PackBattCmdPolling(addr,0x4A,0x49,3,arrayTmp);
	pusartBatt->SendDataEx(ydtSendData,byteNum);
}

u8 ydtQueryCmd[3] = {0x42,0x44,0x47}; 
//u8 test[2] = {0x63,0x12};
void BattCmdPollingCommon(bool parseDoneFlag)
{
	static u8 timeoutCount = 1;
	
	if(checkDis())
	{
		timeoutCount = 5;
		return;
	}

	
	
	if(parseDoneFlag == false)
	{
		if(timeoutCount > 0 )
		{
			timeoutCount--;
			return;
		}
	}
	
	//timeoutCount = 5;
		
	
	//setChargeMos(1,MOS_ON);
	
	u8 byteNum = 0;
	u8 cmdMaxNum = (sizeof(ydtQueryCmd) / sizeof(ydtQueryCmd[0])) - 1;
	static u8 battNum = BATTERY_START_ADDR;
	static u8 cmdNum = 0;

	//byteNum = PackBattCmdPolling(battNum,0x4A,0x47,NULL,NULL);
	byteNum = PackBattCmdPolling(battNum,0x4A,ydtQueryCmd[cmdNum],NULL,NULL);
	if(ydtQueryCmd[cmdNum] == 0x42)
		recvBattCount[0]++;
//	byteNum = PackBattCmdPolling(battNum,0x4A,ydtQueryCmd[cmdNum],2,test);
//	pusartBatt->SendDataEx(ydtSendData,byteNum);
	
	pusartBatt->SendDataEx(ydtSendData,byteNum);
	
	if(battNum >= BATT_MAX_NUM)
	{
		battNum = BATTERY_START_ADDR;
		cmdNum++;
		if(cmdNum > cmdMaxNum)
			cmdNum = 0;
	}
	else
	{
		battNum++;
	}
}

u16 preRemainCapSum = 0;
void updateRemainCap()
{
	u16 remainCapSum = 0;
	
	for(u8 i = 0; i < BATT_MAX_NUM; i++)
	{
		remainCapSum += batt[i].remainCap;
	}
	
	if(preRemainCapSum == 0)
	{
		preRemainCapSum = remainCapSum;
		return;
	}
	
	s16 diff = preRemainCapSum - remainCapSum;
	
	if(diff < 0)
	{
		preRemainCapSum = remainCapSum;
		return;
	}
	
	totalBattdisCap = diff / 100;
	
	
	
}

void parseBatteryData(u16 cycleFrameNumber, u8 address, u8* hexFrame, u16 hexDataLength)
{
	u8 vCellNum = 0, tmpNum = 0;
	static u8 recvBattNum = 0;
	
	batt[address - 1].Nexttime = BATT_ONLINE_DELAT;//超时计数重置
	
	if(cycleFrameNumber == 0x4A42)
	{
		//更新数据
//		recvBattCount[0]++;
		
		TelemetryDataStruct* telemetryData = (TelemetryDataStruct*)hexFrame;
		
		batt[address - 1].Cell_Num = telemetryData->cellCount;
		batt[address - 1].remainCap = swapTwoBytes(telemetryData->batteryRemainingCapacity);
		
		u16 cellVMax = 0,cellVMin = 0;
		
		for(u8 i = 0; i < 16; i++)
		{
			u16 cellVolValue = swapTwoBytes(telemetryData->cellVoltage[i]);
			
			batt[address - 1].CellV[i] = cellVolValue;
			
			if(cellVMax == 0)
				cellVMax = cellVolValue;
			
			if(cellVMin == 0)
				cellVMin = cellVolValue;
			
			if(cellVolValue > cellVMax)
				cellVMax = cellVolValue;
			else if(cellVolValue < cellVMin)
				cellVMin = cellVolValue;
		}
		
		batt[address - 1].CellVMax = cellVMax;
		batt[address - 1].CellVMin = cellVMin;
		
		
		u16 cellTempMax = 0, cellTempMin = 0;
		
		for(u8 j = 0; j < 4; j++)
		{
			u16 cellTmpValue = swapTwoBytes(telemetryData->coreTemp[j]);
			cellTmpValue = cellTmpValue / 100;
			batt[address - 1].CellTemp[j] = cellTmpValue;
			
			if(cellTempMax == 0)
				cellTempMax = cellTmpValue;
			
			if(cellTempMin == 0)
				cellTempMin = cellTmpValue;
			
			if(cellTmpValue > cellTempMax)
				cellTempMax = cellTmpValue;
			else if(cellTmpValue < cellTempMin)
				cellTempMin = cellTmpValue;
			
		}
		
		batt[address - 1].CellTempMax = cellTempMax;
		batt[address - 1].CellTempMin = cellTempMin;
		
		batt[address - 1].Ibus = swapTwoBytes(telemetryData->batteryCurrent);
		batt[address - 1].Vbat = swapTwoBytes(telemetryData->batteryTotalVoltage);
		
		batt[address - 1].B_capacity = swapTwoBytes(telemetryData->batteryTotalCapacity);
		batt[address - 1].B_SOC = swapTwoBytes(telemetryData->soc) * 100;
		batt[address - 1].B_SOH = swapTwoBytes(telemetryData->soh) * 100;
		
		updateRemainCap();
		
		recvBattNum++;
		if(recvBattNum>=LI_BATTERY_NUM)
		{
			recvBattNum=0;
			recvBattEnd=1;
		}
		
	}
	else if(cycleFrameNumber == 0x4A44)
	{
		u16 battWarn = 0;
		u8 vCellNum = 0, tCellNum =0;
		u8 index = 3 + 16 + 1 + 4 + 6;	//电压事件代码
		
		u8 balanEventCode = hexFrame[index-1];//均衡事件代码
		if(((balanEventCode>>5) & 0x01) == 1||((balanEventCode>>6) & 0x01) == 1)//充放电 MOS 故障
		{
			g_devStatusFlags = FLAG_FAULT;
		}
		
		
		u8 volEventCode = hexFrame[index];	//电压事件代码
		
		if((volEventCode & 0x01) == 1)
		{
			battWarn |= (1 << 0);	//单体过压告警
		}
		else if(((volEventCode >> 2) & 0x01) == 1)
		{
			battWarn |= (1 << 1);	//单体欠压告警
		}
		else if(((volEventCode >> 4) & 0x01) == 1)
		{
			battWarn |= (1 << 2);	//总体过压告警
		}
		else if(((volEventCode >> 6) & 0x01) == 1)
		{
			battWarn |= (1 << 3);	//总体欠压告警
		}
		
		u16 tmpEventCode = hexFrame[index + 1];	//温度事件代码
		
		if((tmpEventCode & 0x01) == 1)
		{
			battWarn |= (1 << 6);	//电池充过温告警
		}
		else if(((tmpEventCode >> 2) & 0x01) == 1)
		{
			battWarn |= (1 << 7);	//电池充欠温告警
		}
		else if(((tmpEventCode >> 8) & 0x01) == 1)
		{
			battWarn |= (1 << 8);	//环境高温告警
		}
		else if(((tmpEventCode >> 10) & 0x01) == 1)
		{
			battWarn |= (1 << 9);	//环境低温告警
		}
		else if(((tmpEventCode >> 4) & 0x01) == 1)
		{
			battWarn |= (1 << 13);	//放电过温告警
		}
		else if(((tmpEventCode >> 6) & 0x01) == 1)
		{
			battWarn |= (1 << 14);	//放电低温告警
		}
		 

		
		u8 currentEventCode = hexFrame[index + 3];	//电流事件代码
		
		if((currentEventCode & 0x01) == 1)
		{
			battWarn |= (1 << 4);	//充过流告警
		}
		else if(((currentEventCode >> 2) & 0x01) == 1)
		{
			battWarn |= (1 << 5);	//放过流告警
		}
		else if(((currentEventCode >> 5) & 0x01) == 1)
		{
			battWarn |= (1 << 15);	//短路保护
		}
		
		u8 mosTmpAlarmState = hexFrame[index - 5];	//MOS温度告警状态
		
		if(mosTmpAlarmState == HIGH_ALARM)
		{
			battWarn |= (1 << 10);	//MOS过温告警
		}
		
		u8 socAlarmState = hexFrame[index + 3];
		
		if(socAlarmState == 1)
		{
			battWarn |= (1 << 11);	//SOC告警
		}
		
		u8 fetEventCode = hexFrame[index + 5];	//读取充电MOS状态
		
		if(((fetEventCode >> 1) & 0x01) == 1)
		{
			batt[address - 1].MOS_Charg = 1;
			g_devStatusFlags = FLAG_CHARGING;
		}
		else
		{
			batt[address - 1].MOS_Charg = 0x55;
		}
		
		if((fetEventCode & 0x01) == 1)
		{
			batt[address - 1].MOS_DisCharg = 1;
			g_devStatusFlags = FLAG_DISCHARGING;
		}
		else
		{
			batt[address - 1].MOS_DisCharg = 0x55;
		}
		
		u8 sysStatusCode = hexFrame[index + 6];
		
		//待机
		if((sysStatusCode & 0x01) == 0 && ((sysStatusCode & 0x02) == 0))
			batt[address - 1].B_Status = 8;
		
		//直通放电
		if((sysStatusCode & 0x01) != 0 && ((sysStatusCode & 0x02) == 0))
			batt[address - 1].B_Status = 3;
		
		//直通充电
		if((sysStatusCode & 0x01) == 0 && ((sysStatusCode & 0x02) != 0))
			batt[address - 1].B_Status = 2;
		
		batt[address - 1].BattWarning = battWarn;
		
	}
	else if(cycleFrameNumber == 0x4A47)
	{
		u8 index = 1 + 2*7 + 1 + 2*12 + 2*27 + 2*13;
		u16 chargeClrValue = hexFrame[index] << 8;
		chargeClrValue |= hexFrame[index + 1];
		batt[address - 1].Set_Batt_ChargA = chargeClrValue / 10;
	}
	
	batt[address - 1].Set_BattV = gsetbatt.Set_BattV;
}

void *DuleWithDataBatteryCommon(u8 *recData,u16 *dataLength)
{
	u16 dataLen = *dataLength;
	
	if(dataLen < 1)
		return NULL;
	
	//校验帧头 帧尾
	if (recData[0] != BATTCOMM_SOI || recData[dataLen - 1] != BATTCOMM_EOI)  
		return NULL;

	// 解析基本字段
    u8 version = AsciiToByte(recData[1], recData[2]);
    u8 address = AsciiToByte(recData[3], recData[4]);
    u8 rtn = AsciiToByte(recData[7], recData[8]);
    
    // 校验版本、地址和RTN
    if (version != BATTCOMM_VER || 
        address < BATTERY_START_ADDR || 
        address > BATT_MAX_NUM ||
        rtn != 0) {
        return NULL;
    }
	
	u16 recCRC = AsciiToByte(recData[dataLen - 3], recData[dataLen - 2]);
	recCRC |= AsciiToByte(recData[dataLen - 5], recData[dataLen - 4]) << 8;
		
	u16 calCRC = CalChecksum(&recData[1],dataLen - 6);
	
	if(recCRC != calCRC)
		return NULL;

	//获取数据长度
	u16 tmpLen = (AsciiToByte(recData[9], recData[10]) << 8) | AsciiToByte(recData[11], recData[12]);
	tmpLen &= 0x0FFF; 
	
	//将ASCII码数据长度转为HEX数据长度
	u16 hexDataLength = 0;
	u8 hexFrame[SINGLE_HEX_FRAME_LENGTH] = {0};
	
	//将数据段转为HEX
	AsciiFrameToHex(&recData[13], tmpLen, hexFrame, SINGLE_HEX_FRAME_LENGTH, &hexDataLength);
	
	parseBatteryData(cycleFrameNumber, address, hexFrame, hexDataLength);
	
	cycleFrameNumber = 0;
}

//void SetBatteryCmd() 

