#include "globalval.h"


u8 SendCounterU1=12;
u8 ErrCode=0;
u8 Senbuf[]={0x7E,0x01,0x01,0x14,0x22,0x0D};
u8 SendHotFanPara[9]={0x7E,0x01,0x04,0x18,0x00,0x00,0x00,0x00,0x0D};
u8 gCmdCode=0;
void HotFanOnTick(void)
{
	   if(SendCounterU1>0)
			{
		    SendCounterU1--;
			}
		  else if(SendCounterU1==0)
			{
				  SendCounterU1=7;
				if(gCmdCode==0)
				{
				  pusart1->SendData(Senbuf,6);
				}
				else if(gCmdCode==1)
				{
					 pusart1->SendData(SendHotFanPara,9);
					 gCmdCode=0;
				}
				
			}
			
			
}

u8 BCDtoHEX(u8 _data)
{
	 u8 tmpH,tmpL;
	   tmpH=(_data&0xF0)>>4;
	   tmpL=(_data&0x0F);
	return tmpH*10+tmpL;
}

u8 HEXtoBCD(u8 _data)
{
	return (((_data%100)/10)<<4)|(_data%10);
}



void *DuleWithDataHotFan(u8 *pdat,u16 *plen)
{ u8 recvlen=0,sum=0;
	  if(pdat[0]==0x7E)
		{
			
			u8 i;
			for(i=0;i<MAX_HOT_FAN_RECV_LEN;i++)
			{
				    recvlen++;
				   if(pdat[i]==0x0D)
						 break;
			}
			
			if(i==MAX_HOT_FAN_RECV_LEN){ ErrCode=1;//找不到0D
				return NULL;
			}
			
			if((recvlen-5)!=BCDtoHEX(pdat[2])){ ErrCode=2;//数据长度不对
				return NULL;
			}
			
			
			for(i=1;i<recvlen-2;i++)
			{
				sum+=pdat[i];
			}
			  
			if(HEXtoBCD(sum)!=pdat[recvlen-2]){ ErrCode=3;//校验不对
				return NULL;
			}
			
			
			if(pdat[3]==0x94)
			{
				
				
				
////////////////////////////////实时数据///////////////////////////////////////////					
				
				   u8 sign_status=BCDtoHEX(pdat[4]);
				   gHotFan.Temp1=BCDtoHEX(pdat[5]);
				  if((sign_status&0x10)==0x10)//设为负数
					{
						   gHotFan.Temp1=~gHotFan.Temp1;
						   gHotFan.Temp1+=1;
					}
					
					 gHotFan.Temp2=BCDtoHEX(pdat[6]);
				  if((sign_status&0x01)==0x01)//设为负数
					{
						   gHotFan.Temp2=~gHotFan.Temp2;
						   gHotFan.Temp2+=2;
					}
					
					
					gHotFan.FanSpeed1=BCDtoHEX(pdat[7]);//风扇1转速
					
					gHotFan.FanSpeed2=BCDtoHEX(pdat[8]);//风扇2转速
					
					sign_status=BCDtoHEX(pdat[9]);
					//风扇1启动
					if(((sign_status&0x10)==0x10)&&(gHotFan.Temp1!=99))//温度正常时才设置启动标志,温度99 时表示温度传感器异常
					{
						  gHotFan.FS1=1;
					}
					else
					{
						  gHotFan.FS1=0;
					}
					//风扇2启动
					if(((sign_status&0x01)==0x01)&&(gHotFan.Temp2!=99))//温度正常时才设置启动标志,温度99 时表示温度传感器异常
					{
						  gHotFan.FS2=1;
					}
					else
					{
						   gHotFan.FS2=0;
					}
					
						//风扇1故障
					if((sign_status&0x20)==0x20)
					{
						  gHotFan.FF1=1;
					}
					else
					{
						  gHotFan.FF1=0;
					}
					
					//风扇2故障
					if((sign_status&0x02)==0x02)
					{
						  gHotFan.FF2=1;
					}
					else
					{
						  gHotFan.FF2=0;
					}
					
					
					sign_status=BCDtoHEX(pdat[10]);
				  //加热板1启动
					if((sign_status&0x10)==0x10)
					{
						  gHotFan.HS1=1;
					}
					else
					{
						  gHotFan.HS1=0;
					}
					//加热板2启动
					if((sign_status&0x01)==0x01)
					{
						  gHotFan.HS2=1;
					}
					else
					{
						   gHotFan.HS2=0;
					}
					
					sign_status=BCDtoHEX(pdat[11]);
				  //温度传感器故障
					if((sign_status&0x10)==0x10)
					{
						  gHotFan.TF1=1;
					}
					else
					{
						  gHotFan.TF1=0;
					}
					//加热板2启动
					if((sign_status&0x01)==0x01)
					{
						  gHotFan.TF2=1;
					}
					else
					{
						   gHotFan.TF2=0;
					}
					
					
////////////////////////////////参数///////////////////////////////////////////		
					
					   gHotFanPara.FanStar1=BCDtoHEX(pdat[12]);//风扇1启动温度
				     gHotFanPara.FanStop1=BCDtoHEX(pdat[13]);//风扇1停止温度
				
				     gHotFanPara.FanStar2=BCDtoHEX(pdat[14]);//风扇2启动温度
				     gHotFanPara.FanStop2=BCDtoHEX(pdat[15]);//风扇2停止温度
				
				     sign_status=BCDtoHEX(pdat[16]);
				
				      gHotFanPara.HotStar1=BCDtoHEX(pdat[17]);
				
				    if((sign_status&0x10)==0x10)//设为负数
					{
						    gHotFanPara.HotStar1=~ gHotFanPara.HotStar1;
						    gHotFanPara.HotStar1+=1;
					}
					
					
					  gHotFanPara.HotStop1=BCDtoHEX(pdat[18]);
				
				    if((sign_status&0x20)==0x20)//设为负数
					{
						    gHotFanPara.HotStop1=~ gHotFanPara.HotStop1;
						    gHotFanPara.HotStop1+=1;
					}
					
					
					
				
					
					 gHotFanPara.HotStar2=BCDtoHEX(pdat[19]);
					 if((sign_status&0x01)==0x01)//设为负数
					{
						    gHotFanPara.HotStar2=~ gHotFanPara.HotStar2;
						    gHotFanPara.HotStar2+=1;
					}
					
					
					
					  gHotFanPara.HotStop2=BCDtoHEX(pdat[20]);
				
				    if((sign_status&0x02)==0x02)//设为负数
					{
						    gHotFanPara.HotStop2=~ gHotFanPara.HotStop2;
						    gHotFanPara.HotStop2+=1;
					}
					
			return NULL;	
				
				
			}
			else
			{
				 ErrCode=4;//无效命令
				return NULL;
			}
			
			
			
		}
}


void SetTemp(u8 ch,u8 *pdat)
{
	     SendHotFanPara[4]=HEXtoBCD(ch);
	    s16 tmp=(*(s16*)pdat);
	      SendHotFanPara[5]=0;
	       if(tmp<0)
				 {
					  SendHotFanPara[5]=HEXtoBCD(1);
					  tmp=tmp*(-1);
				 }
				 
				 SendHotFanPara[6]=HEXtoBCD((u8)tmp);
	
			u8 sum=0;
				 for(u8 i=1;i<7;i++)
				 {
					  sum+=SendHotFanPara[i];
				 }
				 SendHotFanPara[7]=HEXtoBCD(sum);
				 
				 
}









