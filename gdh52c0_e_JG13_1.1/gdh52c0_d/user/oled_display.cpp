#include "oled_display.h"
//#include "FNT_GB12.h"
//#include "ascii_font.h"
//#include "f1216.h"

//---------------------------GD32F105 usart硬件相关配置----------------------------------------------
/*	
    RES	  29  (PA4)	   GPIO
		SCLK	30  (PA5)	 SPI1_SCK
		D/C	  31  (PA6)	   GPIO
		SDIN	32  (PA7)	 SPI1_M0SI
*/
// D/C	
#define OLED_RS_PORT              GPIOA
#define OLED_RS_PIN               GPIO_PIN_6
#define Set_OLED_RS  {gpio_bit_set(OLED_RS_PORT,OLED_RS_PIN);}
#define Clr_OLED_RS  {gpio_bit_reset(OLED_RS_PORT,OLED_RS_PIN);}
// RES	
#define OLED_RST_PORT              GPIOA
#define OLED_RST_PIN               GPIO_PIN_4
#define Set_OLED_RST  {gpio_bit_set(OLED_RST_PORT,OLED_RST_PIN);}
#define Clr_OLED_RST  {gpio_bit_reset(OLED_RST_PORT,OLED_RST_PIN);}

//#define FONT_1206 0x08040000
//#define FONT_TXT  0x08041260
//#define ASC2_1206 0x08041470
//#define FONT_LEN 261


#define FONT_DATA_BASE 0x08040000
u16 FONT_LEN=*(u16 *)FONT_DATA_BASE;//读取汉字长度

u32 ASC2_1206=FONT_DATA_BASE+8;//ASCII字模起始位置 固定为偏地址后8个字节
u32 FONT_1206=FONT_DATA_BASE+8+95*9;//汉字模起始位置 //总共95个ascii码，每个占9个字节 再加上8字节信息码
u32 FONT_TXT=FONT_DATA_BASE+(*((u16 *)FONT_DATA_BASE+3));//起始位置第4个字节，指定了汉字内码的偏移地址

//*********************合局变量***********************************/
 OLED gOled;


#ifdef __cplusplus
 extern "C" {
#endif

/*SPI时钟及GPIO配置*/
void spi_gpio_config(void)
{
	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_SPI0);
	rcu_periph_clock_enable(RCU_GPIOA);
	
	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_5|GPIO_PIN_7);
	//gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_7);
	
	gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_4|GPIO_PIN_6);
	//gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_7);
	
}

void spi_config(void)
{
	 spi_parameter_struct spi_init_struct;
	 spi_i2s_deinit(SPI0);
	 spi_struct_para_init(&spi_init_struct);
	
	  /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_4;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);
	  spi_enable(SPI0);
}


/*************************************************
名称：Write_cmd
功能描述:OLED写命令
输入:OLED命令
返回:无
*************************************************/
void  Write_cmd(u8  cmd)
{
		Clr_OLED_RS;  //RS 为0时写命令
	
	
	  spi_disable(SPI0);
	  spi_enable(SPI0);
	  spi_i2s_data_transmit(SPI0, cmd);
    while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS)
			       != RESET){};
		Set_OLED_RS; 
}

/*************************************************
名称：Write_data
功能描述:OLED写数据
输入::OLED数据
返回:无
*************************************************/
void  Write_data(u8 dat)
{
		Set_OLED_RS;;  //RS 为1时写数据
	
	   spi_disable(SPI0);
	   spi_enable(SPI0);

	   spi_i2s_data_transmit(SPI0, dat);	
      while(spi_i2s_flag_get(SPI0, SPI_FLAG_TRANS)
			       != RESET){};	
		Set_OLED_RS; 
}

#ifdef __cplusplus
}
#endif


OLED::OLED(){
	   spi_gpio_config();
	   spi_config();
	
		
	
}

void OLED::initSPI(void)
{
	 spi_gpio_config();
	 spi_config();
}


void OLED::delayms(u8 tick)
{ u8 i,j;
	//  m_delayms=0;
//	while(m_delayms<tick)
//	{
//		;
//	}
	for(i=0;i<50;i++)
	{  
		for(j=0;j<200;j++)
	  {  
			;
		}
	}
	
}



