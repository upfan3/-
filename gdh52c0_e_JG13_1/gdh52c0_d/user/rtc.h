/****************************************Copyright (c)**************************************************
**                               
**                                    
**                                 
**-----------------------------------------------------------文件信息-------------------------------------
**	文   件  	名:	RTC.C
** 	版  		本:	v1.0
** 	日		期:	2007年9月15日
**	描		述:	RTC相关
********************************************************************************************************/
#ifndef _RTC_H
#define _RTC_H
#include "valtype.h"
#ifdef __cplusplus
 extern "C" {
#endif

	 
typedef struct{
	u32 st_min :6;
	u32 st_hour:5;
	u32 st_day:5;
	u32 st_mon:4;
	u32 st_year:12;
}Timer4;
 
	 



typedef struct
{
	u32 sec		:6;
	u32	min		:6;
	u32	hour	:5;
	u32	day		:5;
	u32	mon		:4;	
	u32	NC    :6;
  u16 year;	
}	uTIME,*P_uTIME;

void Rtc_configuration(void);
u8 Is_Leap_Year(u16 year);
u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);
u8 RTC_Get(void);

extern uTIME gTimer;	
extern Timer4 gTimerNow;
extern const u8 mon_table[12];


#ifdef __cplusplus
}
#endif


#endif
