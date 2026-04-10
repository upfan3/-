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



static u16 NormalizePercent_0p01(u16 raw)
{
	if(raw <= 100)
	{
		return (u16)(raw * 100); // 0~100 => 0.01%制
	}
	if(raw <= 1000)
	{
		return (u16)(raw * 10); // 0.1%制 => 0.01%制
	}
	if(raw > 10000)
	{
		return 10000;
	}
	return raw;
}

s8 CalcSingleBatteryRuntimeMin(u8 battIndex, u32 *runtimeMin)
{
	if((runtimeMin == NULL) || (battIndex >= LI_BATTERY_NUM))
	{
		return -1;
	}

	const s32 curr01A = (s32)batt[battIndex].Ibus / 10; // 0.1A
	s32 disCurr01A = -curr01A;
	if(disCurr01A < 0)
	{
		disCurr01A = 0;
	}
	if(disCurr01A < 5) // 小于0.5A不参与续航计算
	{
		*runtimeMin = 0;
		return -2;
	}

	u16 soc = NormalizePercent_0p01(batt[battIndex].B_SOC);
	u16 soh = NormalizePercent_0p01(batt[battIndex].B_SOH);
	u16 capAh = batt[battIndex].B_capacity;
	if((soc == 0) || (soh == 0) || (capAh == 0))
	{
		*runtimeMin = 0;
		return -3;
	}

		// 先算当前可用容量（Ah，按整数逐步缩放）
	int64_t availCapAh = (int64_t)capAh;
	availCapAh = availCapAh * (int64_t)soc / 10000;
	availCapAh = availCapAh * (int64_t)soh / 10000;

	if(disCurr01A <= 0)
	{
		*runtimeMin = 0;
		return -4;
	}

	// 电流单位为0.1A，换算到分钟：runtime_min = Ah / A * 60 = Ah * 600 / I(0.1A)
	int64_t minVal = availCapAh * 600 / (int64_t)disCurr01A;
	if(minVal < 0)
	{
		minVal = 0;
	}
	if(minVal > 0xFFFFFFFF)
	{
		minVal = 0xFFFFFFFF;
	}

	*runtimeMin = (u32)minVal;
	return 0;
}

s8 CalcTotalBatteryRuntimeMin(BatteryRuntimeInfo *pInfo)
{
	if(pInfo == NULL)
	{
		return -1;
	}

	pInfo->totalRuntimeMin = 0;
	pInfo->shortestRuntimeMin = 0;
	pInfo->validGroupCount = 0;

	const s32 totalDisCurr01A = (totalBattI < 0) ? (-(s32)totalBattI) : 0;
	if(totalDisCurr01A < 5)
	{
		return -2;
	}

	int64_t totalAh_0p01 = 0; // 0.01Ah
	u32 shortest = 0xFFFFFFFF;

	for(u8 i = 0; i < LI_BATTERY_NUM; i++)
	{
		u16 soc = NormalizePercent_0p01(batt[i].B_SOC);
		u16 soh = NormalizePercent_0p01(batt[i].B_SOH);
		u16 capAh = batt[i].B_capacity;
		if((soc == 0) || (soh == 0) || (capAh == 0))
		{
			continue;
		}

		int64_t singleAh_0p01 = (int64_t)capAh * 100;
		 singleAh_0p01 = singleAh_0p01 * (int64_t)soc / 10000;
		 singleAh_0p01 = singleAh_0p01 * (int64_t)soh / 10000;
		if(singleAh_0p01 <= 0)
		{
			continue;
		}
		totalAh_0p01 += singleAh_0p01;

		u32 singleMin = 0;
		if(CalcSingleBatteryRuntimeMin(i, &singleMin) == 0)
		{
			if(singleMin < shortest)
			{
				shortest = singleMin;
			}
			pInfo->validGroupCount++;
		}
	}

	if((pInfo->validGroupCount == 0) || (totalAh_0p01 <= 0))
	{
		return -3;
	}

	// total_min = totalAh * 60 / I(A) = totalAh_0p01 * 600 / I(0.1A)
	int64_t totalMin = totalAh_0p01 * 600 / (int64_t)totalDisCurr01A;
	if(totalMin < 0)
	{
		totalMin = 0;
	}
	if(totalMin > 0xFFFFFFFF)
	{
		totalMin = 0xFFFFFFFF;
	}
	pInfo->totalRuntimeMin = (u32)totalMin;
	pInfo->shortestRuntimeMin = (shortest == 0xFFFFFFFF) ? 0 : shortest;

	return 0;
}