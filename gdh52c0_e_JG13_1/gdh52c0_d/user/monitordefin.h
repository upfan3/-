#ifndef _MONITOR_DEFIN_H
#define _MONITOR_DEFIN_H

#include "valtype.h"

#define FLOAT_MOD 0
#define EQUAL_MOD 1
#define TEST_MOD  2
#define MOD_NONE  3  //无
#define TEST_MOD2  4//休眠时，电池检测
#define PEAK_MOD 5//
//#define MOD_NONE 6
#define I10 10//      10小时放电率
#define DAY_TOTAL_MIN  1439  //一天的总分钟数 0~1439

//extern u16 gsafedisV;
extern u8 setPowerLimitFlag;


#endif