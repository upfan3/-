//#include "powercan.h"
#include "globalval.h"
#include "ydt1363.h"

#include "Staggeredpeak.h"

extern u8 acAlarmV[8],AlarmVolitage,mod,galram;
extern u8 ModuleCap;
extern  Item * pSetBranchParaMenu;
extern u8 gEnergySle;
extern u32 acDCEnergydata[6];
//u8 preBattDownMod=0x19;//默认禁用前d为电压下电
u8 preUserDownMod[8]={0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
u8 prebrachMod[32]={0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	                  0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	                  0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
	                  0x10,0x10};
	
u8 * pgetE2Databuff=NULL;
u8 getE2D2=0;
u8 setE2D3=0;
u8 saveE1D3=0;
u8 saveE2D3=0;
u8 save40D7=0;
u8 save4248=0;
u8 setCalibrationI=0;
u8 setCalibrationV=0;
//u8 tenant[10]={1,2,3,4,0,0,0,0,0,0};//配电中的租户分配
u8 tenantcount[10];//不同租户的个数统计.tenantcount[0]储存用作租户1的配电个数，如此类推 tenantcount[2....]
//协议转换（监控告警与《H52C0 监控模块上位机 1363 标准通信协议》之间的告警映射）
const u8 InputDItoWeb[13]={4,6,5,3,12,11,8,9,7,2,1,10,0};//0门禁4，1水浸6，2烟雾5，3防雷3，4风机12，5空调11，6市电8，7油机9，8防雷开关7，9电池熔丝2，10负载熔丝（1）1，11负载熔丝（2）[燃料不足]10，12无0，
const u8 WebtoInputDI[13]={12,10,9,3,0,2,1,8,6,7,11,5,4};
u8 dataflag=0x10;
u8  moduledatatmp3[10];
u8  moduledatatmp4[10];
u8  moduledatatmp5[10];
u8  WorkingStatus[10];
//u16 gAlarmRecord[5]={0,0,0,0,0};
u16 gAlarmRecord[2]={0,0};
u8 nomal=0;
u16 _NONE=0;

u8 battCellVoltAlarm[]={0,0,0,0};
u8 battCellFaultAlarm[]={0,0,0,0};
u8 battCellTAlarm[]={0,0,0,0};




//将小端模式数据转为大端模式数据
u16 LittleToBig(u16 val)
{
	u8 *pval=(u8 *)&val;
	  return (u16)((pval[0]<<8)+pval[1]);
}

void warnChange(u8 * pdat,WarnStruct *pwra)
{
	
	if((pwra->wtype==IN_VA)||(pwra->wtype==IN_VB)||(pwra->wtype==IN_VC))//交流电压
	{
		 
		if(pwra->behavior==LOW_)
		{
			pdat[6]=0;//低 
		}
		else if(pwra->behavior==HIGH_)
		{
			pdat[6]=1;//高 
		}
		pdat[7]=pwra->wtype-2;//市电//3,4,5 对应1,2,3 A,B,C
		pdat[8]=0x01;
		pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	
	else if((pwra->wtype==IN_IA)||(pwra->wtype==IN_IB)||(pwra->wtype==IN_IC))//交流电流
	{
		 
	 if(pwra->behavior==HIGH_)
		{
			pdat[6]=2;//高 
		}
//		pdat[7]=0x10+pwra->wtype-5;//市电//3,4,5 对应1,2,3 A,B,C
//		pdat[8]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
			pdat[7]=pwra->wtype-5;//市电//3,4,5 对应1,2,3 A,B,C
		pdat[8]=0x01;
		pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if(pwra->wtype==FRQ)//频率
	{
		if(pwra->behavior==LOW_)
		{
			pdat[6]=4;//低 
		}
		else if(pwra->behavior==HIGH_)
		{
			pdat[6]=3;//高 
		}
//			pdat[7]=0x10;//市电
//		  pdat[8]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
		pdat[7]=0;//与相位无关
		pdat[8]=0x01;
		pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if(pwra->wtype==ACINPUT)//市电输入，防雷开关
	{
		pdat[6]=8;
//		pdat[7]=0x10;//市电
//		pdat[8]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
		pdat[7]=0;//与相位无关
		pdat[8]=0x01;
		pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if(pwra->wtype==OILINPUT)//油机输入
	{
		pdat[6]=5;
//		pdat[7]=0x20;//油机
//		pdat[8]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
		pdat[7]=0;//与相位无关
		pdat[8]=0x02;
		pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if(pwra->wtype==SPD||pwra->wtype==SPDSW)//防雷
	{
		pdat[6]=6;
//		pdat[7]=0x10;//市电
//		pdat[8]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
		pdat[7]=0;//与相位无关
		pdat[8]=0x01;
		pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}/////////////////以上为交流瓶告警//////////////////////////////////////		
	else if(pwra->wtype==MODULE)//模块相关告警
	{
		if(pwra->behavior==COMM_BROKEN)
		{
			pdat[6]=4;//高 
		}
		else if(pwra->behavior==BROKEN)
		{
			pdat[6]=6;//高 
		}			
		
			pdat[7]=pwra->nb;//模块号
		  pdat[8]=0;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}/////////////////以上为整流模块告警//////////////////////////////////////
	else if(pwra->wtype==OUT_V)//输出电压
	{
			if(pwra->behavior==LOW)
		{
			pdat[6]=0;//低 
		}
		else if(pwra->behavior==HIGH)
		{
			pdat[6]=1;//高 
		}
		
			pdat[7]=0;//
		  pdat[8]=0;//
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if(pwra->wtype==BATTFUSR_BREAK)//电池熔丝
	{ 
		 pdat[6]=2;
		  pdat[7]=0x0;//
		  pdat[8]=0x0;//
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if((pwra->wtype==LOAD_FUSE1)||(pwra->wtype==LOAD_FUSE2))//负载熔丝
	{ 
		  pdat[6]=3;
		  pdat[7]=pwra->wtype-30;//负载分路
		  pdat[8]=0x02;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if(pwra->wtype==BATTDOWN_ALARM)//电池下电
	{ 
		  pdat[6]=5;
		  pdat[7]=0;//
		  pdat[8]=0;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if(pwra->wtype==MONITOR_BRANCH)//主分路下电
	{ 
		  pdat[6]=6;
		  pdat[7]=pwra->nb;//
		  pdat[8]=0x02;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	  else if(pwra->wtype==BATT_TEMP)//电池温度
	{
		if(pwra->behavior==HIGH)
		{
			pdat[6]=8;//高 
		}
		else if(pwra->behavior==LOW)
		{
			pdat[6]=7;//低
		}
		
			pdat[7]=0;
		  pdat[8]=0;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}/////////////////以上为直流瓶告警//////////////////////////////////////
	 else if(pwra->wtype==DOOR_OPEN)//门禁
	{
		
			pdat[6]=4; 
		
		
			pdat[7]=0;
		  pdat[8]=0;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	 else if(pwra->wtype==SMOKE)//烟雾
	{
		
			pdat[6]=5; 
		
		
			pdat[7]=0;
		  pdat[8]=0;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	else if(pwra->wtype==WATER_INVASION)//水浸
	{
		
			pdat[6]=6; 
		
		
			pdat[7]=0;
		  pdat[8]=0;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}/////////////////以上为系统告警//////////////////////////////////////
	
	
	
  else if(pwra->wtype==USER_DOWN)//用户下电
	{
		  pdat[6]=0;
		  pdat[7]=pwra->nb;//
		  pdat[8]=0;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}
	 else if(pwra->wtype==USER_BREAK)//用户熔丝
	{
		  pdat[6]=1;
		  pdat[7]=pwra->nb;//
		  pdat[8]=0;
		  pdat[9]=1-pwra->wstatus;//页面显示与wstatus刚好相反，因此用1减
	}


}




u16 gydtdataflag[2]={0xffff,0xffff};

u8 getDataFlag(u8 cid1,u8 cid2,u8 *port)
{u8 tmp;
  ydtdataflag *pdataflag=NULL;
	
       if(*port==2)
			 {
				   pdataflag=(ydtdataflag *)(&gydtdataflag[0]);//485中端口的dataflag
			 }
			 else if(*port==3)
			 {
				   pdataflag=(ydtdataflag *)(&gydtdataflag[1]);//网络中端口的dataflag
			 }
			 else{  //端口不对直接返回
				  return 0;
			 }
				 



	    if(cid1==0x40)
			{
				  
				tmp= (pdataflag->st_4043_dataflag<<4)|(pdataflag->st_4044_dataflag);
				
				 if(cid2==0x43)
				 {
					
					 pdataflag->st_4043_dataflag=0;
					 
				 }
				 else if(cid2==0x44)
				 {
					 pdataflag->st_4044_dataflag=0;
				 }
				 
				
				
			}
	    else if(cid1==0x41)
			{
				
				tmp= (pdataflag->st_4143_dataflag<<4)|(pdataflag->st_4144_dataflag);
				
				
				   if(cid2==0x43)
				 {
					
					 pdataflag->st_4143_dataflag=0;
					 
				 }
				 else if(cid2==0x44)
				 {
					 pdataflag->st_4144_dataflag=0;
				 }
				
			}
			else if(cid1==0x42)
			{
				tmp= (pdataflag->st_4243_dataflag<<4)|(pdataflag->st_4244_dataflag);
				  if(cid2==0x43)
				 {
					
					 pdataflag->st_4243_dataflag=0;
					 
				 }
				 else if(cid2==0x44)
				 {
					 pdataflag->st_4244_dataflag=0;
				 }
			}
			else if(cid1==0x43)
			{
				tmp= (pdataflag->st_4343_dataflag<<4)|(pdataflag->st_4344_dataflag);
				  if(cid2==0x43)
				 {
					
					 pdataflag->st_4343_dataflag=0;
					 
				 }
				 else if(cid2==0x44)
				 {
					 pdataflag->st_4344_dataflag=0;
				 }
			}
	     	else if(cid1==0xE1)
			{
				
				tmp= (pdataflag->st_E143_dataflag<<4)|(pdataflag->st_E144_dataflag);
				  if(cid2==0x43)
				 {
					
					 pdataflag->st_E143_dataflag=0;
					 
				 }
				 else if(cid2==0x44)
				 {
					 pdataflag->st_E144_dataflag=0;
				 }
			}
			else if(cid1==0xE2)
			{
				
				tmp= (pdataflag->st_E243_dataflag<<4)|(pdataflag->st_E244_dataflag);
				    if(cid2==0x43)
				 {
					
					 pdataflag->st_E243_dataflag=0;
					 
				 }
				 else if(cid2==0x44)
				 {
					 pdataflag->st_E244_dataflag=0;
				 }
			}
	
	return tmp;
}

u8 AsciitoHex(u8 *op,u8* pdat)
{ u8 i=2,tmp;
	  if(*pdat<0x30||*pdat>0x46) return 0;
	    if(*(pdat+1)<0x30||*(pdat+1)>0x46) return 0;
	   *op=0;
	while(i--)
	{
	    if( *pdat<0x40)
			{
				  tmp=*pdat-0x30;
				
			}
			if(*pdat>0x40)
			{
				tmp=*pdat-0x37;
			}
			   pdat++;
			
			  *op=*op|(tmp<<(i*4));
		}
	  return 1;
}
//将4个ACSII合成为一个16位数 数据为0~9 A~F
u16 AsciitoHex16(u8* pdat)//16位ASCii码转为16进制码
{ u8 i=4,tmp;
	  if(*pdat<0x30||*pdat>0x46) return 0;
	    if(*(pdat+1)<0x30||*(pdat+1)>0x46) return 0;
	      if(*(pdat+2)<0x30||*(pdat+2)>0x46) return 0;
	         if(*(pdat+3)<0x30||*(pdat+3)>0x46) return 0;
	   u16 result=0;
	while(i--)
	{
	    if( *pdat<0x40)
			{
				  tmp=*pdat-0x30;
				
			}
			if(*pdat>0x40)
			{
				tmp=*pdat-0x37;
			}
			   pdat++;
			
			  result=result|(tmp<<(i*4));
		}
	  return result;
}

u32 AsciitoHex32(u8* pdat)//32位ASCii码转为16进制码
{ u8 i=8,tmp;
	  if(*pdat<0x30||*pdat>0x46) return 0;
	    if(*(pdat+1)<0x30||*(pdat+1)>0x46) return 0;
	      if(*(pdat+2)<0x30||*(pdat+2)>0x46) return 0;
	         if(*(pdat+3)<0x30||*(pdat+3)>0x46) return 0;
	if(*(pdat+4)<0x30||*(pdat+4)>0x46) return 0;
	    if(*(pdat+5)<0x30||*(pdat+5)>0x46) return 0;
	      if(*(pdat+6)<0x30||*(pdat+6)>0x46) return 0;
	         if(*(pdat+7)<0x30||*(pdat+7)>0x46) return 0;
	   u32 result=0;
	while(i--)
	{
	    if( *pdat<0x40)
			{
				  tmp=*pdat-0x30;
				
			}
			if(*pdat>0x40)
			{
				tmp=*pdat-0x37;
			}
			   pdat++;
			
			  result=result|(tmp<<(i*4));
		}
	  return result;
}





u8 HextoAscii(u8 *op,u8* pdat)//16进制码A转为ASCii码
{ u8 i=2,tmp;
	
	 while(i--)
	{
	   tmp=(*pdat>>(4*i))&0x0f;
			if(tmp<10)
			{
				*op=tmp+0x30;
			}
			else
	    {
				*op=tmp+0x37;
			}
			  op++;
	    
	}
	    return 0;
}


u16 GetPackLen(ytd1363data *pytd,u16 length,u16 *plen)//取得数据包长度
{
	  u16 len=0,typelen=1;
    u16 datalen=length ;
	

	
	    while(pytd[len].st_type!=0)
			{
				u8 looptimes=pytd[len].len&0x3f;
				u8 type=pytd[len].st_type&0x07;
				       if((type==TYPE_FOURBYTE)||(type==TYPE_FOURBYTE_L))
							 {
								  typelen=4;
							 }
							 else if(type==TYPE_TWOBYTE)
							 {
								  typelen=2;
							 }
							 else
				       {
								  typelen=1;
							 }
				
				
				
				if((pytd[len].st_type&TYPE_EXTERN)==TYPE_EXTERN)
				{
				     ytd1363data *pytd2= (ytd1363data *)pytd[len].pdata;
					       if(pytd2==NULL) 
									 return 0;
						   for(u8 i=0;i<looptimes;i++)
						  {
						    datalen=GetPackLen(pytd2,datalen,NULL);
						  }
						    datalen+=2;
							
				}
				else if((looptimes>1)&&(pytd[len].pdata!=NULL))
				{ 
					
				
						     datalen+=typelen*2*looptimes+2;//计算扩展长度，加2为标记扩展数位置的长度
					
				}
				else
				{
				      	datalen+=typelen*2;//计算一般数据长度
					
				}
				  len++;

			}
	
	      if(plen!=NULL)
					*plen=len;
					
	return datalen;

}


u8 *pgtestbuff=NULL;
u8 * Getdata(ytd1363data *pytd ,u8 kp,u8  *pdatbase,u16 *plen2)//将1363数据包转换为待发送数据
{u8 *pdat=pdatbase ;
 u16 datalen=0, datalen2=0,len=0,base=0;


    if(pytd==NULL) return NULL;
		datalen=GetPackLen(pytd,0,&len);
		
		if(plen2!=NULL)
    *plen2=datalen;
	
			
			//缓存区空间计算
	if(pdatbase==NULL){		
	      pdat=(u8 *)pvPortMalloc(datalen+1);//
		   pgtestbuff=pdat;
       if(pdat==NULL) return NULL;
	      memset(pdat,0x00,datalen+1);
	   }
			for(u16 i=0;i<len;i++)
			{
				
				u8 looptimes=pytd[i].len&0x3f;
				if(pytd[i].pdata==NULL)
							{//填入自定义数 P
							   if((pytd[i].len&0x80)==0)
								 {
											HextoAscii(&pdat[base],&looptimes);
											base+=2;
								 }
								 else  // 无有效内容，填充空格（0x20）  pytd[i].len=(1xxx xxxx)二进制数
								 {    
									     u8 type=pytd[i].st_type&0x07;
									      if(type==TYPE_ONEBYTE)
												{
													 pdat[base]=0x20;
													 pdat[base+1]=0x20;
													 base+=2;
												}
									      if(type==TYPE_TWOBYTE)
												{
													  for(u8 j=0;j<2; j++) 
													  { pdat[base]=0x20;
															pdat[base+1]=0x20;
													    base+=2;
														}
													  
												}
												if((type==TYPE_FOURBYTE)||(type==TYPE_FOURBYTE_L))
												{
													    for(u8 j=0;j<4; j++) 
													  { pdat[base]=0x20;
															pdat[base+1]=0x20;
													    base+=2;
														}
												}
									 
									 
								 }
								continue;
							}  
					
			  if((looptimes>1)||(looptimes==0)||((pytd[i].st_type&TYPE_EXTERN)==TYPE_EXTERN))
							{//填入扩展数
								HextoAscii(&pdat[base],&looptimes);
								base+=2;
							}
				
				
				
				    if((pytd[i].st_type&TYPE_EXTERN)==0)//扩展循环
				    {
							
							
							
							
							for(u8 k=0;k<looptimes;k++)
							{
								u32 tmpu32;
								  u8 typelen;
								  u8 type=pytd[i].st_type&0x07;
								      if((type==TYPE_FOURBYTE)||(type==TYPE_FOURBYTE_L))typelen=4;
								 else  if(type==TYPE_TWOBYTE)typelen=2;
								 else  typelen=1;
								
									
							   if(typelen==1) 
							  {  
										
										  u8 bittype=pytd[i].st_type&0x07;
										  
										  u32 mask=0;
								   u8 bitlen=pytd[i].len&0xC0;
								
								    
								if(((pytd[i].st_type&TYPE_CIRCLE_ONEBETY)==TYPE_CIRCLE_ONEBETY)&&looptimes>1)
								{     
												
									     		if(bitlen==LEN_FOURBYTE)//选择合适的输入数据字节数//11XX
										{
											     u32  * pu32=(u32 *)pytd[i].pdata; //4字节数据
											     tmpu32=pu32[kp*looptimes];
											
										}
										else if(bitlen==LEN_TWOBYTE)//10XX
										{
											     u16  * pu16=(u16 *)pytd[i].pdata; //双字节数据
											     tmpu32=pu16[kp*looptimes];
										}
										else if(bitlen==LEN_CIRCLE_ONEBETY)//01XX
										{
											     u8  * pu8=(u8 *)pytd[i].pdata; //单字节数据
											    tmpu32=pu8[kp*looptimes+k/8];
										}
										else
										{
											     u8  * pu8=(u8 *)pytd[i].pdata; //单字节数据
											    tmpu32=pu8[kp*looptimes];
										}
									
									
									
									    if(bittype==TYPE_ONEBIT)
												{
														 mask=1;
														mask=mask<<(1*k);
													 tmpu32=mask&tmpu32;
													tmpu32=tmpu32>>(1*k);
													
												}
												else if(bittype==TYPE_TWOBIT)
												{
														mask=3;
														mask=mask<<(2*k);
													 tmpu32=mask&tmpu32;
													tmpu32=tmpu32>>(2*k);
												}
												else if(bittype==TYPE_THREEBIT)
												{
													if(bitlen==LEN_CIRCLE_ONEBETY)//逻辑参数，0时输出为000，1时输出为(pytd[i].st_type&0x70)>>4
													{
														 mask=1;
														mask=mask<<(k%8);
														 tmpu32=mask&tmpu32;
														if(tmpu32!=0)
														{
															 tmpu32=(pytd[i].st_type&0x70)>>4; 
														}
														
													}
													else
													{
															 mask=7;
														mask=mask<<(3*k);
													 tmpu32=mask&tmpu32;
													tmpu32=tmpu32>>(3*k);
													}
												}
									
									
									}
								else
								{   u8 startbit=(pytd[i].st_type&0xF0)>>4;
									   		if(bitlen==LEN_FOURBYTE)//选择合适的输入数据字节数
										{
											     u32  * pu32=(u32 *)pytd[i].pdata; //4字节数据
											     tmpu32=pu32[k+kp*looptimes];
											
										}else if(bitlen==LEN_TWOBYTE)
										{
											     u16  * pu16=(u16 *)pytd[i].pdata; //双字节数据
											     tmpu32=pu16[k+kp*looptimes];
										}
										else
										{
											     u8  * pu8=(u8 *)pytd[i].pdata; //单字节数据
											    tmpu32=pu8[k+kp*looptimes];
										}
									   
									  if(bittype==TYPE_ONEBIT)
												{
														 mask=1;
														mask=mask<<startbit;
													 tmpu32=mask&tmpu32;
													tmpu32=tmpu32>>startbit;
													
												}
												else if(bittype==TYPE_TWOBIT)
												{

														mask=3;
														mask=mask<<startbit;
													 tmpu32=mask&tmpu32;
													tmpu32=tmpu32>>startbit;
												}
												else if(bittype==TYPE_THREEBIT)
												{											
															 mask=7;
														mask=mask<<startbit;
													 tmpu32=mask&tmpu32;
													tmpu32=tmpu32>>startbit;
												}										  
								}
										
										
								
										
										
										
									}
								 else if(typelen==2)
							 	 {
										if((pytd[i].st_type&TYPE_FLOAT)==0)
										{
										 u16  * pu16=(u16 *)pytd[i].pdata;//双字节数据
										 tmpu32=pu16[k+kp*looptimes];
										}
										else
										{
											 float  * pu16=(float *)pytd[i].pdata;//数据为浮点数扩大100倍
											tmpu32=pu16[k+kp*looptimes]*100;
										}
									}
								 else if(typelen==4) //((pytd[i].st_type&0x07)==4)
								 {
										if((pytd[i].st_type&TYPE_FLOAT)==0)
										{
											u32  * pu32=(u32 *)pytd[i].pdata;//四字节数据
											tmpu32=pu32[k+kp*looptimes];
										}
										else
										{
												float  * pu16=(float *)pytd[i].pdata;//数据为浮点数扩大100倍
												tmpu32=pu16[k+kp*looptimes]*100;
										}
									}
								
									
									
								
								if(type!=TYPE_FOURBYTE_L)//其它情况按大端模式传输
								 {
										for(u8 j=0;j<typelen;j++)	
										{
											HextoAscii(&pdat[base],((u8 *)&tmpu32+(typelen-1)-j));
											base+=2;
										}
								 }
								else//4节浮点数按小端模式传输
								 {
										for(u8 j=0;j<typelen;j++)	
											{
												HextoAscii(&pdat[base],((u8 *)&tmpu32+j));
												base+=2;
											}
								 }
								
								
								
								
								
							}
							
				}
				else{//多字节扩展（不确定字节扩展）
					       u16 base2=base;
					      for(u8 k=0;k<looptimes;k++)
								{   
									ytd1363data *pytd2= (ytd1363data *)pytd[i].pdata;
									 Getdata(pytd2,k,&pdat[base2],&datalen2);//递归调用
									  base2+=datalen2;
								}
					     base=base2;
				}
		}
			pdat[base]='\0';
			
			return pdat;

}




u16 Check_Sum(u8* dat,u16 length)//校验和计算
{
   
	u16 i;
	u16 CheckSum=0;

	

	for(i=0;i<length;i++ )
	{
	    CheckSum+=dat[i+1];//跳过SOI
	}


    CheckSum = ~(CheckSum%65536)+1;

	return CheckSum;
}



void  SetYDT1363(ytd1363data *pcid,u8 type,void*pdata,u8 len)//1363数据包设置
{
	static ytd1363data *p=NULL;
	   if(pcid!=NULL)
		 {
			  p=pcid;
		 }

		 if(p!=NULL)
		 {
			 p->st_type=type;
			 p->pdata=pdata;
			 p->len=len;
		 }
	
		 p++;
}

u8 * get1363data(u8 *cmd,u8 *predat,u8 *port)//打包1363数据包
{
	u8 *pdat=NULL;

  
	if(cmd[1]==0x41)	//cid2 获取模拟量量化后数据(浮点数)
	{
		if(cmd[0]==0x40)
		{
			
				u16 *ptemp=(u16 *)pgACmointor;
			  float fdat[11];	
					 fdat[0]=((float)ptemp[0])/100;//VA
					 fdat[1]=((float)ptemp[1])/100;//VB
				   fdat[2]=((float)ptemp[2])/100;//VC
				   fdat[3]=((float)ptemp[3])/10;//f
					 fdat[4]=((float)(*((u32 *)(&ptemp[16]))))/10;//市电电能
					 fdat[5]=((float)ptemp[10])/100;//
				   fdat[6]=((float)ptemp[11])/100;
				   fdat[7]=((float)ptemp[12])/100;
			     fdat[8]=((float)ptemp[13])/100;
			     fdat[9]=0;
			     fdat[10]=0;
			
			    dataflag=getDataFlag(cmd[0],cmd[1],port);
			

			
			//if(predat!=NULL){//铁塔协议
			if(isTieta==1){
				isTieta=0;
							u8 len=19;
							u8 command_groud=0;
						AsciitoHex(&command_groud,&predat[0]);
						
							if(command_groud==0xff)
							 len=len+1;
							
						 ytd1363data *cid4041=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*len);
						
						
							if(cid4041!=NULL)
							{
							
														 ytd1363data *ytdtmp=&cid4041[6];
														SetYDT1363(cid4041,TYPE_ONEBYTE,&dataflag ,1);
								
														if(command_groud==0xff)
														{ 
															 SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//交流瓶数
																 ytdtmp=&cid4041[7];
														}
														
														
														SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,ytdtmp ,1);//交流瓶数据扩展
														SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[5],1);//A相电流
														SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[6],1);//B相电流//A相1.01版本 单相交流电流读取
														SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[7],1);//C相电流
												
												
														END_SET_YDT1363
														 /////////////交流瓶数据扩展项/////////////
														SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[0],1);//A相电压
														SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[1],1);//B相电压
														SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[2],1);//C相电压 
																			
														SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[3],1);//频率
														SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//自定义数
														SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[4] ,1);//交流总电能
														SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//自定义数
														END_SET_YDT1363//结束标志，用以计算长度

								
							 
							}
						 pdat=Getdata(cid4041,0,NULL,NULL);
						 vPortFree(cid4041);	
						cid4041=NULL;	
			}
      else
			{//自家协议
				  ytd1363data *cid4041=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*16);
				   ytd1363data *ytdtmp=&cid4041[6];
			
					
						if(cid4041!=NULL)
						{
							
											SetYDT1363(cid4041,TYPE_ONEBYTE,&dataflag ,1);
											SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,ytdtmp ,1);//交流瓶数据扩展
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[5],1);//A相电流
							if(gSinglePhase==0)
							{								
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[6],1);//B相电流
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[7],1);//C相电流
							}
							else
							{
							        SetYDT1363(NULL,TYPE_FOURBYTE_L,NULL,0x81);//B相电流
											SetYDT1363(NULL,TYPE_FOURBYTE_L,NULL,0x81);//C相电流 
							}
											
											END_SET_YDT1363
							   
							
							 /////////////交流瓶数据扩展项/////////////
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[0],1);//A相电压	
                   if(gSinglePhase==0)	
									 {										 
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[1],1);//B相电压
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[2],1);//C相电压 
									 }
									 else
									 {
							
							        SetYDT1363(NULL,TYPE_FOURBYTE_L,NULL,0x81);//B相电压
											SetYDT1363(NULL,TYPE_FOURBYTE_L,NULL,0x81);//C相电压 
									 }
													
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[3],1);//频率
											SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,5);//自定义数
										  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[4] ,1);//交流总电能
										  SetYDT1363(NULL,TYPE_FOURBYTE_L,NULL ,0x81);//油机电能
											SetYDT1363(NULL,TYPE_FOURBYTE_L,NULL ,0x81);//功率
											SetYDT1363(NULL,TYPE_FOURBYTE_L,NULL ,0x81);//燃料余量
											END_SET_YDT1363//结束标志，用以计算长度
							
							
						}
				
						
						
				   pdat=Getdata(cid4041,0,NULL,NULL);
			      vPortFree(cid4041);	
			     cid4041=NULL;	
				
				
				
			}				
		}
		
		if(cmd[0]==0x41)
		{
		   
			 dataflag=getDataFlag(cmd[0],cmd[1],port);
			 u16 *moduleCurr=(u16 *)pModuledata->GetDataAddr(CURRENT);
			 s16 *moduleTemp=((s16 *)pModuledata->GetDataAddr(TEMP));
			 u16 *moduleAC=((u16 *)pModuledata->GetDataAddr(VOLTAGE_AC));
			  u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));

			
			  float *pfdat= (float *)pvPortMalloc(sizeof(u32)*ModuleOnlineMessage[2]*3);  
				  
			
				  
			  float fdat;
			     	fdat=((float)(*((s16 *)&gpSysData[DCVOLTAGE])))/100;
					
                 for(u8 i=0;i<ModuleOnlineMessage[2];i++)
									{
										if(moduletype[i]==ACDC)
										{
										  pfdat[i]=((float)moduleCurr[i])/100;
										  pfdat[ModuleOnlineMessage[2]+i]=((float)moduleTemp[i])/100;
											pfdat[2*ModuleOnlineMessage[2]+i]=((float)moduleAC[i])/100;
										}
										else
										{
											pfdat[i]=0;
										  pfdat[ModuleOnlineMessage[2]+i]=0;
											pfdat[2*ModuleOnlineMessage[2]+i]=0;
										}
									}
					   

		
			if(isTieta==1)
			{//铁塔协议	
							ytd1363data *cid4141=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*9);
						 
							if(cid4141!=NULL)
							{  
											 
										
				 
												SetYDT1363(cid4141,TYPE_ONEBYTE,&dataflag ,1);//dataflag
												SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat ,1);//模块电压
												SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4141[4] ,(u8)ModuleOnlineMessage[2]);//模块数据扩展
											  END_SET_YDT1363
											 /////////////模块数据扩展项/////////////
												SetYDT1363(NULL,TYPE_FOURBYTE_L,pfdat ,1);//模块电流
												SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//自定数
												SetYDT1363(NULL,TYPE_FOURBYTE_L,&pfdat[ModuleOnlineMessage[2]] ,1);//模块温度
												SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家自定义数为0
												END_SET_YDT1363//结束标志，用以计算长度
											  pdat=Getdata(cid4141,0,NULL,NULL);	

								
							 
							}
						
							
						 vPortFree(cid4141);	
						 vPortFree(pfdat);
						 //vPortFree(pfdat2);
						cid4141=NULL;	
			}		
      else
		  {//自定议协议
			 	ytd1363data *cid4141=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*9);
		
				if(cid4141!=NULL)
				{  
                 

	 
									SetYDT1363(cid4141,TYPE_ONEBYTE,&dataflag ,1);//dataflag
								  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat ,1);//模块电压
								  SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4141[4] ,(u8)ModuleOnlineMessage[2]);//模块数据扩展
								 END_SET_YDT1363
								 /////////////模块数据扩展项/////////////
									SetYDT1363(NULL,TYPE_FOURBYTE_L,pfdat ,1);//模块电流
									SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,2);//自定数
									SetYDT1363(NULL,TYPE_FOURBYTE_L,&pfdat[ModuleOnlineMessage[2]] ,1);//模块温度
					        SetYDT1363(NULL,TYPE_FOURBYTE_L,&pfdat[2*ModuleOnlineMessage[2]] ,1);//输入电压
									END_SET_YDT1363//结束标志，用以计算长度
											

					
				 
				}
			 pdat=Getdata(cid4141,0,NULL,NULL);
				
			 vPortFree(cid4141);	
			 vPortFree(pfdat);
			 //vPortFree(pfdat2);
			cid4141=NULL;	
			
			
			
			
		}			
		}
		
		
		if(cmd[0]==0x42)
		{
			 
			   float fdat[13];
			 	u8 len=27;
			   u8 command_groud=0;
			   AsciitoHex(&command_groud,&predat[0]);
			
			     if(command_groud==0xff)
			       len=len+1;
			
					 
					 
					 fdat[0]=((float)(*((s16 *)&gpSysData[DCVOLTAGE])))/100;
					     fdat[1]=((float)(*((s16 *)&gpSysData[USER_CURR])))/10; 
					     //fdat[1]=((float)(*((s16 *)&totalUserI)))/10;
					     fdat[2]=((float)(*((s16 *)&gpSysData[BATT_CURR3])))/10;
					     fdat[3]=((float)(*((s16 *)&gpSysData[DCVOLTAGE])))/100;
					     fdat[4]=60*(float)(*(u16*)&gpSysData[BATT_SOC])/10000*gSetBattC/fdat[1];
					 if(fdat[4]>1440)fdat[4]=1440;
					     fdat[5]=((float)(*(u16*)&gpSysData[BATT_SOC]))/100;//00*gSetBattC;
					     fdat[6]=((float)*((s16 *)(&gpSysData[ENV_TEMP])))/100;
					     fdat[8]=0;//gpEnergy_100Wh[0]/10;//用户1电能
					     fdat[7]=(float)1;
					     fdat[9]=(float)2;
					     fdat[10]=((float)(*((s16 *)&gpSysData[LOADCURR])))/10;
					     fdat[11]=0;//gpEnergy_100Wh[1]/10;//用户1电能
					  	 fdat[12]=((float)(*((s16 *)&gpSysData[BATT_CURR4])))/10;	
		     
		 
					       float fbatt[6];
								 u8 j=0;
								 
					       for(u8 i=0;i<6;i++)
								 {
									    if( pgBattBranch[i]!=NULL)//只统计存在的电池分路
											{	   
												fbatt[j]=((float)(*((s16 *)pgBattBranch[i])))/10;
												j++;
											}
								 }
					 
					 
					 
					 
					 
                 float fuserI[34];
								 u8 branchcount=0;
								 for(u8 i=0;i<34;i++)
								 {
									    if(SwitchOnlineCount[i]>1)//查找在线用户分路
											{
												
												 fuserI[branchcount]=((float)(*((s16 *)&gDCdistribution.pst_I[i])))/10;
												 branchcount++;
											}
								 }
	 
	   dataflag=getDataFlag(cmd[0],cmd[1],port);
			 
     float tmpbatt[6]={0,0,0,0,0,0};
           for(u8 i=0;i<6;i++)     
		       tmpbatt[i]=fdat[6];


      if(isTieta==1)
			{//A协议
				  
				  ytd1363data * cid4241=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*(len+6+6+branchcount));//电池组数接6个，分路数按10路
				 if(cid4241!=NULL)
				 {
					   SetYDT1363(cid4241,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					   if(command_groud==0xff)
					   SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//直流瓶数
						 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[0] ,1);//电压
					   SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[1],1);//输出电流
						 
						 if(gbattGroud>0)//当电池组数大于0时
						 {
									SetYDT1363(NULL,TYPE_ONEBYTE,&gbattGroud ,1);//电池电流数
								 for(u8 i=0;i<gbattGroud;i++)
								 {
									   SetYDT1363(NULL,TYPE_FOURBYTE_L,&fbatt[i] ,1);//分路电池电流
								 }
						 }
						 else if(gbattGroud==0)//当电池组数等于0，使用监控内部电池分路
						 {
							  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,2);//电池电流数
					      SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[2] ,1);//电池电流1
							 	SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[12] ,1);//电池电流2
						 }
						 SetYDT1363(NULL,TYPE_ONEBYTE,&branchcount ,1);//直流分路电流数
						 
              for(u8 i=0;i<branchcount;i++)
						  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fuserI[i] ,1);//分路电流
						 
						 u8 tmpp=4+battGroud;
						 SetYDT1363(NULL,TYPE_ONEBYTE   ,&tmpp     ,1);//铁塔定定 5
						 
						 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[0] ,1);//电压
						 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[4] ,1);//后备时间 分钟
						 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[5] ,1);//电池余量	 
						 
						 fdat[7]=battGroud;
             SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[7] ,1);//电池温度数	
            for(u8 i=0;i<battGroud;i++)						 
					   SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpbatt[i] ,1);//电池温度					 

						 SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家自定义
						 END_SET_YDT1363//结束标志，用以计算长度 
				 }
				 pdat=Getdata(cid4241,0,NULL,NULL);	
         vPortFree(cid4241);			
				 cid4241=NULL;
			}
			else
      {	//自定义协议		
            fdat[7]=(float)1;
					  fdat[9]=(float)2;
				  ytd1363data * cid4241=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*(len));
				 if(cid4241!=NULL)
				 {
					     
					 SetYDT1363(cid4241,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					  if(command_groud==0xff)
					 SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//直流瓶数
					 
					 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[0] ,1);//电压
					 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[1],1);//输出电流
					 SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//电池电流数
					 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[2] ,1);//电池电流
					 SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//负载电流数
					 SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,15);//铁塔定定 5
					 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[3] ,1);//电池电压
					 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[4] ,1);//后备时间 分钟
					 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[5] ,1);//电池余量 
						
					 SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[7] ,1);//电池温度数		

						
					 SetYDT1363(NULL,TYPE_FOURBYTE_L,& fdat[6],1);//电池温度
				
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, &fdat[9] ,1);//分路数
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, &fdat[8],1);//用户1电能
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, &fdat[11],1);//用户2电能
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, &fdat[10],1);//模块总电流
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, NULL,0x81);//光伏总电流
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, NULL,0x81);//风机总电流
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, NULL,0x81);//燃料电池电流
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, NULL,0x81);//光伏电能
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, NULL,0x81);//风机发电量
					 SetYDT1363(NULL,TYPE_FOURBYTE_L, NULL,0x81);//燃料电池发电量
		      	END_SET_YDT1363//结束标志，用以计算长度 

					
				 }
         pdat=Getdata(cid4241,0,NULL,NULL);	
         vPortFree(cid4241);			
				 cid4241=NULL;
			}
			 
		}
		
		if(cmd[0]==0xE2)
		{
			
				 
				 float puserI[8];
				
				 float penergy[8];
			
			
			if(gEnergySle==0)
			{
					for(u8 i=0;i<6;i++)
					{
						
						 puserI[i]=(float)gDCdistribution.pst_userI[1+i]/10;
						 
						penergy[i]=(float)gDCdistribution.pst_userEnerqy[1+i]/10;
					}	
					
					//特殊处理  诸存5：行业， 诸存6：铁塔，
					// 发送顺序 1,2,3,4,6,5 因些将5与6对调
				//           0,1,2,3,4,5
					 puserI[4]=(float)gDCdistribution.pst_userI[6]/10;//铁塔
					 puserI[5]=(float)gDCdistribution.pst_userI[5]/10;//行业外
					
					 penergy[4]=(float)gDCdistribution.pst_userEnerqy[6]/10;
					 penergy[5]=(float)gDCdistribution.pst_userEnerqy[5]/10;
				}
			else
			{
				  for(u8 i=0;i<6;i++)
					{
						
						 puserI[i]=(float)gDCdistribution.pst_userI[1+i]/10;
						 
						 penergy[i]=(float)acDCEnergydata[i]/10;
					}	
					
					
					 puserI[4]=(float)gDCdistribution.pst_userI[6]/10;//铁塔
					 puserI[5]=(float)gDCdistribution.pst_userI[5]/10;//行业外
					
					 penergy[4]=(float)acDCEnergydata[5]/10;
					 penergy[5]=(float)acDCEnergydata[4]/10;
					
					
			}
			
			
			
			
			u8 command_groud=0;
			AsciitoHex(&command_groud,&predat[0]);
			 ytd1363data * cidE241;
			 
			 dataflag=getDataFlag(cmd[0],cmd[1],port);
				 
			 if(isTieta==1)
				{//铁塔协议 
					
	
						if(command_groud==0xff)
						{
									 cidE241=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*8);	
								 if(cidE241!=NULL) 
								{
									
											SetYDT1363(cidE241,TYPE_ONEBYTE,&dataflag ,1);//dataflag
											SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cidE241[3] ,6);//模块数据扩展
											END_SET_YDT1363//结束标志，用以计算长度
									
											SetYDT1363(NULL,TYPE_FOURBYTE_L,puserI ,1);//模块数据扩展
											SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//铁塔定义数
											SetYDT1363(NULL,TYPE_FOURBYTE_L,penergy ,1);//模块数据扩展
											SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家定义数
											END_SET_YDT1363//结束标志，用以计算长度
									
									pdat=Getdata(cidE241,0,NULL,NULL);
									
								}
							
						}
						else
						{  

                  if(command_groud>6)
									{
										return NULL;
									}
							
								 cidE241=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*6);
									
								if(cidE241!=NULL) 
								{		
											SetYDT1363(cidE241,TYPE_ONEBYTE,&dataflag ,1);//dataflag	
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&puserI[command_groud-1] ,1);//模块数据扩展
											SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//铁塔定义数
											SetYDT1363(NULL,TYPE_FOURBYTE_L,&penergy[command_groud-1] ,1);//模块数据扩展
											SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家定义数
											END_SET_YDT1363//结束标志，用以计算长度
											
											pdat=Getdata(cidE241,0,NULL,NULL);
										
								}
						}
		  	}
			  else
			  {//自定义协议
				      cidE241=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*8);
				      	 if(cidE241!=NULL) 
						{
							
									SetYDT1363(cidE241,TYPE_ONEBYTE,&dataflag ,1);//dataflag
									SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cidE241[3] ,2);//模块数据扩展
									END_SET_YDT1363//结束标志，用以计算长度
							
									SetYDT1363(NULL,TYPE_FOURBYTE_L,puserI ,1);//模块数据扩展
									SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//铁塔定义数
									SetYDT1363(NULL,TYPE_FOURBYTE_L,penergy ,1);//模块数据扩展
									//SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家定义数
									END_SET_YDT1363//结束标志，用以计算长度
							
							pdat=Getdata(cidE241,0,NULL,NULL);
							
						}
			 }




					 vPortFree(cidE241);
					 cidE241=NULL;
			
			
		}
		if(cmd[0]==0x43)
		{
				
			  u16 *outputV=(u16 *)pModuledata->GetDataAddr(VOLTAGE);
			  u16 *moduleCurr=(u16 *)pModuledata->GetDataAddr(CURRENT);
       
			  u16 *moduleAC=((u16 *)pModuledata->GetDataAddr(VOLTAGE_AC));
			 u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			
			
			 s16 *moduleTemp;
			if(isTieta==1)
			{
				moduleTemp=((s16 *)pModuledata->GetDataAddr(CURRENT_AC));//输入电流
			}
			else
			{
				 moduleTemp=((s16 *)pModuledata->GetDataAddr(TEMP));//温度
			}
			
			  float *pfdat= (float *)pvPortMalloc(sizeof(u32)*ModuleOnlineMessage[2]*4);  
				   
			  
				 
			
		       //   u8 modulecount=0;
			       for(u8 i=0;i<ModuleOnlineMessage[2];i++)
									{
										  pfdat[i]=0;
										  pfdat[ModuleOnlineMessage[2]+i]=0;
											pfdat[2*ModuleOnlineMessage[2]+i]=0;//温度或输出电流
											pfdat[3*ModuleOnlineMessage[2]+i]=0;//输入电压
										
										if(moduletype[i]==DCDC)
										{
										  pfdat[i]=((float)outputV[i])/100;//输出电压
										  pfdat[ModuleOnlineMessage[2]+i]=((float)moduleCurr[i])/100;//输出电流
											pfdat[2*ModuleOnlineMessage[2]+i]=((float)moduleTemp[i])/100;//温度或输出电流
											pfdat[3*ModuleOnlineMessage[2]+i]=((float)moduleAC[i])/100;//输入电压
										
										}

									}

			
			     dataflag=getDataFlag(cmd[0],cmd[1],port);
									
					if(isTieta==1)
				  {//铁塔协议 					
							ytd1363data *	cid4341	=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*9);	
								 if(cid4341!=NULL)	
								 {		

										SetYDT1363(cid4341,TYPE_ONEBYTE,&dataflag ,1);//dataflag
										SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4341[3],ModuleOnlineMessage[2]);//光伏模块数据扩展
										END_SET_YDT1363//结束标志，用以计算长度
										///////////////光伏扩展项///////////////
										SetYDT1363(NULL,TYPE_FOURBYTE_L,pfdat ,1);//光伏电压 
										SetYDT1363(NULL,TYPE_FOURBYTE_L,&pfdat[ModuleOnlineMessage[2]] ,1);//光伏电流 
										SetYDT1363(NULL,TYPE_ONEBYTE,NULL,0);//自定数 p=1
									  SetYDT1363(NULL,TYPE_ONEBYTE,NULL,0);//自定数 p=1
										END_SET_YDT1363//结束标志，用以计算长度
								 }
										pdat=Getdata(cid4341,0,NULL,NULL);
										vPortFree(cid4341);
										cid4341=NULL;
									 vPortFree(pfdat);
							 				 
							
					 }
					else
					{//自定义协议
									ytd1363data *	cid4341	=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*9);	
								 if(cid4341!=NULL)	
								 {		

										SetYDT1363(cid4341,TYPE_ONEBYTE,&dataflag ,1);//dataflag
										SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4341[3],ModuleOnlineMessage[2]);//光伏模块数据扩展
										END_SET_YDT1363//结束标志，用以计算长度
										
										///////////////光伏扩展项///////////////
										SetYDT1363(NULL,TYPE_FOURBYTE_L,pfdat ,1);//光伏电压 
										SetYDT1363(NULL,TYPE_FOURBYTE_L,&pfdat[ModuleOnlineMessage[2]] ,1);//光伏电流 
										SetYDT1363(NULL,TYPE_ONEBYTE,NULL,2);//自定数 p=1
										SetYDT1363(NULL,TYPE_FOURBYTE_L,&pfdat[2*ModuleOnlineMessage[2]] ,1);//温度
										SetYDT1363(NULL,TYPE_FOURBYTE_L,&pfdat[3*ModuleOnlineMessage[2]] ,1);//输入电压 
										END_SET_YDT1363//结束标志，用以计算长度
								 }
										pdat=Getdata(cid4341,0,NULL,NULL);
										vPortFree(cid4341);
										cid4341=NULL;
									 vPortFree(pfdat);
									
					}
		}

	  if(cmd[0]==0xE1)
		{
				ytd1363data *cidE141=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*14);
			 
			    dataflag=getDataFlag(cmd[0],cmd[1],port);
			 
				if(cidE141!=NULL)
				{ 
					  float fdat[12];
					
					  fdat[0]=(float)(*((s16 *)&gpSysData[ENV_TEMP]))/100;
					  fdat[1]=(float)(*((s16 *)&gpSysData[HUMIDITY]));
            fdat[2]=(float)pModule->m_smrCount* gMaxModuleCurr/10;
					  fdat[3]=(float)pModule->m_smrCount* gMaxModuleCurr/10;
					  fdat[4]=(float)gSetBattC;
					  fdat[5]=(float)(*((s16 *)&gpSysData[BATT_SOC]))/100;
					  fdat[6]=100;
					  fdat[8]=(float)dischargeTimer;
					  fdat[9]=(float)acTimer;
					  fdat[10]=0;
					  fdat[11]=0;
					  SetYDT1363(cidE141,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[0] ,1);//环境温度数	
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[1] ,1);//相对湿度温度数	
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[2] ,1);//整流模块总容量	
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[3] ,1);//光伏模块总容量	
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[4] ,1);//电池总容量	
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[5] ,1);//电池 SOC	
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[6] ,1);//电池 SOH	
					
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[8] ,1);//市电连续供电时间
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[9] ,1);//油机连续供电时间
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[10] ,1);//油机连续供电时间
					  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[11] ,1);//光伏连续供电时间
					 END_SET_YDT1363//结束标志，用以计算长度 
					  
				}
				 pdat=Getdata(cidE141,0,NULL,NULL);	
         vPortFree(cidE141);			
				 cidE141=NULL;
			 
		}
	
	}
	
	else if(cmd[1]==0x43)//	获取开关输入状态
	{
		if(cmd[0]==0x40)
		{   
			
         u8 len=7;
			   u8 command_groud=0;
			  // u8 mod=2;
         u8 onoffstatus = 2;//交流输出 未定义
			   AsciitoHex(&command_groud,&predat[0]);
			
			     if(command_groud==0xff)
			       len=len+1;
			
				 dataflag=getDataFlag(cmd[0],cmd[1],port);	 
				ytd1363data * cid4043=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*len);	
				if(cid4043!=NULL)
				{
					
						SetYDT1363(cid4043,TYPE_ONEBYTE,&dataflag ,1);
				     if(command_groud==0xff)
					  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//交流瓶数
					  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//交流开关数量
					  SetYDT1363(NULL,TYPE_ONEBYTE,&onoffstatus ,1);//开关状态
						SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//自定a
						SetYDT1363(NULL,TYPE_ONEBYTE,&mod ,1);
					  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//设置A协议厂家自定数据B
						END_SET_YDT1363//结束标志，用以计算长度
					
				}
			
			pdat=Getdata(cid4043,0,NULL,NULL);
			 vPortFree(cid4043);
       cid4043=NULL;				
		
		}
		else if(cmd[0]==0x41)
		{  
			


			
//		  u8* moduledatatmp3=(u8 *)pvPortMalloc(ModuleOnlineMessage[2]*4);
//      u8* moduledatatmp4=&moduledatatmp3[ModuleOnlineMessage[2]*1];
//			u8* moduledatatmp5=&moduledatatmp3[ModuleOnlineMessage[2]*2];
//			u8* WorkingStatus=&moduledatatmp3[ModuleOnlineMessage[2]*3];
			
			       
				ytd1363data * cid4143=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*10);
			   u8 * moduleOnoff=(u8 *)pModuledata->GetDataAddr(ONOFF);	
         u8 * modulelimitcurr=(u8 *)pModuledata->GetDataAddr(LIMIT_CURR_STATUS);	
				 u8 * moduleph =(u8 *)pModuledata->GetDataAddr(PHASEPOSITION);
				 
				   
			     u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			
			u8 modulecount=ModuleOnlineMessage[2];
			if(ModuleOnlineMessage[2]>10) modulecount=10;
			  for(u8 i=0;i<modulecount;i++)
			{
				    if(moduletype[i]==ACDC)
						{
							
							moduledatatmp3[i]=moduleOnoff[i];
							moduledatatmp4[i]=modulelimitcurr[i];
							
							if(isTieta==1)
							{
								  if(genableTP==1)//模块防盗
								   moduledatatmp5[i]= 0;
									else
									 moduledatatmp5[i]= 1;	
							}
							else
							{
							   moduledatatmp5[i]=moduleph[i]%3;//相位
							  if(moduledatatmp5[i]==0) moduledatatmp5[i]=3;
							}
							
							
							 WorkingStatus[i]=gcourrentWorkStatus;
							  
						
						}
						else
						{
							moduledatatmp3[i]=1;
							moduledatatmp4[i]=1;
							if(genableTP==1)//模块防盗
							{   moduledatatmp5[i]= 0;}
									else
							{		 moduledatatmp5[i]= 1;}	
							WorkingStatus[i]=0; 
						}

						
						
			}
				 
				 
			   dataflag=getDataFlag(cmd[0],cmd[1],port);
				if(cid4143!=NULL)
				{
					     					
					
								SetYDT1363(cid4143,TYPE_ONEBYTE,&dataflag ,1);//dataflag
								SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4143[3],modulecount);//模块状态扩展
								END_SET_YDT1363
								////////状态扩展项///////
								SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp3 ,1);//开机
								SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp4 ,1);//限流
								SetYDT1363(NULL,TYPE_ONEBYTE,WorkingStatus ,1);//充电
					
								
					     
					
								  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//自定数
								  SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp5 ,1);//铁塔协议为模块防盗，自定协议为相位
					        if(isTieta==1)
							   {
									  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家自定义
								 }
							 
					
								END_SET_YDT1363//结束标志，用以计算长度
					
				}
			 pdat=Getdata(cid4143,0,NULL,NULL);
			vPortFree(cid4143);
    //  vPortFree(moduledatatmp3);				
      cid4143=NULL;				
			
		}
		else if(cmd[0]==0x42)
		{ 
			
		  	ytd1363data * cid4243	=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*6);	
			
			   u8 shavestatus=0;
				 

			

			      if(gWorkShiftMode==1)
			      {
			         if(ACRunMode==RUNNULL)
							 {
								 shavestatus=0x00;
							 }
							 else  if(ACRunMode==TroughMode)//谷
							 {
								 shavestatus=0x01;
							 }
							 else if(ACRunMode==NormalMode)
							 {
								 shavestatus=0x02;
							 }
							 else if(ACRunMode==PeakMode)
							 {
								 shavestatus=0x03;
							 }
							 
							 else if(ACRunMode==SpikeMode)
							 {
								 shavestatus=0x04;
							 }
						 }
							
							
							
				
			  dataflag=getDataFlag(cmd[0],cmd[1],port);
			 if(cid4243!=NULL)
			 {			 
						SetYDT1363(cid4243,TYPE_ONEBYTE,&dataflag ,1);//dataflag
						SetYDT1363(NULL,TYPE_ONEBYTE,&gcourrentWorkStatus ,1);//充电状态
						SetYDT1363(NULL,TYPE_ONEBYTE,&gSleepOnoff ,1);//节能状态
						SetYDT1363(NULL,TYPE_ONEBYTE,&gPeakShaveOnoff ,1);//削峰状态
				    SetYDT1363(NULL,TYPE_ONEBYTE,&shavestatus ,1);//削峰状态
						END_SET_YDT1363//结束标志，用以计算长度
			 }
		  pdat=Getdata(cid4243,0,NULL,NULL);
			 vPortFree(cid4243);	
			 cid4243=NULL;
    
			
		}
		else if(cmd[0]==0x43)
		{
			ytd1363data *	cid4343;
			
				
//			u8* moduledatatmp3=(u8 *)pvPortMalloc(ModuleOnlineMessage[2]*3);
//      u8* moduledatatmp4=&moduledatatmp3[ModuleOnlineMessage[2]*1];
//			u8* moduledatatmp5=&moduledatatmp3[ModuleOnlineMessage[2]*2];
	
			
			
			   u8 * moduleOnoff=(u8 *)pModuledata->GetDataAddr(ONOFF);	
			   u8 * pModuleWRANNING=(u8 *)pModuledata->GetDataAddr(WRANNING);	
         u8 * modulelimitcurr=(u8 *)pModuledata->GetDataAddr(LIMIT_CURR_STATUS);	
				 
				  
			     u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			
			 u8 modulecount=ModuleOnlineMessage[2];
			if(ModuleOnlineMessage[2]>10)modulecount=10;
			  for(u8 i=0;i<ModuleOnlineMessage[2];i++)
			{
				    moduledatatmp3[i]=1;
				    moduledatatmp4[i]=0;
				    if(moduletype[i]==DCDC)
						{
							
							moduledatatmp3[i]=moduleOnoff[i];
							moduledatatmp4[i]=(pModuleWRANNING[i]&0x01);
							//modulecount++;
						}
						
						
						
			}
			
			
			 dataflag=getDataFlag(cmd[0],cmd[1],port);
			 if(isTieta==1)
			 {//A协议
				 cid4343=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*8);	 
				 if(cid4343!=NULL)
				 {
						SetYDT1363(cid4343,TYPE_ONEBYTE,&dataflag ,1);//dataflag
						SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4343[3],modulecount);//光伏模块状态数据扩展
						 END_SET_YDT1363//结束标志，用以计算长度
					     
					    SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp4 ,1);//工作状态
							SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp3 ,1);//开关机状态
							SetYDT1363(NULL,TYPE_ONEBYTE,NULL,0);//铁塔自定数 p=0
					    SetYDT1363(NULL,TYPE_ONEBYTE,NULL,0);//厂家自定数 p=0
							END_SET_YDT1363//结束标志，用以计算长度
			   }
			 }
			 else
			 {//自定义协议
				cid4343=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*6);	 
				 if(cid4343!=NULL)
				 {
						SetYDT1363(cid4343,TYPE_ONEBYTE,&dataflag ,1);//dataflag
						SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4343[3],modulecount);//光伏模块状态数据扩展
						 END_SET_YDT1363//结束标志，用以计算长度
					 
							SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp3 ,1);//开关机状态
							SetYDT1363(NULL,TYPE_ONEBYTE,NULL,0);//自定数 p=0
							END_SET_YDT1363//结束标志，用以计算长度
				 }
			  }
				 
				 
			pdat=Getdata(cid4343,0,NULL,NULL);
			 vPortFree(cid4343);	
			// vPortFree(moduledatatmp3);	
       cid4343=NULL;				 
		
		}
		else if(cmd[0]==0x44)
		{
		}
		else if(cmd[0]==0x4A)
		{
				ytd1363data * cid4a43=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*5);	 
				 if(cid4a43!=NULL)
				 {
							SetYDT1363(cid4a43,TYPE_ONEBYTE,&dataflag ,1);//dataflag
							//SetYDT1363(NULL,TYPE_CIRCLE_ONEBETY|TYPE_THREEBIT,&battStatus ,2);//电池组状态
					    SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);
					    SetYDT1363(NULL,TYPE_ONEBYTE,&pMonitor->m_workStatusModeFlag ,1);
							SetYDT1363(NULL,TYPE_ONEBYTE,NULL,0);//自定数 p=0
							 END_SET_YDT1363//结束标志，用以计算长度
				 }
			pdat=Getdata(cid4a43,0,NULL,NULL);
			 vPortFree(cid4a43);
				cid4a43=NULL; 
		}
		
	  else if(cmd[0]==0xE1)
		{
			  	ytd1363data * cidE143=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*15);

        u8 distatus[12]={0,0,0,0,0,0,0,0,0,0,0,0};
			     
			    for(u8 i=0;i<12;i++)
			   {
					 if((pgh52c0->m_di._diData&((u16)1<<i))!=0)
						      distatus[i]=1;
				 }
			
			    
			   dataflag=getDataFlag(cmd[0],cmd[1],port);
			    if(cidE143!=NULL)
				 {
					 	SetYDT1363(cidE143,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					  SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,12);//下电
					    for(u8 i=0;i<12;i++)
					  	SetYDT1363(NULL,TYPE_ONEBYTE,& distatus[i] ,1);
					  END_SET_YDT1363//结束标志，用以计算长度		
				 }
			pdat=Getdata(cidE143,0,NULL,NULL);
			 vPortFree(cidE143);
				cidE143=NULL; 
		}
		else if(cmd[0]==0xE2)
		{
			  // u8 tmptest[4]={0,0,0,0};
			/*	 
				 	memset(tenantcount,0,10);
		
			   for(u8 i=0;i<10;i++)//统计每个租户的个数
				 {
					 if(tenant[i]>0)
					 tenantcount[tenant[i]-1]++;
				 }
				 u8 tenantCount=0;
				 
				 for(u8 i=0;i<10;i++)//统计不同租户个数
				 {
					   if (tenantcount[i]>0)
						 {
							 tenantCount++;
							
						 }
							 
				 }
				 */
				 
			// u8* pDC8data=gDCdistribution.pst_status;
			
			  u8 puserd[2]={0,0};//,0,0};// 用户时段状态
			 // u8 puserf[4]={0,0};//,0,0};
				
				
//				for(u8 i=0;i<2;i++)
//				{
//					if((pDC8data[i]&0x07)!=0)
//								 {
//										puserd[i]=0x80;
//								 }
//								 
////									if((pDC8data[i]&0x80)==0x80)
////								 {
////										puserf[i]=0x03;
////								 }
//				}
//				 
				 
				 
				 
				 
			  	ytd1363data * cidE243=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*5);	 
					
					dataflag=getDataFlag(cmd[0],cmd[1],port);
			    if(cidE243!=NULL)
				 {
					 	SetYDT1363(cidE243,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					  SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cidE243[3] ,2);//模块数据扩展 用户数
					 	END_SET_YDT1363//结束标志，用以计算长度		
					  SetYDT1363(NULL,TYPE_ONEBYTE, puserd ,1);//下电
					  END_SET_YDT1363//结束标志，用以计算长度		
				 }
				 
			 pdat=Getdata(cidE243,0,NULL,NULL);
			 vPortFree(cidE243);
				cidE243=NULL;  
				 
		}
	
	}
	else if(cmd[1]==0x44)	// 获取告警状态
	{
		if(cmd[0]==0x40)
		{
			 u8 len=20;
			 u8 command_groud=0;
			 u8 Ilimit[3]={0,0,0};
			AsciitoHex(&command_groud,&predat[0]);
			
			  if(command_groud==0xff)
			   len=len+1;
			
				
				 if((gWarnbit&0x00100000)!=0)//防雷
					 {
						 acAlarmV[5]=0xE1;
					 }
					 else
					 {
						  acAlarmV[5]=0;
					 }	
			
//					 
//					if( gpAlarmbit->bit_IN_OVER_IA==1)
//						Ilimit[0]=2;
//					if( gpAlarmbit->bit_IN_OVER_IB==1)
//						Ilimit[1]=2;
//					if( gpAlarmbit->bit_IN_OVER_IC==1)
//						Ilimit[2]=2;
						
			
				ytd1363data * cid4044=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*len);
         dataflag=getDataFlag(cmd[0],cmd[1],port);					 
				if(cid4044!=NULL)
				{
					SetYDT1363(cid4044,1,&dataflag ,1);
					if(command_groud==0xff)
					  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);
					SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);
					SetYDT1363(NULL,TYPE_ONEBYTE,&acAlarmV[0] ,1);//a
					if(gSinglePhase==0)
					{
						SetYDT1363(NULL,TYPE_ONEBYTE,&acAlarmV[1] ,1);//b
						SetYDT1363(NULL,TYPE_ONEBYTE,&acAlarmV[2] ,1);//c
					}
					else
					{
						SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0x81);//b
						SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0x81);//c
					}
					SetYDT1363(NULL,TYPE_ONEBYTE,&acAlarmV[3] ,1);//f
					SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//交流熔丝数
					SetYDT1363(NULL,TYPE_ONEBYTE,&acAlarmV[4] ,1);//交流熔丝
					SetYDT1363(NULL,TYPE_ONEBYTE,&acAlarmV[5] ,1);//spd
					
					SetYDT1363(NULL,1,NULL ,2);//p
				  SetYDT1363(NULL,TYPE_ONEBYTE,&acAlarmV[6] ,1);//交流主开关
					SetYDT1363(NULL,TYPE_ONEBYTE,&acAlarmV[7] ,1);//交流停电
					SetYDT1363(NULL,1,NULL ,0);//
