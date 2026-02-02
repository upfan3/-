#include "globalval.h"
#define ONLINE_COUNT_MAX 50
#define DELAY_CHECK_MAX 60

SwitchData gSwitchData[TOTAL_USER];
u8 stopPowrSupply=0;
u8 setFlag[TOTAL_USER];
u8 gcmdAddr=0;
//u8 gswitchFactory=0;  //取0：京硅 取 1:泰永
u32 gsetTmpData=0;
//Timer4 CurrTime={10,12,31,12,1970};//当前时间

u8 Cmd03[2]={0,0};

u8 AddrChange(u8 n,u8 type)
{
	  if(n==0) return 0;
	if(type==0 || type==2)
	{
	    u8 tmp=n%12;
					if(tmp==0) 
					{
						n=n-1;//要将0xOc（12），计数到上一个整除结果之中，因些将n减1
						tmp=0x0c;
					}
			 switch( n/12)
			 {
				 case 0:
					 return 0x10|tmp;
				 case 1:
					 return 0x20|tmp;
				 case 2:
					 return 0x30|tmp;
				 case 3:
					 return 0x40|tmp;
				 default:
						return 0xff;
			 }
	}
 else if(type==1)
	{
		 return n;
	}
}

u8 changeJinGuiAddr(u8 n,u8 type)
{
	  u8 tmp1=0;
	if(type==0||type==2)
	{
    tmp1=n&0xf0;
	   if(tmp1==0x10)
	   {
			 tmp1=0;
		 }
		 else if(tmp1==0x20)
		 {
			 tmp1=12;
		 }
		 else if(tmp1==0x30)
		 {
			 tmp1=24;
		 }
	   else if(tmp1==0x40)
		 {
			 tmp1=36;
		 }
		 else
		 {
			 tmp1=48;
		 }
		 
		
		 
		 tmp1=tmp1+(n&0x0f);
	 }
	 else if(type==1)
	 {
		 tmp1=n;
	 }
		 
		  if(tmp1>TOTAL_USER) return 0;
		 
		  return tmp1;
	   
}







void GetJinGuiDataCmd(u8 *sendbuff,u8 addr,u8 cmd,u16 r_addr1,u16 numb)   //智能空开 地址   命令    初始寄存器地址   读寄存器个数
{

	
	sendbuff[0]=AddrChange(addr, sysPara[SWITCH_FACT]);
	sendbuff[1]=cmd;
	sendbuff[2]=r_addr1>>8;
	sendbuff[3]=r_addr1&0xff;
	sendbuff[4]=numb>>8;
	sendbuff[5]=numb&0xff;

}

u8 SetJinGuiDataCmd(u8 *sendbuff,u8 addr,u8 cmd,u16 r_addr1,u16 numb,u8 isdataflag)   //智能空开 地址   命令    初始寄存器地址   读寄存器个数
{
  u8 len=8;//获取数据时，请求长度为8个字节
	
	sendbuff[0]=AddrChange(addr, sysPara[SWITCH_FACT]);//地址
	sendbuff[1]=cmd;//功能码
	sendbuff[2]=r_addr1>>8;//起始寄存器地址高字节
	sendbuff[3]=r_addr1&0xff;//起始寄存器地址低字节
	sendbuff[4]=numb>>8;//寄存器个数高字节
	sendbuff[5]=numb&0xff;//寄存器个数低字节
	if(isdataflag==1)
	{
		len=11;//设置分合闸时，请求长度为11个字节
		sendbuff[6]=2;//数据字节数
		sendbuff[7]=0x00;//数据,高字节在前
	 if(gSwitchData[addr-1].st_statusflag.bit_setBreak==1)	
		sendbuff[8]=0x33;//数据,低字节在前 分闸
	 else
		sendbuff[8]=0xAA;//数据,低字节在前 合闸
	}
	return len;
}
	

