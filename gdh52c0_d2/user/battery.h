#ifndef __BATTERY_H
#define __BATTERY_H
//#include "globalval.h"
#include "valtype.h"
#include <stdio.h>

//#define RatedCapacity	(u32)50	//设置属性，电池标称容量，默认500AH
#define BatteryCount	24	//设置属性，电池个数，系统为48v时为24，24v时为12，默认24个

#define BATT_IN    3
#define BATT_OUT   1


class BattCapacity
{
public:
	
	
	
 
	
  int64_t Capacity;				//电池实际有效容量 0.1Ams
	int64_t CapacityNow;		//算法属性：当前保有电量 0.1Ams

	u8 LastChargeState;    //上一轮电量计算充放电状态
  u8 reSetCap;           //重设容量标去

  u16 SOC;								//计算属性：电池健康状态 0.01%
  s16 BattCurrentLast;

	u32 TimeStamp1;	//时间属性：时间戳，单位：ms
	
  u32 TotalDisChagerTime;//总放电时长
  u32 LastDisChagerTime;//总放电时长
  u16 IntervalTime;	//时间属性：记录主控函数被调用时间间隔，单位：ms
  char Update(u16 DCVolt, s16 BatCurr, u32 TimeStamp);
  void ClearTotatleDisChagerTime(void);
	void ReSetCap(void);
  void Init(void);
  BattCapacity();
 ~BattCapacity();


};

typedef struct
{
  u32 totalRuntimeMin;    // 总续航（分钟）
  u32 shortestRuntimeMin; // 最短单组续航（分钟）
  u8 validGroupCount;     // 有效参与计算的电池组数量
} BatteryRuntimeInfo;

s8 CalcSingleBatteryRuntimeMin(u8 battIndex, u32 *runtimeMin);
s8 CalcTotalBatteryRuntimeMin(BatteryRuntimeInfo *pInfo);
//extern BattCapacity *pbattCap;
#endif