//					if(isTieta==2)
//					{
					SetYDT1363(NULL,TYPE_ONEBYTE,&Ilimit[0] ,1);//
					SetYDT1363(NULL,TYPE_ONEBYTE,&Ilimit[1] ,1);//
					SetYDT1363(NULL,TYPE_ONEBYTE,&Ilimit[2] ,1);//
//					}
	
					
					
					
					
					 END_SET_YDT1363//结束标志，用以计算长度
				}
			
		     pdat=Getdata(cid4044,0,NULL,NULL);
				 vPortFree(cid4044);
         cid4044=NULL;				
		
		}
		else if(cmd[0]==0x41)
		{
			  u8 * pModuleWRANNING=(u8 *)pModuledata->GetDataAddr(WRANNING);	 
			  u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			  u8 i;
			
			
			
//			u8* moduledatatmp3=(u8 *)pvPortMalloc(ModuleOnlineMessage[2]);
//      u8* moduledatatmp4=(u8 *)pvPortMalloc(ModuleOnlineMessage[2]);

			
				ytd1363data *  cid4144=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*9);
       
			 // u8 comModuleFault[30];
			   for(i=0;i<10;i++)
			   moduledatatmp5[i]=0;
			
           u8 modulecount=ModuleOnlineMessage[2];
			
			      if(ModuleOnlineMessage[2]>10) modulecount=10;
					 for( i=0;i<modulecount;i++)
					{    
			        
							
							 moduledatatmp3[i]=0;
						
						   moduledatatmp4[i]=0;
						
						if(moduletype[i]==ACDC)
					  {
							
							  //moduledatatmp3[i]=(pModuleWRANNING[i]&0x01);
									
									if((pModuleWRANNING[i]&0x01)==0x01)
									{
										 moduledatatmp3[i]=1;
									}
									else
									{
										 moduledatatmp3[i]=0;
									}
							
									
							   if((pModuleWRANNING[i]&0xF0)!=0x10)
								 {
									 moduledatatmp4[i]=1;
								 }
								 else
								 {
									  moduledatatmp4[i]=0;
								 }
						 
									
								 
						}
						    moduledatatmp5[i]=0;
						 if((((u32)1<<(i))&pcanruleln->m_ModuleAddrlistOK)==0)
								{
									moduledatatmp5[i]=1;//模块通信故障
								}	
	
					}

				
				dataflag=getDataFlag(cmd[0],cmd[1],port);
				if(cid4144!=NULL)
				{
					SetYDT1363(cid4144,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4144[3],modulecount);//扩展模块告警
					END_SET_YDT1363
					/////////////模块告警扩展项//////////////
					SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp3 ,1);//模块告警
					if(isTieta==1)
					{
						
						SetYDT1363(NULL,TYPE_ONEBYTE,NULL,2);//自定p=0
						SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp5 ,1);//模块通信故障
						SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp4 ,1);//未鉴权
						SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//设置A协议厂家自定数据B
					}
					else
					{
						SetYDT1363(NULL,TYPE_ONEBYTE,NULL,1);//自定p=0
						SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp5 ,1);//模块通信故障
						SetYDT1363(NULL,TYPE_ONEBYTE,NULL,0);
					}
					
					

					END_SET_YDT1363//结束标志，用以计算长度
					
				}
					 pdat=Getdata(cid4144,0,NULL,NULL);
					vPortFree(cid4144);