void OLED::Init(void)
{
	Clr_OLED_RST;
		delayms(10);
		
		Set_OLED_RST; 
		delayms(10);
		
				  
// 		    Write_cmd(0xa0);  //set seg>        
//        Write_cmd(0xc0);  // seg com>
//        Write_cmd(0x81);  //set vol mode       
//        Write_cmd(0xcc);  // set v5 register
//        Write_cmd(0x27);  // set ra\rb ratio
//        Write_cmd(0x2f);  // set vc vr vf on
//        Write_cmd(0xa2);  // set lcd bias      
//        Write_cmd(0x40);  // set display start line address      
//        Write_cmd(0xaf);   // set lcd display on  


/*	
 Write_cmd(0xae); //Set Display Off  
 Write_cmd(0xd5); //Display divide ratio/osc. freq. mode  
 Write_cmd(0x80);         
 Write_cmd(0xa8); //Multiplex ration mode:63  
 Write_cmd(0x3f); 
 Write_cmd(0xd3); //Set Display Offset 
 Write_cmd(0x00);  
 Write_cmd(0x40); //Set Display Start Line    
 Write_cmd(0xad); //DC-DC Control Mode Set  
 Write_cmd(0x8a); //0X8A: OFF 0X8B: ON 
 
 Write_cmd(0xa0);//设置屏幕翻180,  
 Write_cmd(0xc0); //设置屏幕翻180,       
 Write_cmd(0xda); 
 Write_cmd(0x12); //Common pads hardware: alternative
 Write_cmd(0x81); //Contrast control 
 Write_cmd(0xe0);    
 Write_cmd(0xd9); //Set pre-charge period
 Write_cmd(0x22); 

 Write_cmd(0xdb); //VCOM deselect level mode 
 Write_cmd(0x18); 
 Write_cmd(0xa4); //设置完整显示
 Write_cmd(0xa6); //设置正常显示
 Write_cmd(0xaf); //设置显示开
*/
	
	//翼龙达
	Write_cmd(0xAE);//set display display ON/OFF,AFH/AEH
	
	Write_cmd(0x40);//set display start line:COM0

	Write_cmd(0x81);//set contrast control
	Write_cmd(0x80);	

	Write_cmd(0xA0);//set segment re-map

	Write_cmd(0xA4);//entire display on: A4H:OFF/A5H:ON

	Write_cmd(0xA6);//set normal/inverse display: A6H:normal/A7H:inverse

	Write_cmd(0xA8);//set multiplex ratio
	Write_cmd(0x3F);//1/64duty

	Write_cmd(0xC0);//set com output scan direction

	Write_cmd(0xAD);//DC-DC Control Mode Set
    Write_cmd(0x8A);//DC-DC ON/OFF  Mode Set:ON

	//Write_Command(0x33);//Set Pump voltage value,VPP=9.0V

	Write_cmd(0xD3);//set display  offset
	Write_cmd(0x00);//

	Write_cmd(0xD5);//set display  clock divide ratio/oscillator frequency
	Write_cmd(0x80);//105Hz

  Write_cmd(0xD9);//Dis-charge /Pre-charge Period Mode Set
	Write_cmd(0x1F);//

	Write_cmd(0xDA);//Common Pads Hardware Configuration Mode Set
	Write_cmd(0x12);//

  Write_cmd(0xDB);//set vcomh deselect level
	Write_cmd(0x40);//VCOM = β X VREF = (0.430 + A[7:0] X 0.006415) X VREF 

	Write_cmd(0xAF);//set display display ON/OFF,AEH/AFH
	
	
	
	
	
	
	
		    Clear();
	     Refresh_Gram(); 
}

/*************************************************
名称：GetCNFontId
功能描述: 通过汉字内码，查找字模位置
输入:  pid:汉字内码指针
返回:字模位置 返回-1为找到不字模
*************************************************/
//s16 OLED::GetCNFontId(u8 *pid)
//{
//u16	i;
// u8 l,h,j,k;
//	   l=*pid;
//	    pid++;
//	   h=*pid;
// for(i=0;i<sizeof(GB_12)/sizeof(typFNT_GB12);i++)
// {
//	 j=GB_12[i].Index[0];
//	 k=GB_12[i].Index[1];
//   if((l==j)&&(h==k))
//		{
//			 return i;
//		}
// }
// return -1;
//}


s16 OLED::GetCNFontId2(u8 *pid)
{

u16	i,*strHz;
 u8 l,h,j,k;
	     // strHz=(u16 *)fontText;
	strHz=(u16 *)FONT_TXT;
	   l=*pid;
	    pid++;
	   h=*pid;
// for(i=0;i<(sizeof(fontText)-1)/2;i++)
	for(i=0;i< FONT_LEN;i++)
 {
   
	    j=(u8)strHz[i];
	    k=(u8)(strHz[i]>>8);
	  
	 
   if((l==j)&&(h==k))
		{
			 return i;
		}
 }
 return -1;
 
 
 /*	
	u16 sp;
    s16 mp;
	u16 ep;
	u16 hz;
	u16 hzl;

	
	hz= (pid[0]<<8)+pid[1];	
	sp=0;

	ep=(sizeof(fontText)-1)/2;
	mp=-1;
	while (sp<ep){
		mp=(sp+ep)/2;
		hzl= (fontText[mp*2]<<8)+fontText[mp*2+1];
		if(hz<hzl) ep=mp;
		else if(hz>hzl) sp=mp;
		else break;
	}

	return mp;
	
 */
}




