#include "battery.h"
#include "globalval.h"
BattCapacity gbattCap;
BattCapacity *pbattCap=&gbattCap;


BattCapacity::BattCapacity()
{
}

/**************************************************************************************
	1.函数：Init
	2.功能：初始化成员变员
	3.变量说明：
	4.返回值：代表当前状态+异常
****************************************************************************************/


void BattCapacity::Init()
{


SOC = 10000;					//电池剩余容量 0.01%
LastChargeState = 0;
reSetCap=0;
TotalDisChagerTime=0;	
LastDisChagerTime=0;
BattCurrentLast = 0;	//上一轮容量计算时的电池电流 0.1A
	
//Capacity =(int64_t)gSetBattC *10*3600*1000 ;//gSetBattC Ah= gSetBattC *10(0.1Ah) = gSetBattC * 10 *3600 (0.1As) = gSetBattC * 10 *3600 *1000 (0.1Ams)
Capacity =(int64_t)gSetBattC*3600; //注为计算百分比方便少乘10000
CapacityNow = Capacity*10000;		//算法属性：当前保有电量 (0.1Ams) 

TimeStamp1 = 0;	//时间属性：时间戳，单位：ms
IntervalTime = 0;	//时间属性：记录主控函数被调用时间间隔，单位：ms
}


BattCapacity::~BattCapacity()
{
}


/**************************************************************************************
	1.函数：Control
	2.功能：主控制函数，供系统定时/不定时访问
	3.变量说明：
	4.返回值：代表当前状态+异常
****************************************************************************************/
char BattCapacity::Update(u16 DCVolt, s16 BatCurr, u32 TimeStamp)
{
	u8 CurrChargeState;
	
  if(	reSetCap==1)
	{  
		  reSetCap=0;
		 //Capacity =(int64_t)gSetBattC *10*3600*1000 ;//gSetBattC Ah= gSetBattC *10(0.1Ah) = gSetBattC * 10 *3600 (0.1As) = gSetBattC * 10 *3600 *1000 (0.1Ams)
		 Capacity =(int64_t)gSetBattC*3600; //注为计算百分比方便少乘10000
     CapacityNow = Capacity *10000;		//算法属性：当前保有电量 (0.1Ams)
	}
  	
	if (TimeStamp >= TimeStamp1)
	{
		IntervalTime = TimeStamp - TimeStamp1;	//读取时间戳
		TimeStamp1 = TimeStamp;
	}
	else//处理计数器溢出
	{
	
		 IntervalTime = 0xFFFFFFFF- TimeStamp1+TimeStamp+1;
		 TimeStamp1 = TimeStamp;
	}
	
	   CapacityNow +=(s32) ((BatCurr + BattCurrentLast) / 2 * IntervalTime);
			if(CapacityNow<0) CapacityNow=0;
	    if((CapacityNow/10000)>Capacity) CapacityNow=Capacity*10000;
	
			//SOC = CapacityNow/(Capacity/10000);
       SOC = CapacityNow/Capacity;//Capacity少乘10000   计算当前电量百分比
	
	  if(BatCurr>0)
		{CurrChargeState=BATT_IN;}//充电状态
	  else
		{CurrChargeState=BATT_OUT;}//放电状态
	  
           if(CurrChargeState==BATT_OUT)//当前为放电状态
					 {						
								LastDisChagerTime+=IntervalTime;//单位为ms，累加单次放电时长
					 }
					 else if(CurrChargeState==BATT_IN)//当前为充电状态
					 {
						    if(LastChargeState==BATT_OUT)//上一统计为充电状态
								{
									if(LastDisChagerTime>=300000)//若上次放电时长大于等于5分钟
									{ 
									  TotalDisChagerTime+=LastDisChagerTime;//累加总放电时长
									}
									   LastDisChagerTime=0;
								}
					 }
					 
	         
	
		   
	
	
	
	
	
	
	  BattCurrentLast=BatCurr;
		LastChargeState=CurrChargeState;



}


void BattCapacity::ClearTotatleDisChagerTime(void)
{
	   LastDisChagerTime=0;
	   TotalDisChagerTime=0;
}

void BattCapacity::ReSetCap(void)
{
	  reSetCap=1;
	  ClearTotatleDisChagerTime();
}


