#include "gpio.h"
u8 resetCount=0;//按ESC 5后复位
u8 specilCount=0;//按up 出现特殊菜单
u8 specilCount2=0;//按down 重启显示

//--------------------硬件定义相关----------------------------------------------
Ts_gpio  inputIO[16]={      GPIOC,GPIO_PIN_3,  // READ_DI0
                            GPIOC,GPIO_PIN_2,  // READ_DI1
                            GPIOC,GPIO_PIN_1,  // READ_DI2
                            GPIOC,GPIO_PIN_0,  // READ_DI3
                            GPIOC,GPIO_PIN_13, // READ_DI4
                            GPIOE,GPIO_PIN_6,  // READ_DI5
														GPIOE,GPIO_PIN_5,  // READ_DI6
														GPIOE,GPIO_PIN_4,  // READ_DI7
														GPIOE,GPIO_PIN_3,  // READ_DI8
														GPIOE,GPIO_PIN_2,  // READ_DI9
														GPIOE,GPIO_PIN_1,  // READ_DI10
														GPIOE,GPIO_PIN_0,   // READ_DI11
	////////////////////////////////////key/////////////////////////////////////
	                          GPIOD,GPIO_PIN_11,  //KEY_OK
	                          GPIOD,GPIO_PIN_12,  //KEY_DOWN
	                          GPIOD,GPIO_PIN_13,  //KEY_UP
	                          GPIOB,GPIO_PIN_0   //KEY_ESC
	
	
													  };

														


Ts_gpio outputIO[15]={      GPIOD,GPIO_PIN_0,   //Relayer0
                            GPIOD,GPIO_PIN_1,   //Relayer1
                            GPIOD,GPIO_PIN_2,   //Relayer2
                            GPIOD,GPIO_PIN_3,   //Relayer3
                            GPIOD,GPIO_PIN_4,   //Relayer4
                            GPIOB,GPIO_PIN_8,   //Relayer5
														GPIOE,GPIO_PIN_9,   //OC6
														GPIOE,GPIO_PIN_10,  //OC7
														GPIOE,GPIO_PIN_11,  //OC8
														GPIOE,GPIO_PIN_12,  //OC9
														GPIOE,GPIO_PIN_13,  //OC10
														GPIOE,GPIO_PIN_14,  //OC11
	                          GPIOD,GPIO_PIN_14,  //WARN_LED 
	                          GPIOB,GPIO_PIN_9,   //RUN_LED
	                          GPIOB,GPIO_PIN_15   //BEE
													  };

														
														
														
														
														
														

														
//----------------------全局变量														
gpio gIO;														


//构造函数 初始化gpio
														
gpio::gpio()
{
	   
	

	    
			 rcu_periph_clock_enable(RCU_GPIOB);
			 rcu_periph_clock_enable(RCU_GPIOC);
			 rcu_periph_clock_enable(RCU_GPIOD);
			 rcu_periph_clock_enable(RCU_GPIOE);
	      // GPIO时钟初使能后，才可设置GPIO相关寄存器
	     set(BEE,BEE_OFF);//IO初始化前将蜂鸣器关掉
	     set(OC9,OFF);    //IO初始化前将用户1下电禁用
	     set(OC10,OFF);   //IO初始化前将用户2下电禁用
	     set(OC11,OFF);   //IO初始化前将电池下电禁用
	     set(WARN_LED,LED_OFF);   //IO初始化前告警灯灭
	
	
	      for(u8 i=0;i<16;i++)
	      gpio_init(inputIO[i].gpio_periph,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,inputIO[i].pin);
	
	
	      for(u8 i=0;i<15;i++)
	      gpio_init(outputIO[i].gpio_periph,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,outputIO[i].pin);
	
	
	     m_vkey=KEYNONE;
			 
			 for(u8 i=0;i<4;i++)
			 {
			    keyCtrl[i].presskey=0;
				  keyCtrl[i].keyConut=0;
			 }
	
	
}

//gpio输出
//pin 取值，见头文件 IO pin name output
//_status 取值，见头文件 IO pin status
void gpio::set(u8 pin,u8 _status)
{
	
	if(_status==0)
	{gpio_bit_reset(outputIO[pin].gpio_periph, outputIO[pin].pin);}
	else
	{gpio_bit_set(outputIO[pin].gpio_periph, outputIO[pin].pin);}	
	
	
}