/*************************************************
名称：OLED_Clear
功能描述:清显示缓存，调用OLED_Refresh_Gram后，清屏
输入:
返回:无
*************************************************/
void OLED::Clear(void)  
{  
	u8 i,n;  
	for(i=0;i<8;i++)
	{
		for(n=0;n<128;n++) //128
		{
	    	GRAM[n][i]=0X00; 
		} 
	}
}

/*************************************************
名称：OLED_DrawPoint
功能描述: 在显示缓存指定的坐标上画点,调用
OLED_Refresh_Gram后，在屏幕上画点
输入:  x:0~127,y:0~63,t:1 填充 0,清空	
返回:无
*************************************************/
void OLED:: DrawPoint(u8 x,u8 y,u8 t)
{
	u8 pos,bx,temp=0;
	if(x>127||y>63)return;//超出范围了.
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)GRAM[127-x][pos]|=temp;
	else GRAM[127-x][pos]&=~temp;	    
}

/*************************************************
名称：OLED_ShowChar
功能描述: 在显示缓存指定的坐标上显示一个字符,调用
OLED_Refresh_Gram后，在屏幕上显示一个字符
输入:  x:0~127,y:0~63,t:1 正常显示0,反白显示,
size:选择字体 16/12	
返回:无
*************************************************/
void OLED::ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode)
{ 
	/*
	u8 temp,t,t1;
	u8 y0=y;
	chr=chr-' ';//得到偏移后的值
			   
	for(t=0;t<size;t++)
		{   
			if(size==12)
			{
		  	 //调用1206字体
			//temp=asc2_1206[chr][t];
				temp=*((u8*)ASC2_1206+12*chr+t);
				
				
			}
			else 
			{
		    		 //调用1608字体
				//temp=asc2_1608[chr][t];	
			} 	                          
			for(t1=0;t1<8;t1++)
				{
					if(temp&0x80)
					 DrawPoint(x,y,!mode);
					else  
					  DrawPoint(x,y,mode);
					temp<<=1;
					y++;
					if((y-y0)==size)
						{
						y=y0;
						x++;
						break;
						}
				} 		 	 
		} 
		*/
		

  u8 temp,t,t1,x0=x;
	u8 y0=y;
	   chr=chr-0x20;
    if(chr>94)	return;//0~94共95个字模
	//////////////////////字模上半部分 宽6点，高8点/////////////////////////////
    for(t=0;t<6;t++)
    {   	  y=y0;//纵坐标复位
			    // temp=*(u8 *)(&font12table[fno*18]+t);
			      temp=*((u8 *)ASC2_1206+9*chr+t);
						for(t1=0;t1<8;t1++)
						{
							if(temp&0x80)DrawPoint(x,y,!mode);
							else DrawPoint(x,y,mode);
							temp<<=1;
								y++;
						}  
						x++;	
						
    }
		//////////////////////字模下半部分 宽6点，高4点////////////////////////////
   x=x0;
		for(;t<9;t++)
		{     y=y0+8;////纵坐标指向下半位置
			      //temp=*(u8 *)(&font12table[fno*18]+t);
			     temp=*((u8 *)ASC2_1206+9*chr+t);
			    for(t1=0;t1<8;)//t1，不能置于循环括号里加加
						{
							if(temp&0x80)DrawPoint(x,y,!mode);
							else DrawPoint(x,y,mode);
							temp<<=1;
							y++;
							t1++;//若t1置于循环括号里加加，下面的判断就错过一个循环周期
							if(t1==4)//如果取字模超半个字节，则
							{
								y=y0+8;//纵坐标复位
								x++;//模坐档加1
							}
						} 
            x++;//完成一个字节取模模坐标加1		
			}
			
}

void OLED:: ShowFont2(u8 x,u8 y,u16 fno,u8 mode)
{
	u8 temp,t,t1,x0=x;
	u8 y0=y;
     if(fno==0xffff)	return;
	//////////////////////字模上半部分 宽12点，高8点/////////////////////////////
    for(t=0;t<12;t++)
    {   	  y=y0;//纵坐标复位
			    // temp=*(u8 *)(&font12table[fno*18]+t);
			      temp=*((u8 *)FONT_1206+18*fno+t);
						for(t1=0;t1<8;t1++)
						{
							if(temp&0x80)DrawPoint(x,y,!mode);
							else DrawPoint(x,y,mode);
							temp<<=1;
								y++;
						}  
						x++;	
						
    }
		//////////////////////字模下半部分 宽12点，高4点////////////////////////////
   x=x0;
		for(;t<18;t++)
		{     y=y0+8;////纵坐标指向下半位置
			      //temp=*(u8 *)(&font12table[fno*18]+t);
			     temp=*((u8 *)FONT_1206+18*fno+t);
			    for(t1=0;t1<8;)//t1，不能置于循环括号里加加
						{
							if(temp&0x80)DrawPoint(x,y,!mode);
							else DrawPoint(x,y,mode);
							temp<<=1;
							y++;
							t1++;//若t1置于循环括号里加加，下面的判断就错过一个循环周期
							if(t1==4)//如果取字模超半个字节，则
							{
								y=y0+8;//纵坐标复位
								x++;//模坐档加1
							}
						} 
            x++;//完成一个字节取模模坐标加1						
		}




		
}