u8 JinGuiSendCmdData( u8 addr,u8 *sendbuff)
{
	u8 len;
	
			if( sysPara[SWITCH_FACT]==0||sysPara[SWITCH_FACT]==2)
			{
				 if(gSwitchData[addr-1].CmdStatus==0)
				 {
					 if(gSwitchData[addr-1].st_statusflag.bit_setCmd==0)
					 {
						 if(sysPara[SWITCH_FACT]==2)	//将参数值赋值给常量值，下电板不会响应授权状态查询
						 {
							 gSwitchData[addr-1].authorizeflag =gSwitchPara[ addr-1].authorizeflag;
						 }
						 
						 if(gSwitchData[addr-1].readAuthorize==0)
						 {
							 gSwitchData[addr-1].readAuthorize=1;
							 len=SetJinGuiDataCmd(sendbuff,addr,0x16,0x1804,0X10,0);
							 return len;
						 }
						 else
						 {
								Cmd03[0]=0x30;
								Cmd03[1]=0x22;
								gSwitchData[addr-1].readAuthorize=0;
								len=SetJinGuiDataCmd(sendbuff,addr,0x03,0x3022,0X01,0);
								return len;
						 }
						 
						 
					 }
					 else
					 {
						 len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x3000,0x01,1); 
						 return len;
					 }
				 }
				 else if(gSwitchData[addr-1].CmdStatus==1)//查询过载保护
				 {
					 Cmd03[0]=0x30;
					 Cmd03[1]=0x05;
					 len=SetJinGuiDataCmd(sendbuff,addr,0x03,0x3005,1,0);
					 gSwitchData[addr-1].CmdStatus=0;
					 return len;
				 }
				 else if(gSwitchData[addr-1].CmdStatus==2)//设置过载阀值 
				 {
						gSwitchData[addr-1].CmdStatus=3;
					 
					 len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x3005,1,0);
					 sendbuff[6]=2;
					 u16 tmpdat=gSwitchPara[addr-1].st_maxoverLoadI*10;
						sendbuff[7]= tmpdat>>8;
						sendbuff[8]= tmpdat&0xff;
					 len=11;
					 
					 return len;
					 
				 }
				 else if(gSwitchData[addr-1].CmdStatus==3)//设置过载阀告警值 
				 {
						gSwitchData[addr-1].CmdStatus=0;
						len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x3002,1,0);
					 sendbuff[6]=2;
					 u16 tmpdat=gSwitchPara[addr-1].st_maxoverLoadI*10;
						sendbuff[7]= tmpdat>>8;
						sendbuff[8]= tmpdat&0xff;
					 len=11;
					 return len;
				 }
				 else if(gSwitchData[addr-1].CmdStatus==5)//设置电能
				 {
						gSwitchData[addr-1].CmdStatus=0;
						 len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x180A,1,0);
							sendbuff[6]=4;
							sendbuff[7]=*(((u8 *)&gsetTmpData)+3);
							sendbuff[8]=*(((u8 *)&gsetTmpData)+2);
							sendbuff[9]=*(((u8 *)&gsetTmpData)+1);
							sendbuff[10]=*(((u8 *)&gsetTmpData)+0);
							len=13;
					 gsetTmpData=0;
					 return len;
				 }
				 else if(gSwitchData[addr-1].CmdStatus==6)//复位告警
				 {
					 
							 len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x1825,2,0);
							 sendbuff[6]=4;
							 sendbuff[7]=0;
							 sendbuff[8]=0;
							 sendbuff[9]=0;
							 sendbuff[10]=0;
							 len=13;
					 
					 
					 
						gSwitchData[addr-1].CmdStatus=0;
						gSwitchData[addr-1].reSetAlarmFlag=0;
					 
					 return len;
				 }
				 else if(gSwitchData[addr-1].CmdStatus==7)//授权与去授权
				 {
					  gSwitchPara[ addr-1].authorizeflag =gSwitchData[addr-1].setAuthorize;//用于存储下电板授权
					 pgh52c0->SaveSwitchPara(&gSwitchPara[ addr-1].authorizeflag,1);//保存授权状态
						
					 if(sysPara[SWITCH_FACT]==2)
					 {
						 gSwitchData[addr-1].CmdStatus=0;
						 gSwitchData[addr-1].authorizeflag =gSwitchPara[ addr-1].authorizeflag;
						  gSwitchData[addr-1].setAuthorize=0xff;//清除设置
						 return 0;
					 }
						else
						{
							gSwitchData[addr-1].CmdStatus=0;
							len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x3022,1,0);
							sendbuff[6]=2;
							 sendbuff[7]= gSwitchData[addr-1].setAuthorize;
							 sendbuff[8]=0;
							len=11;
						 return len;
						}

					 
				 }
				 else if(gSwitchData[addr-1].CmdStatus==8)//设置用户（使用上周期电量寄存器储存租户信息）
				 {
					 gSwitchData[addr-1].CmdStatus=0;
						 len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x180C,1,0);
							sendbuff[6]=4;
							sendbuff[7]=*(((u8 *)&gsetTmpData)+3);
							sendbuff[8]=*(((u8 *)&gsetTmpData)+2);
							sendbuff[9]=*(((u8 *)&gsetTmpData)+1);
							sendbuff[10]=*(((u8 *)&gsetTmpData)+0);
							len=13;
					 gsetTmpData=0;
					 return len;
				 }
				 else if(gSwitchData[addr-1].CmdStatus==0xff)
				 {
						return 0;
				 }

			 }
			else if(sysPara[SWITCH_FACT]==1)
			{
				 
				 if(gSwitchData[addr-1].CmdStatus==0)
				 {
					 if(gSwitchData[addr-1].st_statusflag.bit_setCmd==0)
					 {
						 
							 gSwitchData[addr-1].readAuthorize=1;
							 len=SetJinGuiDataCmd(sendbuff,addr,0x16,0x1804,0X10,0);
							 return len;
	 
					 }
					 else
					 {
						 len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x3000,0x01,1); 
						 return len;
					 }
				 }
			 
			    else if(gSwitchData[addr-1].CmdStatus==1)//查询过载保护
				 {
					 Cmd03[0]=0x30;
					 Cmd03[1]=0x04;
					 len=SetJinGuiDataCmd(sendbuff,addr,0x03,0x3004,1,0);
					 gSwitchData[addr-1].CmdStatus=0;
					 return len;
				 }
				 else if(gSwitchData[addr-1].CmdStatus==2)//设置过载阀值 
				 {
						gSwitchData[addr-1].CmdStatus=0;
					 
					 len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x3004,1,0);
					 sendbuff[6]=2;
					 u16 tmpdat=gSwitchPara[addr-1].st_maxoverLoadI/10;
						sendbuff[7]= tmpdat>>8;
						sendbuff[8]= tmpdat&0xff;
					 len=11;
					 return len;		 
				 }
				  else if(gSwitchData[addr-1].CmdStatus==5)//设置电能
				 {
						gSwitchData[addr-1].CmdStatus=0;
						 len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x18AC,2,0);
							sendbuff[6]=4;
							sendbuff[7]=*(((u8 *)&gsetTmpData)+3);
							sendbuff[8]=*(((u8 *)&gsetTmpData)+2);
							sendbuff[9]=*(((u8 *)&gsetTmpData)+1);
							sendbuff[10]=*(((u8 *)&gsetTmpData)+0);
							len=13;
					 gsetTmpData=0;
					 return len;
				 }
				 else if(gSwitchData[addr-1].CmdStatus==7)//授权与去授权
				 {
							gSwitchData[addr-1].CmdStatus=0;
							len=SetJinGuiDataCmd(sendbuff,addr,0x10,0x3000,1,0);
							sendbuff[6]=2;
							 sendbuff[7]=0; 
					     if(gSwitchData[addr-1].setAuthorize==1)
							      sendbuff[8]=0xEE;
							  else if(gSwitchData[addr-1].setAuthorize==0)
                    sendbuff[8]=0xFF;									
							len=11;
						 return len;
					 
				 }
				 else
				 {
					  gSwitchData[addr-1].CmdStatus=0;
					  	return 0;
				 }
			
			}


}