//					vPortFree(moduledatatmp3);	
//				  vPortFree(moduledatatmp4);	
					cid4144=NULL;
		
		}
		else if(cmd[0]==0x42)
		{
			ytd1363data * cid4244;	
			u8* pDC8data=gDCdistribution.pst_status;
					 
			 dataflag=getDataFlag(cmd[0],cmd[1],port);
					 
 
					 
       u8 tmpNB=1+1+battGroud;
			
				 
				   
			  
			   u8 len =17;
			   u8 command_groud=0;
			   AsciitoHex(&command_groud,&predat[0]);
				 
				 
				   	 

             u8 fbatt[7]={0,0,0,0,0,0,0};	
						 
						 if(gbattGroud>0)//电池配电分路
						 { u8  j=0;
             
										for(u8 i=0;i<6;i++)
										{
												if(gUseToBattBranch[i]<35)
												{
													
															 if((gDCdistribution.pst_status[gUseToBattBranch[i]]&0x40)!=0)								
															 {
																	if(j<6)
																	fbatt[j]=0x03;//回路断
												
															 }
															 
																if((gDCdistribution.pst_status[gUseToBattBranch[i]]&0x07)!=0)
																{
																	fbatt[6]=0x80;//电池下电
																}
													
												j++;	
												}	
												
										}	
						}
            else//铅酸电池分路
						{
							
							 GetBattInPut(fbatt);
							
							if(locdDownFlag!=0)//电池下电
							{
								fbatt[6]=0x80;//电池下电
							}
						}		
			 
							 u8 barnchCount=0;


			     if(command_groud==0xff)
			       len=len+1;
					 		 
				    cid4244	=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*(len+6+barnchCount+6));	
				 
				    SetYDT1363(cid4244,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					     if(command_groud==0xff)
			      SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);

						SetYDT1363(NULL,TYPE_ONEBYTE,&AlarmVolitage ,1);//电压告警
							 
					if(gbattGroud>0)//电池配电分路存在时
					{
					  SetYDT1363(NULL,TYPE_ONEBYTE,&gbattGroud ,1);//电池回路开关数
						for(u8 i=0;i<gbattGroud;i++)	 
						SetYDT1363(NULL,TYPE_ONEBYTE,&fbatt[i] ,1);//电池开关回路状态
					} 
					else //铅配电池分路
					{
						   SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,2);//电池回路开关数
						   SetYDT1363(NULL,TYPE_ONEBYTE,&fbatt[0] ,1);
						   SetYDT1363(NULL,TYPE_ONEBYTE,&fbatt[1] ,1);
						
					}
					
						SetYDT1363(NULL,TYPE_ONEBYTE,NULL,barnchCount);//直流负载回路数量
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//dataflag;//绝缘告警
						SetYDT1363(NULL,TYPE_ONEBYTE,&tmpNB ,1);//铁塔自定义字段数量a=1+1+m
						SetYDT1363(NULL,TYPE_ONEBYTE,&fbatt[6] ,1);//电池下电
				    SetYDT1363(NULL,TYPE_ONEBYTE,&battGroud ,1);//电池组数 m
						for(u8 i=0;i<battGroud;i++)		 
				    SetYDT1363(NULL,TYPE_ONEBYTE,&galram ,1);//电池温度告警
				    SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家自定义					 
				     END_SET_YDT1363//结束标志，用以计算长度
			
	
				 
				 
				 
			 pdat=Getdata(cid4244,0,NULL,NULL);
       vPortFree(cid4244);				 
        cid4244=NULL;				 
		
						
		}
		else if(cmd[0]==0x43)
		{
			    
			  	//	u8* moduledatatmp3=(u8 *)pvPortMalloc(ModuleOnlineMessage[2]);

			
			  ytd1363data * cid4344=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*8);	 
			
			
			     u8 * pModuleWRANNING=(u8 *)pModuledata->GetDataAddr(WRANNING);	
       
			     u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			
			      //u8 comModuleFault[30];
			   for(u8 i=0;i<10;i++)
			   moduledatatmp5[i]=0;
			
				u8 modulecount=ModuleOnlineMessage[2];
			  if(ModuleOnlineMessage[2]>10) modulecount=10;
			
			  for(u8 i=0;i<modulecount;i++)
			{      
				    moduledatatmp3[i]=0;
				    if(moduletype[i]==DCDC)
						{						
										if((pModuleWRANNING[i]&0x01)==0x01)
									{
										 moduledatatmp3[i]=1;
									}
									else
									{
										 moduledatatmp3[i]=0;
									}
									
									
									
								
						}
					   moduledatatmp5[i]=0;
						if((((u32)1<<(i))&pcanruleln->m_ModuleAddrlistOK)==0)
								{
									moduledatatmp5[i]=1;
								}
						 
						
						
			}
			
			
			
			
				
       dataflag=getDataFlag(cmd[0],cmd[1],port);
			if(cid4344!=NULL)
				 {
						SetYDT1363(cid4344,TYPE_ONEBYTE,&dataflag ,1);//dataflag
						SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4344[3],modulecount);//光伏模块告警数据扩展
						 END_SET_YDT1363//结束标志，用以计算长度
					 
							SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp3 ,1);
							SetYDT1363(NULL,TYPE_ONEBYTE,NULL,1);//自定数 p=0
					    SetYDT1363(NULL,TYPE_ONEBYTE,moduledatatmp5 ,1);//模块通信故障
					
					
					     if(isTieta==1)
							 {
								 SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//设置A协议厂家自定数据B
							 }
							END_SET_YDT1363//结束标志，用以计算长度
				 }
			 pdat=Getdata(cid4344,0,NULL,NULL);
			 vPortFree(cid4344);	
		//	 vPortFree(moduledatatmp3);	
				cid4344=NULL; 
			
		}
		else if(cmd[0]==0x44)
		{
		}
		else if(cmd[0]==0x4A)
		{
					ytd1363data * cid4a44=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*15);	
				if(cid4a44!=NULL)
				{
						SetYDT1363(cid4a44,TYPE_ONEBYTE,&dataflag ,1);//dataflag
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//电池电压告警
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//电池电流告警
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//反接告警
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//电池环境温度告警
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//电池组温度告警
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//PCB温度告警
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//电量告警
						SetYDT1363(NULL,TYPE_ONEBYTE,&nomal ,1);//电池失效告警
						SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cid4a44[12] ,4);//电池单体扩展
						SetYDT1363(NULL,TYPE_ONEBYTE,battCellTAlarm ,4);//电池单体温度告警
						END_SET_YDT1363//结束标志，用以计算长度 
						
						//////////////////////电池单体扩展项/////////////////////////////
							SetYDT1363(NULL,TYPE_ONEBYTE,battCellVoltAlarm ,1);//电池单体电压告警
							SetYDT1363(NULL,TYPE_ONEBYTE,battCellFaultAlarm ,1);//电池单体失效告警
					    END_SET_YDT1363//结束标志，用以计算长度 
					
				}		
			pdat=Getdata(cid4a44,0,NULL,NULL);
				 vPortFree(cid4a44);	
				cid4a44=NULL;
		}
		else if(cmd[0]==0xE1)
		{  ytd1363data * cidE144=NULL;
//			if(predat!=NULL)
//			{//铁塔协议
				if(isTieta==1){//铁塔协议 
				 cidE144=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*5);	
			  u8 dat[8];
		
			  for(u8 i=0;i<8;i++)
			{
				 dat[i]=((((u8)(pgh52c0->m_di._diData))^0xff)>>i)&0x01;
			}
			
			
			
			  if(cidE144!=NULL)
				{
					SetYDT1363(cidE144,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cidE144[3] ,8);
					END_SET_YDT1363//结束标志，用以计算长度 
					SetYDT1363(NULL,TYPE_ONEBYTE,dat,1);//电池单体温度告警
					END_SET_YDT1363;
				}
			}
			else
			{
				  cidE144=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*7);	
				  u8 dat[8];
				 if((gWarnbit&0x01000000)!=0)//温度低
					 {
						 dat[0]=1;
					 }
					 else  if((gWarnbit&0x02000000)!=0)//温度高
					 {
						  dat[0]=0;
					 }	else{dat[0]=0;}	
				dat[1]=0;
					 if((gWarnbit&0x10000000)!=0)//门禁
					 {
						 dat[2]=1;
					 }
					 else
					 {
						  dat[2]=0;
					 }
					 
					 	 if((gWarnbit&0x08000000)!=0)//烟雾
					 {
						 dat[3]=1;
					 }
					 else
					 {
						  dat[3]=0;
					 }
					 
					  	 if((gWarnbit&0x04000000)!=0)//水浸
					 {
						 dat[4]=1;
					 }
					 else
					 {
						  dat[4]=0;
					 }
					 
					 
				 dataflag=getDataFlag(cmd[0],cmd[1],port);
				 if(cidE144!=NULL)
				{
					SetYDT1363(cidE144,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					SetYDT1363(NULL,TYPE_ONEBYTE,&dat[0] ,1);//环境温度告警
					SetYDT1363(NULL,TYPE_ONEBYTE,&dat[1] ,1);//相对湿度告警
					SetYDT1363(NULL,TYPE_ONEBYTE,&dat[2] ,1);//门禁告警
					SetYDT1363(NULL,TYPE_ONEBYTE,&dat[3] ,1);//烟雾告警
					SetYDT1363(NULL,TYPE_ONEBYTE,&dat[4] ,1);//水浸告警
					END_SET_YDT1363;
				}
			}
				
				pdat=Getdata(cidE144,0,NULL,NULL);
				 vPortFree(cidE144);	
				 cidE144=NULL;
		}
		else if(cmd[0]==0xE2)
		{   
			
 
			
			 u8* pDC8data=gDCdistribution.pst_userStatus;
			
			  u8 puserd[8]={0,0,0,0,0,0,0,0};//,0,0};
			  u8 puserf[8]={0,0,0,0,0,0,0,0};
				
				

				
			
				u8 command_groud=0;
			  AsciitoHex(&command_groud,&predat[0]);
			 ytd1363data * cidE244;
				dataflag=getDataFlag(cmd[0],cmd[1],port);
			
				if(isTieta==1)
				{//铁塔协议
					

								
							//	u8 userCount=0;
								for(u8 i=0;i<8;i++)//统计已配置租户的个数 并取得相关租户参数
								{

											if((pDC8data[i]&0x07)!=0)
											 {
													puserd[i]=0x80;
											 }
											 
												if((pDC8data[i]&0x40)==0x40)
											 {
													puserf[i]=0x03;
											 }

								}
					   
						if(command_groud==0xff)
						{
										cidE244=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*8);	
									 if(cidE244!=NULL) 
									{
												SetYDT1363(cidE244,TYPE_ONEBYTE,&dataflag ,1);//dataflag
												SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cidE244[3] ,6);//模块数据扩展 用户数
												END_SET_YDT1363//结束标志，用以计算长度							
												SetYDT1363(NULL,TYPE_ONEBYTE, &puserd[1] ,1);//下电
												SetYDT1363(NULL,TYPE_ONEBYTE, &puserf[1] ,1);//熔丝
												SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
												SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
												END_SET_YDT1363//结束标志，用以计算长度
										

											pdat=Getdata(cidE244,0,NULL,NULL);
									}
						}
						else
						{
							      
							    	cidE244=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*6);	
									 if(cidE244!=NULL) 
									{
										   SetYDT1363(cidE244,TYPE_ONEBYTE,&dataflag ,1);//dataflag
										   SetYDT1363(NULL,TYPE_ONEBYTE, &puserd[command_groud] ,1);//下电
												SetYDT1363(NULL,TYPE_ONEBYTE, &puserf[command_groud] ,1);//熔丝
												SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
												SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
												END_SET_YDT1363//结束标志，用以计算长度
										    pdat=Getdata(cidE244,0,NULL,NULL);
									}
						}
		
			 }
				else
				{
						cidE244=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*8);	
							 if(cidE244!=NULL) 
							{
										SetYDT1363(cidE244,TYPE_ONEBYTE,&dataflag ,1);//dataflag
										SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cidE244[3] ,2);//模块数据扩展 用户数
										END_SET_YDT1363//结束标志，用以计算长度							
										SetYDT1363(NULL,TYPE_ONEBYTE, puserd ,1);//下电
										SetYDT1363(NULL,TYPE_ONEBYTE, puserf ,1);//熔丝
								   SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
								   // SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
										END_SET_YDT1363//结束标志，用以计算长度
								

								  pdat=Getdata(cidE244,0,NULL,NULL);
							}
				}

			 vPortFree(cidE244);
			 cidE244=NULL;
			
			
		}
	}
	else if(cmd[1]==0x45)//遥控
	{
		
		
		   if(*port==3){//处理网页请求
              
					 if(cmd[2]==0x1F)//转浮充
					 {
							 pMonitor->m_workMode=FLOAT_MOD;	
							 //gworkStatus=FLOAT_MOD;				 
					 }
					 else if(cmd[2]==0x10)//转均充
					 {
							pMonitor->m_workMode=EQUAL_MOD;	
							//gworkStatus=EQUAL_MOD;
					 }
					 else if(cmd[2]==0x11)//转测试
					 {
						 pMonitor->m_workMode=TEST_MOD;	
						// gworkStatus=TEST_MOD;
						 
					 }
					 else if(cmd[2]==0x20)//开机
					 {
							 //pmodule->SendData(cmd[3],0);
						 pcanruleln->SetModuleOnOff(cmd[3],0);
					 }
					 else if(cmd[2]==0x2F)//关机
					{
							 //pmodule->SendData(cmd[3],1);
						pcanruleln->SetModuleOnOff(cmd[3],1);
					}
		}
		else   if(*port==2){//A协议  H52C0F//非网页请求 
		
			    u8 cmd_type,cmd_id;
			   AsciitoHex(&cmd_type,&predat[0]);
			   AsciitoHex(&cmd_id,&predat[2]);
			
			if(cmd[0]==0x41)//整流模块遥控
			{
				 if(cmd_type==0x10)//转均充
				 {
					 pMonitor->m_workMode=EQUAL_MOD;	
				 // gworkStatus=EQUAL_MOD;
				 }
				 else if(cmd_type==0x1F)//转浮充
				 {
					 pMonitor->m_workMode=FLOAT_MOD;	
				  //gworkStatus=FLOAT_MOD;
				 }
				  else if(cmd_type==0x11)//转测试
        {
				 pMonitor->m_workMode=TEST_MOD;	
				// gworkStatus=TEST_MOD;
				 
			  }
				else if(cmd_type==0x20)//开机
				{
					
					  // u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			
//						 u8 modulecount=0;
//						for(u8 i=0;i<ModuleOnlineMessage[2];i++)
//							{
//								if(moduletype[i]==ACDC)
//								{
//									   modulecount++;
//									  if(modulecount==cmd_id)
//										{
//											break;
//										}
//								}
//							}
					
					
					 pcanruleln->SetModuleOnOff(cmd_id,0);
				}
				else if(cmd_type==0x2F)//关机
				{
				   
					
//					 u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
//			
//						 u8 modulecount=0;
//						for(u8 i=0;i<ModuleOnlineMessage[2];i++)
//							{
//								if(moduletype[i]==ACDC)
//								{
//									   modulecount++;
//									  if(modulecount==cmd_id)
//										{
//											break;
//										}
//								}
//							}
					
					
					  pcanruleln->SetModuleOnOff(cmd_id,1);
				}
				else if(cmd_type==0x30)
				{
					       genableTP=1;//开防盗
								 pgh52c0->savePara(&genableTP);
				}
				else if(cmd_type==0x3F)
				{
					       genableTP=0;//关防盗
								 pgh52c0->savePara(&genableTP);
				}
				
				
				
				
				
				
				 
			}
			else if(cmd[0]==0x43)//光伏模块遥控
			{
				if(cmd_type==0x10)//开机
				{
					
//					u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
//			
//						 u8 modulecount=0;
//						for(u8 i=0;i<ModuleOnlineMessage[2];i++)
//							{
//								if(moduletype[i]==DCDC)
//								{
//									   modulecount++;
//									  if(modulecount==cmd_id)
//										{
//											break;
//										}
//								}
//							}
					pcanruleln->SetModuleOnOff(cmd_id,0);
				}
				else if(cmd_type==0x1F)//关机
				{
				  
//					u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
//			
//						 u8 modulecount=0;
//						for(u8 i=0;i<ModuleOnlineMessage[2];i++)
//							{
//								if(moduletype[i]==DCDC)
//								{
//									   modulecount++;
//									  if(modulecount==cmd_id)
//										{
//											break;
//										}
//								}
//							}
					pcanruleln->SetModuleOnOff(cmd_id,1);
				}
				else if(cmd_type==0x20)//开机
				{
					
						u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			
						 u8 modulecount=0;
						for(u8 i=0;i<ModuleOnlineMessage[2];i++)
							{
								if(moduletype[i]==DCDC)
								{
									   modulecount++;
									  if(modulecount==cmd_id)
										{
											break;
										}
								}
							}
					pcanruleln->SetModuleOnOff(modulecount,0);
				}
				else if(cmd_type==0x2F)//关机
				{
				   
					u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			
						 u8 modulecount=0;
						for(u8 i=0;i<ModuleOnlineMessage[2];i++)
							{
								if(moduletype[i]==DCDC)
								{
									   modulecount++;
									  if(modulecount==cmd_id)
										{
											break;
										}
								}
							}
					
					pcanruleln->SetModuleOnOff(modulecount,1);
				}
			}
		}
		
	}
	else if(cmd[1]==0x46) // 获取系统参数(浮点数)
	{
		if(cmd[0]==0x40)
		{float fdat[6];
			ytd1363data * cid4046;
			
			       fdat[0]=((float)gInOverVLimit)/100;
			       fdat[1]=((float)gInOwrVLimit)/100;
			       fdat[2]=((float)gInOverILimit)/100;
			       fdat[3]=((float)gOverFLimit)/100;
			       fdat[4]=((float)gOwrFLimit)/100;
			       fdat[5]=(float)_NONE;
			
			
			
			
			    	cid4046=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*12);
            		if(cid4046!=NULL)	
								{
									  
									   SetYDT1363(cid4046,TYPE_FOURBYTE_L,&fdat[0],1);//交流电压上限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[1],1);//交流电压下限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[2],1);//交流电流上限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[3],1);//频率上限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[4],1);//频率下限
									 if(isTieta==1)//铁塔协议
									 {
										  SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,1);//自定义数
										  SetYDT1363(NULL   ,TYPE_FOURBYTE_L,NULL,0x81);//油机功率
										  SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,0);//自定义数
									 }
                   else
									 {										 
									   SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,5);//自定义数
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,NULL,0x81);//油机功率
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,NULL,0x81);//燃料余量下限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,NULL,0x81);//燃料箱额定容量
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,NULL,0x81);//发电机启动时间
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,NULL,0x81);//发电机关闭时间
									 }
									   END_SET_YDT1363//结束标志，用以计算长度
									
								}
								 pdat=Getdata(cid4046,0,NULL,NULL);
			           vPortFree(cid4046);
			           cid4046=NULL;
								
		}
		else if(cmd[0]==0x41)
		{  float fdat[6];
			
			
			
			
			   ytd1363data * cid4146=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*6); 
          if(cid4146!=NULL)
					{   
						fdat[0]=(float)gMaxModuleCurr/10;
						fdat[1]=gsleepLoadRate;
						fdat[2]=gsleepWakeupLoadRate;
						fdat[3]=47;
						fdat[4]=gsleepStartDelay;
						
						SetYDT1363(cid4146,TYPE_FOURBYTE_L,&fdat[0],1);//模块限流值
						SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[1],1);//进入休眠负载率
						SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[2],1);//退出休眠负载率
						SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[3],1);//退出休眠电压
						SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat[4],1);//休眠启动延迟
						 END_SET_YDT1363//结束标志，用以计算长度
						
					}	
          pdat=Getdata(cid4146,0,NULL,NULL);
			           vPortFree(cid4146);
			           cid4146=NULL;					
			
		}
    else	if(cmd[0]==0x42)
		{
			    float fdat[32];
			    fdat[0]=((float)gOutOverVLimit)/100;
			    fdat[1]=((float)goutPutOwrV)/100;
			    fdat[2]=(float)battGroud;//电池组数
			    fdat[3]=(float)gSetBattC;//电池容量
			    fdat[4]=((float)gSetfloatV)/100;//浮充电压
			    fdat[5]=((float)gSetBattLimitPerC)/100;///电池充电限流系数
    	    fdat[6]=((float)ghtemp)/100;//电池高温阈值
			    fdat[7]=((float)(*(s16 *)(&gltemp)))/100;//电池低温阈值
			    fdat[8]=(float)gSleepOnoff;//节能
			    fdat[9]=(float)gsleepMinCount;//开机数目
			    fdat[10]=(float)gsleepTurnPeriod/24;//轮转周期 
			    fdat[11]=(float)gtestTimeout*60;//电池测试时间 //以分钟为单位
			    fdat[12]=(float)gDcParam0[0].st_downModle;//下电模式
		
			   if(gDcParam0[0].st_downModle==0)
				 {
					 fdat[13]=(float)0;//下电禁止
				 }
				 else
				 {
					  fdat[13]=(float)1;//下电使能
				 }
			    fdat[14]=(float)gDcParam0[0].st_LDVoltage/100;//下电电压
			    fdat[15]=(float)gDcParam1[0].st_LDDelay/10;//下电时间（分钟）
			    fdat[16]=(float)gAutoBootEnable;//周期均充使能
			    fdat[17]=(float)gSetequalV/100;//均充值
			    fdat[18]=((float)gAutoPeriod);//均充周期
			    fdat[19]=(float)gtestPeriodEnable;//周期测试使能
			    fdat[20]=(float)gtestV/100;//测试终止电压
				  fdat[21]=(float)gtestPeriod;//电池测试周期，单位为天
				  fdat[22]=(float)gtestTimeout;//测试终止时间页面以小时表示
			    fdat[23]=(float)0;
				  fdat[24]=(float)0;
				  fdat[25]=(float)gbranchBatt[0];
				  fdat[26]=(float)gbranchBatt[1];
			    fdat[27]=(float)gbranchBatt[2];
				  fdat[28]=(float)gbranchBatt[3];
				  fdat[29]=(float)gbranchBatt[4];
			    fdat[30]=(float)gbranchBatt[5];
				  fdat[31]=(float)0;

//     if(isTieta==1)	
//		 {		//A协议	 
		   u8 defcount=1+battGroud+21;
			     fdat[10]=(float)gsleepTurnPeriod/24;//以天为单位
			 
			     
			 
			 if(gbattGroud>0)//配电单元用用电池
			 {
					 
					 fdat[12]=(float)(gSwitchPara[41].st_downMode&0x0f);
				 
				   if((gSwitchPara[41].st_downMode&0x0f)==0)
					 {
						 fdat[13]=(float)0;//下电禁止
					 }
					 else
					 {
							fdat[13]=(float)1;//下电使能
					 }
				  
					  fdat[14]=(float)gSwitchPara[41].st_downV/100;
					  fdat[15]=(float)gSwitchPara[41].st_downT;
					 
				} 
				 

        fdat[23]=(float)(gSwitchPara[34].st_downMode&0x0F);//负载下电模式
				 
				 
				 
				if(gWorkShiftMode==1)//备电模式
				{
					   fdat[24]=1;
				}
        else if( gPeakShaveOnoff==1)
				{
					  fdat[24]=2;
				}
				else
				{
					 fdat[24]=0;
				}					
				 
				 
			 
			 
			 ytd1363data * cid4246=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*(28+6));	//电池组数按最大6个设置
			     if(cid4246!=NULL)	
				{					
			    SetYDT1363(cid4246,TYPE_FOURBYTE_L,&fdat[0],1);//电压上限
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[1],1);//电压下限
			    SetYDT1363(NULL   ,TYPE_ONEBYTE,&defcount ,1);//自定义数
					
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[2],1);//电池组数	
					for(u8 i=0;i<battGroud;i++)
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[25+i],1);//电池容量  fdat[3]				
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[4],1);//浮充电压
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[5],1);///电池充电限流系数
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[6],1);//电池高温阈值
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[7],1);//电池低温阈值
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[8],1);//节能
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[9],1);//开机最小数目
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[10],1);//轮转周期
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[11],1);//电池测试时间
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[12],1);//下电模式
			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[13],1);//下电使能
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[14],1);//下电电压
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[15],1);//下电时间（分钟）
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[16],1);//周期均充使能
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[17],1);//均充值电压
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[18],1);//均充周期
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[19],1);//周期测试使能
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[20],1);//测试终止电压
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[21],1);//电池测试周期
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[22],1);//测试终止时间
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[24],1);//电池备电
					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[23],1);//负载下电模式
					SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,0);//自定义数
			    END_SET_YDT1363//结束标志，用以计算长度
				}
				 pdat=Getdata(cid4246,0,NULL,NULL);
			   vPortFree(cid4246);
			   cid4246=NULL;