void OLED:: ShowFont(u8 x,u8 y,u16 fno,u8 mode)
{
//	u8 temp,t,t1;
//	u8 y0=y;
//     if(fno==0xffff)	return;
//    for(t=0;t<18;t++)
//    {   
//		temp=GB_12[fno].Msk[t];  //调用22*22汉字	 
//		 // temp=tab[t];                        
//					for(t1=0;t1<8;t1++)
//					{
//						if(temp&0x80)DrawPoint(x,y,!mode);
//						else DrawPoint(x,y,mode);
//						temp<<=1;
//						y++;
//						if((y-y0)==12)
//						{
//							y=y0;
//							x++;
//							break;
//						}
//					}  	 
//    }       
}

void OLED::Show_all(u8 x,u8 y,u8 *p,u8 size,u8 mode)
{
//  #define MAX_CHAR_POSX 122
//	#define MAX_CHAR_POSY 58 	 
//	s16 id;
//	//u16 addr;	
//	u8 qh;//,ql; 
//	while(*p!='\0')
//	{
//	   qh=*p;
//	   if(qh>160)
//	   {
//	    //ql=*(++p);
//			 id=GetCNFontId2(p);
//			  ++p;
//	   }
//	   if(qh<160)
//	   {
//		   if(x>MAX_CHAR_POSX)
//			{
//				 x=0;
//				 y+=16;
//			
//				if(y>MAX_CHAR_POSY)
//				{
//				 y=x=0;
//				 Clear();
//				}
//			}
//			if(size==12)
//			ShowChar(x,y,*p,12,mode);
//	 
//			x+=8;
//			p++;	
//	   
//	   }
//	   if(qh>160)
//	   {
//	 
//       
//				if(x>MAX_CHAR_POSX){x=0;y+=16;}
//				if(y>MAX_CHAR_POSY){y=x=0;Clear();}
//				if(size==12)
//				{	 
//  					if(id>-1)
//						ShowFont(x,y,id,mode);
//						x+=12;
//				}

//		p++;
//	   }

//	}
//				if(qh<160)
//				{
//					x-=8;
//						p--;
//				}
//			 if(qh>160)
//			 {
//					if(size==12)
//					x-=12;
//				if(size==16)
//					x-=16;
//				p--;
//			 }
}

void OLED::Show_all2(u8 x,u8 y,u8 *p,u8 size,u8 mode)
{
  #define MAX_CHAR_POSX 122
	#define MAX_CHAR_POSY 58 	 
	s16 id;
	//u16 addr;	
	u8 qh;//,ql; 
	while(*p!='\0')
	{
	   qh=*p;
	   if(qh>160)
	   {
	    //ql=*(++p);
			 id=GetCNFontId2(p);
			  ++p;
	   }
	   if(qh<160)
	   {
		   if(x>MAX_CHAR_POSX)
			{
				 x=0;
				 y+=16;
			
				if(y>MAX_CHAR_POSY)
				{
				 y=x=0;
				 Clear();
				}
			}
			if(size==12)
			ShowChar(x,y,*p,12,mode);
	 
			x+=8;
			p++;	
	   
	   }
	   if(qh>160)
	   {
	 
       
				if(x>MAX_CHAR_POSX){x=0;y+=16;}
				if(y>MAX_CHAR_POSY){y=x=0;Clear();}
				if(size==12)
				{	 
  					if(id>-1)
						ShowFont2(x,y,id,mode);
						x+=12;
				}

		p++;
	   }

	}
				if(qh<160)
				{
					x-=8;
						p--;
				}
			 if(qh>160)
			 {
					if(size==12)
					x-=12;
				if(size==16)
					x-=16;
				p--;
			 }
}


void OLED::Refresh_Gram(void)
{
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		Write_cmd(0xb0+i);    //设置页地址（0~7）
		Write_cmd(0x02);      //设置显示位置—列低地址,偏移了2列
		Write_cmd(0x10);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)Write_data(GRAM[n][i]); //128
	}   
}

void OLED::Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot)
{
	u8 x,y;  
	for(x=x1;x<=x2;x++)
		{
		for(y=y1;y<=y2;y++)DrawPoint(x,y,dot);
		}	
}	
void  OLED::clrScreen(void)
{
	  Fill(0,0,127,63,0);
}