void JinGuiOnLineCount(void)
{
	  for(u8 i=0;i<TOTAL_USER;i++)
		{
			 if(gSwitchData[i].st_detectCount<ONLINE_COUNT_MAX)
			  gSwitchData[i].st_detectCount++;
			 else
				gSwitchData[i].st_statusflag.bit_online=0;//设置在线标志为不在线
			 

			  if(SwitchOnlineCount[i]>1)
				{
					  SwitchOnlineCount[i]--;  //SwitchOnlineCount==0 不存在，SwitchOnlineCount==1已离线（开关移除），SwitchOnlineCount>1在线
				  
				}
				else if(SwitchOnlineCount[i]==1)
				{
					gDCdistribution.pst_switchtype[i]=0;//将开关类型设为未定义
					gDCdistribution.pst_I[i]=0;//当开关处于离线状态时，清除电流信息
					gDCdistribution.pst_status[i]=0;//当开关处于离线状态时，清除告警信息
				}
				

		}
   
		  if(usart3ConnetTimeOut>0)
				usart3ConnetTimeOut--;



}


void JinGuiOutLineCheckCount(void)
{
	for(u8 i=0;i<TOTAL_USER;i++)
	{
	   if(gSwitchData[i].st_detectCount==ONLINE_COUNT_MAX)
		 {
			   if(gSwitchData[i].st_delayCheckCount<DELAY_CHECK_MAX)
			   { gSwitchData[i].st_delayCheckCount++;}
				 else
				 {gSwitchData[i].st_statusflag.bit_CheckOut=1; }
		 }
	}
}



u8 JinGuiOnTick(void)
{
	static u8 addrNum=0;
	  u8 count=0,result=0xFF ;
	
	if(gcmdAddr==0)
	{
	   while(count<TOTAL_USER+1)
		 {    
			   count++;
			   addrNum++;
			   if(addrNum>TOTAL_USER+1) 
					 addrNum=1;
			 if(addrNum<=TOTAL_USER)
			 {
			   if(gSwitchData[addrNum-1].st_detectCount<ONLINE_COUNT_MAX)//未超时，发请当前地址请求
				 {
					 result=addrNum;
					 break;	 
				 }				 
			   if(gSwitchData[addrNum-1].st_statusflag.bit_CheckOut==1)//延时检测到时，发请当前地址请求
				 {
					 gSwitchData[addrNum-1].st_statusflag.bit_CheckOut=0;
					 gSwitchData[addrNum-1].st_delayCheckCount=0;
					 result=addrNum;
					 break;
				 }
			 }
			 else
			 {
				  result=addrNum;
				  break;
			 }
			 
		 }
	 }
	 else if(gcmdAddr<=TOTAL_USER)//设置发送突发命令
	 {
		   result=gcmdAddr;
		   gcmdAddr=0;
	 }

	return result;
	    

	
}