//		 }
//     else
//		 {			 
//				 
//			  	ytd1363data * cid4246=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*27);	
//			     if(cid4246!=NULL)	
//				{					
//			    SetYDT1363(cid4246,TYPE_FOURBYTE_L,&fdat[0],1);//电压上限
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[1],1);//电压下限
//			    SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,1);//自定义数
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[2],1);//电池组数
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[3],1);//电池容量1
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[4],1);//浮充电压
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[5],1);///电池充电限流系数
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[6],1);//电池高温阈值
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[7],1);//电池低温阈值
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[8],1);//节能
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[9],1);//开机最小数目
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[10],1);//轮转周期
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[11],1);//电池测试时间
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[12],1);//下电模式
//			    SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[13],1);//下电使能
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[14],1);//下电电压
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[15],1);//下电时间（分钟）
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[16],1);//周期均充使能
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[17],1);//均充值电压
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[18],1);//均充周期
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[19],1);//周期测试使能
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[20],1);//测试终止电压
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[21],1);//电池测试周期
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[22],1);//测试终止时间
//					SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[24],1);//电池备电
//					//SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,0);//自定义数
//			    END_SET_YDT1363//结束标志，用以计算长度
//				}
//				 pdat=Getdata(cid4246,0,NULL,NULL);
//			   vPortFree(cid4246);
//			   cid4246=NULL;
//			}
			
		}
		else if(cmd[0]==0x43)
		{
			   float fdat[2];
			    fdat[0]=((float)gOutOverVLimit)/100;
			    
			    	ytd1363data * cid4346=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*4);
   			
			      if(cid4346!=NULL)	
			   	{		
						   SetYDT1363(cid4346,TYPE_FOURBYTE_L,&fdat[0],1);//电压上限
						   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,NULL,0x81);//电流上限
						   SetYDT1363(NULL   ,TYPE_ONEBYTE,   NULL,0);//自定义数p=0
						    END_SET_YDT1363//结束标志，用以计算长度
					}
			
			   pdat=Getdata(cid4346,0,NULL,NULL);
			   vPortFree(cid4346);
			   cid4346=NULL;
			
			
			
			
		}
		else if(cmd[0]==0xE1)
		{
			
			  float fdat[11];
			ytd1363data * cidE146;
			
			       fdat[0]=(float)70;//环境温度上限
			       fdat[1]=(float)10;//环境温度下限
			       fdat[2]=(float)98;//相对湿度上限
			       fdat[3]=(float)50;//相对湿度下限
			       fdat[4]=(float)9600;//波特率
			       fdat[5]=(float)1;//设备地址
			       fdat[6]=(float)1;//1363透传端口
			       fdat[7]=(float)2;//Modbus端口
			        if(gbeeEnable==1)
			           fdat[8]=(float)0;//告警声禁止
							else
								 fdat[8]=(float)1;//告警声禁止
			       fdat[9]=(float)gGetEnergyDay;//自动抄表日
			       fdat[10]=(float)0;//自动抄表时间
			
			
			   cidE146=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*78);
            		if(cidE146!=NULL)	
								{
			              SetYDT1363(cidE146,TYPE_FOURBYTE_L,&fdat[0],1);//环境温度上限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[1],1);//环境温度下限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[2],1);//相对湿度上限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[3],1);//相对湿度下限
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[4],1);//波特率
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[5],1);//设备地址
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[6],1);//1363透传端口
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[7],1);//Modbus端口
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[8],1);//告警声禁止
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[9],1);//自动抄表日
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[10],1);//自动抄表时间
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L,&fdat[10],1);//手动控制维持时间
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L  ,gIP,1);//IP
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L  ,gMask,1);//子网掩码
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L  ,gGW,1);//默认网关
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L  ,gDNS,1);//主DNS
									   SetYDT1363(NULL   ,TYPE_FOURBYTE_L  ,gDNS,1);//备DNS
									
									
									    for(u8 i=0;i<20;i++)
									{
										   SetYDT1363(NULL,TYPE_ONEBYTE, NULL,0x81);//业主名称
									}
									
									
									    for(u8 i=0;i<20;i++)
									{
										   SetYDT1363(NULL,TYPE_ONEBYTE, NULL,0x81);//资产编号
									}
									
									 for(u8 i=0;i<20;i++)
									{
										   SetYDT1363(NULL,TYPE_ONEBYTE, NULL,0x81);//站点名称
									}
									
									
									   END_SET_YDT1363//结束标志，用以计算长度
									
									
									
								}
								
								pdat=Getdata(cidE146,0,NULL,NULL);
			           vPortFree(cidE146);
			           cidE146=NULL;
			
			
			       
		}
		else if(cmd[0]==0xE2)
		{
			  	
			    u8 command_groud=0;
			    AsciitoHex(&command_groud,&predat[0]);
			    ytd1363data * cidE246;
			  
						 
					 float fdat[6]={0,0,0,0,0,0};//,1,1};
					 float tdowntime[6]={0,0,0,0,0,0};//,10,10}; 
					 float tdownVolit[6]={0,0,0,0,0,0};//,46.5,46.5};
					 float tname[6]={1,2,3,4,6,5};//移动，联通，电信，广电，铁塔，行业外
					 u16 _year[2]={2121,2121};//,2121,2121};
					 u8 _mon[2]={5,5};//,5,5};
					 u8 _day[2]={5,5};//,5,5};
					 u8 _h[2]={4,4};//,4,4};
					 u8 _m[2]={4,4};//,4,4};	
           u8 _s[2]={4,4};//,4,4};						 
					 

					 
					 
					

     	
          if(isTieta==1){//铁塔协议							 
						 

								
						
								for(u8 i=0;i<6;i++)//统计已配置租户的个数 并取得相关租户参数
								{
									  
										
											if((gSwitchPara[35+i].st_downMode&0x0F)>0)
											{
												fdat[i]=1;
											}
											else
											{
												 fdat[i]=0;
											}
											
											tdownVolit[i]=(float)gSwitchPara[35+i].st_downV/100;
											tdowntime[i]=(float)gSwitchPara[35+i].st_downT;
											//tname[i]=gSwitchPara[35+i].st_userGroud>>4;
										
								}
								
								if(command_groud==0xff)
							{
								
              
								cidE246=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*9);//29
									
											 if(cidE246!=NULL)
											 {
				 
													
														 SetYDT1363(cidE246,TYPE_EXTERN|TYPE_ONEBYTE,&cidE246[2] ,6);//模块数据扩展 用户数
														 END_SET_YDT1363//结束标志，用以计算长度
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,&fdat[0],1);//一次下电使能
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,&tdowntime[0],1);//一次下电时间
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,&tdownVolit[0],1);//一次下电时间
														 SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,1);//自定义数
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,&tname[0],1);	//租户名称编码
														 SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,0);//自定义数
												     END_SET_YDT1363//结束标志，用以计算长度
														
												      pdat=Getdata(cidE246,0,NULL,NULL);
			       
															 vPortFree(cidE246);
															 cidE246=NULL;
											 }										 
							}
							else
							{
								        if(command_groud>6) return NULL;
								      	cidE246=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*7);//29
								         if(cidE246!=NULL)
											 {
								             SetYDT1363(cidE246,TYPE_FOURBYTE_L ,&fdat[command_groud],1);//一次下电使能
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,&tdowntime[command_groud],1);//一次下电时间
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,&tdownVolit[command_groud],1);//一次下电时间
														 SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,1);//自定义数
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,&tname[command_groud],1);	//租户名称编码
														 SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,0);//自定义数
								              END_SET_YDT1363//结束标志，用以计算长度
								             pdat=Getdata(cidE246,0,NULL,NULL);
													 
													 vPortFree(cidE246);
													 cidE246=NULL;
											 }
								
							}
							
							
							

	       }
				 else//自定义协议
				 {    cidE246=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*36);//29
	
					       
									
											 if(cidE246!=NULL)
											 {
														 
														
														 SetYDT1363(cidE246,TYPE_EXTERN|TYPE_ONEBYTE,&cidE246[2] ,2);//模块数据扩展 用户数
														 
														 END_SET_YDT1363//结束标志，用以计算长度
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,fdat,1);//一次下电使能
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,tdowntime,1);//一次下电时间
														 SetYDT1363(NULL,TYPE_FOURBYTE_L ,tdownVolit,1);//一次下电时间
														 SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,2);//自定义数
																for(u8 i=0;i<20;i++)
														SetYDT1363(NULL   ,TYPE_ONEBYTE, (u8 *)&userName[4*i] ,1);
												    SetYDT1363(NULL   ,TYPE_TWOBYTE, (u8 *)&_year ,1);
															 SetYDT1363(NULL   ,TYPE_ONEBYTE, (u8 *)&_mon ,1);
															 SetYDT1363(NULL   ,TYPE_ONEBYTE, (u8 *)&_day ,1);
															 SetYDT1363(NULL   ,TYPE_ONEBYTE, (u8 *)&_h ,1);
															 SetYDT1363(NULL   ,TYPE_ONEBYTE, (u8 *)&_m ,1);
															 SetYDT1363(NULL   ,TYPE_ONEBYTE, (u8 *)&_s ,1);
															 END_SET_YDT1363//结束标志，用以计算长度
															
											 }	
											 
								
							
											 
						


							pdat=Getdata(cidE246,0,NULL,NULL);
			    
			       vPortFree(cidE246);
			       cidE246=NULL;
											 
				 }
			
				
			
		}
		
		
		
	}
	else if(cmd[1]==0x48)//参数设置 
	{
		
		
		
		
		if(cmd[0]==0x40)
		{
			 u32 cmd_dat;
			 u8 cmd_type;
			  
			//if(predat!=NULL){
			if(*port==2){//A协议  H52C0F
			   AsciitoHex(&cmd_type,&predat[0]);
			   cmd_dat=bswap_32(AsciitoHex32(&predat[2]));
			}
			else//网页 (*port==3)
			{
				   cmd_type=cmd[2];
				   cmd_dat=*((u32 *)&cmd[3]);
			}

			

			  
			if(cmd_type==0x80)//交流电压高阀值
			{
				  gInOverVLimit=*((float*)&cmd_dat)*100;
				  pgh52c0->savePara(&gInOverVLimit);
				
			}
			else if(cmd_type==0x81)//交流电压低阀值
			{
				  gInOwrVLimit=*((float*)&cmd_dat)*100;
				  pgh52c0->savePara(&gInOwrVLimit);
			}
			else if(cmd_type==0x82)//交流电流高阀值
			{
				   gInOverILimit=*((float*)&cmd_dat)*100;
				   pgh52c0->savePara(&gInOverILimit);
			}
			else if(cmd_type==0x83)//交流频率高阀值
			{
				  gOverFLimit=*((float*)&cmd_dat)*100;
				  pgh52c0->savePara(&gOverFLimit);
			}
			else if(cmd_type==0x84)//交流频率低阀值
			{
				  gOwrFLimit=*((float*)&cmd_dat)*100;
				  pgh52c0->savePara(&gOwrFLimit);
			}
		
			
			
			
			
		}
		
	  else if(cmd[0]==0x41)
		{    
			   u32 cmd_dat;
			   u8 cmd_type;
				
		
       if(*port==2){//A协议  H52C0F
			   AsciitoHex(&cmd_type,&predat[0]);
			   cmd_dat=bswap_32(AsciitoHex32(&predat[2]));
			}
			else//网页 (*port==3)
			{
				   cmd_type=cmd[2];
				   cmd_dat=*((u32 *)&cmd[3]);
			}
			
			   	if(cmd_type==0x80)//模块限流值
					{
									gMaxModuleCurr=*((float*)&cmd_dat)*10;
						       pgh52c0->savePara(&gMaxModuleCurr);
								   
					}
			   else if(cmd_type==0x81)//进入休眠负载率
					{
									gsleepLoadRate=*((float*)&cmd_dat);
								   pgh52c0->savePara(&gsleepLoadRate);
					}
			   else if(cmd_type==0x82)//退出休眠负载率
					{
									gsleepWakeupLoadRate=*((float*)&cmd_dat);
								  pgh52c0->savePara(&gsleepLoadRate);
					}
			   else if(cmd_type==0x83)//退出休眠电压
				 {
									
				 }
			   else if(cmd_type==0x84)//启动休眠延迟
				 {
									gsleepStartDelay=*((float*)&cmd_dat);
							
				 }
			
			
		}
		else if(cmd[0]==0x42)
		{
			 u32 cmd_dat;
			 u8 cmd_type;
		
		if(*port==2){//A协议  H52C0F
			   AsciitoHex(&cmd_type,&predat[0]);
			   cmd_dat=bswap_32(AsciitoHex32(&predat[2]));
		
			}
			else//网页 (*port==3)
			{
				   cmd_type=cmd[2];
				   cmd_dat=*((u32 *)&cmd[3]);
				   
			}

			
			      	if(cmd_type==0x80)//直流电压高阀值
							{
									gOutOverVLimit=*((float*)&cmd_dat)*100;
								   pgh52c0->savePara(&gOutOverVLimit);
							}
							else if(cmd_type==0x81)//直流电压低阀值
							{
 
								
									goutPutOwrV=*((float*)&cmd_dat)*100;
								  pgh52c0->savePara(&goutPutOwrV);   
							}
							else if(cmd_type==0xA0)//电池组1容量 //电池组容量1
							{
								if(battGroud>0)
								{

									gbranchBatt[0]=*((float*)&cmd_dat);
									pgh52c0->savePara(& gbranchBatt[0]);
									gSetBattC=0;
									for(u8 i=0;i<battGroud;i++)
									{
										  gSetBattC+=gbranchBatt[i];
									}
									 pbattCap->Init();
									 pgh52c0->savePara(&gSetBattC);
								}

							}
							else if(cmd_type==0xA1)//电池组容量2
							{
								if(battGroud>0)
								{
									gbranchBatt[1]=*((float*)&cmd_dat);
									pgh52c0->savePara(& gbranchBatt[1]);
								  gSetBattC=0;
									for(u8 i=0;i<battGroud;i++)
									{
										  gSetBattC+=gbranchBatt[i];
									}
									 pbattCap->Init();
									 pgh52c0->savePara(&gSetBattC);
								}
							}
							else if(cmd_type==0xA2)//电池组容量3
							{
								if(battGroud>0)
								{
									gbranchBatt[2]=*((float*)&cmd_dat);
									pgh52c0->savePara(& gbranchBatt[2]);
								  gSetBattC=0;
									for(u8 i=0;i<battGroud;i++)
									{
										  gSetBattC+=gbranchBatt[i];
									}
									 pbattCap->Init();
									 pgh52c0->savePara(&gSetBattC);
								}
							}
							else if(cmd_type==0xA3)//电池组容量4
							{
								if(battGroud>0)
								{
									gbranchBatt[3]=*((float*)&cmd_dat);
									pgh52c0->savePara(& gbranchBatt[3]);
								  gSetBattC=0;
									for(u8 i=0;i<battGroud;i++)
									{
										  gSetBattC+=gbranchBatt[i];
									}
									 pbattCap->Init();
									 pgh52c0->savePara(&gSetBattC);
								}
							}
							else if(cmd_type==0xA4)//电池组容量5
							{
								if(battGroud>0)
								{
									gbranchBatt[4]=*((float*)&cmd_dat);
									pgh52c0->savePara(& gbranchBatt[4]);
								  gSetBattC=0;
									for(u8 i=0;i<battGroud;i++)
									{
										  gSetBattC+=gbranchBatt[i];
									}
									 pbattCap->Init();
									 pgh52c0->savePara(&gSetBattC);
								}
							}
								else if(cmd_type==0xA5)//电池组容量6
							{
								if(battGroud>0)
								{
									gbranchBatt[5]=*((float*)&cmd_dat);
									pgh52c0->savePara(& gbranchBatt[5]);
								  gSetBattC=0;
									for(u8 i=0;i<battGroud;i++)
									{
										  gSetBattC+=gbranchBatt[i];
									}
									 pbattCap->Init();
									 pgh52c0->savePara(&gSetBattC);
								}
							}
							else if(cmd_type==0xA8)//浮充电压
							{
									gSetfloatV=*((float*)&cmd_dat)*100;
								  pgh52c0->savePara(&gSetfloatV);
							}
							else if(cmd_type==0xA9)//电池充电系数
							{
									gSetBattLimitPerC=*((float*)&cmd_dat)*100;
								  pgh52c0->savePara(&gSetBattLimitPerC);
								
							}
							else if(cmd_type==0xAA)//电池温度高
							{
									ghtemp=*((float*)&cmd_dat)*100;
								  pgh52c0->savePara(&ghtemp);
							}
							else if(cmd_type==0xAB)//电池温度低
							{
								  *((s16 *)&gltemp)=*((float*)&cmd_dat)*100;
								   pgh52c0->savePara(&gltemp);
							}
							else if(cmd_type==0xAE)//节能
							{
								  gSleepOnoff=(u8)(*((float*)&cmd_dat));
								 pgh52c0->savePara(&gSleepOnoff);
							}
							else if(cmd_type==0xAF)//最小开机数
							{
								  gsleepMinCount=(u8)(*((float*)&cmd_dat));
								  pgh52c0->savePara(&gsleepMinCount);
							}
							else if(cmd_type==0xB0)//轮换周期
							{
								

								 gsleepTurnPeriod=(*((float*)&cmd_dat))*24;//A协议中以天为单位，转为以小时为单位		
								 pMonitor->m_sleepTurnPeriodCount=gsleepTurnPeriod*3600;
								 pgh52c0->savePara(&gsleepTurnPeriod);
							}
			        else if(cmd_type==0xB1)//电池测试时长
							{
								 gsleepBattTestTime=(u16)(*((float*)&cmd_dat));
								  pgh52c0->savePara(&gsleepBattTestTime);
							}
							 else if(cmd_type==0xB2)//下电模式
							{
								 if(gbattGroud==0)
								 {
//									 gbattDownMod=(u8)(*((float*)&cmd_dat));
//									 pgh52c0->savePara(&gbattDownMod);
									 
									    gDcParam0[0].st_downModle=(u8)(*((float*)&cmd_dat));
					
					            pgh52c0->savePara((u16 *)&gDcParam0[0]);//指向st_downModle位置
									 
									     gDcParam0[1].st_downModle=(u8)(*((float*)&cmd_dat));
					
					            pgh52c0->savePara((u16 *)&gDcParam0[1]);//指向st_downModle位置
									 
									 
								 }
								 else
								 {
									 gSwitchPara[41].st_downMode&=0xF0;
								   gSwitchPara[41].st_downMode|=(u8)(*((float*)&cmd_dat));
								   gSetDownParaCode=12;//下电模式命令码
										    gSetDownParaGroudType=0x52;//分组设置
										    gSetDownParaAddr=7;//电池分组
										 
										 
													for(u8 i=0;i<34;i++)
											 {
													if(gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)//同步所有电池分组数据
													{
																	 gSwitchPara[i].st_downMode=gSwitchPara[41].st_downMode;
													}
											 }
									 
								 }
							}
							else if(cmd_type==0xB3)//下电使能
							{
								
								 
                   if(gbattGroud==0)
									 {   
										
//										    gbattDownMod=(u8)(*((float*)&cmd_dat));
//									      pgh52c0->savePara(&gbattDownMod);
										 
										   gDcParam0[0].st_downModle=(u8)(*((float*)&cmd_dat));
					
					             pgh52c0->savePara((u16 *)&gDcParam0[0]);//指向st_downModle位置
										 
										 
										  gDcParam0[1].st_downModle=(u8)(*((float*)&cmd_dat));
					
					             pgh52c0->savePara((u16 *)&gDcParam0[1]);//指向st_downModle位置
										           
									 }
									 else 
									 {
										    gSwitchPara[41].st_downMode&=0xF0;
										    gSwitchPara[41].st_downMode|=(u8)(*((float*)&cmd_dat));
										    gSetDownParaCode=12;//下电模式命令码
										    gSetDownParaGroudType=0x52;//分组设置
										    gSetDownParaAddr=7;//电池分组
										 
										 
													for(u8 i=0;i<34;i++)
											 {
													if(gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)//同步所有电池分组数据
													{
																	 gSwitchPara[i].st_downMode=gSwitchPara[41].st_downMode;
													}
											 }
										 
										 
										 
									 }
							 
								
									 
							}
							else if(cmd_type==0xB4)//下电电压
							{
								 if(gbattGroud==0)
								 {
//										 gdownV=*((float*)&cmd_dat)*100;
//										 pgh52c0->savePara(&gdownV);
									 
									 gDcParam0[0].st_LDVoltage=*((float*)&cmd_dat)*100;
                   pgh52c0->savePara((u16 *)&gDcParam0[0]+3);		//指st_LDVoltage位置			
									 
									 gDcParam0[1].st_LDVoltage=*((float*)&cmd_dat)*100;
                   pgh52c0->savePara((u16 *)&gDcParam0[1]+3);		//指st_LDVoltage位置		
								 }
								 else
								 {
									 
									   gSwitchPara[41].st_downV=(*((float*)&cmd_dat))*100;
									    gSetDownParaCode=4;//下电模式命令码
										    gSetDownParaGroudType=0x52;//分组设置
										    gSetDownParaAddr=7;//电池分组
									 
									  for(u8 i=0;i<34;i++)
											 {
												  if(gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)//同步所有电池分组数据
													{
														       gSwitchPara[i].st_downV=gSwitchPara[41].st_downV;
													}
											}
									 
								 }
								
							}
							else if(cmd_type==0xB5)//下电时间
							{
								
								if(gbattGroud==0)
								{
//									 gbattDownTime=(u16)(*((float*)&cmd_dat));
//									 pgh52c0->savePara(&gbattDownTime);
									
									 gDcParam1[0].st_LDDelay=(u16)(*((float*)&cmd_dat))*10;  
					         pgh52c0->savePara((u16 *)&gDcParam1[0]+2);	//指st_LDDelay位置	

                   gDcParam1[1].st_LDDelay=(u16)(*((float*)&cmd_dat))*10;  
					         pgh52c0->savePara((u16 *)&gDcParam1[1]+2);	//指st_LDDelay位置										
									
								}
								else
								{//电组数设置大于0时，使能在设项设置
									
											gSwitchPara[41].st_downT=(u16)(*((float*)&cmd_dat));
									      gSetDownParaCode=5;//时间下电模式命令码
									      gSetDownParaGroudType=0x52;//分组设置
										    gSetDownParaAddr=7;//电池分组
									 
									  for(u8 i=0;i<34;i++)
											 {
												  if(gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)//同步所有电池分组数据
													{
														       gSwitchPara[i].st_downT=gSwitchPara[41].st_downT;
													}
											}
									
								}
										
								
							}
							else if(cmd_type==0xB6)//周期均充使能
							{
								gAutoBootEnable=(u8)(*((float*)&cmd_dat));
								  pgh52c0->savePara(&gAutoBootEnable);
							}
							else if(cmd_type==0xB7)//均充电压
							{
								   gSetequalV=*((float*)&cmd_dat)*100;
								  pgh52c0->savePara(&gSetequalV);
							}
							else if(cmd_type==0xB8)//电池均充周期
							{
								 gAutoPeriod=*((float*)&cmd_dat);
								 pMonitor->m_Autobootcount=86400*gAutoPeriod;
								  pgh52c0->savePara(&gAutoPeriod);
								
							}
								else if(cmd_type==0xB9)//电池周期测试使能
							{
								  gtestPeriodEnable=(u8)(*((float*)&cmd_dat));
								   pgh52c0->savePara(&gtestPeriodEnable);
							}
								else if(cmd_type==0xBA)//电池测试终止电压
							{
								   gtestV=*((float*)&cmd_dat)*100;
								   pgh52c0->savePara(&gtestV);
							}
							
							else if(cmd_type==0xBB)//电池测试周期
							{    
								   gtestPeriod=(*((float*)&cmd_dat));
								   pgh52c0->savePara(&gtestPeriod);

							}
			        else if(cmd_type==0xBC)//电池测试终止时间
							{
								   gtestTimeout=*((float*)&cmd_dat); //页面以小时表示
									 pgh52c0->savePara(&gtestTimeout);
							}
			        else if(cmd_type==0xBD)//电池备电模式
							{
								   u8 tmp8=(u8)(*((float*)&cmd_dat)); //页面以小时表示，因些"*60",变为分钟
								
								     if(tmp8==0)
										 {
											 gWorkShiftMode=0;	
											 gPeakShaveOnoff=0;
											
										 }
										 else if(tmp8==1)
										 {
											  gWorkShiftMode=1;
											  gPeakShaveOnoff=0;
										 }
										 else if(tmp8==2)
										 {
											  gWorkShiftMode=0;
											  gPeakShaveOnoff=1;
										 }
								
								   	
								
									 pgh52c0->savePara(&gWorkShiftMode);
									 pgh52c0->savePara(&gPeakShaveOnoff);
							}
							else if(cmd_type==0xBE)//负载下电模式
							{
							
								 gSwitchPara[34].st_downMode&=0xF0;
								 gSwitchPara[34].st_downMode|=(u8)(*((float*)&cmd_dat));
								
								       gSetDownParaCode=12;//下电模式命令码
										    gSetDownParaGroudType=0x52;//分组设置
										    gSetDownParaAddr=0x0F;//设置所有分路
										 
										 
													for(u8 i=0;i<34;i++)
											 {
													
																	 gSwitchPara[i].st_downMode=gSwitchPara[34].st_downMode;
													
											 }
							}
			
			
			
		}
	  else if(cmd[0]==0xE1)
		{ 
			
			 u32 cmd_dat;
			 u8 cmd_type;
			
			
			   if(*port==2){//A协议  H52C0F
			   AsciitoHex(&cmd_type,&predat[0]);
			   cmd_dat=bswap_32(AsciitoHex32(&predat[2]));
				 //ptypy=0;
			}
			else//网页 (*port==3)
			{
				   cmd_type=cmd[2];
				   cmd_dat=*((u32 *)&cmd[3]);
				    //ptypy=1;
			}
			
			    if(cmd_type==0x80)
					{
						// 环境温度上限
					}
					else if(cmd_type==0x81)
					{
						  //环境温度下限
					}
					else if(cmd_type==0x82)
					{    
						   //相对湿度上限
					}
					else if(cmd_type==0x83)
					{
						   // 相对湿度下限
					}
						else if(cmd_type==0xA0)
					{
						   // 波特率
					}
			    		else if(cmd_type==0xA1)
					{
						   // 设备地址
					}
								else if(cmd_type==0xA2)
					{
						   //1363透传端口
					}
									else if(cmd_type==0xA3)
					{
						   //Modbus端口
					}
								else if(cmd_type==0xA4)
					{
						u8 tmp=(u8)(*((float*)&cmd_dat));
									 if(tmp==0) 
											gbeeEnable=1;
									 else
											gbeeEnable=0;
								 pgh52c0->savePara(&gbeeEnable);//告警蜂鸣器
					}
						else if(cmd_type==0xA5)
					{
						  gGetEnergyDay=(u8)(*((float*)&cmd_dat));
								 pgh52c0->savePara(&gGetEnergyDay);//自动抄表日
					}
					else if(cmd_type==0xA6)
					{
						 //自动抄表时间
					}
							else if(cmd_type==0xA7)
					{
						 //手动控制维持时间
					}
						else if(cmd_type==0xA8)
					{
						
						  
						   *(u32 *)gIP=bswap_32(AsciitoHex32(&predat[2]));
						   pgh52c0->writeI2C(SYS_INFO_START+40,gIP,4);
						
						 //IP
					}
							else if(cmd_type==0xA9)
					{
						
                   *(u32 *)gMask=bswap_32(AsciitoHex32(&predat[2]));
//						   
						 //子网掩码
					}
					
								else if(cmd_type==0xAA)
					{
						
                 *(u32 *)gGW=bswap_32(AsciitoHex32(&predat[2]));
   						   pgh52c0->writeI2C(SYS_INFO_START+44,gGW,4);
						 //网关
					}
			
					else if(cmd_type==0xAB)
					{
						
               *(u32 *)gDNS=bswap_32(AsciitoHex32(&predat[2]));
//						 //主DNS
					}
					
					else if(cmd_type==0xAC)
					{
						
              *(u32 *)gDNS=bswap_32(AsciitoHex32(&predat[2]));
						 //备用DNS
					}
					else if(cmd_type==0xAD)
					{
						
						  //业主名
					}
					else if(cmd_type==0xAE)
					{
						
						  //资产编号
					}
			    	else if(cmd_type==0xAF)
					{
						
						  //站点名
					}
		}
		else if(cmd[0]==0xE2) //租户参数
		{
				u32 cmd_dat;
			  u8 cmd_type;
			  u8 commd_groud;
			  u8 setaddr;
			  
				
				       
          if(*port==2){//A协议  H52C0F
									AsciitoHex(&commd_groud,&predat[0]);
									AsciitoHex(&cmd_type,&predat[2]);
									cmd_dat=bswap_32(AsciitoHex32(&predat[4]));
							 }
							 else//自定议协议
							 {
									commd_groud=cmd[2];
									cmd_type=cmd[3];
									cmd_dat=*((u32 *)&cmd[4]);
							 }
							 
							 if((cmd_type<0x80)||(cmd_type>0x82)) return NULL;//无效命令类型，直接退出
						

			     if(((commd_groud>0)&&(commd_groud<7))||(commd_groud==0xff))
					 {
						 
								 if(commd_groud==0xff)
								 {
									 commd_groud=0;
									 gSetDownParaAddr=0x0f;
								 }
							 
						 
							 if(cmd_type==0x80)//下电1使能
									{
										 gSwitchPara[34+commd_groud].st_downMode&=0xF0;
										 gSwitchPara[34+commd_groud].st_downMode|=(u8)(*((float*)&cmd_dat));
											 gSetDownParaCode=12;//下电模式命令码
														gSetDownParaGroudType=0x52;//分组设置
														gSetDownParaAddr=commd_groud;//电池分组
										
										
										    	for(u8 i=0;i<34;i++)
											 {
													if((gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)||(commd_groud==0xff))//同步所有分组数据
													{
																	 gSwitchPara[i].st_downMode=gSwitchPara[34+commd_groud].st_downMode;
													}
											 }
									}
									else if(cmd_type==0x81)//下电时间
									{
										  gSwitchPara[34+commd_groud].st_downT=(u16)(*((float*)&cmd_dat));
											 gSetDownParaCode=5;//下电时间命令码
														gSetDownParaGroudType=0x52;//分组设置
														gSetDownParaAddr=commd_groud;//电池分组
										
										
										    	for(u8 i=0;i<34;i++)
											 {
													if((gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)||(commd_groud==0xff))//同步所有分组数据
													{
																	 gSwitchPara[i].st_downT=gSwitchPara[34+commd_groud].st_downT;
													}
											 }
									}
									else if(cmd_type==0x82)//下电电压
										{
										  gSwitchPara[34+commd_groud].st_downV=(*((float*)&cmd_dat))*100;
											 gSetDownParaCode=4;//下电电压命令码
														gSetDownParaGroudType=0x52;//分组设置
														gSetDownParaAddr=commd_groud;//电池分组
										
										
										    	for(u8 i=0;i<34;i++)
											 {
													if((gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)||(commd_groud==0xff))//同步所有分组数据
													{
																	 gSwitchPara[i].st_downV=gSwitchPara[34+commd_groud].st_downV;
													}
											 }
									}
										
						}
		
		
		}
		
	}
	else if(cmd[1]==0x4D) // 7.6	获取设备时间
	{
		        u16 Year=gTimer.year;
		        u8   Mon=gTimer.mon;
		        u8   Day=gTimer.day;
		        u8   Hour=gTimer.hour;
		        u8   Min=gTimer.min;
		        u8   Sec=gTimer.sec;
		
		   ytd1363data * cidXX4D=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*7);	
		     if(cidXX4D!=NULL)
				 {
					 SetYDT1363(cidXX4D,TYPE_TWOBYTE,&Year ,1);//年
					 SetYDT1363(NULL,   TYPE_ONEBYTE,&Mon ,1);//月
					 SetYDT1363(NULL,   TYPE_ONEBYTE,&Day ,1);//日 
					 SetYDT1363(NULL,   TYPE_ONEBYTE,&Hour ,1);//时
					 SetYDT1363(NULL,   TYPE_ONEBYTE,&Min ,1);//分
					 SetYDT1363(NULL,   TYPE_ONEBYTE,&Sec ,1);//秒
					 
				  END_SET_YDT1363//结束标志，用以计算长度
				}
				 pdat=Getdata(cidXX4D,0,NULL,NULL);
			   vPortFree(cidXX4D);
			   cidXX4D=NULL;				 
	}
	else if(cmd[1]==0x4E) //  7.7	设置设备时间
	{
		       u8  Mon=0;
					 u8  Day=0;
					 u8  Hour=0;
					 u8  Min=0;
					 u8  Sec=0;
					 u16 Year=0;
		
		
		   
            if(*port==2){//A协议  H52C0F
		          Year=AsciitoHex16(&predat[0]);
					        
					         AsciitoHex(&Mon,&predat[4]);			         
					         AsciitoHex(&Day,&predat[6]);   
					         AsciitoHex(&Hour,&predat[8]);		       
					         AsciitoHex(&Min,&predat[10]);
					         AsciitoHex(&Sec,&predat[12]);
					 
					      RTC_Set(Year,Mon,Day,Hour,Min,Sec);
					        
					  
			   }
				 else//网页
				 {
					       Year =(cmd[2]<<8)|cmd[3];
					        Mon=cmd[4];
					        Day=cmd[5];
					       Hour=cmd[6];
					        Min=cmd[7];
					        Sec=cmd[8];
					 
					      RTC_Set(Year,Mon,Day,Hour,Min,Sec);
				 }
	}
		
	else if(cmd[1]==0x4F) //  7.8	获取协议版本号
	{
//		 if(cmd[0]==0x40)
//		 {
//		 }
	 }
	else if(cmd[1]==0x50) // 7.9	获取地址
	{
		
	}
	else if(cmd[1]==0x51) //  7.10	获取厂家信息
	{
		
		//if((cmd[2]==0x10)&&(predat!=NULL))//A协议
		if(isTieta==1)
		{  	ytd1363data * cidXX51=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*71);	
			   u8 name[]="H52C0";
			   
			     u8 fm[]="Mentech";
			     //u8 fm[]="ZTKD";
			   
			    
			     u8 fill=0x20;
			   if(cidXX51!=NULL)
			     {					
								SetYDT1363(cidXX51,TYPE_ONEBYTE,&name[0] ,1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&name[1] ,1);	
								SetYDT1363(NULL,TYPE_ONEBYTE,&name[2] ,1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&name[3] ,1);	
								SetYDT1363(NULL,TYPE_ONEBYTE,&name[4] ,1);				 
									 for(u8 i=0;i<25;i++)
									SetYDT1363(NULL,TYPE_ONEBYTE,&fill,1);
						 
								SetYDT1363(NULL,TYPE_ONEBYTE,&gsw[0],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&gsw[1],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&gsw[2],1);  
								SetYDT1363(NULL,TYPE_ONEBYTE,&gsw[3],1);	 
								SetYDT1363(NULL,TYPE_ONEBYTE,&gsw[4],1);
						    SetYDT1363(NULL,TYPE_ONEBYTE,&gsw[5],1);
											 for(u8 i=0;i<14;i++)
								SetYDT1363(NULL,TYPE_ONEBYTE,&fill,1);	
						 
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[0],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[1],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[2],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[3],1);	 
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[4],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[5],1);	
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[6],1);	 									
								for(u8 i=0;i<13;i++)
								SetYDT1363(NULL,TYPE_ONEBYTE,&fill,1);	
								END_SET_YDT1363//结束标志，用以计算长度
				}
				 pdat=Getdata(cidXX51,0,NULL,NULL);
			   vPortFree(cidXX51);
			   cidXX51=NULL;
			
			
			
		}
		else
		{
			  	ytd1363data * cidXX51=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*71);	
			   u8 name[]="H52C0";
			     u8 sw[]={1,0};
			     u8 fm[]="Mentech Power";
			     u8 fill=0x20;
					 
				
					 
					 
			   if(cidXX51!=NULL)
			     {	
						 SetYDT1363(cidXX51,TYPE_ONEBYTE,&name[0] ,1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&name[1] ,1);	
								SetYDT1363(NULL,TYPE_ONEBYTE,&name[2] ,1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&name[3] ,1);	
								SetYDT1363(NULL,TYPE_ONEBYTE,&name[4] ,1);							 
									 for(u8 i=0;i<5;i++)
						   SetYDT1363(NULL,TYPE_ONEBYTE,&fill,1);
						 
						   SetYDT1363(NULL,TYPE_ONEBYTE,&sw[0],1);
						   SetYDT1363(NULL,TYPE_ONEBYTE,&sw[1],1);
						 
						    SetYDT1363(NULL,TYPE_ONEBYTE,&fm[0],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[1],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[2],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[3],1);	 
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[4],1);
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[5],1);	
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[6],1);	 
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[7],1);	 
								SetYDT1363(NULL,TYPE_ONEBYTE,&fm[8],1);	
  							SetYDT1363(NULL,TYPE_ONEBYTE,&fm[9],1);	
                SetYDT1363(NULL,TYPE_ONEBYTE,&fm[10],1);	
                SetYDT1363(NULL,TYPE_ONEBYTE,&fm[11],1);
                SetYDT1363(NULL,TYPE_ONEBYTE,&fm[12],1);	
										
										for(u8 i=0;i<7;i++)
									SetYDT1363(NULL,TYPE_ONEBYTE,&fill,1);	
								END_SET_YDT1363//结束标志，用以计算长度
						 
					 }
			
			   pdat=Getdata(cidXX51,0,NULL,NULL);
			   vPortFree(cidXX51);
			   cidXX51=NULL;
		}
		
	}
	else if(cmd[1]==0x80) //运行状态
	{
		
	
		
		
		if(cmd[0]==0x42)
		{
			 u8 len=7;
			 u8 command_groud=0;
			   AsciitoHex(&command_groud,&predat[0]);
			
			
			     if(command_groud==0xff)
			       len=len+1;
					 
					 ytd1363data * cid4280=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*len);
							  if(cid4280!=NULL)
			    {			 
				 
					 SetYDT1363(cid4280,TYPE_ONEBYTE,&dataflag ,1);//dataflag
					 if(command_groud==0xff)
					 SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//瓶数
					 
					
						SetYDT1363(NULL,TYPE_ONEBYTE,&gcourrentWorkStatus,1);//充电状态
						SetYDT1363(NULL,TYPE_ONEBYTE,&gSleepOnoff ,1);//节能状态
					  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,1);//自定义数a
						SetYDT1363(NULL,TYPE_ONEBYTE,&gPeakShaveOnoff ,1);//削峰状态
					  SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//自定义数b
						END_SET_YDT1363//结束标志，用以计算长度
			    }
		  pdat=Getdata(cid4280,0,NULL,NULL);
			 vPortFree(cid4280);	
			 cid4280=NULL;
    
					 
					 
			
		}
	}
	else if(cmd[1]==0xA0)//复位系统
	{
		  if(cmd[0]==0xE1)
			{
				
			 u8 cmd_type;
			  
			   AsciitoHex(&cmd_type,&predat[0]);
		
				if(cmd_type==0x80)
				{
					SoftReset();//复位系统
				}
				
			}
	}
	else if(cmd[1]==0xA1)//获取系统信息
	{
	
		  if(cmd[0]==0xE1)
			{
				
				ytd1363data * cidE1A1=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*35);
				   if(cidE1A1!=0)
					 {
						    u8 strSN1[]="H52C0";
						    u8 strSN2=0x20;
						    float fdat=(float)(ModuleCap*ModuleOnlineMessage[1]);
						   
						  SetYDT1363(cidE1A1,TYPE_ONEBYTE,&strSN1[0],1);
						  SetYDT1363(NULL,TYPE_ONEBYTE,&strSN1[1],1);
						  SetYDT1363(NULL,TYPE_ONEBYTE,&strSN1[2],1);
						  SetYDT1363(NULL,TYPE_ONEBYTE,&strSN1[3],1);
						  SetYDT1363(NULL,TYPE_ONEBYTE,&strSN1[4],1);
						   for(u8 i=0;i<27;i++)
						  SetYDT1363(NULL,TYPE_ONEBYTE,&strSN2,1);
						  SetYDT1363(NULL,TYPE_FOURBYTE_L,&fdat,1);
						  SetYDT1363(NULL,TYPE_ONEBYTE,&ModuleOnlineMessage[1],1);  
						  END_SET_YDT1363//结束标志，用以计算长度
						 
					 }
				
				    pdat=Getdata(cidE1A1,0,NULL,NULL);
			      vPortFree(cidE1A1);
			      cidE1A1=NULL;			
				
				
				
			}
			
	}
	else if(cmd[1]==0xA3) //配电单元模拟量
	{
			if(cmd[0]==0xE2)
		{

			
			u8 command_groud=0;
			u8 branchcount=0;
			AsciitoHex(&command_groud,&predat[0]);

			
			u8 *ptmpbuff=(u8 *)pvPortMalloc(16*34);
			  if(ptmpbuff<(u8 *)0x20000000)
				{
					vPortFree(ptmpbuff);
					return NULL;
				}
				float * fuserI=(float *)ptmpbuff;
				float * fenergy=(float *)&ptmpbuff[34*4];
				float * ftmp=(float *)&ptmpbuff[2*34*4];
				u8 * fnb=(u8 *)&ptmpbuff[3*34*4];
			
			
	
								 
                 branchcount=0;
								 for(u8 i=0;i<34;i++)
								 {
									    if(SwitchOnlineCount[i]>1)//查找在线用户分路
											{
												
												 fuserI[branchcount]=((float)(*((s16 *)&gDCdistribution.pst_I[i])))/10;
												 fenergy[branchcount]=(float)gDCdistribution.pst_enerqy[i]/10;
												 ftmp[branchcount]=28;
												 fnb[branchcount]=i+1;
												 branchcount++;
											}
								 }
			
			

  
			
			
			
			
			 ytd1363data * cidE2A3;
			  if(command_groud==0xff)
				{
							 cidE2A3=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*10);	
						 if(cidE2A3!=NULL) 
						{
							
									SetYDT1363(cidE2A3,TYPE_ONEBYTE,&dataflag ,1);//dataflag
									SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cidE2A3[3] ,branchcount);//模块数据扩展
									END_SET_YDT1363//结束标志，用以计算长度
							
									SetYDT1363(NULL,TYPE_FOURBYTE_L,fuserI ,1);//模块数据扩展
									SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,3);//铁塔定义数
									SetYDT1363(NULL,TYPE_FOURBYTE_L,fenergy ,1);//模块数据扩展
							    SetYDT1363(NULL,TYPE_FOURBYTE_L,ftmp ,1);
							    SetYDT1363(NULL,TYPE_ONEBYTE,fnb ,1);
									SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家定义数
									END_SET_YDT1363//结束标志，用以计算长度
							
							pdat=Getdata(cidE2A3,0,NULL,NULL);
							
						}
					
			  }
				else
				{       if(command_groud>branchcount)//请求超范围
								{
									  vPortFree(ptmpbuff);
									  return NULL;
								}
					
					
					   cidE2A3=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*8);
					
            if(cidE2A3!=NULL) 
						{		

								  SetYDT1363(cidE2A3,TYPE_ONEBYTE,&dataflag ,1);//dataflag	
					        SetYDT1363(NULL,TYPE_FOURBYTE_L,&fuserI[command_groud-1] ,1);//模块数据扩展
									SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,3);//铁塔定义数
									SetYDT1363(NULL,TYPE_FOURBYTE_L,&fenergy[command_groud-1] ,1);//模块数据扩展
									 SetYDT1363(NULL,TYPE_FOURBYTE_L,&ftmp[command_groud-1] ,1);
							    SetYDT1363(NULL,TYPE_ONEBYTE,&fnb[command_groud-1] ,1);
									SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家定义数
									END_SET_YDT1363//结束标志，用以计算长度
									
									pdat=Getdata(cidE2A3,0,NULL,NULL);
							
						}
				}
				
				 
					 vPortFree(cidE2A3);
			  	vPortFree(ptmpbuff);
					 cidE2A3=NULL;
			
			
		}
	}
	else if(cmd[1]==0xA4)//配电单元告警量
	{
		 if(cmd[0]==0xE2)
		{   
				u8 command_groud=0;
			  AsciitoHex(&command_groud,&predat[0]);
			 ytd1363data * cidE2A4;
			

			  u8 *ptmpbuff=(u8 *)pvPortMalloc(8*34);
			  if(ptmpbuff<(u8 *)0x20000000)
				{					
					//vPortFree(ptmpbuff);
					return NULL;
				}
			
				u8 * puserd=ptmpbuff;//下电
				u8 * puserf=&puserd[34];// 熔丝
				u8 * poverI=&puserf[34];//过流
			  u8 * pbreak=&poverI[34];//故障
			  u8 * preverse=&pbreak[34];//反接
				u8 * fnb=(u8 *)&preverse[34];//序号
			  u8 * povertmp=(u8 *)&fnb[34];//过温
			
			u8* pDC8data=gDCdistribution.pst_status;
			u8 j=0;
			for(u8 i=0;i<34;i++)
			{
				 if(SwitchOnlineCount[i]>1)//查找在线用户分路
					{
					   if(j<34)
						 {     //下电
							    if((pDC8data[i]&0x07)!=0)//&&(gSwitchPara[i].st_downV>*((u16 *)&gpSysData[DCVOLTAGE])))
									{
										  puserd[j]=0x80;
							    }
									else
									{
										 puserd[j]=0;
									}
									
							    //熔丝
				          if((pDC8data[i]&0x40)==0x40)//&&(*((u16 *)&gpSysData[DCVOLTAGE])>=gSwitchPara[i].st_downV))
                  {
											puserf[j]=0x03;
                  }
									else
									{
										   puserf[j]=0;
									}
									
									
									//过载
									 if((pDC8data[i]&0x08)==0x08)
                  {
											poverI[j]=0x80;
                  }
									else
									{
										   poverI[j]=0;
									}
									
									//开关故障
									 if((pDC8data[i]&0x80)==0x80)
                  {
											pbreak[j]=0x80;
                  }
									else
									{
										   pbreak[j]=0;
									}
									
									
									
									//poverI[j]=0;
									//pbreak[j]=0;
							    preverse[j]=0;
									povertmp[j]=0;
							    fnb[j]=i+1;
						 }

						 j++;

				}
				
			}
			
			

			  cidE2A4=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*13);
         	if(cidE2A4<(ytd1363data *)20000000)	
					{
						   vPortFree(cidE2A4);
						   return NULL;
					}						
			
			  if(command_groud==0xff)
				{
								
							 
										SetYDT1363(cidE2A4,TYPE_ONEBYTE,&dataflag ,1);//dataflag
										SetYDT1363(NULL,TYPE_EXTERN|TYPE_ONEBYTE,&cidE2A4[3] ,j);//模块数据扩展 用户数
										END_SET_YDT1363//结束标志，用以计算长度							
										SetYDT1363(NULL,TYPE_ONEBYTE, puserd ,1);//下电
										SetYDT1363(NULL,TYPE_ONEBYTE, puserf ,1);//熔丝
								    SetYDT1363(NULL,TYPE_ONEBYTE, povertmp ,1);//过温
								    SetYDT1363(NULL,TYPE_ONEBYTE, poverI ,1);//过流
								    SetYDT1363(NULL,TYPE_ONEBYTE, pbreak ,1);//故障
								    SetYDT1363(NULL,TYPE_ONEBYTE, preverse ,1);//反接
								    SetYDT1363(NULL,TYPE_ONEBYTE, fnb ,1);//序号
								    SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
								    SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
										END_SET_YDT1363//结束标志，用以计算长度
								

								  pdat=Getdata(cidE2A4,0,NULL,NULL);
				}	
				else
				{
					  if(command_groud>j)
						{
							  	 vPortFree(cidE2A4);
		            	vPortFree(ptmpbuff);
							    return NULL;
						}
					
								    SetYDT1363(cidE2A4,TYPE_ONEBYTE,&dataflag ,1);//dataflag
									  SetYDT1363(NULL,TYPE_ONEBYTE, &puserd[command_groud-1] ,1);//下电
										SetYDT1363(NULL,TYPE_ONEBYTE, &puserf[command_groud-1] ,1);//熔丝
								    SetYDT1363(NULL,TYPE_ONEBYTE, &povertmp[command_groud-1] ,1);//过温
								    SetYDT1363(NULL,TYPE_ONEBYTE, &poverI[command_groud-1] ,1);//过流
								    SetYDT1363(NULL,TYPE_ONEBYTE, &pbreak[command_groud-1] ,1);//故障
								    SetYDT1363(NULL,TYPE_ONEBYTE, &preverse[command_groud-1] ,1);//反接
								    SetYDT1363(NULL,TYPE_ONEBYTE, &fnb[command_groud-1] ,1);//序号
								    SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
								    SetYDT1363(NULL,TYPE_ONEBYTE, NULL ,0);//用户数
										END_SET_YDT1363//结束标志，用以计算长度
										 pdat=Getdata(cidE2A4,0,NULL,NULL);
									
							
					
					
				}
			
			 vPortFree(cidE2A4);
			vPortFree(ptmpbuff);
			 cidE2A4=NULL;
			
			
		}
	}
	else if(cmd[1]==0xA5) // 
	{	
		 if(cmd[0]==0xE2)
		{
			   	
			    u8 command_groud=0;
			    AsciitoHex(&command_groud,&predat[0]);
			    ytd1363data * cidE2A5;
			  
					
	
		if(command_groud!=0xff)			
		{		
		    u8 *ptmpdata=(u8 *)pvPortMalloc(40);
			
			
			   float *pfdat=(float *)ptmpdata;
		     float *pdowntime=&pfdat[1];
			   float *pdownVolit=&pdowntime[1];
			   float *pname=&pdownVolit[1];
			  
			   float *pauthorize=&pname[1];
			   float *ptype=&pauthorize[1];
			   float *poverI=&ptype[1];
			   float *pmod=&poverI[1];
			   u32   *pdate=(u32 *)&pmod[1];
			    u8   *psetid=(u8 *)&pdate[1];
			    u8 branchCount=0,SearchCount=0;
			
				if((command_groud>34)||(command_groud==0))
				{
					 vPortFree(ptmpdata);
					 return NULL;
				}
			
			  
			
			 for( branchCount=0;branchCount<34;branchCount++)
				 {
							if(SwitchOnlineCount[branchCount]>1)//查找在线用户分路
							{
								 SearchCount++;
								
								
								if(SearchCount==command_groud)
									break;
							}
				 }
			

				if((SearchCount==0)||(branchCount==34))//分路不在线或找不分路
				{
					vPortFree(ptmpdata);
					 return NULL;
				}
			
				u8 i=branchCount;
			    pfdat[0]=0;
				   if((gSwitchPara[i].st_downMode&0x0F)>0)
					 {
						 pfdat[0]=1;
						 
					 }
				   
					 pdowntime[0]=gSwitchPara[i].st_downT;
				   pdownVolit[0]=(float)gSwitchPara[i].st_downV/100;
				   pname[0]=gSwitchPara[i].st_userGroud&0x0F;
					 if(pname[0]==0x0B)
						pname[0]=0; 
					 
				   pdate[0]=(gSwitchPara[i].st_stopTime.st_year<<16)|(gSwitchPara[i].st_stopTime.st_mon<<8)|gSwitchPara[i].st_stopTime.st_day;
					   pauthorize[0]=0;
				    if(*(u32*)&gTimerNow<=*(u32*)&gSwitchPara[i].st_stopTime)
						{
							  pauthorize[0]=1;
						}
				    
						ptype[0]=0;
						if(gSwitchPara[i].st_userGroud==0x7B)
						{
							 ptype[0]=2;
						}
						
						poverI[0]=(float)gSwitchPara[i].st_maxoverLoadI/10;
						
						pmod[0]=gSwitchPara[i].st_downMode&0x0F;
						
						psetid[0]=i+1;
			
			
			cidE2A5=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*15);
			        if(cidE2A5!=NULL)
								{
									    
									
							      
								     //SetYDT1363(cidE2A5,TYPE_ONEBYTE,NULL ,1);//模块数据扩展
										 SetYDT1363(cidE2A5,TYPE_FOURBYTE_L ,pfdat,1);//一次下电使能
								     SetYDT1363(NULL,TYPE_FOURBYTE_L ,pdowntime,1);//一次下电时间
										 SetYDT1363(NULL,TYPE_FOURBYTE_L ,pdownVolit,1);//一次下电电压
										 SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,7);//自定义数
										 SetYDT1363(NULL,TYPE_FOURBYTE_L,pname ,1);//配电单元名称
										 SetYDT1363(NULL,TYPE_FOURBYTE,pdate ,1);//年月日
								     SetYDT1363(NULL,TYPE_FOURBYTE_L,pauthorize ,1);//上电授权
										 SetYDT1363(NULL,TYPE_FOURBYTE_L,ptype ,1);//用途
										 SetYDT1363(NULL,TYPE_FOURBYTE_L,poverI ,1);// 过流告警阀值
										 SetYDT1363(NULL,TYPE_FOURBYTE_L,pmod ,1);//下电模式
										 SetYDT1363(NULL   ,TYPE_ONEBYTE,psetid ,1);//序号
										 SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,0);//自定义数
										 END_SET_YDT1363//结束标志，用以计算长度
										 
										   pdat=Getdata(cidE2A5,0,NULL,NULL);
								}
									 
			    vPortFree(cidE2A5);
				  vPortFree(ptmpdata);				
         cidE2A5=NULL;
			
			
			
			
		}
	  else if(command_groud==0xff)
		{
			u8 *ptmpdata=(u8 *)pvPortMalloc(40*34);
			
			
			   float *pfdat=(float *)ptmpdata;
		     float *pdowntime=&pfdat[34];
			   float *pdownVolit=&pdowntime[34];
			   float *pname=&pdownVolit[34];
			  
			   float *pauthorize=&pname[34];
			   float *ptype=&pauthorize[34];
			   float *poverI=&ptype[34];
			   float *pmod=&poverI[34];
			   u32   *pdate=(u32 *)&pmod[34];
			    u8   *psetid=(u8 *)&pdate[34];
			   
			  u8 i=0;
			 for(u8 j=0;j<34;j++)
			{
				
				if(SwitchOnlineCount[j]>1)
				{
				   pfdat[i]=0;
				   if((gSwitchPara[j].st_downMode&0x0F)>0)
					 {
						 pfdat[i]=1;
						 
					 }
				   
					 pdowntime[i]=gSwitchPara[j].st_downT;
				   pdownVolit[i]=(float)gSwitchPara[j].st_downV/100;
				   pname[i]=gSwitchPara[j].st_userGroud&0x0F;
					 if(pname[i]==0x0B)
						pname[i]=0; 
					 
				   pdate[i]=(gSwitchPara[j].st_stopTime.st_year<<16)|(gSwitchPara[j].st_stopTime.st_mon<<8)|gSwitchPara[j].st_stopTime.st_day;
					   pauthorize[i]=0;
				    if(*(u32*)&gTimerNow<=*(u32*)&gSwitchPara[j].st_stopTime)
						{
							  pauthorize[i]=1;
						}
				    
						ptype[i]=0;
						if(gSwitchPara[j].st_userGroud==0x7B)
						{
							 ptype[i]=2;
						}
						
						poverI[i]=gSwitchPara[j].st_maxoverLoadI/10;
						
						pmod[i]=(gSwitchPara[j].st_downMode&0x0F);
						
						psetid[i]=j+1;
						i++;
					}
				
			}
			
			
			
			
			
			
			    
			cidE2A5=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*15);
			        if(cidE2A5!=NULL)
								{
									    
									
							      
								     SetYDT1363(cidE2A5,TYPE_EXTERN|TYPE_ONEBYTE,&cidE2A5[2] ,i);//模块数据扩展
										 END_SET_YDT1363//结束标志，用以计算长度
										 SetYDT1363(NULL,TYPE_FOURBYTE_L ,pfdat,1);//一次下电使能
								     SetYDT1363(NULL,TYPE_FOURBYTE_L ,pdowntime,1);//一次下电时间
										 SetYDT1363(NULL,TYPE_FOURBYTE_L ,pdownVolit,1);//一次下电电压
										 SetYDT1363(NULL,TYPE_ONEBYTE    ,NULL ,7);//自定义数
										 SetYDT1363(NULL,TYPE_FOURBYTE_L,pname ,1);//配电单元名称
										 SetYDT1363(NULL,TYPE_FOURBYTE,pdate ,1);//年月日
								     SetYDT1363(NULL,TYPE_FOURBYTE_L,pauthorize ,1);//上电授权
										 SetYDT1363(NULL,TYPE_FOURBYTE_L,ptype ,1);//用途
										 SetYDT1363(NULL,TYPE_FOURBYTE_L,poverI ,1);// 过流告警阀值
										 SetYDT1363(NULL,TYPE_FOURBYTE_L,pmod ,1);//下电模式
										 SetYDT1363(NULL   ,TYPE_ONEBYTE,psetid ,1);//序号
										 SetYDT1363(NULL   ,TYPE_ONEBYTE,NULL ,0);//自定义数
										 END_SET_YDT1363//结束标志，用以计算长度
										 
										   pdat=Getdata(cidE2A5,0,NULL,NULL);
								}
									 
			    vPortFree(cidE2A5);
				  vPortFree(ptmpdata);				
         cidE2A5=NULL;
		}
	 




		}
	}
	else if(cmd[1]==0xA6)
	{
		 if(cmd[0]==0xE2)
			{
				u32 cmd_dat;
			  u8 cmd_type;
			  u8 commd_groud;
				u8 setaddr;
			   AsciitoHex(&commd_groud,&predat[0]);
				 AsciitoHex(&cmd_type,&predat[2]);
			   cmd_dat=bswap_32(AsciitoHex32(&predat[4]));
	
           
	     u8 branchCount=0,SearchCount=0;
			 
				 
				 if((commd_groud>0)&&(commd_groud<35))
					 {
						 
						 
						 
						 
									 
									for( branchCount=0;branchCount<34;branchCount++)
								 {
											if(SwitchOnlineCount[branchCount]>1)//查找在线用户分路
											{
												 SearchCount++;
												
												
												if(SearchCount==commd_groud)
													break;
											}
								 }
			

								if((SearchCount==0)||(branchCount==34))
								{
							    	cmd[3]=1;
									 return NULL;
									
								}
	
						    commd_groud=branchCount+1;
						 
						 
						 
						 
						 
						     

							  gSetDownParaGroudType=0x51;//分组设置
								gSetDownParaAddr=commd_groud;//电池分组
						 
							 if(cmd_type==0x80)//下电1使能
									{
										 gSwitchPara[commd_groud-1].st_downMode&=0xF0;
										 gSwitchPara[commd_groud-1].st_downMode|=(u8)(*((float*)&cmd_dat));
											 gSetDownParaCode=12;//下电模式命令码
														
										  if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
										
										    	
									}
									else if(cmd_type==0x81)//下电时间
									{
										  gSwitchPara[commd_groud-1].st_downT=(u16)(*((float*)&cmd_dat));
											 gSetDownParaCode=5;//下电时间命令码
													
										
										if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
										    	
									}
									else if(cmd_type==0x82)//下电电压
									{
										  gSwitchPara[commd_groud-1].st_downV=(*((float*)&cmd_dat))*100;
											 gSetDownParaCode=4;//下电电压命令码
															
										if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
											{
												pdisDisPlayData[1]=35;
											}											
									}
										
									else if(cmd_type==0xA0)//名称
									{
											
											 u8 usegroud[12]={0,1,1,2,2,3,3,4,4,6,5,7};//0无, 1移动 ，2联通，3电信，4广电，6铁塔，5行业外，7电池
											 u8 tmp8=(u8)(*((float*)&cmd_dat));
											 
											 
											 if((gSwitchPara[commd_groud-1].st_userGroud==0x7B)&&(tmp8!=0x0B))//若设置前为电池分路并且将设为非电池分路，则移除分路电池绑定
														 {
															  for(u8 i=0;i<6;i++)
															 {
																  if(gUseToBattBranch[i]==(commd_groud-1))
																	{
																		
																		   pgBattBranch[i]=NULL;//移除电池绑定
																		   gUseToBattBranch[i]=0xff;//设置电池分路设为未定义
																		   gbattGroud--;//电池分路数减一
																		   break;
																	
																	}
															 }
														 }
														 else if((gSwitchPara[commd_groud-1].st_userGroud!=0x7B)&&(tmp8==0x0B))//若设置前为非电池分路并且将设为电池分路，则分路绑定电池
														 {
															 
																 if(gDCdistribution.pst_switchtype[commd_groud-1]!=2)//若当前分路开关不是双向开关
																 {
																		
																		 return NULL;//不能设为电池，返回
																 }
																 u8 i;
															   for(i=0;i<6;i++)
																 {
																		if(gUseToBattBranch[i]==0xff)
																		{
																			
																				 pgBattBranch[i]=&gDCdistribution.pst_I[commd_groud-1];//电池绑定
																				 gUseToBattBranch[i]=commd_groud-1;//设置电池分路
																				 gbattGroud++;//电池分路数加一
																			   break;
																		
																		}
																 }
																  if(i==6)
																 {
																	  return NULL;//绑定电池失败
																 }
														 }
											 
											 
										  gSwitchPara[commd_groud-1].st_userGroud=((usegroud[tmp8]<<4)|tmp8);
											gSetDownParaCode=1;//下电电压命令码
												if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}		 
														 
														    	
									}
									else if(cmd_type==0xA1)
									{
										u32 tmp32=AsciitoHex32(&predat[4]);
										  gSwitchPara[commd_groud-1].st_stopTime.st_year=(u16)(tmp32>>16);
										  gSwitchPara[commd_groud-1].st_stopTime.st_mon=(u8)(tmp32>>8);
										  gSwitchPara[commd_groud-1].st_stopTime.st_day=(u8)tmp32;
										  gSetDownParaCode=2;
										
										if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
									}
									else if(cmd_type==0xA2)
									{
										 u8 tmp8=(u8)(*((float*)&cmd_dat));
										 if(tmp8==1)
										
										{
										  gSwitchPara[commd_groud-1].st_stopTime.st_year=2099;
										  gSwitchPara[commd_groud-1].st_stopTime.st_mon=12;
										  gSwitchPara[commd_groud-1].st_stopTime.st_day=31;
											gSetDownParaCode=2;
										
										}
										else if(tmp8==0)
										{
											 gSwitchPara[commd_groud-1].st_stopTime.st_year=1900;
										   gSwitchPara[commd_groud-1].st_stopTime.st_mon=12;
										   gSwitchPara[commd_groud-1].st_stopTime.st_day=31;
											 gSetDownParaCode=2;
										}
										
										if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
									}
								  else if(cmd_type==0xA3)
									{
										 u8 tmp8=(u8)(*((float*)&cmd_dat));
										 // u8 usegroud[12]={0,1,1,2,2,3,3,4,4,6,5,7};//0无, 1移动 ，2联通，3电信，4广电，6铁塔，5行业外，7电池
										   if((tmp8==2)||(tmp8==3))
											 {
												 
												 if(gSwitchPara[commd_groud-1].st_userGroud!=0x7B)//若设置前为非电池分路并且将设为电池分路
														 {
															 
																 if(gDCdistribution.pst_switchtype[commd_groud-1]!=2)//若当前分路开关不是双向开关
																 {
																		
																		 return NULL;//不能设为电池，返回
																 }
																 u8 i;
															   for(i=0;i<6;i++)
																 {
																		if(gUseToBattBranch[i]==0xff)
																		{
																			
																				 pgBattBranch[i]=&gDCdistribution.pst_I[commd_groud-1];//电池绑定
																				 gUseToBattBranch[i]=commd_groud-1;//设置电池分路
																				 gbattGroud++;//电池分路数加一
																			   break;
																		
																		}
																 }
																 if(i==6)
																 {
																	  return NULL;//绑定电池失败
																 }
																 
																 
																  gSwitchPara[commd_groud-1].st_userGroud=0x7B;
										            	gSetDownParaCode=1;//下电电压命令码
																 
														 }
												 
												 
											 }
										
										   if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
												{
													pdisDisPlayData[1]=35;
												}
										
										
									}
									else if(cmd_type==0xA4)//过载电流
									{
										  gSwitchPara[commd_groud-1].st_maxoverLoadI=(*((float*)&cmd_dat))*10;
											 gSetDownParaCode=11;//过载电流命令码
											
											if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
														    	
									}
									else if(cmd_type==0xA5)//下电模式
									{
										  gSwitchPara[commd_groud-1].st_downMode&=0xF0;
										  gSwitchPara[commd_groud-1].st_downMode|=(u8)(*((float*)&cmd_dat));
											 gSetDownParaCode=12;//下电模式命令码
										
										
											if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
											{
												pdisDisPlayData[1]=35;
											}
														    	
									}	
									
								}
					      else
								{
									 cmd[3]=1;
								}
			
			
			
			
			}
	}		
	else if(cmd[1]==0xA7)//错峰参数1获取
	{  
							
		 if(cmd[0]==0x42)
		 {  
			  u8 tmp1=103;
		    //u8 tmp4=1;

		
		    u8 *tmpdata=(u8 *)pvPortMalloc(4*103);
			     for(u16 i=0;i<4*103;i++)
			        {tmpdata[i]=0;}
							
		    float * tmpfloatdata=(float *)tmpdata;
							
				tmpfloatdata[0]=(float)*(s16 *)&gSetBattLimitPerCSpike/100;
		    tmpfloatdata[1]=(float)*(s16 *)&gSetBattLimitPerCPeak/100;
		    tmpfloatdata[2]=(float)gSetBattLimitPerCNormal/100;
		    tmpfloatdata[3]=(float)gSetBattLimitPerCTrough/100;
				tmpfloatdata[4]=(float)eleRate[0]/10000; //以 元/KWH为单位
				tmpfloatdata[5]=(float)eleRate[1]/10000; //以 元/KWH为单位
				tmpfloatdata[6]=(float)eleRate[2]/10000; //以 元/KWH为单位
				tmpfloatdata[7]=(float)eleRate[3]/10000; //以 元/KWH为单位
		
		    tmpfloatdata[8]=gWorkShiftMode;
			 
			     ytd1363data *cid42A7=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*105);
			   
			            SetYDT1363(cid42A7,TYPE_ONEBYTE,&tmp1 ,1);//      厂家自定义
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[0] ,1);//尖峰时段充电系数   0
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[1] ,1);//高峰时段充电系数   1
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[2] ,1);//平段时段充电系数   2
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[3] ,1);//低谷时段充电系数   3
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[4] ,1);//尖峰时段电价       4
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[5] ,1);//高峰时段电价       5
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[6] ,1);//平段时段电价       6
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[7] ,1);//低谷时段电价       7
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[8] ,1);//电池充电模式   //时段模块
			 
			   
			   
					              u8 *tmp2=&tmpdata[4*9];
			            for(u8 i=0;i<5;i++)//日期
			           {
							
									  YeardayToDate(gsPeakTimeInteval[i].st_yday,&tmp2[4*i+2],0);
									  SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmp2[4*i] ,1);//起始时段       
			           
			           }//9~13
								       tmp2=&tmpdata[4*14];

								for(u8 i=0;i<4;i++)
								 {
									 
									  YeardayToDate((gsPeakTimeInteval[i+1].st_yday-1),&tmp2[4*i+2],0);
									  SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmp2[4*i] ,1);//结束时段 
									    
								 }//14~17
								 
								 {
									   tmp2=&tmpdata[4*18];
										YeardayToDate((gsPeakTimeInteval[0].st_yday-1),&tmp2[2],0);
									  SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmp2[0] ,1);//结束时段 
								
								 }//18
								 
								 
								 
								 
								 
								 
								 
								tmpfloatdata=(float*)&tmpdata[4*19];
								 for(u8 j=0;j<5;j++)//时段1选择  21
								 {
												for(u8 i=0;i<12;i++)//时点
      								{
													u8 tmptype=GetSpeakFlag(j,i);
													tmpfloatdata[12*j+i]=(float)(tmptype);
													SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[12*j+i] ,1);//时段1选择      
																
											 }//32
							  }//80


								
								
								
								
								
								tmp2=&tmpdata[4*(19+60)];
