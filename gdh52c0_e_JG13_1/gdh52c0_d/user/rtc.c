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
#include "rtc.h"
u16 tmp=5;
/**************************************************************
备份寄存器初始化
**************************************************************/
void Rtc_configuration(void)
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
	  
	  if (bkp_data_read(BKP_DATA_19)!=0x5A5A)
		{
			    bkp_deinit();
					/* enable LXTAL */
					rcu_osci_on(RCU_LXTAL);
					/* wait till LXTAL is ready */
					rcu_osci_stab_wait(RCU_LXTAL);
					
					/* select RCU_LXTAL as RTC clock source */
					rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

					/* enable RTC Clock */
					rcu_periph_clock_enable(RCU_RTC);

					/* wait for RTC registers synchronization */
					rtc_register_sync_wait();

					/* wait until last write operation on RTC registers has finished */
					rtc_lwoff_wait();

					/* enable the RTC second interrupt*/
					rtc_interrupt_enable(RTC_INT_SECOND);

					/* wait until last write operation on RTC registers has finished */
					rtc_lwoff_wait();

					/* set RTC prescaler: set RTC period to 1s */
					rtc_prescaler_set(32767);

					/* wait until last write operation on RTC registers has finished */
					rtc_lwoff_wait();
					
					
					 RTC_Set(2022,12,19,11,11,11);
					bkp_data_write(BKP_DATA_19,0x5A5A);
			}
		else
		{
			;
		}
}




/********************************************************
//判断是否是闰年函数

//月份   1  2  3  4  5  6  7  8  9  10 11 12

//闰年   31 29 31 30 31 30 31 31 30 31 30 31

//非闰年 31 28 31 30 31 30 31 31 30 31 30 31

//输入:年份

//输出:该年份是不是闰年.1,是.0,不是
********************************************************/
u8 Is_Leap_Year(u16 year)

{                     

       if(year%4==0) //必须能被4整除

       { 

              if(year%100==0) 

              { 

                     if(year%400==0)return 1;//如果以00结尾,还要能被400整除          

                     else return 0;   

              }else return 1;   

       }else return 0; 

}                           
/******************************************************************
RTC_Set
//设置时钟

//把输入的时钟转换为秒钟

//以1970年1月1日为基准

//1970~2099年为合法年份

//返回值:0,成功;其他:错误代码.

//月份数据表                                                                        

u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表   

//平年的月份日期表
*******************************************************************/
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)

{

       u16 t;

       u32 seccount=0;

       if(syear<1970||syear>2099)return 1;//syear范围1970-2099，此处设置范围为2000-2099        

       for(t=1970;t<syear;t++) //把所有年份的秒钟相加

       {

              if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数

              else seccount+=31536000;                    //平年的秒钟数

       }

       smon-=1;

       for(t=0;t<smon;t++)         //把前面月份的秒钟数相加

       {

              seccount+=(u32)mon_table[t]*86400;//月份秒钟数相加

              if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数         

       }

       seccount+=(u32)(sday-1)*86400;//把前面日期的秒钟数相加 

       seccount+=(u32)hour*3600;//小时秒钟数

    seccount+=(u32)min*60;      //分钟秒钟数

       seccount+=sec;//最后的秒钟加上去

       // PWR_BackupAccessCmd(ENABLE);
			 pmu_backup_write_enable();

       //RTC_WaitForLastTask();
			 	rtc_lwoff_wait();

       //RTC_SetCounter(seccount);
			 rtc_counter_set(seccount);

       //RTC_WaitForLastTask();
      rtc_counter_set(seccount);
       return 0;      

}
/***********************************************
RTC_Get
//得到当前的时间
//取得时间存放在gTimer中
//gYear 为完整年份
//返回值:0,成功;其他:错误代码.
*************************************************/
uTIME gTimer;
Timer4 gTimerNow;
u16 gYear;

u8 RTC_Get(void)

{
       static u16 daycnt=0;
       u32 timecount=0; 
       u32 temp=0;
       u16 temp1=0;
       timecount=rtc_counter_get();//RTC_GetCounter();    
       /*timecount=RTC->CNTH;//得到计数器中的值(秒钟数)
       timecount<<=16;
       timecount+=RTC->CNTL;    */           
       temp=timecount/86400;   //得到天数(秒钟数对应的)
       if(daycnt!=temp)//超过一天了
       {       
              daycnt=temp;
            temp1=1970;  //从1970年开始
			      // temp1=2000;   //从2000年开始	 。
              while(temp>=365)
              {                          
                    if(Is_Leap_Year(temp1))//是闰年
                     {
                            if(temp>=366)temp-=366;//闰年的秒钟数
                            else {/*temp1++;*/break;}  
                     }
                     else temp-=365;       //平年 
                     temp1++;  
              }   
			  gYear=temp1;
              gTimer.year=gYear;//得到年份 
              temp1=0;
              while(temp>=28)//超过了一个月
              {
                     if(Is_Leap_Year(gYear)&&temp1==1)//当年是不是闰年/2月份
                     {
                            if(temp>=29)temp-=29;//闰年的秒钟数
                            else break; 
                     }
                     else 
                     {
                            if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
                            else break;
                     }
                     temp1++;  
              }
							
              gTimer.mon=temp1+1;//得到月份
              gTimer.day=temp+1;  //得到日期 
       }
       temp=timecount%86400;     //得到秒钟数      
       gTimer.hour=temp/3600;     //小时
       gTimer.min=(temp%3600)/60; //分钟      
       gTimer.sec=(temp%3600)%60; //秒钟
			 
			 
			 gTimerNow.st_year=gTimer.year;
			 gTimerNow.st_mon=gTimer.mon;
			 gTimerNow.st_day=gTimer.day;
			 gTimerNow.st_hour=gTimer.hour;
			 gTimerNow.st_mon=gTimer.mon;
			 
      // timer.week=RTC_Get_Week(timer.w_year,timer.w_month,timer.w_date);//获取星期   
       return 0;
}     
/******************************************************************
RTC_Get_Week
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号
*******************************************************************/                                                                                                                                                     
//u8 RTC_Get_Week(u16 year,u8 month,u8 day)
//{     
//       u16 temp2;
//       u8 yearH,yearL;
//       yearH=year/100;     yearL=year%100; 
//       // 如果为21世纪,年份数加100  
//       if (yearH>19)yearL+=100;
//       // 所过闰年数只算1900年之后的  
//       temp2=yearL+yearL/4;
//       temp2=temp2%7; 
//       temp2=temp2+day+table_week[month-1];
//       if (yearL%4==0&&month<3)temp2--;
//       return(temp2%7);
//}
