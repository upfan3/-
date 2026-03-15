#include "Staggeredpeak.h"






s16 currLimitCap;
u8 Speakenable;
u8 gmon=0,gday=0;
//SpeakFlag ACRunMode=RUNNULL;



/*****************************************
//将具体日期转为日计数（1~366）
//mon取1~12
//day取1~31
*******************************************/
s16 DateToYearday(u8 mon,u8 day)
{
	
	
	if(mon<1||mon>12) return -1;
	//if(day<1||day>31) return -1;
	  s16 sday=0;
	
	for(u8 i=0;i<mon-1;i++)//从1月加到当前月
	{
		   sday+=mon_table[i]; 
		   if(Is_Leap_Year(gTimer.year)&&i==1)//润年国加1日 
			 {
				 sday+=1;
			 }
	}
	sday+=day;//加日
	return sday;
}
/******************************************
将一个中的某一天变为其体日期
yday 1~366
pdata 用来存月日的指针
rtu 1 交换日月的存储位置 0不交换
返回 -1 输入参数不在范围内（1~366）,1正确
******************************************/
s8 YeardayToDate(u16 yday,u8 *pdate,u8 rtu)
{  s8 result;
	pdate[0]=0;
	if((yday>366)||(yday==0))//返回前一年最后一天
	{		
		pdate[0]=12;
		pdate[1]=31;
		result= -1;
	}
	else
	{
	 yday--;
	 while(yday>=28)//超过了一个月
              {
                     if(Is_Leap_Year(gTimer.year)&&pdate[0]==1)//当年是不是闰年/2月份
                     {
                            if(yday>=29)yday-=29;//闰年的秒钟数
                            else break; 
                     }
                     else 
                     {
                            if(yday>=mon_table[pdate[0]])yday-=mon_table[pdate[0]];//平年
                            else break;
                     }
                     pdate[0]++;  
              }
							pdate[0]++;
							pdate[1]=yday+1;
							
	}
	          if(rtu==1)//将月日储存位置对调
							{
								u8 tmp=pdate[0];
								pdate[0]=pdate[1];
								pdate[1]=tmp;
							}
							result=1;
							return result;
}

s8 GetTimeInMin(u16 min,u8 *ptime,u8 rtu)
{  s8 result;
	 if(min>=1440)//若输入参数少于0，则指向前一天的最后一分
	 {
		 ptime[0]=24;
		 ptime[1]=00;
		 result= -1;
	 }
	 else
	 {
		 ptime[0]=min/60;
		 ptime[1]=min%60;
	   result=1;
	 }
	 
	  if(rtu==1)//将时分储存位置对调
							{
								u8 tmp=ptime[0];
								ptime[0]=ptime[1];
								ptime[1]=tmp;
							}
							result=1;
							return result;
}

/**********************************************
错峰大时段日期设置
//设置时段日期
mon取1~12
day取1~31
返回，成功返回1，失败返回-1(新设置日期少于前一个日期,或日期错误)
************************************************/
s8 SetSPeakData(u8 n,u8 mon,u8 day)
{  s16 tmp=0;
	  tmp= DateToYearday(mon ,day);
	if(tmp<=0) return -1;
	if(n>0)//从第二个日期开始，要与前一个日期比较。
	{   
		if(gsPeakTimeInteval[n-1].st_yday>=tmp)//新设置日期小于前一个日期,或日期错误
		{
			
			return -1;//返回错误标志
		}
	}
	gsPeakTimeInteval[n].st_yday=tmp;//设错误，恢复之前
	return 1;
	
}

/*********************************************
错峰小时段设置
n：大时段 取0~4
m：小时段 取0~11
hour：小时 0~23
min:分钟   0~59
返回：-1 设置无效 ，1设置正确
**********************************************/

s8 SetIimeInteval(u8 n,u8 m,u8 hour,u8 min)
{ u16 tmp;
	   	if(hour>24) return -1;
	    if(min>59) return -1;
	
	    if(hour==24) min=0;
	    tmp=hour*60+min;
	    if(m>0)//从第二个日期开始，要与前一个日期比较。
			{
				if(gsPeakTimeInteval[n].st_time[m-1]>=tmp)//新设置时间(m)小于前一个时间（m-1）
				{
					
					return -1;//返回错误标志
				}
			
			}
			gsPeakTimeInteval[n].st_time[m]=tmp;
		  return 1;
}


/*********************************************
错峰状态设置
n：大时段 取0~4
m：小时段 取0~11
flag:错峰状态设置 类型为SpeakFlag
返回：无
**********************************************/

void SetSpeakFlag(u8 n,u8 m,SpeakFlag flag)
{ u8 tmp=0;

	   if(m>11)return;

	        u8 *pSPeakFlag=(u8 *)(&gSpeakFlag[6*n]);//取得目标时间段（n）的首条记录地址
	         tmp=pSPeakFlag[m/2];//查找时间分段m记录位置，（注一个位置储存两个时间分段的记录）
	          tmp&=(~(0x0F<<((m%2)*4)));//将对应目位清零
	          tmp|=(flag<<((m%2)*4));//将数据写入目标分段位置（一个节中，用4bit存一个分段记录）
	       pSPeakFlag[m/2]=tmp;//回写相关记录
	
}