//								
//									
									    
										 for(u8 i=0;i<12;i++)//时段1起点时间 81
										{   
											  GetTimeInMin(gsPeakTimeInteval[0].st_time[i],&tmp2[48*0+4*i+2],0);
												SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmp2[48*0+4*i] ,1);//时段1起点时间     
															
										 }
						
										 tmp2=&tmpdata[4*(19+60+12)];//时段1结束时间 93
										  for(u8 i=0;i<11;i++)
										{   
											  GetTimeInMin(gsPeakTimeInteval[0].st_time[i+1]-1,&tmp2[48*0+4*i+2],0);
												SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmp2[48*0+4*i] ,1);//时段1起点时间     
															
										
										}
										 tmp2=&tmpdata[4*(19+60+12+11)];
										   GetTimeInMin(gsPeakTimeInteval[0].st_time[0]-1,&tmp2[2],0);
												SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmp2[0] ,1);//时段1起点时间  
										

			       END_SET_YDT1363//结束标志，用以计算长度 
			 
			    pdat=Getdata(cid42A7,0,NULL,NULL);
							 vPortFree(cid42A7);				
               vPortFree(tmpdata);									 
            		
				       cid42A7=NULL;
			
			 
			 
			 
			 
			 
			 
			 
			 
	   }
	}
	else if(cmd[1]==0xA8)//错峰参数2获取
	{
		 
	
		 if(cmd[0]==0x42)
		 {    
			 u8 tmp3=98;
		
		      u8 *tmpdata=(u8 *)pvPortMalloc(4*98);
		     for(u16 i=0;i<4*98;i++)
			        {tmpdata[i]=0;}
			 
			 
			     ytd1363data *cid42A8=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*100);
			      SetYDT1363(cid42A8,TYPE_ONEBYTE,&tmp3 ,1);//dataflag      1
			 
			  
			 
			    for(u8 j=0;j<4;j++)//第2，3，4，5大段
			 { 
				  for(u8 i=0;i<12;i++)//时段1起点时间 81
										{   
											  GetTimeInMin(gsPeakTimeInteval[j+1].st_time[i],&tmpdata[96*j+4*i+2],0);
												SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpdata[96*j+4*i] ,1);//时段1起点时间     
											
										 }
						

										  for(u8 i=0;i<11;i++)
										{   
											  GetTimeInMin(gsPeakTimeInteval[j+1].st_time[i+1]-1,&tmpdata[96*j+48+4*i+2],0);
												SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpdata[96*j+48+4*i] ,1);//时段1起点时间     
														
										
										}
									
										   GetTimeInMin(gsPeakTimeInteval[j+1].st_time[0]-1,&tmpdata[96*j+48+4*11+2],0);
												SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpdata[96*j+48+4*11] ,1);//时段1起点时间  
			 }
			 
			 
			            tmpdata[4*96]=(float)gsafedisV/100;
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpdata[4*96] ,1);//峰期错峰终止电压   0
			            SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpdata[4*97] ,1);//平段错峰终止电压  1
			 
			 
			  END_SET_YDT1363//结束标志，用以计算长度 
			 
			    pdat=Getdata(cid42A8,0,NULL,NULL);
							
               vPortFree(cid42A8);			 
               vPortFree(tmpdata);									 
            		
				       cid42A8=NULL;
			 
			 
			 
			 
			 
		 }
	}
	else if(cmd[1]==0xA9)//削峰参数获取
	{
		   if(cmd[0]==0x42)
		 { 
			  u8 tmp8[2]={8,1-gSinglePhase};
			 
			  float *tmpfloatdata=(float *)pvPortMalloc(4*6);
			 if(tmpfloatdata>(float *)20000000)
			 {
					 tmpfloatdata[0]=(float)gSinglePhasePower/10;
					 tmpfloatdata[1]=(float)gAPhasePower/10;
					 tmpfloatdata[2]=(float)gBPhasePower/10;
					 tmpfloatdata[3]=(float)gCPhasePower/10;
					 tmpfloatdata[4]=(float)220;
					 tmpfloatdata[5]=(float)1;
			 }
			 else{
				 return NULL;
			 }
			 
			 
			 
			 
			 
			  ytd1363data *cid42A9=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*11);
			 if(cid42A9!=NULL)
			 {
						 SetYDT1363(cid42A9,TYPE_ONEBYTE,&tmp8[0] ,1);//厂家自定义  
						 SetYDT1363(NULL,TYPE_ONEBYTE,&gPeakShaveOnoff ,1);//削峰开关
			       SetYDT1363(NULL,TYPE_ONEBYTE,&tmp8[1] ,1);//单相，三相
			 
			       SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[0] ,1);//单相模式总功率
			       SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[1] ,1);//L1相功率
			       SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[2] ,1);//L2相功率
			       SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[3] ,1);//L3相功率
			       SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[4] ,1);//交流额定相电压
			       SetYDT1363(NULL,TYPE_FOURBYTE_L,&tmpfloatdata[5] ,1);//交流降额系数
			       SetYDT1363(NULL,TYPE_ONEBYTE,NULL ,0);//厂家自定义 
			       END_SET_YDT1363//结束标志，用以计算长度 
				 
				  
			 
       pdat=Getdata(cid42A9,0,NULL,NULL);			 
			 vPortFree(cid42A9);
			 }
			 vPortFree(tmpfloatdata);
			 
		 }
		
	}
	else if(cmd[1]==0xAA)//错峰参数设置
	{
		 if(cmd[0]==0x42)
		 { 
			   u32 cmd_dat;
			   u8 cmd_type;
			    if(*port==2)
						{//A协议  H52C0F
								 AsciitoHex(&cmd_type,&predat[0]);
								 cmd_dat=bswap_32(AsciitoHex32(&predat[2]));
									float tmpdatfloat= *((float *)&cmd_dat);	
										u8  tmpdatu8= *((u8 *)&cmd_dat);	
										
                     u16 *ptmp16=NULL;
                    if((cmd_type>=0x10)&&(cmd_type<=0x13))//错峰系数 0x10~0x13
										{
											 ptmp16=&gSetBattLimitPerCSpike;
											    ptmp16[cmd_type-0x10]=(u16)tmpdatfloat*100;
											     pgh52c0->savePara(&ptmp16[cmd_type-0x10]);
										}
						        else if((cmd_type>=0x14)&&(cmd_type<=0x17))//错峰电价0x14~0x17
										{
											ptmp16=&eleRate[1];
											     ptmp16[cmd_type-0x14]=(u16)tmpdatfloat*10000;
											       pgh52c0->savePara(&ptmp16[cmd_type-0x14]);
										} 
										else if(cmd_type==0x18)//电池充电模式
										{
											 gWorkShiftMode=tmpdatu8;
											      pgh52c0->savePara(&gWorkShiftMode);
										}
										else if((cmd_type>=0x19)&&(cmd_type<=0x1E))//开始时段设置
										{
											  
											  u8 *ptmp8=(u8 *)&cmd_dat;
											
											 SetSPeakData(cmd_type-0x19,ptmp8[2],ptmp8[3]);	    
											 pgh52c0->savePara(& gsPeakTimeInteval[cmd_type-0x19].st_yday);
											
											
										}
										else if((cmd_type>=0x23)&&(cmd_type<=0x2E))//时段1选择
										{
											  
											     SetSpeakFlag(0,cmd_type-0x23,(SpeakFlag)tmpdatu8);
											  pgh52c0->savePara(& gSpeakFlag[0*6+2*(cmd_type-0x23)]);
										}
										else if((cmd_type>=0x2F)&&(cmd_type<=0x3A))//时段2选择
										{
											  
											     SetSpeakFlag(1,cmd_type-0x2F,(SpeakFlag)tmpdatu8);
											  pgh52c0->savePara(& gSpeakFlag[1*6+2*(cmd_type-0x2F)]);
										}
											else if((cmd_type>=0x3B)&&(cmd_type<=0x46))//时段3选择
										{
											  
											     SetSpeakFlag(2,cmd_type-0x3B,(SpeakFlag)tmpdatu8);
											  pgh52c0->savePara(& gSpeakFlag[2*6+2*(cmd_type-0x3B)]);
										}
										else if((cmd_type>=0x47)&&(cmd_type<=0x52))//时段4选择
										{
											  
											     SetSpeakFlag(3,cmd_type-0x47,(SpeakFlag)tmpdatu8);
											  pgh52c0->savePara(& gSpeakFlag[3*6+2*(cmd_type-0x47)]);
										}	
						        else if((cmd_type>=0x53)&&(cmd_type<=0x5E))//时段5选择
										{
											  
											     SetSpeakFlag(4,cmd_type-0x53,(SpeakFlag)tmpdatu8);
											  pgh52c0->savePara(& gSpeakFlag[4*6+2*(cmd_type-0x53)]);
										}	
										else if((cmd_type>=0x5F)&&(cmd_type<=0x6A))//时段1各小段
										{
											   
											      u8 *ptmp8=(u8 *)&cmd_dat;
											    SetIimeInteval(0,cmd_type-0x5F,ptmp8[2],ptmp8[3]);
											    pgh52c0->savePara((u8*)&gsPeakTimeInteval[0].st_time[cmd_type-0x5F]);
										}
										else if((cmd_type>=0x77)&&(cmd_type<=0x82))//时段2各小段
										{
											   
											      u8 *ptmp8=(u8 *)&cmd_dat;
											    SetIimeInteval(1,cmd_type-0x77,ptmp8[2],ptmp8[3]);
											    pgh52c0->savePara((u8*)&gsPeakTimeInteval[1].st_time[cmd_type-0x77]);
										}
						        else if((cmd_type>=0x8F)&&(cmd_type<=0x9A))//时段3各小段
										{
											   
											      u8 *ptmp8=(u8 *)&cmd_dat;
											    SetIimeInteval(2,cmd_type-0x8F,ptmp8[2],ptmp8[3]);
											    pgh52c0->savePara((u8*)&gsPeakTimeInteval[2].st_time[cmd_type-0x8F]);
										}
										 else if((cmd_type>=0xA7)&&(cmd_type<=0xB2))//时段4各小段
										{
																 
														u8 *ptmp8=(u8 *)&cmd_dat;
													SetIimeInteval(3,cmd_type-0xA7,ptmp8[2],ptmp8[3]);
													pgh52c0->savePara((u8*)&gsPeakTimeInteval[3].st_time[cmd_type-0xA7]);
										}
						         else if((cmd_type>=0xBF)&&(cmd_type<=0xCA))//时段5各小段
										{
																 
														u8 *ptmp8=(u8 *)&cmd_dat;
													SetIimeInteval(4,cmd_type-0xBF,ptmp8[2],ptmp8[3]);
													pgh52c0->savePara((u8*)&gsPeakTimeInteval[4].st_time[cmd_type-0xBF]);
										}
										else if(cmd_type>=0xD7)
										{
											     gsafedisV=tmpdatfloat*100;
											    	pgh52c0->savePara(&gsafedisV);
										}
						
			} 
		 }
	}
	else if(cmd[1]==0xAB)//削峰参数设置
	{
		   if(cmd[0]==0x42)
		 { 
     		u32 cmd_dat;
			   u8 cmd_type;
			    if(*port==2)
						{//A协议  H52C0F
								 AsciitoHex(&cmd_type,&predat[0]);
								 cmd_dat=bswap_32(AsciitoHex32(&predat[2]));
									float tmpdatfloat= *((float *)&cmd_dat);	
										u8  tmpdatu8= *((u8 *)&cmd_dat);	
							
							
							   if(cmd_type==0x10)//削峰开关 
								 {
									     gPeakShaveOnoff=tmpdatu8;
									      pgh52c0->savePara(&gPeakShaveOnoff);
									 
								 }
								 else if(cmd_type==0x11)//削峰配置 
								 {
									   gSinglePhase=(1-tmpdatu8);
									    pgh52c0->savePara(&gSinglePhase);
								 }
								 else if(cmd_type==0x12)//单相模式总功率 
								 {
									    gSinglePhasePower=tmpdatfloat*10;
									     pgh52c0->savePara(&gSinglePhasePower);
								 }
								 else if(cmd_type==0x13)//L1 相功率
								 {
									    gAPhasePower=tmpdatfloat*10;
									     pgh52c0->savePara(&gAPhasePower);
								 }
								 else if(cmd_type==0x14)//L2 相功率
								 {
									    gBPhasePower=tmpdatfloat*10;
									     pgh52c0->savePara(&gBPhasePower);
								 }
								 else if(cmd_type==0x15)//L3 相功率
								 {
									    gCPhasePower=tmpdatfloat*10;
									     pgh52c0->savePara(&gCPhasePower);
								 }
								 else if(cmd_type==0x16)
								 {
								 }
								 else if(cmd_type==0x17)
								 {
								 }
							
							
						}
		 }
	}
	else if(cmd[1]==0xAC)
	{
		 if(cmd[0]==0xE2)
		{
			    u8 command_groud=0;
			    AsciitoHex(&command_groud,&predat[0]);
			
			    if((command_groud<1)||((command_groud>34))) return NULL;
			
			    	u8 *ptmpdata=(u8 *)pvPortMalloc(40);
			      
			      float *pfdat=(float *)ptmpdata;
			
			        pfdat[0]=(float)gSwitchPara[command_groud-1].st_downEnery/100;//电量
			       pfdat[1]=0;
			
						 if((gSwitchPara[command_groud-1].st_downMode&0x10) !=0)//定时下电使能
						 {
								pfdat[1]=1;
						 }
			       
						/// pfdat[2]=gSwitchPara[command_groud-1].startTime0;//起点时间
						 u32 *p=(u32 *)&pfdat[2];
						 *p=0;
						 *p=*p|((gSwitchPara[command_groud-1].startTime0/60)<<8);
						 *p=*p|(gSwitchPara[command_groud-1].startTime0%60);
						 
						 
						 
//			      if(gSwitchPara[command_groud-1].stopTime0>=gSwitchPara[command_groud-1].startTime0)//时长
//						{
//							pfdat[3]=gSwitchPara[command_groud-1].stopTime0-gSwitchPara[command_groud-1].startTime0;
//						}
//						else
//						{
//							pfdat[3]=1440-gSwitchPara[command_groud-1].startTime0+gSwitchPara[command_groud-1].stopTime0;
//						}
			
						pfdat[3]=gSwitchPara[command_groud-1].stopTime0;
			      pfdat[4]=0;//第2~5起点，时长
						
						  pfdat[5]=0;
						if((gSwitchPara[command_groud-1].st_downMode&0x20) !=0)//免责下电使能
						 {
								pfdat[5]=1;
						 }
			       
						// pfdat[6]=gSwitchPara[command_groud-1].startTime1;//免责起点时间
						  p=(u32 *)&pfdat[6];
						 *p=0;
						 *p=*p|((gSwitchPara[command_groud-1].startTime1/60)<<8);
						 *p=*p|(gSwitchPara[command_groud-1].startTime1%60);
						 
//			      if(gSwitchPara[command_groud-1].stopTime1>=gSwitchPara[command_groud-1].startTime1)//免责时长
//						{
//							pfdat[7]=gSwitchPara[command_groud-1].stopTime1-gSwitchPara[command_groud-1].startTime1;
//						}
//						else
//						{
//							pfdat[7]=1440-gSwitchPara[command_groud-1].startTime1+gSwitchPara[command_groud-1].stopTime1;
//						}
			        pfdat[7]=gSwitchPara[command_groud-1].stopTime1;
						
						
					ytd1363data *	 cidE2AC=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*30);
						
						if(cidE2AC!=NULL)
						{
							 SetYDT1363(cidE2AC,TYPE_ONEBYTE ,NULL,24);//一次下电使能
							  SetYDT1363(NULL,TYPE_FOURBYTE_L ,&pfdat[0],1);////电量
							  SetYDT1363(NULL,TYPE_FOURBYTE_L ,&pfdat[1],1);//定时下电使能
							  SetYDT1363(NULL,TYPE_FOURBYTE ,&pfdat[2],1);//定时起点时间
							  SetYDT1363(NULL,TYPE_FOURBYTE_L ,&pfdat[3],1);//定时起点时间
							  for(u8 i=0 ;i<8;i++)
							{
								  SetYDT1363(NULL,TYPE_FOURBYTE_L ,&pfdat[4],1);//其它定时参数，不支持
							}
							
							 SetYDT1363(NULL,TYPE_FOURBYTE_L ,&pfdat[5],1);//免责下电使能
							  SetYDT1363(NULL,TYPE_FOURBYTE ,&pfdat[6],1);//免责起点时间
							  SetYDT1363(NULL,TYPE_FOURBYTE_L ,&pfdat[7],1);//免责起点时间
							  for(u8 i=0 ;i<8;i++)
							{
								  SetYDT1363(NULL,TYPE_FOURBYTE_L ,&pfdat[4],1);//其它定时参数，不支持
							}
							
							 SetYDT1363(NULL,TYPE_ONEBYTE ,&command_groud,1);//单元序号
							 SetYDT1363(NULL,TYPE_ONEBYTE ,NULL          ,0);
							  END_SET_YDT1363//结束标志，用以计算长度
							
							pdat=Getdata(cidE2AC,0,NULL,NULL);
						}
						
			
			
			
			 vPortFree(cidE2AC);
				  vPortFree(ptmpdata);				
         cidE2AC=NULL;
					
		}	
					
					
	}
	else if(cmd[1]==0xAD)
	{
		
		    u32 cmd_dat;
			  u8 cmd_type;
			  u8 commd_groud;
		if(cmd[0]==0xE2)
			{
				   AsciitoHex(&commd_groud,&predat[0]);
				   AsciitoHex(&cmd_type,&predat[2]);
			     cmd_dat=bswap_32(AsciitoHex32(&predat[4]));
				
				  u8 branchCount=0,SearchCount=0;
				
				    if((commd_groud>0)&&(commd_groud<35))
					 {
						 
						 
						 
						 for( branchCount=0;branchCount<34;branchCount++)
								 {
											if(SwitchOnlineCount[branchCount]>1)//查找在线用户分路
											{
												 SearchCount++;
												
												
												if(SearchCount==commd_groud)
													break;
											}
								 }
			

								if((SearchCount==0)||(branchCount==34))
								{
								   cmd[3]=1;
									 return NULL;
								}
	
						    commd_groud=branchCount+1;
						 
						 
						 
						 
						 
						 
						 
						 
						 
						  gSetDownParaGroudType=0x51;//分组设置
							gSetDownParaAddr=commd_groud;//电池分组
						 
						
						 
						  if(cmd_type==0xA0)
							{
								  gSwitchPara[commd_groud-1].st_downEnery=(*((float*)&cmd_dat))*100;
								 gSetDownParaCode=3;//
								
								
									if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
									{
										pdisDisPlayData[1]=35;
									}
								
							}
							else if(cmd_type==0xA1)
							{
								
								if(*((float*)&cmd_dat)==1)
								 gSwitchPara[commd_groud-1].st_downMode|=0x10;
								else
									gSwitchPara[commd_groud-1].st_downMode&=0xEF;
								
								gSetDownParaCode=12;//
								
								  if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
									{
										pdisDisPlayData[1]=35;
									}
							}
							else if(cmd_type==0xA2)
							{
								u8 tmp=cmd_dat>>24;// 分钟
								u8 tmp1=cmd_dat>>16;// 时
								
								
								  gSwitchPara[commd_groud-1].startTime0=tmp1*60+tmp;
								  gSwitchPara[commd_groud-1].st_downMode&=0xEF;
								  gSetDownParaCode=7;//
								
								    if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
							}
							
								else if(cmd_type==0xA3)
							{
								/*
//								u16 tmp;
//								tmp = gSwitchPara[commd_groud-1].startTime0+(u16)(*((float*)&cmd_dat));
//								
//								if(tmp<1440)
//								{
//									gSwitchPara[commd_groud-1].stopTime0=tmp;
//								}
//								else if(tmp==1440)
//								{
//									gSwitchPara[commd_groud-1].stopTime0=0;
//								}
//								else if(tmp>1440)
//								{
//									 gSwitchPara[commd_groud-1].stopTime0=tmp-1440;
//								}
								*/
								 gSwitchPara[commd_groud-1].stopTime0=(u16)(*((float*)&cmd_dat));
								  gSwitchPara[commd_groud-1].st_downMode&=0xEF;
								  gSetDownParaCode=8;//
								
								    if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
							}
							else if(cmd_type==0xAC)
							{
								
								if(*((float*)&cmd_dat)==1)
								 gSwitchPara[commd_groud-1].st_downMode|=0x20;
								else
									gSwitchPara[commd_groud-1].st_downMode&=0xDF;
								
								gSetDownParaCode=12;//
								     if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
											{
												pdisDisPlayData[1]=35;
											}
							}
							else if(cmd_type==0xAD)
							{
								
								u8 tmp=cmd_dat>>24;// 分钟
								u8 tmp1=cmd_dat>>16;// 时
								
								  gSwitchPara[commd_groud-1].startTime1=tmp1*60+tmp;
								  	gSwitchPara[commd_groud-1].st_downMode&=0xDF;
								  gSetDownParaCode=9;//
								    if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
							}
							
							else if(cmd_type==0xAE)
							{
								/*
								//u16 tmp;
//								tmp = gSwitchPara[commd_groud-1].startTime1+(u16)(*((float*)&cmd_dat));
//								
//								if(tmp<1440)
//								{
//									gSwitchPara[commd_groud-1].stopTime1=tmp;
//								}
//								else if(tmp==1440)
//								{
//									gSwitchPara[commd_groud-1].stopTime1=0;
//								}
//								else if(tmp>1440)
//								{
//									 gSwitchPara[commd_groud-1].stopTime1=tmp-1440;
//								}
								*/
								   gSwitchPara[commd_groud-1].stopTime1=(u16)(*((float*)&cmd_dat));
								   gSwitchPara[commd_groud-1].st_downMode&=0xDF;
								   gSetDownParaCode=10;//
								
								
								   if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[1]=35;
										}
							}
							
							
							
							
							
					 }
					 else
					 {
						  cmd[3]=1;
					 }
				
			
			
			
			}
	}
	else if(cmd[1]==0xD0) // 7.11	获取抄表记录
	{
		   u8 cmd_groud,endflag=0;
		   u16 getaddr;
		   u16 sourceAdd;
		 static u16 couter[8]={0,0,0,0,0,0,0,0};//偶数项存访问记录指针，奇数项存最大记录数 
		    if(*port==3){//在网页请求
					    cmd_groud=cmd[2];
					  if(cmd_groud==0x00)//请求首条记录 天
						{
							 u16 tmp=0;
							couter[0]=0;
					     RecordInfo Record={ &EnergyDataWriteConunt90,&genergyDateRecord90Max,&tmp};	
					     getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_90_RECORD_LEN);
				     		sourceAdd=ENERGY_90DAY_DATA_START;
							  couter[0]++;
						}
						else if(cmd_groud==0x01)//请求下一条记录 天
						{
							
							
							 RecordInfo Record={ &EnergyDataWriteConunt90,&genergyDateRecord90Max,&couter[0]};	
					     getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_90_RECORD_LEN);
							 if(couter[0]>=couter[1]) return NULL;//已超访问最大数，不回复
							    couter[0]++;
							 if(couter[0]+1==couter[1])
								 endflag=1;
						
							   
							    sourceAdd=ENERGY_90DAY_DATA_START;
						}
            else if(cmd_groud==0x10)//请求首条记录 月
						{
							 u16 tmp=0;
							couter[2]=0;
					     RecordInfo Record={ &EnergyDataWriteConunt,&genergyDateRecordMax,&tmp};	
					     getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_RECORD_LEN);
				     		sourceAdd=ENERGY_DATA_STAR;
							  couter[2]++;
						}
						else if(cmd_groud==0x11)//请求下一条记录 月
						{
							
							
							 RecordInfo Record={ &EnergyDataWriteConunt,&genergyDateRecordMax,&couter[2]};	
					     getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_RECORD_LEN);
							   if(couter[2]>=couter[3]) return NULL;//已超访问最大数，不回复
							    couter[2]++;
							 if(couter[2]+1==couter[3])
								 endflag=1;
							
							   
							    sourceAdd=ENERGY_DATA_STAR;
						}
						
						
				}
				else if(*port==2)//485请求
				{
					
					   AsciitoHex(&cmd_groud,&predat[0]);
					
								 if(cmd_groud==0x00)//请求首条记录 天
									{
										 u16 tmp=0;
										couter[4]=0;
										 RecordInfo Record={ &EnergyDataWriteConunt90,&genergyDateRecord90Max,&tmp};	
										 getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_90_RECORD_LEN);
											sourceAdd=ENERGY_90DAY_DATA_START;
											couter[4]++;
									}
									else if(cmd_groud==0x01)//请求下一条记录 天
									{
										
										
										 RecordInfo Record={ &EnergyDataWriteConunt90,&genergyDateRecord90Max,&couter[4]};	
										 getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_90_RECORD_LEN);
										if(couter[4]>=couter[5]) return NULL;	//已超访问最大数，不回复
										 couter[4]++;
										 if(couter[4]+1==couter[5])
											 endflag=1;
									
											 
												sourceAdd=ENERGY_90DAY_DATA_START;
									}
									else if(cmd_groud==10)//请求首条记录 月
									{
										 u16 tmp=0;
										couter[6]=0;
										 RecordInfo Record={ &EnergyDataWriteConunt,&genergyDateRecordMax,&tmp};	
										 getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_RECORD_LEN);
											sourceAdd=ENERGY_DATA_STAR;
											couter[6]++;
									}
									else if(cmd_groud==11)//请求下一条记录 月
									{
										
										
										 RecordInfo Record={ &EnergyDataWriteConunt,&genergyDateRecordMax,&couter[6]};	
										 getaddr=RecordOrderNumToGetAddr(&Record,ENERGY_DATE_RECORD_LEN);
										 if(couter[6]>=couter[7]) return NULL;//已超访问最大数，不回复		
										 couter[6]++;
										 if(couter[6]+1==couter[7])
											 endflag=1;
										
											 
												sourceAdd=ENERGY_DATA_STAR;
									}
					
				}
				
				
				
				
		
		  if(cmd[0]==0x40)
			{
				 uTIME tTimer;
				u32 tmp32[12];
				
				if((cmd_groud==0x00)||(cmd_groud==0x01))//取得日记录
				{
				   pgh52c0->readI2C2(sourceAdd+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);//获取日期
				   pgh52c0->readI2C2(sourceAdd+getaddr*TOTAL_DATA_LEN+TIME_LEN+USER_ENERGY_DATA_LEN,(u8*)&tmp32[0],48);//读取交流电能记录
				}
				else//取得月记录
				{
					 pgh52c0->readI2C(sourceAdd+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);//获取日期
				   pgh52c0->readI2C(sourceAdd+getaddr*TOTAL_DATA_LEN+TIME_LEN+USER_ENERGY_DATA_LEN,(u8*)&tmp32[0],48);//读取交流电能记录
				}
				
				
				  *(u8 *)&couter[1]=tTimer.mon;//couter[1]每次都能再新生成最大条数，所以可临时借用couter[1] 暂存月日
				
				if((cmd_groud==0x00)||(cmd_groud==0x01))//取得日记录
				{
					   *((u8 *)&couter[1]+1)=tTimer.day;
				}
				else
				{
					  *((u8 *)&couter[1]+1)=0;
				}
				
				
		     
				for(u8 i=0;i<6;i++)
				{
					    (*(float *)&tmp32[i])=(float)tmp32[i]/10;
					    (*(float *)&tmp32[i+6])=(float)tmp32[i+6]/100000;
					
				}
				    tmp32[5]=0;
				
				 ytd1363data *cid40D0=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*15);
			  if(cid40D0!=NULL){
				       SetYDT1363(cid40D0,TYPE_ONEBYTE,&endflag,1);
					     SetYDT1363(NULL,   TYPE_TWOBYTE,&tTimer.year,1);
					     SetYDT1363(NULL,   TYPE_ONEBYTE,(u8 *)&couter[1],1);
					     SetYDT1363(NULL,   TYPE_ONEBYTE,(u8 *)&couter[1]+1,1);
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[0],1);//总电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[5],1);//油机电能
				       SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[SpikeMode],1);//尖峰电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[PeakMode],1);//高峰电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[NormalMode],1);//段电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[TroughMode],1);//高峰电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[SpikeMode+6],1);//尖峰电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[PeakMode+6],1);//高峰电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[NormalMode+6],1);//段电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[TroughMode+6],1);//高峰电能
					      END_SET_YDT1363//结束标志，用以计算长度
				}
				        pdat=Getdata(cid40D0,0,NULL,NULL);
			 
    		 
			 vPortFree(cid40D0);
			 cid40D0=NULL;
				
				
			}
			else if(cmd[0]==0x42)
			{
					 uTIME tTimer;
				u32 tmp32[9];
				
				if((cmd_groud==0x00)||(cmd_groud==0x01)) //取得日记录
				{
				   pgh52c0->readI2C2(sourceAdd+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);//获取日期
				   pgh52c0->readI2C2(sourceAdd+getaddr*TOTAL_DATA_LEN+TIME_LEN,(u8*)&tmp32[0],32);//读取交流电能记录
				}
				else //取得月记录
				{
					 pgh52c0->readI2C(sourceAdd+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);//获取日期
				   pgh52c0->readI2C(sourceAdd+getaddr*TOTAL_DATA_LEN+TIME_LEN,(u8*)&tmp32[0],32);//读取交流电能记录
				}
				
				
				  *(u8 *)&couter[1]=tTimer.mon;//couter[1]每次都能再新生成最大条数，所以可临时借用couter[1] 暂存月日
				  if((cmd_groud==0x00)||(cmd_groud==0x01))//取得日记录
					{
							 *((u8 *)&couter[1]+1)=tTimer.day;
					}
					else
					{
							*((u8 *)&couter[1]+1)=0;
				   }
				
				
				
		     
				for(u8 i=0;i<8;i++)
				{
					    (*(float *)&tmp32[i])=(float)tmp32[i]/10;
					
					
				}
				   
				  tmp32[8]=0;
				 ytd1363data *cid42D0=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*20);
			  if(cid42D0!=NULL){
				       SetYDT1363(cid42D0,TYPE_ONEBYTE,&endflag,1);
					     SetYDT1363(NULL,   TYPE_TWOBYTE,&tTimer.year,1);
					     SetYDT1363(NULL,   TYPE_ONEBYTE,(u8 *)&couter[1],1);
					     SetYDT1363(NULL,   TYPE_ONEBYTE,(u8 *)&couter[1]+1,1);
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//光伏电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//风机电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//燃料电池电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[0],1);//负载分路1电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[1],1);//负载分路2电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[2],1);//负载分路3电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[3],1);//负载分路4电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[4],1);//负载分路5电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[5],1);//负载分路6电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[6],1);//负载分路7电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[7],1);//负载分路8电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//负载分路9电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//负载分路10电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//负载分路11电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//负载分路12电能
					      END_SET_YDT1363//结束标志，用以计算长度
				}
				        pdat=Getdata(cid42D0,0,NULL,NULL);
			 
    		 
			 vPortFree(cid42D0);
			 cid42D0=NULL;
				
			}
			else if(cmd[0]==0xE2)
			{
				uTIME tTimer;
				u32 tmp32[9];
				
				if((cmd_groud==0x00)||(cmd_groud==0x01))//取得日记录
				{
				   pgh52c0->readI2C2(sourceAdd+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);//获取日期
				   pgh52c0->readI2C2(sourceAdd+getaddr*TOTAL_DATA_LEN+TIME_LEN,(u8*)&tmp32[0],32);//读取交流电能记录
				}
				else//取得月记录
				{
					 pgh52c0->readI2C(sourceAdd+TOTAL_DATA_LEN*getaddr,(u8 *)&tTimer,6);//获取日期
				   pgh52c0->readI2C(sourceAdd+getaddr*TOTAL_DATA_LEN+TIME_LEN,(u8*)&tmp32[0],32);//读取交流电能记录
				}
				
				
				  *(u8 *)&couter[1]=tTimer.mon;//couter[1]每次都能再新生成最大条数，所以可临时借用couter[1] 暂存月日
							 if((cmd_groud==0x00)||(cmd_groud==0x01))//取得日记录
							{
									 *((u8 *)&couter[1]+1)=tTimer.day;
							}
							else
							{
									*((u8 *)&couter[1]+1)=0;
							}
				
				
				
		     
				for(u8 i=0;i<8;i++)
				{
					    (*(float *)&tmp32[i])=(float)tmp32[i]/10;
					
					
				}
				   
				  tmp32[8]=0;
				 ytd1363data *cidE2D0=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*19);
			  if(cidE2D0!=NULL){
				       SetYDT1363(cidE2D0,TYPE_ONEBYTE,&endflag,1);
					     SetYDT1363(NULL,   TYPE_TWOBYTE,&tTimer.year,1);
					     SetYDT1363(NULL,   TYPE_ONEBYTE,(u8 *)&couter[1],1);
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[0],1);//用户分路1电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[1],1);//用户分路2电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[2],1);//用户分路3电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[3],1);//用户分路4电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[4],1);//用户分路5电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[5],1);//用户分路6电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[6],1);//用户分路7电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[7],1);//用户分路8电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//用户分路9电能
					     SetYDT1363(NULL,   TYPE_FOURBYTE_L,&tmp32[8],1);//用户分路10电能

					      END_SET_YDT1363//结束标志，用以计算长度
				}
				        pdat=Getdata(cidE2D0,0,NULL,NULL);
			 
    		 
			 vPortFree(cidE2D0);
			 cidE2D0=NULL;
				
			}
			
	}
	else if(cmd[1]==0xD1) // 7.12	获取扩展参数
	{
		
	}
	else if(cmd[1]==0xD2) //  7.13	获取高级参数
	{
		    
		
		if(cmd[0]==0x40)
		{
			  u16 tmp16data[21];
			  tmp16data[0]=gSetBattLimitPerCSpike;//尖峰充电系数
				tmp16data[1]=gSetBattLimitPerCPeak;//高峰充电系数
				tmp16data[2]=gSetBattLimitPerCNormal;//平段充电系数
				tmp16data[3]=gSetBattLimitPerCTrough;//低谷充电系数
			
				tmp16data[4]=eleRate[SpikeMode];//60;	//尖峰电价
				tmp16data[5]=eleRate[PeakMode];//60;  //高峰电价
				tmp16data[6]=eleRate[NormalMode];//60;  //平段电价
				tmp16data[7]=eleRate[TroughMode];//60;  //低谷电价
				
				tmp16data[8]=gSinglePhasePower; //市电总限功率
				tmp16data[9]=gAPhasePower; //A相限功率
				tmp16data[10]=gBPhasePower; //B相限功率
				tmp16data[11]=gCPhasePower; //C相限功率
				
				tmp16data[12]=60; //A相限电流
				tmp16data[13]=60; //B相限电流
				tmp16data[14]=60; //C相限电流
				tmp16data[15]=60; //油机A相限电流
				
				tmp16data[16]=60;// gAcRateP/10; //油机B相限电流
				tmp16data[17]=60;//gAcRateI; //油机C相限电流
				tmp16data[18]=60;//gAcPhase; //额定相电压 
				tmp16data[19]=gsafedisV; //安全放电电压
				tmp16data[20]=gdisDeep; //安全放电电量 
			
			
			
			
			
			
			
			  ytd1363data *cid40D2=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*22);
			  if(cid40D2!=NULL){
			   SetYDT1363(cid40D2,TYPE_TWOBYTE,&tmp16data[0] ,1);//尖峰充电系数
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[1] ,1);//高峰充电系数
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[2] ,1);//平段充电系数
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[3] ,1);//低谷充电系数
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[4] ,1);//尖峰电价
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[5] ,1);//高峰电价
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[6] ,1);//平段电价
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[7] ,1);//低谷电价
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[8] ,1);//市电总限功率
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[9] ,1);//A相限功率
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[10] ,1);//B相限功率
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[11] ,1);//C相限功率
			   SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//A相限电流
			   SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//B相限电流
			   SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//C相限电流
			   SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//油机A相限电流
			   SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//油机B相限电流
			   SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//油机C相限电流
			   SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81); //额定相电压 
				 SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[19] ,1);//安全放电电压
			   SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[20] ,1);//安全放电电量 
				 END_SET_YDT1363//结束标志，用以计算长度 
			 }
				   pdat=Getdata(cid40D2,0,NULL,NULL);
			 
    		 
			 vPortFree(cid40D2);
			 cid40D2=NULL;
			 
		}
		
		if(cmd[0]==0x41)
		{
			  u8 tmp8data=0;
			   if(gSinglePhase==0)//三相
				 {
					 tmp8data=2;
				 }
				 else if(gSinglePhase==1)//单相
				 {
					  tmp8data=1;
				 }
			
			   ytd1363data *cid41D2=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*5);
			    if(cid41D2!=NULL){
						   SetYDT1363(cid41D2,TYPE_ONEBYTE,&tmp8data ,1);//相位模式
						   SetYDT1363(cid41D2,TYPE_ONEBYTE,&genableTP ,1);//模块防盗
						   SetYDT1363(cid41D2,TYPE_ONEBYTE,&gWorkShiftMode ,1);//错峰使能
						   SetYDT1363(cid41D2,TYPE_ONEBYTE,&gPeakShaveOnoff ,1);//削峰使能
						   END_SET_YDT1363//结束标志，用以计算长度
					}
		 
       pdat=Getdata(cid41D2,0,NULL,NULL);			 
			 vPortFree(cid41D2);
					cid41D2=NULL;
		}
		
		
		
		
		
	  if(cmd[0]==0x42)
		{  u16 tmp16data[11];
			     tmp16data[0]=gbootBattCap*100;  //0,1,2,3
			     tmp16data[1]=gSetStartBRI;//4,5,6,7
			     tmp16data[2]=gstartdelay*60;//8,9,10,11
			     tmp16data[3]=5600;//12,13,14,15
			     tmp16data[4]=gSetEndBRI;//16,17,18,19
			     tmp16data[5]=genddelay*60;//20,21,22,23
			     tmp16data[6]=10;//24,25,26,27
			
			     tmp16data[7]=0;//28,29,30,31
			     tmp16data[8]=gequalTimeMax*60;//32,33,34,35
			     tmp16data[9]=5350;//36;37,38,39
			     tmp16data[10]=gMaxModuleCurr;//40,41,42,43
			
			
			
			
			  ytd1363data *cid42D2=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*15);
			   if(cid42D2!=NULL){
						SetYDT1363(cid42D2,TYPE_TWOBYTE,NULL ,0x81);//均充开始电压
					     SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[1] ,1);//均充开始电流
					     SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//均充开始SOC
					     SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[2] ,1);//均充开始延迟
					    
					     SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//均充结束电压
					     SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[4] ,1);//均充结束电流
					     SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//均充结束SOC
					     SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[5] ,1);//均充结束延迟
					 
					     SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[6] ,1);//放电测试电流
					 
					     SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//周期均充时刻
					     SetYDT1363(NULL,TYPE_TWOBYTE,NULL ,0x81);//周期测试时刻
					     SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[8] ,1);//最长均充时长
					     SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[9] ,1);//系统额定电压
					     SetYDT1363(NULL,TYPE_TWOBYTE,&tmp16data[10] ,1);//系统额定电流
						   END_SET_YDT1363//结束标志，用以计算长度
					}
				  pdat=Getdata(cid42D2,0,NULL,NULL);
			 
      	 
			 vPortFree(cid42D2);
				cid42D2=NULL;	
		}
	
	   if(cmd[0]==0x43)
	  {
		   ytd1363data *cid43D2=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*2);
		   if(cid43D2!=NULL)
			 {
				    SetYDT1363(cid43D2,TYPE_TWOBYTE,NULL ,0x81);//最大功率点
				     END_SET_YDT1363//结束标志，用以计算长度
			 }
			  pdat=Getdata(cid43D2,0,NULL,NULL);
			 
      	 
			 vPortFree(cid43D2);
				cid43D2=NULL;	
	  }
	
	
	   if(cmd[0]==0xE1)
		{  u8 tmp8data[37];
			   tmp8data[0]=1;//直流电压通道数 M
			   tmp8data[1]=1;//系统母线电压
			   tmp8data[2]=3;//直流电流通道数 N
			   tmp8data[3]=1;//电池电流
			   tmp8data[4]=2;//负载分路电流
			   tmp8data[5]=2;//负载分路电流
			   tmp8data[6]=12;//数字输入通道数 P
			   tmp8data[7]=InputDItoWeb[gInPutAlarm[0].st_AlarmType]|(gInPutAlarm[0].st_AlarmTrigger<<7);//数字输入通道数 1
			   tmp8data[8]=InputDItoWeb[gInPutAlarm[1].st_AlarmType]|(gInPutAlarm[1].st_AlarmTrigger<<7);//数字输入通道数 2                  
			   tmp8data[9]=InputDItoWeb[gInPutAlarm[2].st_AlarmType]|(gInPutAlarm[2].st_AlarmTrigger<<7);//数字输入通道数 3    
			   tmp8data[10]=InputDItoWeb[gInPutAlarm[3].st_AlarmType]|(gInPutAlarm[3].st_AlarmTrigger<<7);//数字输入通道数 4    
			   tmp8data[11]=InputDItoWeb[gInPutAlarm[4].st_AlarmType]|(gInPutAlarm[4].st_AlarmTrigger<<7);//数字输入通道数 5    
			   tmp8data[12]=InputDItoWeb[gInPutAlarm[5].st_AlarmType]|(gInPutAlarm[5].st_AlarmTrigger<<7);//数字输入通道数 6    
			   tmp8data[13]=InputDItoWeb[gInPutAlarm[6].st_AlarmType]|(gInPutAlarm[6].st_AlarmTrigger<<7);//数字输入通道数 7 
			   tmp8data[14]=InputDItoWeb[gInPutAlarm[7].st_AlarmType]|(gInPutAlarm[7].st_AlarmTrigger<<7);//数字输入通道数 8 
			   tmp8data[15]=InputDItoWeb[gInPutAlarm[8].st_AlarmType]|(gInPutAlarm[8].st_AlarmTrigger<<7);//数字输入通道数 9 
			   tmp8data[16]=InputDItoWeb[gInPutAlarm[9].st_AlarmType]|(gInPutAlarm[9].st_AlarmTrigger<<7);//数字输入通道数 10 
			   tmp8data[17]=InputDItoWeb[gInPutAlarm[10].st_AlarmType]|(gInPutAlarm[10].st_AlarmTrigger<<7);//数字输入通道数 11 
			   tmp8data[18]=InputDItoWeb[gInPutAlarm[11].st_AlarmType]|(gInPutAlarm[11].st_AlarmTrigger<<7);//数字输入通道数 12 
	       tmp8data[19]=9;// 数字输出通道数 Q 
			
			    tmp8data[20]=gInPutAlarm[0].st_behavior;
			    tmp8data[21]=gInPutAlarm[1].st_behavior;
			    tmp8data[22]=gInPutAlarm[2].st_behavior;
			    tmp8data[23]=gInPutAlarm[3].st_behavior;
					tmp8data[24]=gInPutAlarm[4].st_behavior;
					tmp8data[25]=gInPutAlarm[5].st_behavior;
					
					tmp8data[26]=gInPutAlarm[6].st_behavior;
					tmp8data[27]=gInPutAlarm[7].st_behavior;
					tmp8data[28]=gInPutAlarm[8].st_behavior;
			
			   tmp8data[29]=1;//温度传感器数 R
			   tmp8data[30]=1;//电池温度
				 tmp8data[31]=10;//十板扩展板


					 
					 tmp8data[32]=3;//整流模块类型
				   tmp8data[33]=3;//光伏模块类型
				   tmp8data[34]=0;//风机管理类型
				   tmp8data[35]=0;//燃料电池类型
					 tmp8data[36]=0;//管理平台
					 
			
			 ytd1363data *cidE1D2=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*50);
			   if(cidE1D2!=NULL){
					 SetYDT1363(cidE1D2,TYPE_ONEBYTE,&tmp8data[0] ,1);//直流电压通道数 M

        for(u8 i=1;i<32;i++)
           SetYDT1363(NULL,TYPE_ONEBYTE,&tmp8data[i] ,1);//燃料电池类型
				for(u8 i=0;i<10;i++)
				{
					 SetYDT1363(NULL,TYPE_ONEBYTE,&branch[i] ,1);
				}
				       SetYDT1363(NULL,TYPE_ONEBYTE,&tmp8data[32] ,1);//整流模块类型
						   SetYDT1363(NULL,TYPE_ONEBYTE,&tmp8data[33] ,1);//光伏模块类型
							 SetYDT1363(NULL,TYPE_ONEBYTE,&tmp8data[34] ,1);//风机管理类型
							 SetYDT1363(NULL,TYPE_ONEBYTE,&tmp8data[35] ,1);//燃料电池类型
				       SetYDT1363(NULL,TYPE_ONEBYTE,&tmp8data[36] ,1);//燃料电池类型
								
					   END_SET_YDT1363//结束标志，用以计算长度
				 }
				 pdat=Getdata(cidE1D2,0,NULL,NULL);
			 
      	 
			 vPortFree(cidE1D2);
					cidE1D2=NULL;	 
		}
	   
	
	
	  if(cmd[0]==0xE2)
		{ //u8 *tmpv8data=NULL;
		  u8 datdelay=0;
      
			
			u8 broadNum;//板号
			u8 branchNum;//分路号
			u8 accCount=branch[0];//累计分路数
			
	     //设算板号及分路数		
				   for(u8 i=0;i<10;i++)
				  {
						
						if(cmd[2]>accCount)//cmd[2]表示请求第几分路数据
						{
							
							   accCount+=branch[i+1];//累计下一分路的分路数
						}
						else
						{
							
							  broadNum=i+1;////当查找分路数小于等于当前累计分路数时,记录当前分路板号
							branchNum=branch[i]+cmd[2]-accCount;
							
							break;
							
						}
				  }
			
					
					
					
			//验证所请分路所在的板是否在线
					if(gDcmStatus[broadNum-1]==1)//若相关板不存在线 
					{
						cmd[2]=6;//以cmd[2]标识为无效数据并返回空

						return NULL;
					}
			
			//伸请数据空间
			    u8 *ptmv8=(u8 *)pvPortMalloc(sizeof(u8)*33);
			    if(ptmv8==NULL) return NULL;
			
			
			 getE2D2=(((broadNum<<4)&0xF0)|0x01);//高4位为请求板号，低4位为获取标志
			ptmv8[2]=branchNum;//通道号
			pgetE2Databuff=ptmv8;
			
			
			//挂起任务并设超时，等待CAN返回数据
			while(1){
				datdelay++;
			  vTaskDelay(20 / portTICK_RATE_MS);	
			  if(getE2D2==0xf2)
				{
					break;//成功收到CAN数据，结束等待循环
				}
				
				if(datdelay>0x2f) //超时返回无效
				{
					vPortFree(ptmv8);//释放所伸请内存
					getE2D2=0;//清空标志
					cmd[2]=0xFF;//以cmd[2]标识为空据并返回空
					return NULL;
					
				}
			}

			   *(u16 *)&ptmv8[17]/=10;
		
		   u8 tmpT[3]={23,59,59};
			 u8 tmpcount=0;
			 for(u8 i=0;i<10;i++)
			 {
				  tmpcount+=branch[i];
			 }
			 
			 ytd1363data *cidE2D2=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*42);
			   if(cidE2D2!=NULL){
					
					  
					 	  SetYDT1363(cidE2D2,TYPE_ONEBYTE,&tmpcount ,1);//负载分路数
					  	SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[0] ,1);//下电模式
					    SetYDT1363(NULL,TYPE_TWOBYTE,&ptmv8[9] ,1);//下电电压
					    SetYDT1363(NULL,TYPE_TWOBYTE,&ptmv8[17] ,1);//下电时长
					    SetYDT1363(NULL,TYPE_TWOBYTE,&ptmv8[19] ,1);//下电电量
					   for(u8 i=0;i<20;i++)
					   SetYDT1363(NULL   ,TYPE_ONEBYTE, (u8 *)&userName[4*i+0] ,1);
					     SetYDT1363(NULL,TYPE_TWOBYTE,&ptmv8[11] ,1);//截止年
					     SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[13] ,1);//截止月
					     SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[15] ,1);//截止日
					     SetYDT1363(NULL,TYPE_ONEBYTE,&tmpT[0] ,1);//截止时
					     SetYDT1363(NULL,TYPE_ONEBYTE,&tmpT[1] ,1);//截止分
					     SetYDT1363(NULL,TYPE_ONEBYTE,&tmpT[2] ,1);//截止秒
					     SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[32] ,1);//所属用户
					      SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[22] ,1);//下电开始时
					      SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[23] ,1);//下电开始分
					      SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[24] ,1);//下电结束时
					      SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[25] ,1);//下电结束分
					      SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[27] ,1);//免责开始时
					      SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[28] ,1);//免责开始分
					      SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[29] ,1);//免责结束时
					      SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[30] ,1);//免责结束分
								SetYDT1363(NULL,TYPE_ONEBYTE,&ptmv8[26] ,1);//油机供电
								
					 END_SET_YDT1363//结束标志，用以计算长度
						 
					 pdat=Getdata(cidE2D2,0,NULL,NULL); 
					 vPortFree(cidE2D2);
					 cidE2D2=NULL; 
				 }
				 
			 
      	 
			 
				 vPortFree(ptmv8);
				 getE2D2=0;
					
		}
	}
	

  else if(cmd[1]==0xD3) //  7.13	设置高级参数
	{
		
		if(cmd[0]==0x40)
		{
				
			 if(*port==3){//在网页请求
						  //尖峰充电系数
							 gSetBattLimitPerCSpike=(predat[0]<<8)+predat[1];
						  	pgh52c0->savePara(&gSetBattLimitPerCSpike);
				  		//高峰充电系数
							gSetBattLimitPerCPeak=(predat[2]<<8)+predat[3];
							pgh52c0->savePara(&gSetBattLimitPerCPeak);
						  //平段充电系数
							 gSetBattLimitPerCNormal=(predat[4]<<8)+predat[5];
							pgh52c0->savePara(&gSetBattLimitPerCNormal);
							 //低谷充电系数
							gSetBattLimitPerCTrough=(predat[6]<<8)+predat[7];
						 pgh52c0->savePara(&gSetBattLimitPerCTrough);
		
				 
				 			eleRate[SpikeMode]=(predat[8]<<8)+predat[9];//尖峰电价
				        pgh52c0->savePara(&eleRate[SpikeMode]);
				 
				      eleRate[PeakMode]=(predat[10]<<8)+predat[11];//高峰电价
							  pgh52c0->savePara(&eleRate[PeakMode]);
								
				      eleRate[NormalMode]=(predat[12]<<8)+predat[13];//平段电价
							   pgh52c0->savePara(&eleRate[NormalMode]);
							
				      eleRate[TroughMode]=(predat[14]<<8)+predat[15];//低谷电价
							   pgh52c0->savePara(&eleRate[TroughMode]);
				 

				
				 
				//			
				//			//尖峰时段交流限功率
								gSinglePhasePower=(predat[16]<<8)+predat[17];
								pgh52c0->savePara(&gSinglePhasePower);
				//			 //高峰时段交流限功率
								gAPhasePower=(predat[18]<<8)+predat[19];
								 pgh52c0->savePara(&gAPhasePower);
				//			//平段时段交流限功率
								gBPhasePower=(predat[20]<<8)+predat[21];
							  pgh52c0->savePara(&gBPhasePower);
				//			 //低谷时段交流限功率
								gCPhasePower=(predat[22]<<8)+predat[23];
							  pgh52c0->savePara(&gCPhasePower);
				//			tmp16data[12]=60; //尖峰时段交流限电流
				//			tmp16data[13]=60; //高峰时段交流限电流
				//			tmp16data[14]=60; //平段时段交流限电流
				//			tmp16data[15]=60; //低谷时段交流限电流
				//			
				//		//空调额定功率
								gAcRateP=(predat[32]<<8)+predat[33];
						   pgh52c0->savePara(&gAcRateP);
							 //空调额定电流
								//gAcRateI=(predat[34]<<8)+predat[35];
						
				//		 //空调交流相位 

								gAcPhase=predat[37];
								pgh52c0->savePara(&gAcPhase);
					
				//	    //安全放电电压
								gsafedisV=(predat[38]<<8)+predat[39];
								pgh52c0->savePara(&gsafedisV);
						
				//			//安全放电电量 
								gdisDeep=(predat[40]<<8)+predat[41];
								 pgh52c0->savePara(&gdisDeep);
						
	
		}
	     else if(*port==2)
		   {
			u8 tmp1,tmp2;
			
			      AsciitoHex(&tmp1,&predat[0]);
			      AsciitoHex(&tmp2,&predat[2]);
			      gSetBattLimitPerCSpike=(tmp1<<8)+tmp2;
			      	
			
			      
			        AsciitoHex(&tmp1,&predat[4]);
			        AsciitoHex(&tmp2,&predat[6]);
			      gSetBattLimitPerCPeak=(tmp1<<8)+tmp2;
			     
			
			      AsciitoHex(&tmp1,&predat[8]);
			      AsciitoHex(&tmp2,&predat[10]);
			      gSetBattLimitPerCNormal=(tmp1<<8)+tmp2;
			
			
			
			      AsciitoHex(&tmp1,&predat[12]);
			      AsciitoHex(&tmp2,&predat[14]);
			      gSetBattLimitPerCTrough=(tmp1<<8)+tmp2;
						
						
						
						 AsciitoHex(&tmp1,&predat[16]);
			      AsciitoHex(&tmp2,&predat[18]);
			      eleRate[SpikeMode]=(tmp1<<8)+tmp2;//尖峰电价
						
						
						 AsciitoHex(&tmp1,&predat[20]);
			      AsciitoHex(&tmp2,&predat[22]);
			      eleRate[PeakMode]=(tmp1<<8)+tmp2; //高峰电价
						
						
						 AsciitoHex(&tmp1,&predat[24]);
			      AsciitoHex(&tmp2,&predat[26]);
			      eleRate[NormalMode]=(tmp1<<8)+tmp2; //高峰电价
						
						
						AsciitoHex(&tmp1,&predat[28]);
			      AsciitoHex(&tmp2,&predat[30]);
			      eleRate[TroughMode]=(tmp1<<8)+tmp2; //低谷电价
						
						
						
						
						
						
//							tmp16data[4]=eleRate[SpikeMode];//60;	
//				tmp16data[5]=eleRate[PeakMode];//60;  //高峰电价
//				tmp16data[6]=eleRate[NormalMode];//60;  //平段电价
//				tmp16data[7]=eleRate[TroughMode];//60;  //低谷电价
			
			
			
			      AsciitoHex(&tmp1,&predat[32]);
			      AsciitoHex(&tmp2,&predat[34]);
			      gSinglePhasePower=((tmp1<<8)+tmp2);
						
						
						AsciitoHex(&tmp1,&predat[36]);
			      AsciitoHex(&tmp2,&predat[38]);
			      gAPhasePower=((tmp1<<8)+tmp2);
						
						
						AsciitoHex(&tmp1,&predat[40]);
			      AsciitoHex(&tmp2,&predat[42]);
			      gBPhasePower=((tmp1<<8)+tmp2);
						
						
						AsciitoHex(&tmp1,&predat[44]);
			      AsciitoHex(&tmp2,&predat[46]);
			      gCPhasePower=((tmp1<<8)+tmp2);
						
						AsciitoHex(&tmp1,&predat[64]);
			      AsciitoHex(&tmp2,&predat[66]);
			      gAcRateP=((tmp1<<8)+tmp2)*10;
						
						
						AsciitoHex(&tmp1,&predat[72]);
			      AsciitoHex(&tmp2,&predat[74]);
			      gAcPhase=((tmp1<<8)+tmp2);
						
						AsciitoHex(&tmp1,&predat[76]);
			      AsciitoHex(&tmp2,&predat[78]);
			      gsafedisV=((tmp1<<8)+tmp2);
						
						AsciitoHex(&tmp1,&predat[80]);
			      AsciitoHex(&tmp2,&predat[82]);
			      gdisDeep=((tmp1<<8)+tmp2);
						
						
					s16	 offset=pgh52c0->GetOffSet(&gsafedisV);
	       	pgh52c0->writeI2C(SYS_PARA+offset,(u8 *)&gsafedisV,32);
					
					
					offset=pgh52c0->GetOffSet(&eleRate);
	       	pgh52c0->writeI2C(SYS_PARA+offset,(u8 *)&eleRate,10);
			      
			
		}
					
	  }
	  if(cmd[0]==0x41)
		{
			u8 tmp1;
			
			 if(*port==2){//非网页请求
			 
				    	AsciitoHex(&tmp1,&predat[0]);
				    
				   if(tmp1==0x02)//三相
					 {
						    gSinglePhase=0;
					 }
					 else  if(tmp1==0x01)
					 {
						  gSinglePhase=1;//单相
					 }
				 pgh52c0->savePara(&gSinglePhase);
					 
			 }
			
		  
	  }
		if(cmd[0]==0x42)
		{
			   if(*port==2){//非网页请求  
					gSetStartBRI=AsciitoHex16(&predat[4]);
					pgh52c0->savePara(&gSetStartBRI);
					
					gbootBattCap=AsciitoHex16(&predat[8])/100;
					pgh52c0->savePara(&gbootBattCap);
					
					gstartdelay=AsciitoHex16(&predat[12])/60;
					
					
					pgh52c0->savePara(&gstartdelay);
					
					gSetEndBRI=AsciitoHex16(&predat[20]);
					 pgh52c0->savePara(&gSetEndBRI);
					
					genddelay=AsciitoHex16(&predat[28])/60;
					 pgh52c0->savePara(&genddelay);
					
					gequalTimeMax=AsciitoHex16(&predat[44])/60;
					pgh52c0->savePara(&gequalTimeMax);
					}

			
		}
	
	  if(cmd[0]==0xE1)
		{
			
		 if(*port==3){//在网页请求
	     u8 tmptype;
					
					for(u8 i=0;i<12;i++)
					{
						tmptype=WebtoInputDI[0x7F&predat[i+7]];
						
						
						if((bitAlarmTypeList[tmptype]!=0xFF)&&tmptype<12)
						{
							bitAlarmTypeList[gInPutAlarm[i].st_AlarmType]&=(~((u8)1<< (gInPutAlarm[i].st_InputNb%8)));
							
							  u8 j;
							for( j=0;j<8;j++)
							{
								if(((1<<j)&bitAlarmTypeList[tmptype])==0)
								{
									break;
								}
							}
							
							
							
							
							  gInPutAlarm[i].st_InputNb=j;
								gInPutAlarm[i].st_AlarmType=tmptype ;
								gInPutAlarm[i].st_AlarmTrigger=(predat[i+7]&0x80)>>7;
							  bitAlarmTypeList[gInPutAlarm[i].st_AlarmType]|=1<<(gInPutAlarm[i].st_InputNb%8);
							
							
							
							if((i<6)&&(predat[i+20]<7))//只有6个继电器其它忽略,另参数暂时只取0~6其它忽略
							{
								gInPutAlarm[i].st_behavior=predat[i+20];
							}
							
												
						}
						
					}
					  
					  saveE1D3=1;     
				
				}
			else if(*port==2)//非网页请求
			{
			u8 tmp1;
				 u8 tmptype;
				     for(u8 i=0;i<12;i++)
				{
				       AsciitoHex(&tmp1,&predat[2*i+14]);
				     tmptype=WebtoInputDI[0x7F&tmp1];
					   
					   if((bitAlarmTypeList[tmptype]!=0xFF)&&tmptype<12)
						{
							bitAlarmTypeList[gInPutAlarm[i].st_AlarmType]&=(~((u8)1<< (gInPutAlarm[i].st_InputNb%8)));
							
							  u8 j;
							for( j=0;j<8;j++)
							{
								if(((1<<j)&bitAlarmTypeList[tmptype])==0)
								{
									break;
								}
							}
							
							 AsciitoHex(&tmp1,&predat[2*i+14]);
							
							  gInPutAlarm[i].st_InputNb=j;
								gInPutAlarm[i].st_AlarmType=tmptype ;
								gInPutAlarm[i].st_AlarmTrigger=(tmp1&0x80)>>7;
							  bitAlarmTypeList[gInPutAlarm[i].st_AlarmType]|=1<<(gInPutAlarm[i].st_InputNb%8);
							
							
							   AsciitoHex(&tmp1,&predat[2*i+40]);
							  if((i<9)&&(tmp1<10))//9个继电器其它忽略,另参数暂时只取0~8其它忽略
							{
								gInPutAlarm[i].st_behavior=tmp1;
							}
							
				    }
			}
				
								for(u8 i=0;i<10;i++)
							{
								        AsciitoHex(&tmp1,&predat[2*i+64]);
								    branch[i]=tmp1;
							}
			
			
				 saveE1D3=1; 
     }
			
		}
	  if(cmd[0]==0xE2)
		{
			 
			      if(setE2D3==1) return NULL;//已在设置状态下，直接退出设置命令，（485端与网页端不能同时设置）
			
			    if(*port==3){//在网页请求
						
						   
						 u8 *ptmv8=(u8 *)pvPortMalloc(sizeof(u8)*33);
						
						
			        if(ptmv8==NULL) return NULL;
					    	pgetE2Databuff=ptmv8;
						   u8 *ptem8data=&predat[1];
						   ptmv8[4]=(ptem8data[0]<<4);//取下电模式低4位
						   ptmv8[4]|=(0x0F&ptem8data[34]);
						   *((u16 *)&ptmv8[9])=LittleToBig(*((u16*)(&ptem8data[1])));// 下电电压
						   *((u16 *)&ptmv8[17])=LittleToBig(*((u16*)(&ptem8data[3])));//下电时间
						     *((u16 *)&ptmv8[17])=  *((u16 *)&ptmv8[17])*10;
						   *((u16 *)&ptmv8[19])=LittleToBig(*((u16*)(&ptem8data[5])));//下电电量
						   *((u16 *)&ptmv8[11])=LittleToBig(*((u16*)(&ptem8data[27])));//年
						   *((u16 *)&ptmv8[13])=ptem8data[29];//月
						   *((u16 *)&ptmv8[15])=ptem8data[30];//日
						
						            if((ptem8data[35]==ptem8data[37])&&(ptem8data[36]==ptem8data[38]))
												{
													  ptmv8[21]=0;
												}
												else
												{
													  ptmv8[21]=1;
												}
						
						
						             ptmv8[22] =ptem8data[35];//下电开始时
						             ptmv8[23] =ptem8data[36];//下电开始分
						             ptmv8[24] =ptem8data[37];//下电结束时
						             ptmv8[25] =ptem8data[38];//下电结束分
												
												  if((ptem8data[39]==ptem8data[41])&&(ptem8data[40]==ptem8data[42]))
												{
													  ptmv8[26]=0;
												}
												else
												{
													  ptmv8[26]=1;
												}
						
												
						
						             ptmv8[27] =ptem8data[39];//下电开始时
						             ptmv8[28] =ptem8data[40];//下电开始分
						             ptmv8[29] =ptem8data[41];//下电结束时
						             ptmv8[30] =ptem8data[42];//下电结束分
												
												
										u8 broadNum;//板号
										u8 branchNum;//分路号
										u8 accCount=branch[0];//累计分路数
										
										 //设算板号及分路数		

												 for(u8 i=0;i<10;i++)
												{
													
													if(predat[0]>accCount)
													{
														
															 accCount+=branch[i+1];//累计下一分路的分路数
													}
													else
													{
														
															broadNum=i+1;////当查找分路数小于等于当前累计分路数时,记录当前分路板号
														branchNum=branch[i]+predat[0]-accCount;
														
														break;
														
													}
												}			
												
												
												
												
												
												
						      ptmv8[1]=broadNum;//设置板号
									ptmv8[3]=branchNum;//设置分路号
						  
						      ptmv8[31]=0x80;
						      setE2D3=1;
												
												
												
												
									u8	 datdelay=0;							
									while(1){ //挂起任务等待can设置
									datdelay++;
									vTaskDelay(20 / portTICK_RATE_MS);	
									if(setE2D3==0)
									{
										break;
									}
									
									if(datdelay>0x2f) //超时返回无效
									{
										vPortFree(ptmv8);
										setE2D3=0;
										return NULL;
										
									}
								}
						    vPortFree(ptmv8);
						
/*						
//					for(u8 i=0;i<2;i++)
//					{
//					 u8 *ptem8data=&predat[1+44*i];
//						gDcParam0[i].st_downModle=ptem8data[0];
//					  gDcParam0[i].st_LDVoltage=LittleToBig(*((u16*)(&ptem8data[1])));
//						gDcParam1[i].st_LDDelay=LittleToBig(*((u16*)(&ptem8data[3])));
//						gDcParam1[i].st_LDEnergy=LittleToBig(*((u16*)(&ptem8data[5])));
//						
//						gDcParam1[i].st_stopDate.st_year=LittleToBig(*((u16*)(&ptem8data[27])));
//						gDcParam1[i].st_stopDate.st_mon=ptem8data[29];
//					  gDcParam1[i].st_stopDate.st_day=ptem8data[30];
//						
//						gDcParam2[i].st_TimingDownElecStarHour=ptem8data[35];//下电开始时
//						gDcParam2[i].st_TimingDownElecStarMin=ptem8data[36];//下电开始分
//						gDcParam2[i].st_TimingDownElecStopHour=ptem8data[37];//下电结束时
//						gDcParam2[i].st_TimingDownElecStopMin=ptem8data[38];//下电结束分
//						gDcParam3[i].st_ExemptionStarHour=ptem8data[39];//免责开始时
//						gDcParam3[i].st_ExemptionStarMin=ptem8data[40];//免责开始分
//						gDcParam3[i].st_ExemptionStopHour=ptem8data[41];//免责结束时
//						gDcParam3[i].st_ExemptionStopMin=ptem8data[42];//免责结束分
//						gDcParam3[i].st_OilMachineExemption=ptem8data[43];
//						
//						
//					}
//					
//                 saveE2D3=1;
*/
				}
			 else if(*port==2)//H52C0F协试
			 {
/*				
				
				   isTieta=0;
				 u16 tmp16;
				 u8 tmp8;
				 u8 lentmp;
				 u8 *pdata; 
				      AsciitoHex(&lentmp,&predat[0]);
				        pdata=predat+2;
				 if(lentmp<3)//2分路
				 {
				         for(u8 i=0;i<lentmp;i++)
				      {
								
								     AsciitoHex(&tmp8,&pdata[0+88*i]);//下电模式
				           gDcParam0[i].st_downModle=tmp8;
								   
								   tmp16= AsciitoHex16(&pdata[2+88*i]);//下电电压
								   gDcParam0[i].st_LDVoltage=tmp16;
								
								     tmp16= AsciitoHex16(&pdata[6+88*i]);//下电时间
								   gDcParam1[i].st_LDDelay=tmp16;
								
								
								        AsciitoHex(&tmp8,&pdata[70+88*i]);
								   gDcParam2[i].st_TimingDownElecStarHour=tmp8;//定时下电开始时
								
								       AsciitoHex(&tmp8,&pdata[72+88*i]);
								   gDcParam2[i].st_TimingDownElecStarMin=tmp8;//定时下电开始分
								
								       AsciitoHex(&tmp8,&pdata[74+88*i]);
								  gDcParam2[i].st_TimingDownElecStopHour=tmp8;//定时下电结束时
								        
								        AsciitoHex(&tmp8,&pdata[76+88*i]);
					      	gDcParam2[i].st_TimingDownElecStopMin=tmp8;//定时下电结束分
									
									
									
									if((gDcParam2[i].st_TimingDownElecStarHour==gDcParam2[i].st_TimingDownElecStopHour)&&
										(gDcParam2[i].st_TimingDownElecStarMin==gDcParam2[i].st_TimingDownElecStopMin))
									{
										gDcParam2[i].st_TimingDownElecEnable=0;
									}
									else
									{
										  gDcParam2[i].st_TimingDownElecEnable=1;
									}
									
								
							}
							
							saveE2D3=1;
						}
				 
				 
				 
		*/		
		
		    u8 *ptmv8=(u8 *)pvPortMalloc(sizeof(u8)*33); 
		            if(ptmv8==NULL) return NULL;
									pgetE2Databuff=ptmv8;
									
									 u8 *pdata; 
									 
							
							
				        
										u8 tmpbranch;		 
										AsciitoHex(&tmpbranch,&predat[0]);//取出配电分路号
								
								
							    	u8 broadNum;//板号
										u8 branchNum;//分路号
										u8 accCount=branch[0];//累计分路数
										
										
										
										
												
												
												
												
									u8	 datdelay=0;							
									while(1){ //挂起任务等待can设置
									datdelay++;
									vTaskDelay(20 / portTICK_RATE_MS);	
									if(setE2D3==0)
									{
										break;
									}
									
									if(datdelay>0x2f) //超时返回无效
									{
										vPortFree(ptmv8);
										setE2D3=0;
										return NULL;
										
									}
								}
												
												
								pdata=predat+2;
												
												
												 AsciitoHex(&tmpbranch,&pdata[0]);//下电模式
								                 ptmv8[4]=(tmpbranch<<4);
												 AsciitoHex(&tmpbranch,&pdata[68]);
												       ptmv8[4]|=(0x0F&tmpbranch);//用户
												
												

												
												
												
												  *((u16 *)&ptmv8[9])= AsciitoHex16(&pdata[2]);//下电电压
								          *((u16 *)&ptmv8[17])= (AsciitoHex16(&pdata[6]))/10;//下电时间
                          *((u16 *)&ptmv8[19])= AsciitoHex16(&pdata[10]);//下电时间
								          *((u16 *)&ptmv8[11])=AsciitoHex16(&pdata[54]);//年
													  AsciitoHex(&tmpbranch,&pdata[58]);//月
													   *((u16 *)&ptmv8[13])=tmpbranch;
														AsciitoHex(&tmpbranch,&pdata[60]);//月
														 *((u16 *)&ptmv8[15])=tmpbranch;//日
														 
														 
														 
														 AsciitoHex(&tmpbranch,&pdata[70]);//定时下电开始时
														 ptmv8[22]=tmpbranch;
														 
														  AsciitoHex(&tmpbranch,&pdata[72]);//定时下电开始分
														 ptmv8[23]=tmpbranch;
														 
														  AsciitoHex(&tmpbranch,&pdata[74]);//定时下电结束时
														 ptmv8[24]=tmpbranch;
														 
														  AsciitoHex(&tmpbranch,&pdata[76]);//定时下电结束分
														 ptmv8[25]=tmpbranch;
														 
														 if((ptmv8[22]== ptmv8[24])&&(ptmv8[23]== ptmv8[25]))
														 {
															 ptmv8[21]=0;
														 }
														 else
														 {
															 ptmv8[21]=1;
														 }
														 
														 
														 
														  AsciitoHex(&tmpbranch,&pdata[78]);//免责下电开始时
														 ptmv8[27]=tmpbranch;
														 
														  AsciitoHex(&tmpbranch,&pdata[80]);//免责下电开始分
														 ptmv8[28]=tmpbranch;
														 
														  AsciitoHex(&tmpbranch,&pdata[82]);//免责下电结束时
														 ptmv8[29]=tmpbranch;
														 
														  AsciitoHex(&tmpbranch,&pdata[84]);//免责下电结束分
														 ptmv8[30]=tmpbranch;
														 
								
                           if((ptmv8[27]== ptmv8[29])&&(ptmv8[28]== ptmv8[30]))
														 {
															 ptmv8[26]=0;
														 }
														 else
														 {
															 ptmv8[26]=1;
														 }
												
												     
												      //设算板号及分路数		

												 for(u8 i=0;i<10;i++)
												{
													
													if(tmpbranch>accCount)
													{
														
															 accCount+=branch[i+1];//累计下一分路的分路数
													}
													else
													{
														
															broadNum=i+1;////当查找分路数小于等于当前累计分路数时,记录当前分路板号
														branchNum=branch[i]+tmpbranch-accCount;
														
														break;
														
													}
												}	
												
												
									ptmv8[1]=broadNum;//设置板号
									ptmv8[3]=branchNum;//设置分路号
						  
						      ptmv8[31]=0x80;
						      setE2D3=1;
												
												
								
								
		
		       vPortFree(ptmv8);
				 
			 
			 
			 
			 
			 }
			 
		}
	
	}
