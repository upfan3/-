#include "adc.h"
#include "globalval.h"
#include <math.h>


//#define NUMOFADCCHN 8
//#define NUMOFADCBUF 64

#define NUMOFADCCHN 7
#define NUMOFADCBUF 1

//---------------------------全局变量----------------------------------------------	 
u16 uADCvalue[NUMOFADCCHN*NUMOFADCBUF];
adc gadc(uADCvalue,NUMOFADCCHN*NUMOFADCBUF); 
adc * padc=&gadc;
u32 AD_channel[8];
//s16 temperature;
#ifdef __cplusplus
 extern "C" {
#endif


	 
//---------------------------GD32F105 usart硬件相关配置----------------------------------------------	 
void adc_gpio_config(void)
{
	rcu_periph_clock_enable(RCU_ADC0);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
  gpio_init(GPIOA,GPIO_MODE_AIN,GPIO_OSPEED_50MHZ,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	gpio_init(GPIOB,GPIO_MODE_AIN,GPIO_OSPEED_50MHZ,GPIO_PIN_1);
	gpio_init(GPIOC,GPIO_MODE_AIN,GPIO_OSPEED_50MHZ,GPIO_PIN_4|GPIO_PIN_5);

	 rcu_periph_clock_enable(RCU_DMA0);
	 rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6 );//ADC时钟最大为14M,从APB2取得，因此对APB2 6分频，时钟为12M
}		 





void adc_config(void)
{
	 adc_deinit(ADC0);//reset ADC 
	 adc_mode_config(ADC_MODE_FREE);//ADC mode config
	 adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);// ADC contineous function DISABLE 单次模式
	 adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);// ADC scan mode ENABLE 多通道扫描
	 adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);//ADC data alignment config 
	 //adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 8); //ADC channel length config 共8个通道
	adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 7); //ADC channel length config 共8个通道
	
	 /* ADC regular channel config */
//	 adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_0,ADC_SAMPLETIME_13POINT5);//传感器1
//	 adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_1,ADC_SAMPLETIME_13POINT5);//传感器2
//	 adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_2,ADC_SAMPLETIME_13POINT5);//分流器1
//	 adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_3,ADC_SAMPLETIME_13POINT5);//母线电压采样
//	 adc_regular_channel_config(ADC0, 4, ADC_CHANNEL_14,ADC_SAMPLETIME_13POINT5);//分流器2
//	 adc_regular_channel_config(ADC0, 5, ADC_CHANNEL_15,ADC_SAMPLETIME_13POINT5);//湿度传感器
//	 adc_regular_channel_config(ADC0, 6, ADC_CHANNEL_9,ADC_SAMPLETIME_13POINT5);//温度传感顺
//	 adc_regular_channel_config(ADC0, 7, ADC_CHANNEL_17,ADC_SAMPLETIME_239POINT5);
	
	 adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_0,ADC_SAMPLETIME_13POINT5);//传感器1
	 adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_1,ADC_SAMPLETIME_13POINT5);//传感器2
	 adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_3,ADC_SAMPLETIME_13POINT5);//母线电压采样
	 adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_9,ADC_SAMPLETIME_13POINT5);//温度传感器
	 adc_regular_channel_config(ADC0, 4, ADC_CHANNEL_2,ADC_SAMPLETIME_13POINT5);//分流器1
	 adc_regular_channel_config(ADC0, 5, ADC_CHANNEL_14,ADC_SAMPLETIME_13POINT5);//分流器1
	 adc_regular_channel_config(ADC0, 6, ADC_CHANNEL_15,ADC_SAMPLETIME_13POINT5);//湿度传感器
	
	 
	 /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
	
	 /* enable ADC interface */
    adc_enable(ADC0);
    //delay_1ms(1);
	
	 /* ADC DMA function enable */
    adc_dma_mode_enable(ADC0);
    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
	
}

void dma_config(uint16_t * adc_value,uint16_t data_size)
{
    /* ADC_DMA_channel configuration */
    dma_parameter_struct dma_data_parameter;
    
    /* ADC DMA_channel configuration */
    dma_deinit(DMA0, DMA_CH0);
    
    /* initialize DMA single data mode */
    dma_data_parameter.periph_addr  = (uint32_t)(&ADC_RDATA(ADC0));
    dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr  = (uint32_t)adc_value;//数据存放的所在内存地址
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;  
    dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number       = data_size;//缓冲空间大小，要在DMA未启动前设置
    dma_data_parameter.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH0, &dma_data_parameter);
    dma_circulation_enable(DMA0, DMA_CH0);
  
    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH0);
}


	 
	 
