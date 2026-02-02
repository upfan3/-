#ifndef _SP_H
#define _SP_H
#include "rtc.h"
#include "globalval.h"






s8 SetSPeakData(u8 n,u8 mon,u8 day);
s8 SetIimeInteval(u8 n,u8 m,u8 hour,u8 min);
void SetSpeakFlag(u8 n,u8 m,SpeakFlag flag);
SpeakFlag GetSpeakFlag(u8 n,u8 m);
SpeakFlag FindOnSolt(u8 n);

s8 YeardayToDate(u16 yday,u8 *pdate,u8 rtu);
s16 DateToYearday(u8 mon,u8 day);
s8 GetTimeInMin(u16 min,u8 *ptime,u8 rtu);



#endif 