/*
	else if(cmd[1]==0xD4) //  7.13	告警记录
  { u8 tmp8=0;
		if(cmd[0]==0x40)
		{   
        tmp8=0;		//交流瓶	
		}
		else if((cmd[0]==0x41)||(cmd[0]==0x43))
		{
			 tmp8=3;		//模块
		}
		else if(cmd[0]==0x42)	
		{
		  tmp8=1;		//直流瓶
		}
		else if(cmd[0]==0xE1)	
		{
			tmp8=2;		 //系统
		}
		else if(cmd[0]==0xE2)	
		{
			tmp8=4;		 //用户配电
		}			
			u8 tmpwarn[11];
			         tmpwarn[0]=0;
			         tmpwarn[1]=1;
						   tmpwarn[2]=30;
			         tmpwarn[3]=15;
			         tmpwarn[4]=20;
			         tmpwarn[5]=33;
			         tmpwarn[6]=3;
			         tmpwarn[7]=0x10;
			         tmpwarn[8]=1;
			     u16 _year=2021;
			
			
			
			
			
			     if(cmd[2]==0)//第一条记录
					 {
						 //gAlarmRecord[tmp8]=alarmhead[tmp8];
						   gAlarmRecord[tmp8]=alarmtail[tmp8];//取得某一类型告警的最近一条记录
						 //if(gAlarmRecord[tmp8]==0xFFFF) return NULL;//首条记录为空，直接返回
		
						 
					 }
					
			      if(gAlarmRecord[tmp8]==0xFFFF) return NULL;//首条记录为空，直接返回
			      pgh52c0->readI2C(ALARM_DATA_STAR+12*gAlarmRecord[tmp8],&tmpwarn[6],6);//获取当记录时间
						       _year=((uTIME *)(&tmpwarn[6]))->year;
						        tmpwarn[1]=((uTIME *)(&tmpwarn[6]))->mon;
						        tmpwarn[2]=((uTIME *)(&tmpwarn[6]))->day;
						        tmpwarn[3]=((uTIME *)(&tmpwarn[6]))->hour;
						        tmpwarn[4]=((uTIME *)(&tmpwarn[6]))->min;
						        tmpwarn[5]=((uTIME *)(&tmpwarn[6]))->sec;
						 
						 pgh52c0->readI2C(ALARM_DATA_STAR+6+12*gAlarmRecord[tmp8],&tmpwarn[9],2);//获取告警数据
						 
						 
						 warnChange(tmpwarn,(WarnStruct *)&tmpwarn[9]);//告警数据映射转换
					 
						 //////////////////////////////////将告警记录指针指向下一条记录//////////////////////////////////////
//					  if(cmd[2]<2)// 0,1
//						  pgh52c0->readI2C(ALARM_DATA_STAR+10+12*gAlarmRecord[tmp8],(u8 *)&gAlarmRecord[tmp8],2);//获取后向索引
//					 else if(cmd[2]==2)
						   pgh52c0->readI2C(ALARM_DATA_STAR+8+12*gAlarmRecord[tmp8],(u8 *)&gAlarmRecord[tmp8],2);//获取前向索引
						 
						 if(gAlarmRecord[tmp8]==0xffff)//指针为0xFFFF 表示为最后一条记录，后面无记录
						 {
							 tmpwarn[0]=0x01;
						 }
						 
			
			
			   ytd1363data *cidD440=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*12);
						
							if(cidD440!=NULL)
							{
								    SetYDT1363(cidD440,TYPE_ONEBYTE,&tmpwarn[0] ,1);// DATATYPE
								    SetYDT1363(NULL,TYPE_TWOBYTE,&_year ,1);//DATATIME1
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[1] ,1);//DATATIME3
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[2] ,1);//DATATIME4
								    SetYDT1363(NULL,TYPE_ONEBYTE,& tmpwarn[3] ,1);//DATATIME5
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[4] ,1);//DATATIME6
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[5] ,1);//DATATIME7
								    SetYDT1363(NULL,TYPE_ONEBYTE,& tmpwarn[6] ,1);//WARNTYPE
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[8] ,1);//WRANPARA H
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[7] ,1);//WRANPARA L
								     SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[9] ,1);//EVENTFLAG
								     END_SET_YDT1363//结束标志，用以计算长度 
								
								 pdat=Getdata(cidD440,0,NULL,NULL);
							}
							
							
							 vPortFree(cidD440);
			       cidD440=NULL;
							  
		//}
	}


*/
	else if(cmd[1]==0xD4)//导出告警记录
	{
		if(cmd[0]==0xD0)
		{ 
			 
			 u8 cmd_type;
			u8 tmpwarn[10];
			u16 _year;
			
			 AsciitoHex(&cmd_type,&predat[0]);
			
		//	u16 tmp16[2];
			s16 getaddr=0;			
					
					
			
			
			if(cmd_type==0)
			{
				 gAlarmRecord[0]=0;//取第一条

			}
			else if(cmd_type==1)
			{
    			 gAlarmRecord[0]++;//取下一条

			}
			
			
			  RecordInfo Record={ &recordAlarmCount,&galarmRecordMax,&gAlarmRecord[0]};						
			  getaddr=RecordOrderNumToGetAddr(&Record,ALARM_RECORD_LEN);
		  	tmpwarn[0]=0;
				
			 if(gAlarmRecord[0]==gAlarmRecord[1])//最后一条
			 {
				 tmpwarn[0]=1; 
				 gAlarmRecord[0]--;
				 
			 }
			
			
			 pgh52c0->readI2C(ALARM_DATA_STAR+12*getaddr,&tmpwarn[6],6);//获取当记录时间
						       _year=((uTIME *)(&tmpwarn[6]))->year;
						        tmpwarn[1]=((uTIME *)(&tmpwarn[6]))->mon;
						        tmpwarn[2]=((uTIME *)(&tmpwarn[6]))->day;
						        tmpwarn[3]=((uTIME *)(&tmpwarn[6]))->hour;
						        tmpwarn[4]=((uTIME *)(&tmpwarn[6]))->min;
						        tmpwarn[5]=((uTIME *)(&tmpwarn[6]))->sec;
			 
			  pgh52c0->readI2C(ALARM_DATA_STAR+6+12*getaddr,&tmpwarn[6],2);//获取告警数据
			
			  ytd1363data *cidD0D4=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*22);
			
			   if(cidD0D4!=NULL)
							{
								    SetYDT1363(cidD0D4,TYPE_ONEBYTE,&tmpwarn[0] ,1);// DATATYPE
				            SetYDT1363(NULL,TYPE_TWOBYTE,&_year ,1);//DATATIME1
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[1] ,1);//DATATIME3
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[2] ,1);//DATATIME4
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[3] ,1);//DATATIME5
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[4] ,1);//DATATIME6
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[5] ,1);//DATATIME7
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[6] ,1);//告警码
								    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpwarn[7] ,1);//告警码

								     END_SET_YDT1363//结束标志，用以计算长度 
								
								 pdat=Getdata(cidD0D4,0,NULL,NULL);
							}
			
			  vPortFree(cidD0D4);
			  
		}
	}
	else if(cmd[1]==0xD5)//产品信息
	{
		 if(cmd[0]==0xE1)
		 {  // u8 tmp8[2]={0x14,0xE6};//额定电压
			  u8 *tmpv8data;
			  u8 *ptmpv8=(u8 *)pvPortMalloc(sizeof(u8)*62);
			     
			     if(ptmpv8==NULL) return NULL;
			      tmpv8data=ptmpv8;
			 
			 
			 
			 
			    ytd1363data *cidE1D5=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*63);
			    if(cidE1D5!=NULL)
					{  
						     * ((u16*)&tmpv8data[0])= LittleToBig(5350);
                 * ((u16*)&tmpv8data[2])= LittleToBig(gMaxModuleCurr);
                      tmpv8data[4]=0;//保留
						          tmpv8data[5]=0;//保留
						          tmpv8data[6]=VerNB/100;//版本
						          tmpv8data[7]=VerNB%100;//版本

							 
						  	SetYDT1363(cidE1D5,TYPE_ONEBYTE,&tmpv8data[0],1);//半个额定电压
						       for(u8 i=1;i<8;i++)
					    	SetYDT1363(NULL,TYPE_ONEBYTE,&tmpv8data[i],1);//半个额定电压，系统额定电流 ,主版本号，次版本号
						 
						 tmpv8data=&tmpv8data[8];
						// tmpv8data=(u8 *)"H52c0";
						memcpy(tmpv8data,(const u8 *)"MER048-600",10);
						  for(u8 i=10;i<16;i++)
						  tmpv8data[i]=' ';
						   for(u8 i=0;i<16;i++)
					    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpv8data[i],1);//系统型号
						
						    tmpv8data=&tmpv8data[16];
						 memcpy(tmpv8data,(const u8 *)gmSN,16);
						for(u8 i=0;i<16;i++)
					    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpv8data[i],1);//监控模块序列号
							
							 tmpv8data=&tmpv8data[16];
						 memcpy(tmpv8data,(const u8 *)gsSN,16);
						for(u8 i=0;i<16;i++)
					    SetYDT1363(NULL,TYPE_ONEBYTE,&tmpv8data[i],1);//系统序列号
							