void DealWithJinGuiData(void *databuf )
{
	u8 *pdat=(u8*)databuf;
	u8 addr=0;
	
	if(pdat[0]<0x6C)//智能空开(包括京硅或泰永开关地址)
	{
	  addr=changeJinGuiAddr(pdat[0], sysPara[SWITCH_FACT]);
		
		 if(addr==0) return ;//地址无效不处理
		 addr=addr-1;
		 gSwitchData[addr].st_detectCount=0;//清在线超时计数
		 //gSwitchData[addr].st_onlineCount=0;
		 gSwitchData[addr].st_statusflag.bit_online=1;//设置在线标志
		 gSwitchData[addr].st_statusflag.bit_CheckOut=0;//清除超时标志
		 SwitchOnlineCount[addr]=ONLINE_COUNT_MAX;
		
				if(pdat[1]==0x16)
			{
			
/////////////////////////////////////////协议差异部分//////////////////////////////////////			
				
												if( sysPara[SWITCH_FACT]==0	||sysPara[SWITCH_FACT]==2)//京硅
												{
																
													
													       u32 i_tmp=pdat[7]<<24|pdat[8]<<16|pdat[9]<<8|pdat[10];
											        	gDCdistribution.pst_I[addr]=(s16)i_tmp/10;
											       	if((i_tmp&0x80000000)!=0) gDCdistribution.pst_I[addr]= gDCdistribution.pst_I[addr]*(-1);
													
													     gDCdistribution.pst_enerqy[addr]=pdat[15]<<24|pdat[16]<<16|pdat[17]<<8|pdat[18];//本期(当月)电量
													
															if(
																	(
																	pdat[22]==0x11|pdat[22]==0x12|pdat[22]==0x23|pdat[22]==0x24|pdat[22]==0x35|
																	 pdat[22]==0x36|pdat[22]==0x47|pdat[22]==0x48|pdat[22]==0x69|pdat[22]==0x5A//|pdat[22]==0x7B
																	)&&(gSwitchPara[ addr].st_userGroud!=0x7B)
															  )
																{
																		gSwitchPara[ addr].st_userGroud=pdat[22];
																}
															/*	
															if(gbattGroud<6)
															 {
																	 if(gSwitchPara[addr].st_userGroud==0x7B)//分路电池
																	 {
																		 u8 isOnly=1;
																				for(u8 i=0;i<6;i++)//检查电路分路对应下电分路的唯一性
																			 {
																				 if(gUseToBattBranch[i]==addr)//下电分路在电池分路中已存在
																					 isOnly=0;//不唯一
																			 }
																			if(isOnly==1)
																			{
																				
																				gUseToBattBranch[gbattGroud]=addr;//设置电池分路
																				pgBattBranch[gbattGroud]=&gDCdistribution.pst_I[addr];//绑定对应电流分路地址
																				gbattGroud++;
																			}
																			
																			
																			gSwitchPara[addr].st_downV=gSwitchPara[TOTAL_USER+7].st_downV;//设该分路下电电压设为电池下电电压
																			
																	 }
															 }
															 */
															 u16 tmpV=0;
																	tmpV=pdat[19]<<8|pdat[20];
															 
															 if((tmpV>=4000)&&(tmpV<=5300)&&(gSwitchPara[addr].st_userGroud!=0x7B)) //满足电压条件且为非电池分路
															 {
																	 gSwitchPara[addr].st_downV=tmpV;
															 }
															 
															 

																 
																 gSwitchData[addr].rateI=pdat[41];
																if( gSwitchData[addr].rateI>=80)
																	gDCdistribution.pst_switchtype[addr]=2;
																else
																	gDCdistribution.pst_switchtype[addr]=1;
																
																

																 if(pdat[38]!=0)
																 {
																			if(gSwitchData[addr].reSetAlarmFlag==0)  
																			{						
																				gSwitchData[addr].alarmDelay=30;
																				gSwitchData[addr].reSetAlarmFlag=1;
																			}
																 }
																 else
																 {
																			 gSwitchData[addr].alarmDelay=0;
																			 gSwitchData[addr].reSetAlarmFlag=0;
																 }

														}
												else if(sysPara[SWITCH_FACT]==1)//泰永
												{
																
													
													     u32 i_tmp=pdat[7]<<24|pdat[8]<<16|pdat[9]<<8|pdat[10];
											        	gDCdistribution.pst_I[addr]=(s16)i_tmp/10;
													
													    gDCdistribution.pst_enerqy[addr]=pdat[23]<<24|pdat[24]<<16|pdat[25]<<8|pdat[26];//输出电量
													
													       gSwitchData[addr].rateI=pdat[44];
																if( gSwitchData[addr].rateI>=80)
																	gDCdistribution.pst_switchtype[addr]=2;
																else
																	gDCdistribution.pst_switchtype[addr]=1;
																
													  
														
																if((pdat[36]&0x40)!=0)//读取授权状态
																{
																													
																	  gSwitchData[addr].authorizeflag=0;//未授权
																}
																else
																{
																	
																		gSwitchData[addr].authorizeflag=1;//已授权
																}
																

															 if(gSwitchData[addr].setAuthorize!=0xff)//检查授权是否成功
																{
																		if(gSwitchData[addr].setAuthorize==gSwitchData[addr].authorizeflag)//授权成功
																		{
																			gSwitchData[addr].setAuthorize=0xff;//清除设置
																		}
																		else//授权不成功
																		{
																			gSwitchData[addr].CmdStatus=7;//重发授权指令
																		}
																}

													}
/////////////////////////////////////////协议相同部分////////////////////////////////////////////////						
								         
													
												
								
								         	 if((pdat[36]&0x80)!=0)
												 {
														 gSwitchData[addr].st_statusflag.bit_fuse=0;//熔丝正常
														 gSwitchData[addr].st_statusflag.bit_break=0;//合闸状态
															gSwitchData[addr].recpowerFlag=0;	
												 }
												 else
												 {
														 gSwitchData[addr].st_statusflag.bit_fuse=1;//熔丝断
														 gSwitchData[addr].st_statusflag.bit_break=1;//合闸状态
														
												 }
												 
												 
												 if((pdat[35]&0x02)!=0)//故障保护
												 {
													 gSwitchData[addr].st_statusflag.bit_broken=1;
												 }
												 else
												 {
														gSwitchData[addr].st_statusflag.bit_broken=0;
												 }
												 
												 if((pdat[35]&0x01)!=0)//告警
												 {
													 gSwitchData[addr].st_statusflag.bit_warn=1;
												 }
												 else
												 {
														gSwitchData[addr].st_statusflag.bit_warn=0;
												 }
								
								
								
								
												 if(gSwitchData[addr].authorizeflag==0)//未授权
												 {
															gDCdistribution.pst_status[addr]=0;
												 }
												else if(gSwitchData[addr].authorizeflag==1)//已授权
												{					
														 if( gSwitchData[addr].st_statusflag.bit_stopTimeout==0)//在授权状态下产生相应告警
															{
																
																	if((gSwitchData[addr].st_statusflag.bit_fuse==1)&&(gSwitchData[addr].st_statusflag.bit_setdown==1))//设置下电状态
																	{  
																		if(gSwitchData[addr].st_statusflag.NC13==1) 
																			 gDCdistribution.pst_status[addr]|=0x01;//设置下电
																	}
																	else
																	{
																			 gDCdistribution.pst_status[addr]&=0xFE;
																	}
																

																	if((gSwitchData[addr].st_statusflag.bit_fuse==1)&&(gSwitchData[addr].st_statusflag.bit_setdown!=1))//设置熔丝状态
																	{  
																				gDCdistribution.pst_status[addr]|=0x40;//设置熔丝
																	}
																	else
																	{
																			 gDCdistribution.pst_status[addr]&=0xBF;
																	}
																	
																	// if(gSwitchData[addr].st_statusflag.overLoad==1)//过流状态
																	if((pdat[38]&0x80)||(pdat[40]&0x04))
																	{  
																		gSwitchData[addr].st_statusflag.overLoad=1;//过流状态
																				gDCdistribution.pst_status[addr]|=0x08;
																	}
																	else
																	{
																		gSwitchData[addr].st_statusflag.overLoad=0;//过流状态
																				gDCdistribution.pst_status[addr]&=0xF7;
																	}
												
															}
																if(gSwitchData[addr].st_statusflag.bit_broken==1)//||(gSwitchData[addr].st_statusflag.bit_warn==1))//开关故障
																	{
																			 gDCdistribution.pst_status[addr]|=0x80;
																	}
																	else
																	{
																			 gDCdistribution.pst_status[addr]&=0x7F;
																	}
												}
												 
												 
						
						
						
			
/////////////////////////////////////////协议相同部分////////////////////////////////////////////////		
			}
			else if(pdat[1]==0x10)//参数（下电）设置返回
			{
				gSwitchData[addr].st_statusflag.bit_setCmd=0;//收到设置正确设置应答改为查询参数
			}
			else if(pdat[1]==0x03)
			{
				if((Cmd03[0]==0x30)&&(Cmd03[1]==0x05))
				{
					gSwitchPara[addr].st_maxoverLoadI=((pdat[3]<<8)|pdat[4])/10;
				}
				if((Cmd03[0]==0x30)&&(Cmd03[1]==0x04))
				{
					gSwitchPara[addr].st_maxoverLoadI=((pdat[3]<<8)|pdat[4])*10;
				}
				else if((Cmd03[0]==0x30)&&(Cmd03[1]==0x22))//京硅授权状态返回
				{
						if(sysPara[SWITCH_FACT]==2)
						{
							gSwitchData[addr].setAuthorize=0xff;//清除设置
							return;
						}

					gSwitchData[addr].authorizeflag=pdat[3];
					
					
					if(gSwitchData[addr].setAuthorize!=0xff)//检查授权是否成功
					{
							if(gSwitchData[addr].setAuthorize==gSwitchData[addr].authorizeflag)//授权成功
							{
								gSwitchPara[ addr-1].authorizeflag=gSwitchData[addr].authorizeflag;
								gSwitchData[addr].setAuthorize=0xff;//清除设置
							}
							else//授权不成功
							{
								gSwitchData[addr].CmdStatus=7;//重发授权指令
							}
					}
					
					
					
				}
				
				Cmd03[0]=0;
				Cmd03[1]=0;
		 
	}
	
	}
	else if(pdat[0]>=0x6E)//电能表地址段
	{
		//addr=pdat[0];
		  if(pdat[1]==0x03)
			{
				  if(pgACmointor==NULL) return;
			 pgACmointor->st_energy1=((pdat[3]<<24)|   //电流
			                             (pdat[4]<<16)|
																	 (pdat[5]<<8)|
																	  pdat[6])/10;
																		
			
        pgACmointor->st_Ua=((pdat[15]<<8)|             //电压
																 pdat[16])*10;

         pgACmointor->st_Ia=((pdat[17]<<24)|   //电流
			                             (pdat[18]<<16)|
																	 (pdat[19]<<8)|
																	  pdat[20])/10;
																		
						 pgACmointor->st_Pa=((pdat[21]<<24)|   //有功功率
			                             (pdat[22]<<16)|
																	 (pdat[23]<<8)|
																	  pdat[24])/100;	
																		
              pgACmointor->st_Frq=((pdat[27]<<8)|  //频率
																    pdat[28])/10;

   
	       usart3ConnetTimeOut=60;//重置超时计数
				 pgACmointor->st_totalenergy=pgACmointor->st_energy1;
				
				
				
				
			}
		
		
		
		
		
		
	}
 
	
	
	
	
}
	