/*********************************************
错峰状态获取
n：大时段 取0~4
m：小时段 取0~11
返回：大小时段下所对应的错峰状态
**********************************************/
SpeakFlag GetSpeakFlag(u8 n,u8 m)
{  
	  u8 tmp=0;
	 u8 *pSPeakFlag=(u8 *)(&gSpeakFlag[6*n]);//取得目标时间段（n）的首条记录地址
	            tmp=pSPeakFlag[m/2];//查找时间分段m记录位置，（注一个位置储存两个时间分段的记录）
	
	           return (SpeakFlag)((tmp>>((m%2)*4))&0x0F);//读出目标分段位置记录（一个节中，用4bit存一个分段记录）
	
}
/*********************************************
错峰状态获取
n：大时段 取0~4
返回当前时间所在大时段下的错峰状态
**********************************************/
SpeakFlag FindOnSolt(u8 n)
{
	  u16 cu_min;
	  cu_min=gTimer.hour*60+gTimer.min;
	  if((cu_min>=gsPeakTimeInteval[n].st_time[0])&&(cu_min<gsPeakTimeInteval[n].st_time[1]))
		{
			    gday=0;
			    return GetSpeakFlag(n,0);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[1])&&(cu_min<gsPeakTimeInteval[n].st_time[2]))
		{
			
			    gday=1;
			   return GetSpeakFlag(n,1);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[2])&&(cu_min<gsPeakTimeInteval[n].st_time[3]))
		{
			    gday=2;
			   return GetSpeakFlag(n,2);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[3])&&(cu_min<gsPeakTimeInteval[n].st_time[4]))
		{
			   gday=3;
			   return GetSpeakFlag(n,3);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[4])&&(cu_min<gsPeakTimeInteval[n].st_time[5]))
		{
			   gday=4;
			   return GetSpeakFlag(n,4);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[5])&&(cu_min<gsPeakTimeInteval[n].st_time[6]))
		{
			   gday=5;
			   return GetSpeakFlag(n,5);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[6])&&(cu_min<gsPeakTimeInteval[n].st_time[7]))
		{
			   gday=6;
			   return GetSpeakFlag(n,6);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[7])&&(cu_min<gsPeakTimeInteval[n].st_time[8]))
		{
			   gday=7;
			   return GetSpeakFlag(n,7);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[8])&&(cu_min<gsPeakTimeInteval[n].st_time[9]))
		{
			   gday=8;
			   return GetSpeakFlag(n,8);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[9])&&(cu_min<gsPeakTimeInteval[n].st_time[10]))
		{
			   gday=9;
			   return GetSpeakFlag(n,9);
		}
		else if((cu_min>=gsPeakTimeInteval[n].st_time[10])&&(cu_min<gsPeakTimeInteval[n].st_time[11]))
		{
			   gday=10;
			   return GetSpeakFlag(n,10);
		}
		else if(((cu_min>=gsPeakTimeInteval[n].st_time[11])&&(cu_min<=1439))||(cu_min<gsPeakTimeInteval[n].st_time[0]))
		{
			   gday=11;
			   return GetSpeakFlag(n,11);
		}
		return RUNNULL;
}


/***************************************************
主函数执行错锋逻辑
设置ACRunMode在不同时该的错峰状态
****************************************************/
u8 prACRunStatus=RUNNULL;
u8 getEnergyFlag=1;



void SpeakOnTick(void)
{ 
	 
	
	
	 u8  prACRunMode=ACRunMode;
	if(gWorkShiftMode!=0)//使能错峰时段
	{// return;
				u16 cu_day;

					cu_day=DateToYearday(gTimer.mon,gTimer.day);
				
				 // YeardayToDate(cu_day,a);
				
				if((cu_day>=gsPeakTimeInteval[0].st_yday)&&(cu_day<gsPeakTimeInteval[1].st_yday))//第一时段间
				{
					   gmon=0;
						 ACRunMode = FindOnSolt(0);
				}
				else if((cu_day>=gsPeakTimeInteval[1].st_yday)&&(cu_day<gsPeakTimeInteval[2].st_yday))//第二时段间
				{
					    gmon=1;
							ACRunMode = FindOnSolt(1);
				}	
				else if((cu_day>=gsPeakTimeInteval[2].st_yday)&&(cu_day<gsPeakTimeInteval[3].st_yday))//第三时段间
				{
					    gmon=2;
							ACRunMode = FindOnSolt(2);
				}
				else if((cu_day>=gsPeakTimeInteval[3].st_yday)&&(cu_day<gsPeakTimeInteval[4].st_yday))//第四时段间
				{
					   gmon=3;
						 ACRunMode= FindOnSolt(3);
				}	
				else if(((cu_day>=gsPeakTimeInteval[4].st_yday)&&((cu_day<=365)||(cu_day<=366)))||//第五时段间
								 ((cu_day<gsPeakTimeInteval[0].st_yday)&&(cu_day>=1))) 
				{
					   gmon=4;
						 ACRunMode = FindOnSolt(4);
				}	
				else
				{
					  gmon=5;
					  gday=12;
						ACRunMode= RUNNULL;
				}		
				
				if((gmon<5)&&(gday<12))
				{
					 currLimitCap = limitCap[gmon][gday];
				}
				else
				{
					 currLimitCap = 0;
				}
				
		}
	else
	{
		    ACRunMode= RUNNULL;
	}
	
	     if(prACRunMode!=ACRunMode)
			 {
				    prACRunStatus=prACRunMode;
				    getEnergyFlag=1;
				    pMonitor->m_limitBattDischargeflag=0;
				    pMonitor->m_vailDischarge=0;
			 }
	    
			 
	
}