//							 tmpv8data=&tmpv8data[16];
//						 memcpy(tmpv8data,(const u8 *)"123456",6);
						for(u8 i=0;i<6;i++)
					    SetYDT1363(NULL,TYPE_ONEBYTE,&gMAC[i],1);//MAC
							
							END_SET_YDT1363//结束标志，用以计算长度
							
							 pdat=Getdata(cidE1D5,0,NULL,NULL); 
					    vPortFree(cidE1D5);
					    cidE1D5=NULL; 
						
					}
					vPortFree(ptmpv8);
		 }
	}
	
	else if(cmd[1]==0xD6) // 时刻表
	{
			if(cmd[0]==0x40)
			{
				u8 *ptmpTimeTable=(u8 *)pvPortMalloc(sizeof(u8)*36*5);
				
				if(ptmpTimeTable!=NULL)
				{
				   
				   ytd1363data *cid40D6=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*122);
				   if(cid40D6!=NULL)
					 {
										 SetYDT1363(cid40D6,TYPE_ONEBYTE,NULL ,5);//时间表数量5   
									 for(u8 i=0;i<5;i++ )	
								 { 				
									 
									u16 *ptmp16=(u16 *)&ptmpTimeTable[i*36]; 
									u8  *ptmp8=(u8 *)&ptmp16[12];
										 
									 
											YeardayToDate(gsPeakTimeInteval[i].st_yday,(u8*)&ptmp16[0],1);
										 SetYDT1363(NULL,TYPE_TWOBYTE,&ptmp16[0] ,1);//时刻表生效日期
									 
										for(u8 j=1;j<12;j++)//时段2~12
									 {
										 
												 GetTimeInMin(gsPeakTimeInteval[i].st_time[j],(u8*)&ptmp16[j],1);
												SetYDT1363(NULL,TYPE_TWOBYTE,&ptmp16[j] ,1);//时段开始时刻 ，时段1默认为0不用发
									 }
									 
									 
										for(u8 j=0;j<12;j++)//时段类型1~12
									 {
														ptmp8[j]=GetSpeakFlag(i,j);
												SetYDT1363(NULL,TYPE_ONEBYTE,&ptmp8[j] ,1);//时段开始时刻
									 }
									 
									 
					
								 }
									END_SET_YDT1363//结束标志，用以计算长度 
										pdat=Getdata(cid40D6,0,NULL,NULL);
											
											 vPortFree(cid40D6);
					}								 
               vPortFree(ptmpTimeTable);									 
			   }    		
				       ptmpTimeTable=NULL;
				
			}
	}

	 else if(cmd[1]==0xD7)//时刻表设置
	{
		 if(cmd[0]==0x40)
		 {
			 //if(isTieta!=2){//网页 ,协议无相关指令
			   if(*port==3){//在网页请求
				//	if(predat[0]==5) {//总共为5个列表
						
//							for(u8 i=0;i<5;i++)
//								{
					        u8  i=predat[0]-1;
									//u8 *ptmp=&predat[i*36+1];
									u8 *ptmp=&predat[1];
								
									SetSPeakData(i,ptmp[0],ptmp[1]);
										for(u8 j=1;j<12;j++)
									{
										SetIimeInteval(i,j,ptmp[2*j],ptmp[2*j+1]);
									}
									
										 ptmp=&ptmp[24];
									
									for(u8 j=0;j<12;j++)
									{
										SetSpeakFlag(i,j,(SpeakFlag)ptmp[j]);
									}
									
									
									
									
								//}

					//}
				   
				}
        else if(*port==2)//非网页设置
				{
					u8 tmp1,tmp2,tmp3;
					   AsciitoHex(&tmp1,&predat[0]);//第几大段
					   tmp1=tmp1-1;
					    AsciitoHex(&tmp2,&predat[2]);
					    AsciitoHex(&tmp3,&predat[4]);
					
					  SetSPeakData(tmp1,tmp2,tmp3);//设置大段日期
					
					  SetIimeInteval(tmp1,0,0,0);//第一个时段默认为0时0分
					  for(u8 i=0;i<11;i++)
					{
						
						  AsciitoHex(&tmp2,&predat[6+4*i]);
					    AsciitoHex(&tmp3,&predat[6+4*i+2]);
						 
						 SetIimeInteval(tmp1,i+1,tmp2,tmp3);
					}
					
					
					 for(u8 i=0;i<12;i++)
					{
						
						  AsciitoHex(&tmp2,&predat[50+2*i]);
					  
						 
						 SetSpeakFlag(tmp1,i,(SpeakFlag)tmp2);
					}
					
					
					
				}		

          save40D7=1;				
		 }
	}
	else if(cmd[1]==0xD8)//定义数校准
	{
		 u8 _type;
		 if(*port==2){//非网页设置
				AsciitoHex(&_type,&predat[0]);
				AsciitoHex(&CalibrationI_ch,&predat[2]);
				CalibrationI_val=bswap_32(AsciitoHex32(&predat[4])) ;
		 }
	
		if(_type==1)
		{
			setCalibrationI=1;
		}
		else if(_type==0)
		{
			setCalibrationV=1;
		}
		
		
	}
	else if(cmd[1]==0xD9)//定义数校准
	{
		 int a=2;
	}

  else if(cmd[1]==0xDA)//身份认证
	{
		
		if(cmd[0]==0xE1)
		{
			  u8 auser=0;
			 ytd1363data * cid42DA=NULL;
			   for(u8 i=0;i<8;i++)
			{
				     if(predat[i]>0x30)
						 {
							   predat[i]=(~(predat[i]-0x30))&0x0F;
						 }
			}
			 
			
			                                       //  <- 
			if((predat[0]==2)&&(predat[1]==1)&&    //  2   1
				  (predat[2]==4)&&(predat[3]==3)&&   //  4   3
          (predat[4]==6)&&(predat[5]==5)&&   //  6   5
			   (predat[6]==0x2F)                        
		     )//123456
			{
				   auser=1; //L1
			}                                          //    <-
			else if((predat[0]==5)&&(predat[1]==6)&&   //   5   6
				       (predat[2]==3)&&(predat[3]==4)&&   //  3   4
               (predat[4]==1)&&(predat[5]==2)&&  //   1   2
			        (predat[6]==0x2F)                   //  
		     )//123457
			{
				   auser=2;  //L2
			}
			else {
				       if((predat[6]==0x2F)|| (predat[7]==0x2F))
							 {
								 auser=0;//第 7 8位为空时不用再作判断，返回无效密码即可
							 }
							 else
							 {
									 u8 _mon=predat[1]*10+predat[0];
									 u8 _day=predat[3]*10+predat[2];
									 u8 _hour=predat[5]*10+predat[4];
									 u8 _min=predat[7]*10+predat[6];
								
								if( (_mon==gTimer.mon)&&
										(_day==gTimer.day)&&
										(_hour==gTimer.hour)&&
										(_min==gTimer.min)
									 )
									 {
									 auser=3;  //L3
									 }
								 }
				
				
				
			}
			
			
			
			 auser=3; 
			
			  cid42DA=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*2);	
			if(cid42DA!=NULL)
			{
			   	SetYDT1363(cid42DA,TYPE_ONEBYTE,&auser ,1);//dataflag
			   END_SET_YDT1363//结束标志，用以计算长度
			   pdat=Getdata(cid42DA,0,NULL,NULL);
				 vPortFree(cid42DA);
				
			}
		}
	}
	else if(cmd[1]==0xDC)//设备配置
  {
                u8 ppdat[11]={5,0x40,1,0x41,6,0x42,1,0xE1,1,0xE2,2};
                            
                        
                    ytd1363data *cid4XDC=(ytd1363data *)pvPortMalloc(sizeof(ytd1363data)*12);
                                 if(cid4XDC!=NULL)
                   {
                                           SetYDT1363(cid4XDC,TYPE_ONEBYTE,&ppdat[0],1);//??5  
                                     for(u8 i=1;i<11;i++)
                                                 {
                                                           SetYDT1363(NULL,TYPE_ONEBYTE,&ppdat[i],1);
                                                 }
                                           END_SET_YDT1363//????,??????
                                                 
                                                  pdat=Getdata(cid4XDC,0,NULL,NULL); 
                                            vPortFree(cid4XDC);
                                            cid4XDC=NULL; 
                                                 
                                        
                        }
   }
	else if(cmd[1]==0xDD)//序列号获取
	{
		 	if(cmd[0]==0xE1)
		{
			 u8 cmd_type;
			 AsciitoHex(&cmd_type,&predat[0]);
			
			if(cmd_type==0x80)
			{
				for(u8 i=0;i<16;i++)
				{
					//gmSN[i]=predat[i+2];
						AsciitoHex(&gmSN[i],&predat[(i+1)*2]);
				}
				gmSN[16]=0;
					pgh52c0->writeI2C(SYS_INFO_START,gmSN,17);
			}
			else if(cmd_type==0x81)
			{
				  for(u8 i=0;i<16;i++)
					{
						//gsSN[i]=predat[i+2];
						AsciitoHex(&gsSN[i],&predat[(i+1)*2]);
					}
					gsSN[16]=0;
					pgh52c0->writeI2C(SYS_INFO_START+17,gsSN,17);
			}
			else if(cmd_type==0x82)
			{
				   for(u8 i=0;i<6;i++)
					{
						///gMAC[i]=predat[i+2];
						AsciitoHex(&gMAC[i],&predat[(i+1)*2]);
					}
					
					pgh52c0->writeI2C(SYS_INFO_START+34,gMAC,6);
			}
		}
	}
	
	return  pdat;
}