//取得IO状态
//pin 取值，见头文件 IO pin name input
//返回 RESET or SET
FlagStatus gpio::get(u8 pin)
{
	
	 return  gpio_input_bit_get(inputIO[pin].gpio_periph,inputIO[pin].pin);
	
}

//gpio输出取反
//pin 取值，见头文件 IO pin name output
//_status 取值，见头文件 IO pin status
void gpio::setOnOff(u8 pin)
{
	  gpio_bit_write(outputIO[pin].gpio_periph,
	                  outputIO[pin].pin, 
	( bit_status)(1-gpio_input_bit_get(outputIO[pin].gpio_periph,outputIO[pin].pin)));
}

void gpio::scankey(u16 _delay)
{
	
			
//			if(m_delay<_delay) return;
//			 m_delay=0;
			
			if(keyCtrl[0].presskey<1)	
		{
			//if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_11) == 0 )	 //按下OK键
			if(get(KEY_OK)==DOWN)
			 {	
								 keyCtrl[0].presskey++;
						
			 }
			 else
			 {
					keyCtrl[0].presskey=0;
			 }
		 }
		else
		{
			
			//if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_11) ==1 )	 //释放OK键触发
			 if(get(KEY_OK)==UP)
			 {	
								 m_vkey=KEYENTER;
									keyCtrl[0].presskey=0;
						
			 }
			 
			
		}

		if(keyCtrl[1].presskey<1)	
		{
			//if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12) == 0 )	 //按下下键
			 if(get(KEY_DOWN)==DOWN)
			 {	
								 keyCtrl[1].presskey++;
						
			 }
			 else
			 {
					keyCtrl[1].presskey=0;
			 }
		 }
		else
		{
			
			//if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12) ==1 )	 //释放下键触发
			 if(get(KEY_DOWN)==UP)
			 {	
								 m_vkey=KEYDOWN;        
								 keyCtrl[1].presskey=0;
				         keyCtrl[1].keyConut=0;
				         specilCount2=0;
			 }
			 
						 keyCtrl[1].keyConut++;
					 if(keyCtrl[1].keyConut==10)//按下连继按键触发
					 {
						 m_vkey=KEYDOWN;
						 keyCtrl[1].keyConut=0;
						 specilCount2++;
					 }

			 
			
		}


		if(keyCtrl[2].presskey<1)	
		{
			//if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_13) == 0 )	 //按下上键
			 if(get(KEY_UP)==DOWN)
			 {	
								 keyCtrl[2].presskey++;
						
			 }
			 else
			 {
					keyCtrl[2].presskey=0;
			 }
		 }
		else
		{
			
			//if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_13) ==1 )	 //释放上键触发
			if(get(KEY_UP)==UP)
			 {	
								 m_vkey=KEYUP;
								 keyCtrl[2].presskey=0;
				          keyCtrl[2].keyConut=0;
				          specilCount=0;
				         
			 }
			 
					 keyCtrl[2].keyConut++;
					 if(keyCtrl[2].keyConut==10)//按下连继按键触发
					 {
						 m_vkey=KEYUP;
						 keyCtrl[2].keyConut=0;
						 specilCount++;
					 }
			 
			
		}


		if(keyCtrl[3].presskey<1)	
		{
			//if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) == 0 )	 //按下ESC键
			if(get(KEY_ESC)==DOWN)
			 {	
								keyCtrl[3].presskey++;
						
			 }
			 else
			 {
				 keyCtrl[3].presskey=0;
			 }
		 }
		else
		{
			
			//if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) ==1 )	 //释放ESC键触发
			 if(get(KEY_ESC)==UP)
			 {	
								 m_vkey=KEYESC;
								 keyCtrl[3].presskey=0;
				          keyCtrl[3].keyConut=0;
				         resetCount=0;
			 }
			 
			  keyCtrl[3].keyConut++;
					 if(keyCtrl[3].keyConut==10)//按下连继按键触发
					 {
						 //m_vkey=KEYUP;
						 keyCtrl[3].keyConut=0;
						 resetCount++;
					 }
			 
			
		}
}