void DownOnTicK(void)
{
	  for(u8 i=0;i<TOTAL_USER;i++)
		{
			    if((gSwitchData[i].st_statusflag.bit_ready==1)
						&&(gSwitchData[i].st_statusflag.bit_time0flag==0)//处于非定时状态
					  &&(gSwitchData[i].st_statusflag.bit_time1flag==0)//处于非免责定时状态
					)
					{
						   if((gSwitchData[i].st_stopSupplyCount>0)&&((gSwitchPara[i].st_downMode&0x0F)!=3)&&(setFlag[i]==1))
								gSwitchData[i].st_stopSupplyCount--;
					}
					
					
					if(gSwitchData[i].alarmDelay>2)
					{
						   gSwitchData[i].alarmDelay--;
					}
					else if(gSwitchData[i].alarmDelay==2)
					{
						   
								   gSwitchData[i].CmdStatus=6;
							     gSwitchData[i].alarmDelay=1;
					}

					
					
					
		}
}

void DownElectricty(void)
{
	u16 currMinTime=gTimer.hour*60+gTimer.min;
	
//	   CurrTime.st_hour=gTimer.hour;
//	   CurrTime.st_min=gTimer.min;
//	   CurrTime.st_year=gTimer.year;
//	   CurrTime.st_mon=gTimer.mon;
//	   CurrTime.st_day=gTimer.day;
   
	
	for(u8 i=0;i<TOTAL_USER;i++)
	{
		
		 gSwitchData[i].stopTime0= gSwitchPara[i].startTime0+gSwitchPara[i].stopTime0;
		  if( gSwitchData[i].stopTime0>1440)
			{
				 // gSwitchData[i].stopTime0=1440-gSwitchData[i].stopTime0;
				 gSwitchData[i].stopTime0=gSwitchData[i].stopTime0-1440;
			}
			else if(gSwitchData[i].stopTime0==1440)
			{
				  gSwitchData[i].stopTime0=0;
			}
		
		   gSwitchData[i].stopTime1= gSwitchPara[i].startTime1+gSwitchPara[i].stopTime1;
		  if( gSwitchData[i].stopTime1>1440)
			{
				  //gSwitchData[i].stopTime1=1440-gSwitchData[i].stopTime1;
				   gSwitchData[i].stopTime1=gSwitchData[i].stopTime1-1440;
				
			}
			else if(gSwitchData[i].stopTime1==1440)
			{
				  gSwitchData[i].stopTime1=0;
			}
		
		
		
		if(gSwitchData[i].st_statusflag.bit_online==1)
		 {
			 
			 
			 
			 if((gSwitchData[i].authorizeflag==0)||(gSwitchData[i].authorizeflag==0xff))
			 {
				 gSwitchData[i].st_statusflag.bit_stopTimeout=1;// 未授权
				 if(sysPara[SWITCH_FACT]==2&&gSwitchData[i].st_statusflag.bit_break==0)
				 {
						gSwitchData[i].CmdStatus=0;
						gSwitchData[i].st_statusflag.bit_setBreak=1;//设置分闸
						gSwitchData[i].st_statusflag.bit_setCmd=1;
						gSwitchData[i].st_statusflag.bit_break=1;
				 }

				 continue;//结束本次开关下电操作
			 }

	    //以下操作为已授权操作	
				 //自动下电模块选择
             gSwitchData[i].st_statusflag.bit_stopTimeout=0;//授权
				 if((gSwitchPara[i].st_downMode&0x0F)==0)//强制不下电
				 {

					     gSwitchData[i].st_statusflag.bit_ready=0;
					     setFlag[i]=0;
					     gSwitchData[i].st_stopSupplyCount=0xff;
					 
				 }
					else  if((gSwitchPara[i].st_downMode&0x0F)==1)//电压下电
				 {
					     if(*((s16 *)&gpSysData[DCVOLTAGE])<gSwitchPara[i].st_downV)
							 {
								 if( gSwitchData[i].st_statusflag.bit_ready==0)
								 {
								   
								    gSwitchData[i].st_stopSupplyCount=10;
									  gSwitchData[i].st_statusflag.bit_ready=1;
									  setFlag[i]=1;
								 }
							 }
							 else if(*((s16 *)&gpSysData[DCVOLTAGE])>gSwitchPara[i].st_recoverV)
							 {
								  gSwitchData[i].st_statusflag.bit_ready=0;
							 }

					 
				 }
				 else if((gSwitchPara[i].st_downMode&0x0F)==2)//时间下电
				 {
					       if(stopPowrSupply==1)
								 {
									 if( gSwitchData[i].st_statusflag.bit_ready==0)
									 {
										    gSwitchData[i].st_stopSupplyCount=gSwitchPara[i].st_downT*60;
										    gSwitchData[i].st_statusflag.bit_ready=1;
										    setFlag[i]=1;
									 }
 
								 }
								 else
								 {
									   gSwitchData[i].st_statusflag.bit_ready=0;
								 }
					  
				 }
			   else if((gSwitchPara[i].st_downMode&0x0F)==3)//电量下电
				 {
					 
					   if(stopPowrSupply==1)
						{
					      if(gSwitchData[i].st_statusflag.bit_ready==0)
									 {
										 
										    gSwitchData[i].st_BaseEnergy=gDCdistribution.pst_enerqy[i];//gSwitchData[i].st_Energy;//从停电那该起记录起始电能
										    gSwitchData[i].st_statusflag.bit_ready=1;
										    gSwitchData[i].st_stopSupplyCount=200;
										    setFlag[i]=1;
									 }
					 
					 
					    if(gSwitchData[i].st_BaseEnergy<(gSwitchData[i].st_BaseEnergy+gSwitchPara[i].st_downEnery))
							{
					        //if(gSwitchData[i].st_Energy>=gSwitchData[i].st_BaseEnergy+gSwitchPara[i].st_downEnery)//
									if(gDCdistribution.pst_enerqy[i]>=gSwitchData[i].st_BaseEnergy+gSwitchPara[i].st_downEnery)
								 {
										  
										  if(setFlag[i]==1)
													gSwitchData[i].st_stopSupplyCount=0;
										
									}
							}
							else if(gSwitchData[i].st_BaseEnergy>=(gSwitchData[i].st_BaseEnergy+gSwitchPara[i].st_downEnery))//处理设定值溢出的情况
							{
					       // if((gSwitchData[i].st_Energy>gSwitchData[i].st_BaseEnergy+gSwitchPara[i].st_downEnery)&&(gSwitchData[i].st_Energy<gSwitchData[i].st_BaseEnergy))
									if((gDCdistribution.pst_enerqy[i]>gSwitchData[i].st_BaseEnergy+gSwitchPara[i].st_downEnery)&&(gDCdistribution.pst_enerqy[i]<gSwitchData[i].st_BaseEnergy))
								 {
										  
												if(setFlag[i]==1) 
													gSwitchData[i].st_stopSupplyCount=0;
		
									}
							}
						}
						else
						{
							    gSwitchData[i].st_statusflag.bit_ready=0;
							    //gSwitchData[i].st_stopSupplyCount=0xff;
						}
				 }
			 
		    //定时下电		 		 	 
				if(gSwitchPara[i].startTime0!=gSwitchData[i].stopTime0)//开始时间与结束时间不等时，使能定时下电
				{
					
					if(gSwitchPara[i].startTime0<gSwitchData[i].stopTime0)//不夸天
					{
						     if(gSwitchPara[i].startTime0<=currMinTime)
								 {
									    if((gSwitchPara[i].st_downMode&0x10)!=0)
									 		gSwitchData[i].st_statusflag.bit_time0flag=1;//触发下电
								 }
								 
								 if(gSwitchData[i].stopTime0<=currMinTime)
								 {
									    gSwitchData[i].st_statusflag.bit_time0flag=0;
								 }
					}
					else if(gSwitchPara[i].startTime0>gSwitchData[i].stopTime0)//夸一天
					{
						
						      if((gSwitchPara[i].startTime0<=currMinTime)||((0<=currMinTime)&&(currMinTime<gSwitchData[i].stopTime0)))
								 {
									    if((gSwitchPara[i].st_downMode&0x10)!=0)
									 		gSwitchData[i].st_statusflag.bit_time0flag=1;//触发下电
								 }
								 
								 
								 if((gSwitchData[i].stopTime0<=currMinTime)&&(currMinTime<gSwitchPara[i].startTime0))
								 {
									    gSwitchData[i].st_statusflag.bit_time0flag=0;
								 }	
						
					}
					
					
					
				}
				else
				{
					  gSwitchData[i].st_statusflag.bit_time0flag=0;
				}
				
				
				//免责下电	
					if(gSwitchPara[i].startTime1!=gSwitchData[i].stopTime1)//开始时间与结束时间不等时，使能免责下电
				{
				
						 if(stopPowrSupply==1)
						{
									
									
								if(gSwitchPara[i].startTime1<gSwitchData[i].stopTime1)//不夸天
								{
											 if(gSwitchPara[i].startTime1<=currMinTime)
											 {
														if((gSwitchPara[i].st_downMode&0x20)!=0)
														gSwitchData[i].st_statusflag.bit_time1flag=1;//触发下电
											 }
											 
											 if(gSwitchData[i].stopTime1<=currMinTime)
											 {
														gSwitchData[i].st_statusflag.bit_time1flag=0;
											 }
								}
								else if(gSwitchPara[i].startTime1>gSwitchData[i].stopTime1)//夸一天
								{
									
												if((gSwitchPara[i].startTime1<=currMinTime)||((0<=currMinTime)&&(currMinTime<gSwitchData[i].stopTime1)))
											 {
														if((gSwitchPara[i].st_downMode&0x20)!=0)
														gSwitchData[i].st_statusflag.bit_time1flag=1;//触发下电
											 }
											 
											 
											 if((gSwitchData[i].stopTime1<=currMinTime)&&(currMinTime<gSwitchPara[i].startTime1))
											 {
														gSwitchData[i].st_statusflag.bit_time1flag=0;
											 }	
									
								}
						}
						else
						{
							gSwitchData[i].st_statusflag.bit_time1flag=0;
						}
				 }
				else
				{
					   gSwitchData[i].st_statusflag.bit_time1flag=0;
				}
				
				
				
				
			
				 
				 
				 
				 
				 
				 
				 
				 
		//下电动作	 
		if(gSwitchData[i].st_statusflag.bit_break==0)
			{
				 if(gSwitchData[i].st_stopSupplyCount==0)//下电超时到触发 电压,时间,电量
				    {
					   
								 gSwitchData[i].st_statusflag.bit_setBreak=1;//设置分闸
							   gSwitchData[i].st_statusflag.bit_setCmd=1;
					     //   gSwitchData[i].st_statusflag.setCmdAsk=1;
					        gSwitchData[i].st_statusflag.bit_break=1;
								  gSwitchData[i].st_statusflag.NC13=1;//标记下电是由电压，时间，电量
								  gSwitchData[i].st_statusflag.bit_setdown=1;
					       if( gSwitchData[i].st_stopSupplyCount==0)
 							    gSwitchData[i].st_stopSupplyCount=0xff;
								  setFlag[i]=0;


						}
						
						else if((gSwitchData[i].st_statusflag.bit_time0flag==1)||(gSwitchData[i].st_statusflag.bit_time1flag==1))//定时下电,免责下电触发
						{
							  gSwitchData[i].st_statusflag.bit_setBreak=1;//设置分闸
							  gSwitchData[i].st_statusflag.bit_setCmd=1;
					    //  gSwitchData[i].st_statusflag.setCmdAsk=1;
					      gSwitchData[i].st_statusflag.bit_break=1;
							  gSwitchData[i].st_statusflag.bit_setdown=1;
						}
									
			}
	 else if(gSwitchData[i].st_statusflag.bit_break==1)	//处于下电状态
			{
				if((gSwitchData[i].st_statusflag.overLoad==0 || (gSwitchData[i].st_statusflag.overLoad==1 && sysPara[SWITCH_FACT]==2))&&(gSwitchData[i].st_statusflag.bit_broken==0||(gSwitchData[i].st_statusflag.bit_broken==1 &&gSwitchData[i].recpowerFlag==1)))//&&(gSwitchData[i].st_statusflag.bit_warn==0)
				{
				 if(
					                                            (((gSwitchPara[i].st_downMode&0x0F)==1)&&//在电压下电情况下，电压高于恢复电压可恢复  
				                                              (*((s16 *)&gpSysData[DCVOLTAGE])>gSwitchPara[i].st_recoverV)&&
				                                              (gSwitchData[i].st_statusflag.bit_time0flag==0)&&//定时
				                                              (gSwitchData[i].st_statusflag.bit_time1flag==0)//免责
			                                                )||
				                                              (((gSwitchPara[i].st_downMode&0x0F)!=1)&&//非电压下电情况下，有时电才可恢复，高于恢复电压可恢复 
				                                              (*((s16 *)&gpSysData[DCVOLTAGE])>gSwitchPara[i].st_recoverV)&&
				                                              (gSwitchData[i].st_statusflag.bit_time0flag==0)&&//定时
				                                              (gSwitchData[i].st_statusflag.bit_time1flag==0)&&//免责
				                                              (stopPowrSupply==0)
			                                                )||//在非电压，时间，电量下电情况下，定时免责可恢复
				                                              ((gSwitchData[i].st_statusflag.NC13==0)&&
				                                                (gSwitchData[i].st_statusflag.bit_time0flag==0)&&//定时
				                                              (gSwitchData[i].st_statusflag.bit_time1flag==0))//免责
																											||(gSwitchData[i].st_statusflag.bit_broken==1 &&gSwitchData[i].recpowerFlag==1)
				 
					 )
				   {
					 
								 gSwitchData[i].recpowerFlag=0;
								 gSwitchData[i].st_statusflag.bit_setBreak=0;//设置合闸
							   gSwitchData[i].st_statusflag.bit_setCmd=1;
					    //   gSwitchData[i].st_statusflag.setCmdAsk=1;
					       gSwitchData[i].st_statusflag.bit_break=0;
					       gSwitchData[i].st_statusflag.NC13=0;
					       //stopPowrSupply=0;//市电 恢复
					       gSwitchData[i].st_statusflag.bit_setdown=0;
						 
								
						}
				}
			}
				 
				 
			 
			 
			 
		 }
	}
}



