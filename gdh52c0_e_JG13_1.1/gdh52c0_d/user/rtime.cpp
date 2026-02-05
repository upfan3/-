#include "globalval.h"
#include "powercan.h"
extern PowrCan PowCanDevice;	
extern u16 gEqudelay;
/*
#ifdef __cplusplus
 extern "C" {
#endif
	 
volatile unsigned int ulHighFrequencyTimerTicks = 0UL;
	 
u32 gcount=0;
void TIM2_IRQHandler(void)	 //定时5ms
{
   	 
     if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 
     {     
          TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update); //定时1s
		            ;
                 //ulHighFrequencyTimerTicks++;  
                 gcount++;	
              JinGuiOnLineCount();
              JinGuiOutLineCheckCount();	
              DownOnTicK();		 
		 }
}

#ifdef __cplusplus
}
#endif


void TIM2_Init(void)//1ms中断一次
{  
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   NVIC_InitTypeDef NVIC_InitStructure;
     SystemInit();
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);       
   NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
   NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
   NVIC_Init(&NVIC_InitStructure);
       //基础设置，时基和比较输出设置，由于这里只需定时，所以不用OC比较输出
        //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);      
        //TIM_OCInitTypeDef TIM_OCInitStructure;
        //reset TIM2 
        TIM_DeInit(TIM2);
        //a period is 10000 times
        TIM_TimeBaseStructure.TIM_Period=(10000-1); //1000为1ms进一次中断，2000为2ms进一次中断
        //
        TIM_TimeBaseStructure.TIM_Prescaler=7200;//division number   is 7200 
        //set clock division 
        TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //or TIM_CKD_DIV2 or TIM_CKD_DIV4
        //count up
        TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;

        TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
      //clear the TIM2 overflow interrupt flag
      TIM_ClearFlag(TIM2,TIM_FLAG_Update);
      //TIM2 overflow interrupt enable
      TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
      //enable TIM2
     
      TIM_Cmd(TIM2,ENABLE);     
 }  
*/

//u16 gtimeCount100ms=0;
u16 gtimeCount1ms=0;
u32 gtimeCount1s=0;

#ifdef __cplusplus
 extern "C" {
#endif
void TIMER1_IRQHandler(void)
{
	
	  if( timer_interrupt_flag_get(TIMER1, TIMER_INT_FLAG_UP)==SET)
		{
			    timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
			   
			    gtimeCount1ms++;
			     
			   padc->ReadADResultforLowPassfilter3order();
			


			   if(gtimeCount1ms%100==0)//100ms
				 {
			     
					  Swtmr2_Callback(NULL);
					 if(gtimeCount1ms%1000==0)//1s
					 {
						 gtimeCount1ms=0;
						 Swtmr1_Callback(NULL);
						 gtimeCount1s++;
						  JinGuiOnLineCount();
              JinGuiOutLineCheckCount();	
						  detectBatteryOnline();
						  DownOnTicK();
						 
						if(gChargeStatus==1)//处于限充状态下
						{
							gEqudelay++;
						}
							
						 
						 
					 }
					 
					
				 }

		 }
	
	
}

#ifdef __cplusplus
}
#endif



void TIM1_Init(void)
{
	 timer_parameter_struct timer_parameter;
	
	  timer_deinit(TIMER1);
	
	rcu_periph_clock_enable(RCU_TIMER1);
	
	timer_struct_para_init(&timer_parameter);
	timer_parameter.prescaler       = 7200;
	timer_parameter.alignedmode     = TIMER_COUNTER_EDGE;
	timer_parameter.counterdirection= TIMER_COUNTER_UP;
//	timer_parameter.period          = (1000-1);//100ms
	  timer_parameter.period          = (10-1);//1ms
	timer_parameter.clockdivision   = TIMER_CKDIV_DIV1;
	
	timer_init(TIMER1, &timer_parameter);
	timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
	
	
		
	nvic_irq_enable(TIMER1_IRQn,3,0);
	
	timer_interrupt_enable(TIMER1, TIMER_INT_UP);
	
	timer_enable(TIMER1);
}
 
 
 