u8 * protocol_TieTa_1363(u8 Ver, u8 COMMAND_GROUP, u8 ADDR_Temp,u8 CID1_Temp,u8 CID2_Temp,u8 * Info_Data )//生成1363协议包头
{
	
   u8 * Point;
	u16 Info_Data_Length;  //Info长度
	u16 Data_Length_TOT;

	u16 Info_Data_Length_S;  //转换后

 
	u16 CheckSum;
	
  u8 LCHKSUM;
  u8 temp;
	
	u8 L1=0;
	u8 L2=0;
	u8 L3=0;
	//u8 myaddr=monitor_addr;
 

 if((CID2_Temp==0x4F)||(CID2_Temp==0x50))//请求版本号，或地址，或用无效地址请求
  {
	   Info_Data_Length=0;
		 CID2_Temp=0;
		 ADDR_Temp=monitor_addr;
	}
 else
 {
	 if(Info_Data!=NULL)
	  {
	   Info_Data_Length = strlen((const char *)Info_Data);//获取字据段长度
	    if( Info_Data_Length <=0  )
			 return 0;
			CID2_Temp=0;
		}
		else
		{
			Info_Data_Length=0;
		 


			
		}
		
		 if(ADDR_Temp!=monitor_addr)	//地址不是本机，且不是查询地址指令不返回任何东西
		 return 0;		
		
 }
			
			 
			
	
	Data_Length_TOT=Info_Data_Length+18;//计算总帧长
  
//   if(COMMAND_GROUP==0xFF)
//	 Data_Length_TOT=Data_Length_TOT+2;//若COMMAND_GROUP为0xFF，再插入2个字节长度	 
			
   Point = (u8*)pvPortMalloc(Data_Length_TOT+1);//获取空间，预留一个空字节
	  if( Point == NULL )
		return 0;		
			

    Point[0] = SOI ; //帧头  
    HextoAscii(&Point[1],&Ver);//版本号
    HextoAscii(&Point[3],&ADDR_Temp);//地址
    HextoAscii(&Point[5],&CID1_Temp);//CID1
    HextoAscii(&Point[7],&CID2_Temp);//CID2



	/******************** Length = LENID + LCHKSUM *******************/
	 L1 =    (u8)(Info_Data_Length>>8)&0x0F;    
	 L2 =  (((u8)Info_Data_Length) &0xF0)>>4;
   L3 =    (u8)Info_Data_Length &0x0F;
	 LCHKSUM =  ~((L1 + L2 + L3)%16) +1  ;
	 LCHKSUM<<=4;
   Info_Data_Length_S = (LCHKSUM<<8)| Info_Data_Length;

     temp=(Info_Data_Length_S&0xff00)>>8;
	   HextoAscii(&Point[9],&temp);//Len1
		 temp=(u8)Info_Data_Length_S;
	   HextoAscii(&Point[11],&temp);//Len2
	
	
	
	 u16 base=13;//数据段开始
	
    //Info 数据
	 if(Info_Data!=NULL)//数据指针不为空
	 {
	   memcpy( (char*)&Point[base],(const char *)Info_Data,Info_Data_Length );
     vPortFree(Info_Data);//释放 Info_Data 所占用的内存
	 }
	 CheckSum = Check_Sum(Point,Data_Length_TOT-6);//扣除SOI 校验和 EOI所占的长度
   base=Data_Length_TOT-5;//第一个校验字节的位置
    
	 temp=(CheckSum&0xff00)>>8;
	 HextoAscii(&Point[base],&temp);//校验字节1
	 base+=2;
	 temp=(u8)CheckSum;
	 HextoAscii(&Point[base],&temp);//校验字节2
	 base+=2;
   Point[base]=EOI;
	 Point[base+1]='\0';
	 return Point;
}