#ifdef __cplusplus
}
#endif



adc::adc(uint16_t * adc_value,uint16_t data_size)
{
	adc_gpio_config();
	dma_config( adc_value,data_size);
	adc_config();
	m_adc_value=adc_value;
	m_data_size=data_size;
	m_enableflag=0;
}



void adc::reInit(void)
{
	adc_gpio_config();
	adc_config();
	dma_config( m_adc_value,m_data_size);

}


void adc:: startADC(void)
{
	 adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

void adc::ReadADResult(void)
{u8 i,k;
 u32 result=0;	
  
	

	for(k=0;k<NUMOFADCCHN-1;k++)
	   {
	     for(i=0;i<NUMOFADCBUF;i++)
	     {
				   result+=uADCvalue[i*NUMOFADCCHN+k];
			 }
			 
			 AD_channel[k]=result;
			    result=0;
			 
			 if(k==6)
														{
															
															float ftmp;//,ln,A;
													
															 ftmp=(float)3.38* AD_channel[k]/4096/64;		
                                                 ftmp=(float)log(0.47*ftmp/(3.38-ftmp));//0.47  上拉电阻为4.7K,温度传感器基准电阻为10K 4700/10000=0.47
	                                             ftmp=(float)298.15*ftmp/3950;
                                                 ftmp=(float)(25-273.15*ftmp)/(ftmp+1);
															
															    if(ftmp>-30)
																	{
																		*((s16 *)(&gpSysData[ENV_TEMP]))=ftmp*100;
																	}
																	else
																	{
																		*((s16 *)(&gpSysData[ENV_TEMP]))=2500;
																	}
														}								
			 
			 
			 
		 }
		 
		 
		 
		 
		 
		 
		  startADC() ;
	
}

/*二阶滤波	系数，采样频面1KHZ，截止频率10Hz,滤波器通带增益为1.35 
float Bx[4]={0.000945,0.00189,0.000945,0};
float Ay[4]={1,-1.9112,0.914,0};
*/

/*三阶滤波	系数，采样频面1KHZ，截止频率10Hz,滤波器通带增益为1.165*/
float Bx[4]={0.00002915,0.00008744,0.00008744,0.00002915};
float Ay[4]={1         ,-2.8744   ,2.7565    ,-0.8819};


/*三阶滤波	系数，采样频面1KHZ，截止频率5Hz
float Bx[4]={0.000003757,0.000011271,0.000011271,0.000003757};
float Ay[4]={1         ,-2.9372   ,2.8763    ,-0.9391};
*/
u16 Yout[7]={0,0,0,0,0,0,0};//输出数据 0 传感器1，1 传感器2， 2 母线电压，3 温度，4 分流器1，5 分流器2 ，6温度
float Xin[7][4]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//输入历史数据
float Yin[7][4]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//输出历史数据
//u16 testAD[4]={1000,1500,2000,2500};
void adc::ReadADResultforLowPassfilter3order(void)//滤除输入信号中的工频干扰
{  //本函数每隔1ms执行一次
	      for(u8 i=0;i<7;i++)
				{

					
            //三阶滤波			

					        //Xin[i][3]=testAD[i];
					Xin[i][3]=uADCvalue[i];//AD采样信号输入
					Yin[i][3]=Bx[0]*Xin[i][3]+Bx[1]*Xin[i][2]+Bx[2]*Xin[i][1]+Bx[3]*Xin[i][0]- Ay[1]*Yin[i][2]-Ay[2]*Yin[i][1]-Ay[3]*Yin[i][0];
					
					
					Xin[i][0]=Xin[i][1];  
					Xin[i][1]=Xin[i][2];
					Xin[i][2]=Xin[i][3];
					
					Yin[i][0]=Yin[i][1];
					Yin[i][1]=Yin[i][2];
					Yin[i][2]=Yin[i][3];
					
					if(Yin[i][3]>0)
					Yout[i]=(u16)(Yin[i][3]/1.165);
					
					
				}
				
				 startADC(); 

}










u8 log2( u32 x)
{
	  double a=log((double) x);
	  double c=log((double) 2);
	
	return (u8)(a/c);
}



