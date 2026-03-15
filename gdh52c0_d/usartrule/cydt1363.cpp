#include "cydt1363.h"
#include "Staggeredpeak.h"


CYDT1363 com1363;


extern u16 Yout[7];
extern u8 galram;
extern u32 gTotalOcrEnergy;
extern u8 battbroadfuse[2];
extern u8 setPoint;
extern u16 gDcpower;
extern u32 gTotalOilEnergy;
extern u8 battIndex;
CYDT1363::CYDT1363(void)
{
}

CYDT1363::~CYDT1363(void)
{
}
/***********************************************************/
//Ascii码转16进制数
//op:接收数据缓冲区
//pdat:待转换数据缓冲区
//返回:0转换失败,1转换成功
/***********************************************************/
u8 CYDT1363::AsciitoHex(u8 *op,u8* pdat)//Ascii码转16进制数
{ 
	u8 i=2,tmp;
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

/***********************************************************/
//Ascii码转半字节十六进制数
//op:接收数据缓冲区
//pdat:待转换数据缓冲区
//返回:0转换失败,1转换成功
/***********************************************************/
u8 CYDT1363::AsciitoHexOneByte(u8 *op,u8* pdat)//Ascii码转半字节十六进制数
{
	   if(*pdat<0x30||*pdat>0x46) return 0;
	      if( *pdat<0x40)
			{
				  *op=*pdat-0x30;
				
			}
			if(*pdat>0x40)
			{
				*op=*pdat-0x37;
			}
			return 1;
}


/***********************************************************/
//Ascii码转两字节16进制数
//op:接收数据缓冲区
//pdat:待转换数据缓冲区
//返回:0转换失败,1转换成功
/***********************************************************/
u8 CYDT1363::AsciitoHex16(u16 *op,u8* pdat)//16位ASCii码转为16进制码
{ 
	  u8 i=4,tmp;
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
	   *op=result;
	  return 1;
}

/***********************************************************/
//Ascii码转4字节浮点数（小端模式）
//op32:接收数据缓冲区
//pdat:待转换数据缓冲区
//返回:0转换失败,1转换成功
/***********************************************************/
u8 CYDT1363::AsciitoHex32L(u32 *op32 ,u8* pdat)//Ascii码转4字节浮点数（小端模式）
{
	  if(*pdat<0x30||*pdat>0x46) return 0;
	    if(*(pdat+1)<0x30||*(pdat+1)>0x46) return 0;
	      if(*(pdat+2)<0x30||*(pdat+2)>0x46) return 0;
	         if(*(pdat+3)<0x30||*(pdat+3)>0x46) return 0;
	if(*(pdat+4)<0x30||*(pdat+4)>0x46) return 0;
	    if(*(pdat+5)<0x30||*(pdat+5)>0x46) return 0;
	      if(*(pdat+6)<0x30||*(pdat+6)>0x46) return 0;
	         if(*(pdat+7)<0x30||*(pdat+7)>0x46) return 0;
	
	u32 tmp32=0;
	     
	    AsciitoHex(((u8*)&tmp32+3), &pdat[6]);
	    AsciitoHex(((u8*)&tmp32+2), &pdat[4]);
	    AsciitoHex(((u8*)&tmp32+1), &pdat[2]);
	    AsciitoHex(((u8*)&tmp32+0), &pdat[0]);
	
	   *op32=tmp32;
	
	   return 1;
	
}




/***********************************************************/
//16进制数码转两字节Ascii码
//op:接收数据缓冲区
//pdat:待转换数据缓冲区
//返回:无
//注：转换结果为两字节，注意偏移量为2
/***********************************************************/
void  CYDT1363::HextoAscii(u8 *op,u8 dat )//16进制码A转为ASCii码
{ u8 i=2,tmp;
	
	
		 while(i--)
		{
			 tmp=(dat>>(4*i))&0x0f;
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
	
	     
	   
}
/***********************************************************/
//设置单字节数据到1363发送缓冲区
//dat:待转换数据
//def:默认取1,正常发送,非1发送0x20
//返回:无
//注：转换结果为两字节，注意偏移量为2
/***********************************************************/
void CYDT1363::SetOneByteData(u8 dat,u8 def)//设置单字节数据
{
	if(def==1)
	{
	   HextoAscii(&m_senddata[13+m_sdatlen], dat);
	}
	else
	{
		m_senddata[13+m_sdatlen]=0x20;
		m_senddata[13+m_sdatlen+1]=0x20;
	}
	   m_sdatlen+=2;
}
/***********************************************************/
//设置双字节数据到1363发送缓冲区（大端模式）
//dat:待转换数据
//def:默认取1,正常发送,非1发送0x20
//返回:无
//注：转换结果为两字节，注意偏移量为4
/***********************************************************/
void CYDT1363::SetTowByeteData(u16 dat,u8 def)
{
	
	
	    SetOneByteData(*((u8 *)&dat+1),def);
	    SetOneByteData(*((u8 *)&dat),def);
	
}
/***********************************************************/
//设置双字节数据到1363发送缓冲区（小端模式）
//dat:待转换数据
//def:默认取1,正常发送,非1发送0x20
//返回:无
//注：转换结果为两字节，注意偏移量为4
/***********************************************************/
void CYDT1363::SetTowByeteDataL(u16 dat,u8 def)
{
	
	
	    SetOneByteData(*((u8 *)&dat),def);
	    SetOneByteData(*((u8 *)&dat+1),def);
	
}
/***********************************************************/
//设置4字节数据(浮点数)到1363发送缓冲区
//dat:待转换数据（浮点数）
//def:默认取1,正常发送,非1发送0x20
//返回:无
//注：转换结果为8字节，注意偏移量为8
/***********************************************************/
void CYDT1363::SetFourByteData(float dat,u8 def)//设置4字节数据（浮点数）
{
	   
	  for(u8 j=0;j<4;j++)	
		{
			SetOneByteData(*((u8 *)&dat+j),def);
			
		}


}
void CYDT1363::SetFourByteU32Data(u32 dat,u8 def)//设置4字节数据 大端发送
{
	   
	
//	for(u8 j=3;j>=0;j--)	
//		{
//			SetOneByteData(*((u8 *)&dat+j),def);
//			
//		}
	
	 for(u8 j=0;j<4;j++)	
		{
			SetOneByteData(*((u8 *)&dat+(3-j)),def);
			
		}


}

/***********************************************************/
//设置开关量单字节到1363发送缓冲区
//dat:为logic非零时的发送值，
//logic：为非零时，发送值为dat,为0时发送值为0
//def:默认取1,正常发送,非1发送0x20
//注：转换结果为2字节，注意偏移量为2
/***********************************************************/
void CYDT1363::SetOneByteSwitchStatus(u8 dat,u32 logic,u8 def)//设置开关量单字节到1363发送缓冲区
{
	   if(def==1)
		 {
			 if(logic !=0 )
			 {
					 HextoAscii(&m_senddata[13+m_sdatlen], dat);
			 }
			 else
			 {
					 HextoAscii(&m_senddata[13+m_sdatlen], 0);
			 }
		 }
		 else
		 {
			  m_senddata[13+m_sdatlen]=0x20;
		    m_senddata[13+m_sdatlen+1]=0x20;
		 }
		 
		  m_sdatlen+=2;
}


/***********************************************************/
//无效响应返回
//rtn:异常的rtn值
/***********************************************************/
void CYDT1363::ReturnVailData(u8 rtn)//无效响应
{
	  m_rtn=rtn;
	  SetDataLen(m_sdatlen);
	  Set1363Packet();
	
}



/***********************************************************/
//获取数据长度
//op:接收数据缓冲区
//pdat:待转换数据缓冲区
//返回:0获取失败,1获取成功
/************************************************************/
u8   CYDT1363::Getdatalen(u16 *op,u8 *pdat)//获取数据长度
{
	
	u8 lensum[4];
	   //lensum=pdat[9];
	   AsciitoHexOneByte(&lensum[0],&pdat[0]);
	   AsciitoHexOneByte(&lensum[1],&pdat[1]);
     AsciitoHexOneByte(&lensum[2],&pdat[2]);
	   AsciitoHexOneByte(&lensum[3],&pdat[3]);
	
	   if(((lensum[0]+lensum[1]+lensum[2]+lensum[3])&0x0F)!=0) return 0;
	   
	   u16 result=0;
     AsciitoHex16(&result,&pdat[0]);
	
	    *op=result&0x0FFF;
		 
	     return 1;
}


/***********************************************************/
//设置数据长度到1363发送缓冲区
//datlen:待设置长度
//返回:无
/************************************************************/
void CYDT1363::SetDataLen(u16 datlen)//设置数据长度到1363发送缓冲区
{
	u8 L1=0,L2=0,L3=0,LCHKSUM=0,tmp=0;
	u16 tmpdat=0;
	
	 L1 =    (u8)(datlen>>8)&0x0F;    
	 L2 =  (((u8)datlen) &0xF0)>>4;
   L3 =    (u8)datlen &0x0F;
	 LCHKSUM =  ~((L1 + L2 + L3)%16) +1  ;
	 LCHKSUM<<=4;
  tmpdat=(LCHKSUM<<8)| datlen;
	
	   tmp=(tmpdat&0xff00)>>8;
	   HextoAscii(&m_senddata[9],tmp);//Len1
		 tmp=(u8)tmpdat;
	   HextoAscii(&m_senddata[11],tmp);//Len2
	
	  
	
}


/***********************************************************/
//设置发送缓冲区指针 设置版本号
//pdat:发送缓冲区指针
/************************************************************/
void CYDT1363::Init(u8 * pdat)//初始化
{
	m_senddata=pdat;
	m_dataflag=0x11;
	m_MyVer=0x22;
	m_BigVer=0;
	m_warnCount[0]=0;
	m_warnCount[1]=0;
}
/***********************************************************/
//设置版本号
//pdat:0 H52c0_F,1 H52C0_F0 2 TA2.0(铁塔2.0）
//返回：无
/************************************************************/
void CYDT1363::SetVersion(u8 dat)//设置协议版本
{
	
	    if(dat==0)
			{
				m_MyVer=0x22;
				m_BigVer=0;
			}
			else if(dat==1)
			{
				m_MyVer=0x22;
				m_BigVer=1;
			}
			else if(dat==2)
			{
				m_MyVer=0x12;
				m_BigVer=2;
			}
			else if(dat==3)
			{
					m_MyVer=0x10;
				  m_BigVer=3;
			}
			else if(dat==4)
			{
					m_MyVer=0x12;
				  m_BigVer=4;
			}
}



/***********************************************************/
//最终打包1363数据
//返回：无
/************************************************************/
void CYDT1363::Set1363Packet(void)//最终打包1363数据
{
	u16 tmp_sum=0;
	
	  m_senddata[0]=0x7E;
	  HextoAscii(&m_senddata[1],m_MyVer);
	  HextoAscii(&m_senddata[3],monitor_addr);
	  HextoAscii(&m_senddata[5],m_cid1);
	  HextoAscii(&m_senddata[7],m_rtn);
	 
	
	  for(u16 i=1;i<m_sdatlen+13;i++)
		{
				tmp_sum+=m_senddata[i];
		}
		
		
		 tmp_sum = ~(tmp_sum%65536)+1;
		
		u8 tmp=(tmp_sum&0xff00)>>8;
	   HextoAscii(&m_senddata[m_sdatlen+13],tmp);//Len1
		 tmp=(u8)tmp_sum;
	   HextoAscii(&m_senddata[m_sdatlen+15],tmp);//Len2
		
		  m_senddata[m_sdatlen+17]=0x0D;
		  m_senddata[m_sdatlen+18]='\0';
		
		
		
}



/***********************************************************/
//获得1363请求
//pdat:待转换数据缓冲区
//返回:0获取失败（长度错误,校验错误）,
//1获取成功（取得 CID1，CID2,版本号,设置参数指针等信息）
/************************************************************/
u8   CYDT1363::Get1363Data(u8 *pdat)//获得1363设求
{  
                             
	      m_senddata[0]='\0';//清空发送内容
	      m_sdatlen=0;   //清空发送数数据长度
	      m_rtn=0;//复位rtn
	
	      //取得基本数据信息
	      AsciitoHex(&m_cid2,&pdat[7]);//CID2
	      AsciitoHex(&m_cid1,&pdat[5]);//CID1
	      AsciitoHex(&m_addr,&pdat[3]);//地址
		    AsciitoHex(&m_ver, &pdat[1]);//版本号
	
	      if(Getdatalen(&m_datlen,&pdat[9])==0)//验证数据长度是否正常
				{
					   m_rtn=3;//长度错误
					   SetDataLen(m_sdatlen);
					   Set1363Packet();
					   return 0;
				}
				
				
		  u16	tmp_sum= 0;
				
				AsciitoHex16(&tmp_sum,&pdat[m_datlen+13]);//获取数据校验和
				
				for(u16 i=1;i<m_datlen+13;i++) //计算数据校验和
				{
						  tmp_sum+=pdat[i];
					}
				
				if(tmp_sum!=0)//验证校验和是否正确
				{
						m_rtn=2;//校验和错误
						SetDataLen(m_sdatlen);
						Set1363Packet();
						return 0;
				}
				 
///////////////////////////获取接收数据////////////////////////////////				
				
	      m_recvdata=&pdat[13];
				
				return 1;

}


/***********************************************************/
//1363协议打包处理
//返回：无
/************************************************************/
void CYDT1363::DuleWith1363Data(void)//1363协议打包处理
{
	  if(m_cid2==0x4f)//请求版本号
		{
		
			m_rtn=0;
			SetDataLen(m_sdatlen);
			Set1363Packet();
		
			
		}
		else if(m_cid2==0x50)//请求设备地址
		{
			 m_rtn=0;
			 SetDataLen(m_sdatlen);
			 Set1363Packet();
			 
		}
		else if(m_ver==0x12)//检查版本号
		{//铁塔白盒化指令
			 m_MyVer = 0x12;
			if(m_addr==monitor_addr)//检查通信地址 若通信地址不正确时不返回数据
			{
			
							if(m_cid2==0x41)    //模拟量指令
							{
									 if(m_cid1==0x40)
									 {      
											 set4041();//交流瓶
									 }
									 else if(m_cid1==0x41)
									 {
											 set4141();//整流模块
									 }
									 else if(m_cid1==0x42)
									 {
											 set4241();//直流瓶
									 }
									 else if(m_cid1==0x43)
									 {
											 set4341();//光伏模块

									 }
									 else if(m_cid1==0xE2)
									 { 
										 setE241();//配电分路
									 }
									 else if(m_cid1==0x4A)
									 { 
										 set4A41();//锂电
									 }
									 else if(m_cid1==0xE1)
									 {
											 setE141();
									 }
									  else if(m_cid1==0xE5)
									 { 
										 setE541();//锂电
									 }
							     else//无效CID2
									 {
											 m_rtn=4;
											 SetDataLen(m_sdatlen);
											 Set1363Packet();
									}
							}
							else if(m_cid2==0x43)//状态指令
							{
									 if(m_cid1==0x40)
									 {   
										 set4043();//交流瓶
									 }
									 else if(m_cid1==0x41)
									 {
										 set4143();//整流模块
									 }
									  else if(m_cid1==0x43)
									 {
										 set4343();//整流模块
									 }
									 else if(m_cid1==0x4A)
									 {
										 set4A43();//锂电模块
									 }
								    else if(m_cid1==0xE5)
									 { 
										 setE543();//锂电
									 }
									 else//无效CID2
									 {
											 m_rtn=4;
											 SetDataLen(m_sdatlen);
											 Set1363Packet();
									 } 
							}
							else if(m_cid2==0x44)//告警指令
							{
									if(m_cid1==0x40)
									{
											set4044();//交流瓶
									}
									else if(m_cid1==0x41)
									{
											set4144();//整流模块
									}
									else if(m_cid1==0x42)
									{
											set4244();//直流瓶
									 }
									else if(m_cid1==0x43)
									{
										set4344();//光伏模块
									}
									else if(m_cid1==0xE1)
									{ 
										setE144();//系统
									 }
									else if(m_cid1==0xE2)
									{ 
										setE244();//配电分路
									}
									else if(m_cid1==0x4A)
									{ 
										set4A44();//锂电池
									}
									 else if(m_cid1==0xE5)
									 { 
										 setE544();//锂电
									 }
									else//无效CID2
									{
											 m_rtn=4;
											 SetDataLen(m_sdatlen);
											 Set1363Packet();
									}
							}
							else if(m_cid2==0x45)//遥控指令
							{
									if(m_cid1==0x41)
									{
										set4145();//整流模块
									 }
									else if(m_cid1==0x43)
									{
										set4345();//直流屏
									 }
									 else//无效CID2
									{
											 m_rtn=4;
											 SetDataLen(m_sdatlen);
											 Set1363Packet();
									}
									
							}
							else if(m_cid2==0x46)//获取参数
							{
									 if(m_cid1==0x40)
									 {
										 set4046();//交流屏
//                                                                 }else if(m_cid1==0x41){  set4146();//整流模块
									 }else if(m_cid1==0x42)
									 {
										 set4246();//直流瓶
//                                                                 }else if(m_cid1==0xE1){  setE146();//系统
									 }
									 else if(m_cid1==0xE2)
									 {
										 setE246();//配电分路
										 
									 }
									  else if(m_cid1==0x4A)
									 {
										 set4A46();//锂电
										 
									 }
									  else if(m_cid1==0xE5)
									 { 
										 setE546();//锂电
									 }
							     else//无效CID2
										{
												 m_rtn=4;
												 SetDataLen(m_sdatlen);
												 Set1363Packet();
										}
							}
							else if(m_cid2==0x48)//设置参数
							{
									 if(m_cid1==0x40)
									 {
										 set4048();//交流瓶
//                                                                 }else if(m_cid1==0x41){set4148();//整流模块
									 }
									 else if(m_cid1==0x42)
									 {
										 set4248();//直流屏
//                                                                 }else if(m_cid1==0xE1){setE148();//系统
//                                                                 }else if(m_cid1==0xE2){setE248();//直流瓶
									 }
									 else if(m_cid1==0x4A)
									 {
										 set4A48();//锂电
									 }
									 else if(m_cid1==0xE5)
									 { 
										 setE548();//锂电
									 }
							     else if(m_cid1==0xE2)
									 {
										 setE248();//本地下电与扩展下电
									 }
									 else//无效CID2
										{
												 m_rtn=4;
												 SetDataLen(m_sdatlen);
												 Set1363Packet();
										}
							}
							else if(m_cid2==0x80)
							{
									set4280();
							}
							else if(m_cid2==0xA0)
							{
									setE1A0();
							}
							else if(m_cid2==0xA1)
							{
									setE1A1();
							}
							else if(m_cid2==0xA3)
							{
									setE2A3();
							}
							else if(m_cid2==0xA4)
							{
									setE2A4();
							}
							else if(m_cid2==0xA5)
							{
									setE2A5();
							}
							
							else if(m_cid2==0xA6)
							{
									setE2A6();
							}
							else if(m_cid2==0xA7)
							{
									set42A7();
							}
							else if(m_cid2==0xA8)
							{
									set42A8();
							}
							else if(m_cid2==0xA9)
							{
									set42A9();
							}
							else if(m_cid2==0xAA)
							{
									set42AA();
							}
							else if(m_cid2==0xAB)
							{
									set42AB();
							}
							else if(m_cid2==0xAC)
							{
									setE2AC();
							}
							else if(m_cid2==0xAD)
							{
									setE2AD();
							}
							else if(m_cid2==0xAE)
							{
								   set42AE();
							}
							else if(m_cid2==0xAF)
							{
								   set42AF();
							}
							else if(m_cid2==0x4D)//获取设备时间
							{
									 setXX4DF();
							}
							else if(m_cid2==0x4E)//设置设备时间
							{
									 setXX4EF();
							}
							else if(m_cid2==0x51)//厂家信息
							{
											 setXX51F();
							}
							else if(m_cid2==0xD4)//获取告警记录
							{
									
											  setXXD4F();
									
							}
							else if(m_cid2==0xD8)//校准电流，电压
							{
									 
											  setXXD8F();
									 
							}
							else if(m_cid2==0xDD)//设备信息设置
							{
									  setXXDDF();
							}
							else//无效CID2
							{
									 m_rtn=4;
									 SetDataLen(m_sdatlen);
									 Set1363Packet();
		          }

				 
			}
			
		}
		else if(m_ver == 0x22)
		{//产线生产序列号信息
			  m_MyVer = 0x22;
			if(m_addr==monitor_addr)//检查通信地址 若通信地址不正确时不返回数据
			{
				   	  if(m_cid2==0xDD)//设备信息设置
							{
									  setXXDDF();
							}
							else//无效CID2
							{
									 m_rtn=4;
									 SetDataLen(m_sdatlen);
									 Set1363Packet();
		          }

			}
		}
		else
		{
//			 m_rtn=1;//rtn=1;版本号不对
//			  SetDataLen(m_sdatlen);
//			  Set1363Packet();
			 
		}
}



/////////////////获取模拟量量化数据///////////////////////////////////////////

void CYDT1363::set4041(void)//交流瓶获取模拟量量化数据
{
		u8 command_group=0;
				SetOneByteData(m_dataflag); 
/////////////////////////交流配电遥测数据/////////////////////////////////////////////////////
	
	
	      if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	      if(command_group==0xff)
				SetOneByteData(1);//交流屏数量 M=1
				SetOneByteData(1);//本屏交流配电数量 N=1
	      SetFourByteData((float)pgACmointor->st_Ua/100);//A相电压
	      SetFourByteData((float)pgACmointor->st_Ub/100);//B相电压
	      SetFourByteData((float)pgACmointor->st_Uc/100);//C相电压
	      SetFourByteData((float)pgACmointor->st_Frq/10);//频率
	      SetOneByteData(1);//铁塔自定义字段数量 a =1
	      SetFourByteData((float)pgACmointor->st_totalenergy/10);//市电电能
	      SetOneByteData(1);//厂家自定义字段数量 b=1
				 SetFourByteData((float)gTotalOilEnergy/1000);//油机电能
	      SetFourByteData((float)pgACmointor->st_Ia/100);//A相电流
	      SetFourByteData((float)pgACmointor->st_Ib/100);//B相电流
	      SetFourByteData((float)pgACmointor->st_Ic/100);//C相电流
	
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
				 SetDataLen(m_sdatlen);
				 Set1363Packet();
}

void CYDT1363::set4141(void)//整流模块获取模拟量量化数据
{
	//////////////////////////整流模块遥测数据////////////////////////////////////////////////////
	
	    
					u16 moduleNum=0;
					u16 tmplen=0;
	      u16 *moduleCurr=(u16 *)pModuledata->GetDataAddr(CURRENT);
	     //  u16 *moduleCurr=testI;
			    s16 *moduleTemp=((s16 *)pModuledata->GetDataAddr(TEMP));
			    u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
					u8 *prateI=((u8 *)pModuledata->GetDataAddr(MODULE_RATE_I));
	      
	       SetOneByteData(m_dataflag); 
	       SetFourByteData((float)(*((s16 *)&gpSysData[DCVOLTAGE]))/100);//直流电压
	
	       tmplen=m_sdatlen;//暂存模块数位置
	       SetOneByteData(0);//临时写入整流模块数量 M
	          
	       for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历编所有在线的整流模块
					{
						    	if((moduletype[i]==ACDC)||(moduletype[i]==0x0F))
										{
											moduleNum++;
											//SetOneByteData(i+1);//整流模块编号
											SetFourByteData((float)moduleCurr[i]/100);//输出电流
											//SetOneByteData(2);//自定义数
											SetOneByteData(1);//自定义数
											
											
											SetFourByteData((float)moduleTemp[i]/100);//模块温度
											//SetFourByteData((float)prateI[i]);//整流模块额定输出容量
											SetOneByteData(0); //厂家自定义字段数量 b = 0
										}
					}
				HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置真实整流模块数量 M
					
					///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}

void CYDT1363::set4341(void)//光伏模块获取模拟量量化数据
{
	//////////////////////////整流模块遥测数据////////////////////////////////////////////////////
	
	    
					u16 moduleNum=0;
					u16 tmplen=0;
	
	
	        u16 *moduleAC=((u16 *)pModuledata->GetDataAddr(VOLTAGE_AC));//输入电压
	        u16 *moduleACi=((u16 *)pModuledata->GetDataAddr(CURRENT_AC));//输入电流
	
	
	
	        u16 *moduleCurr=(u16 *)pModuledata->GetDataAddr(CURRENT);
			    u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
			
	      
	       SetOneByteData(m_dataflag); 
	      
	
	       tmplen=m_sdatlen;//暂存模块数位置
	       SetOneByteData(0);//临时写入整流模块数量 M
	          
	       for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历编所有在线的整流模块
					{
						    	if((moduletype[i]==DCDC)||(moduletype[i]==0x0E))
										{
											moduleNum++;
											
											SetFourByteData((float)moduleAC[i]/100);//输入电压
											SetFourByteData((float)moduleACi[i]/1000);//输入电流
										  SetFourByteData((float)(*((s16 *)&gpSysData[DCVOLTAGE]))/100);//输出电压
											SetFourByteData((float)moduleCurr[i]/100);//输出电流
											SetOneByteData(0);//铁塔自定义字段数量 a = 0		
											SetOneByteData(0); //厂家自定义字段数量 b = 0
										}
					}
				HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置真实整流模块数量 M
					
					///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}

void CYDT1363::set4241(void)//直流瓶获取模拟量量化数据
{
   	u8 command_group=0;
	///////////////////////////直流配电遥测内容/////////////////////////////////////////////////////////
									SetOneByteData(m_dataflag);


	                if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	                if(command_group==0xff)
					        SetOneByteData(1);//直流屏数量 M＝ 1表105
	
					        	SetFourByteData((float)(*((s16 *)&gpSysData[DCVOLTAGE]))/100);//直流电压//表106
	                  SetFourByteData((float)(*((s16 *)&gpSysData[USER_CURR]))/10); //负载总电流
									
									  SetFourByteData((float)(gDcpower)/1000); //太阳能总输出功率
                    SetFourByteData((float)(gTotalOcrEnergy)/1000); //太阳能累计电量
									
									
	
										if(gbattGroud==0)//非智能开关，电池分路
										 {
                         SetOneByteData(3);//电池路数
											 
											   SetFourByteData((float)totalBattI/10); //锂电电池电流
											   SetFourByteData((float)(*((s16 *)&gpSysData[BATT_CURR3]))/10); //电池1电流（分流器1 AD2）
				                 SetFourByteData((float)(*((s16 *)&gpSysData[BATT_CURR4]))/10); //电池2电流（分流器2 AD4） 
					              
				                
											 
										 }
										 else if(gbattGroud>0)//智能开关电池分路
										 { 
											 u8 tmpBattCount=0;
											 
											    tmpBattCount=gbattGroud;
											 
											    if(gbattGroud>6) tmpBattCount=6;
											 
												  SetOneByteData(tmpBattCount+1);
											 
												  SetFourByteData((float)totalBattI/10); //锂电电池电流
												 for(u8 i=0;i<6;i++)
												 {
														 if(pgBattBranch[i]!=NULL)
														 {	 
															 SetFourByteData((float)(*((s16 *)pgBattBranch[i]))/10);//分路电池电流
														 }
												 }
											 
										 }
										 
										 float tmpI=((float)(*((s16 *)&gpSysData[LOADCURR]))/10   //模块总电流
											           -(float)(*((s16 *)&gpSysData[TOTAL_BATTI]))/10  //铅酸电池电流
															   -(float)totalBattI/10                          //锂电电池电流
															   -(float)totalUserI/10);                         //总用户电流
																 
									 if(tmpI<0)tmpI=0;
									  	SetOneByteData(1);//监测直流分路数量 N
										 
										 
										  SetFourByteData(tmpI); //铁塔自定义分路（非智能开关）
										 
					            SetOneByteData(4+battGroud);//铁塔自定义字段数量 a=3+1+m m=1
										 
					            SetFourByteData((float)(*((s16 *)&gpSysData[DCVOLTAGE]))/100);//电池电压
										 
										 	float bkTime=60*(float)(*(u16*)&gpSysData[BATT_SOC])/10000;
				              bkTime=bkTime*gSetBattC;
										 if(gpSysData[USER_CURR]>0)
											 // 用户电流>0时，用容量除以电流得到时间（电流单位可能为0.1A，故除以10转换为A）
										 {bkTime=bkTime/((float)(*((s16 *)&gpSysData[USER_CURR]))/10);}
										 else 
										 {
											 bkTime=1441;
										 }
				
				              if(bkTime>1440)bkTime=1440;
				
				              SetFourByteData(bkTime);//电池后备时间
										  SetFourByteData(((float)(*(u16*)&gpSysData[BATT_SOC]))/100/*00*gSetBattC*/);//电池剩余容量Ah
					            SetFourByteData(battGroud);//电池温度数Q
										 for(u8 i=0; i<battGroud; i++)
										  SetFourByteData(((float)*((s16 *)(&gpSysData[ENV_TEMP])))/100);//电池温度
					            SetOneByteData(0);// 厂家自定义字段数量 b = 0
										 
										 ///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}
void CYDT1363::setE241(void)//租户获取模拟量量化数据
{
	u8 command_group=0;
		///////////////////////////////租户部分的遥测内容//////////////////////////////////////////////////////////////////
										
	
	
	                  
	
	                    if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	                   if(command_group==0xff)
										 {			
                        SetOneByteData(m_dataflag); 											 
												SetOneByteData(4);//租户数量 M
												for(u8 i=0;i<4;i++)
												{
													
													  SetFourByteData((float)gBattPara[i].battCurr1/10);    
												    SetOneByteData(1);//铁塔自定义字段数量
												    SetFourByteData((float)gBattPara[i].battEnergy/1000);
												    SetOneByteData(0);//厂家自定义字段数量  
													
//													 SetFourByteData((float)gDCdistribution.pst_userI[i+1]/10);//租户电流
//													 SetOneByteData(1);//铁塔自定义字段数量
//													 SetFourByteData((float)gDCdistribution.pst_userEnerqy[i+1]/100);//租户电能
//													 SetOneByteData(0);//厂家自定义字段数量 
												}
										}
										else if(command_group<=4)
										{
											  SetOneByteData(m_dataflag); 
											  SetFourByteData((float)gBattPara[command_group].battCurr1/10);    
												SetOneByteData(1);//铁塔自定义字段数量
												SetFourByteData((float)gBattPara[command_group].battEnergy/1000);
												SetOneByteData(0);//厂家自定义字段数量  
										}
										else
										{
											ReturnVailData(6);return ;
										}
											
											///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}


void CYDT1363::set4A41(void)//锂电模拟量
{
		u8 command_group=0;

	
			if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(command_group==0xff)
			{
				 	SetOneByteData(m_dataflag); 
					SetOneByteData(4);//电池数
					for(u8 i=0;i<4;i++)
					{
							SetFourByteData((float)batt[i].B_SOC/100);//SOC
							SetFourByteData((float)batt[i].Vbat/100);//电池组电压V
							SetOneByteData(16);//单体数	
							for(u8 j=0;j<16;j++)
								{
									 SetFourByteData((float)batt[i].CellV[j]/1000);//电池组电压V
								}
							SetFourByteData(batt[i].CellTempMax);//电池组温度℃
							SetOneByteData(6);//电池温度数N（N=0）
								for(u8 j=0;j<6;j++)
								{
									 SetFourByteData((float)batt[i].CellTemp[j]);//电池组电压V
								}
							SetFourByteData((float)batt[i].Ibus/100);//电池组电流
							SetFourByteData((float)batt[i].B_SOH/100);//SOH
							SetOneByteData(1);//自定义数
							SetFourByteData((float)batt[i].B_capacity/100);//电池额定容量
					}
		}
			else if((command_group>0)&&(command_group<=16))
			{
					if(batt[command_group-1].Nexttime>1)
					{
							 SetOneByteData(m_dataflag); 
							 SetFourByteData((float)batt[command_group-1].B_SOC/100);//SOC
								SetFourByteData((float)batt[command_group-1].Vbat/100);//电池组电压V
								SetOneByteData(16);//单体数
								for(u8 j=0;j<16;j++)
									{
										 SetFourByteData((float)batt[command_group-1].CellV[j]/1000);//电池组电压V
									}
								SetFourByteData(batt[command_group-1].CellTempMax);//电池组温度℃
								SetOneByteData(6);//电池温度数N（N=0）
									for(u8 j=0;j<6;j++)
									{
										 SetFourByteData((float)batt[command_group-1].CellTemp[j]);//电池组电压V
									}
								SetFourByteData((float)batt[command_group-1].Ibus/100);//电池组电流
								SetFourByteData((float)batt[command_group-1].B_SOH/100);//SOH
								SetOneByteData(1);//自定义数
								SetFourByteData((float)batt[command_group-1].B_capacity/100);//电池额定容量
					}
					else
					{
						ReturnVailData(0xE6);return ;
					}
			}
			else
			{
				ReturnVailData(6);return ;
			}
		
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	 SetDataLen(m_sdatlen);
	 Set1363Packet();
}









extern s16 mt11Tmp;
extern s16 mt11Humi;
void CYDT1363::setE141(void)
{
	SetOneByteData(m_dataflag);										   // dataflag
	SetFourByteData((float)mt11Tmp / 10);	   // 环境温度
	SetFourByteData((float)mt11Humi/10);		   // 相对湿度
	SetFourByteData((float)pModule->m_smrCount * gMaxModuleCurr / 10); // 整流模块总容量
	SetFourByteData((float)pModule->m_scrCount * gMaxModuleCurr / 10); // 光伏模块总容量
	SetFourByteData(gSetBattC);										   // 电池总容量
	SetFourByteData((float)(*((s16 *)&gpSysData[BATT_SOC])) / 100);	   // SOC
	SetFourByteData(100);											   // SOH
	SetFourByteData(0);												   // 电池连续放电时间 min
	SetFourByteData(100);											   // 市电连续供电时间 s
	SetFourByteData(0, 0x20);										   // 油机连续供电时间 min
	SetFourByteData(0, 0x20);										   // 光伏连续供电时间 min


//		SetFourByteData(0, 0x20); // 尖峰累计用电量
//		SetFourByteData(0, 0x20); // 高峰累计用电量
//		SetFourByteData(0, 0x20); // 平时累计用电量
//		SetFourByteData(0, 0x20); // 低谷累计用电量
//		SetFourByteData(0, 0x20); // 尖峰累计电费
//		SetFourByteData(0, 0x20); // 高峰累计电费
//		SetFourByteData(0, 0x20); // 平时累计电费
//		SetFourByteData(0, 0x20); // 低谷累计电费
//		SetFourByteData(0, 0x20); // 累计收益
		SetFourByteData(0, 0x20); // PUE

	SetDataLen(m_sdatlen);
	Set1363Packet();
}


void  CYDT1363::setE541(void)//空调
{
	
		u8 command_group=0;
		if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(command_group==0xff)
			{
					SetOneByteData(m_dataflag);
				  SetOneByteData(8);//空调数量
				  for(u8 i=0;i<8;i++)
					{
                  SetFourByteData((float)g_astAIRpara[i].ast_cabinetTemp/10);
						      SetFourByteData((float)g_astAIRpara[i].ast_infanSpeed);
						      SetFourByteData((float)g_astAIRpara[i].ast_exfanSpeed);
					}
				
				
			}
			else if((command_group>0)&&(command_group<=8))
			{
				          SetOneByteData(m_dataflag);          
				          SetFourByteData((float)g_astAIRpara[command_group-1].ast_cabinetTemp/10);
						      SetFourByteData((float)g_astAIRpara[command_group-1].ast_infanSpeed);
						      SetFourByteData((float)g_astAIRpara[command_group-1].ast_exfanSpeed);
				  
			}
			else
			{
				ReturnVailData(6);return ;
			}
			
				///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	 SetDataLen(m_sdatlen);
	 Set1363Packet();
			
}





/////////////////获取状态量数据///////////////////////////////////////////
void CYDT1363::setE2A3(void)
{
		u8 command_group=0;
	/////////////////////////////直流配电单元遥测内容//////////////////////////////////////////////////////////////////			E2A3
											u16 moduleNum=0;
											u16 tmplen=0;
										
											 
											moduleNum=0;
	                    s8 branchCount=0;
	
	                   if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	                   if(command_group==0xff)
										 {
											SetOneByteData(m_dataflag); 
											tmplen=m_sdatlen;//暂存直流配电单元数量位置
											SetOneByteData(0);//临时写入直流配电单元数量 M
											for(u8 i=0;i<TOTAL_USER;i++)
											{
												if(SwitchOnlineCount[i]>0)
												{
													 moduleNum++;
					
													 SetFourByteData(((float)(*((s16 *)&gDCdistribution.pst_I[i])))/10);//配电单元电流		
													 SetOneByteData(3);//铁塔自定义字段数量 a =3
													 SetFourByteData((float)gDCdistribution.pst_enerqy[i]/100);//配电单元电能
													 SetFourByteData(25);//配电单元直流配电单元机内温度
													 SetOneByteData(i+1);//配电单元序号
													 SetOneByteData(0);//厂家自定义字段数量 b
												}
										  
											
											}
											HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置配电单元数量 M
										}
										else if((command_group>0)&&(command_group<=TOTAL_USER))
										{
											    u8 i=0;
											     for( i=0;i<TOTAL_USER;i++)
											    {
														 if(SwitchOnlineCount[i]>0)
														{
																 moduleNum++;
															
															if(command_group==moduleNum)
															{
																 SetOneByteData(m_dataflag); 
																 SetFourByteData(((float)(*((s16 *)&gDCdistribution.pst_I[i])))/10);//配电单元电流		
																 SetOneByteData(3);//铁塔自定义字段数量 a =3
																 SetFourByteData((float)gDCdistribution.pst_enerqy[i]/100);//配电单元电能
																 SetFourByteData(25);//配电单元直流配电单元机内温度
																 SetOneByteData(i+1);//配电单元序号
																 SetOneByteData(0);//厂家自定义字段数量 b
																 break;
															 }
														}
															
												  }
													if(i==TOTAL_USER)
													{
														  ReturnVailData(0xE6);return ;//找不到对应分路
													}
										}
										else
										{
												ReturnVailData(6);return ;
										}	
											///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}
void CYDT1363::set4043(void)//交流瓶获取开关输入状态
{
		u8 command_group=0;
	
	SetOneByteData(m_dataflag); 
/////////////////////////交流配运行状态/////////////////////////////////////////////////////
	                    if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	                    if(command_group==0xff)
	                    SetOneByteData(1);//交流屏数量 M=1
	
				              SetOneByteData(1);//监测的输出开关数量 N
	                    SetOneByteData(2);//交流输出空开状态   未配置
											SetOneByteData(1);//铁塔自定义字段数量 a
											SetOneByteData(mod);//自定义字节（系统供电方式）
											SetOneByteData(0);//厂家自定义字段数量 b 
	
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}

void CYDT1363::set4143(void)//整流模块获取开关输入状态
{
	
	//////////////////////////整流模运行状态////////////////////////////////////////////////////	
		SetOneByteData(m_dataflag); 
	                    u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
	                    u8 *moduleonoff=((u8*)pModuledata->GetDataAddr(ONOFF));
	                    u8 * modulelimitcurr=(u8 *)pModuledata->GetDataAddr(LIMIT_CURR_STATUS);	
	
	                    u16 moduleNum=0;
	                    u16 tmplen=0;
	                     tmplen=m_sdatlen;//暂存模块数位置
	                    SetOneByteData(0);//临时写入整流模块数量 M
	          
										 for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历编所有在线的整流模块
											{
															if((moduletype[i]==ACDC)||(moduletype[i]==0x0F))
																{
																	moduleNum++;							
																	SetOneByteData(moduleonoff[i]);//关机
																	SetOneByteData(modulelimitcurr[i]);//限流	
																	
																	SetOneByteData(gcourrentWorkStatus);//充电状态

																	SetOneByteData(1);//铁塔自定义字段数量 a
																	
																	if(genableTP==1)//模块防盗		
                                  {SetOneByteData(0);}//开启
                                 else																	
																	{SetOneByteData(1);}//未开启	
											            SetOneByteData(0);//厂家自定义字段数量 b 
																}
											}
										HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置真实整流模块数量 M
	
	
		///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}

void CYDT1363::set4343(void)//整流模块获取开关输入状态
{
	
	//////////////////////////整流模运行状态////////////////////////////////////////////////////	
		SetOneByteData(m_dataflag); 
	                    u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
	                    u8 *moduleonoff=((u8*)pModuledata->GetDataAddr(ONOFF));
	                    u8 * modulelimitcurr=(u8 *)pModuledata->GetDataAddr(LIMIT_CURR_STATUS);	
	
	                    u16 moduleNum=0;
	                    u16 tmplen=0;
	                     tmplen=m_sdatlen;//暂存模块数位置
	                    SetOneByteData(0);//临时写入整流模块数量 M
	          
										 for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历编所有在线的整流模块
											{
															if((moduletype[i]==DCDC)||(moduletype[i]==0x0E))
																{
																	moduleNum++;	
																	
																	SetOneByteData(0);//工作状态	
																	SetOneByteData(moduleonoff[i]);//关机
																	
																	
																	

																	SetOneByteData(0);//铁塔自定义字段数量 a
											            SetOneByteData(0);//厂家自定义字段数量 b 
																}
											}
										HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置真实整流模块数量 M
	
	
		///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}
void CYDT1363::set4280(void)//直流屏获取输入状态
{
		u8 command_group=0;
	
	
	
	                SetOneByteData(m_dataflag);//dataflag
		 
		              if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	                if(command_group==0xff)
	                 SetOneByteData(1);//直流屏数量 M=1
/////////////////////////直流屏的状态状态/////////////////////////////////////////////////////
									 SetOneByteData(gcourrentWorkStatus);//充电状态
									 SetOneByteData(gSleepOnoff);//节能状态		
									 SetOneByteData(1);//铁塔自定义字段数量 a
									 SetOneByteData(gPeakShaveOnoff);//削峰状态
									 SetOneByteData(0);//厂家自定义字段数量 b 
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
									 SetDataLen(m_sdatlen);
									 Set1363Packet();
}



void CYDT1363::set4A43(void)//锂电模状态量
{
	
		u8 command_group=0;

	
			if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(command_group==0xff)
			{
				 	
	
					SetOneByteData(m_dataflag); 
					
					SetOneByteData(4);//电池组数
					for(u8 i=0;i<4;i++)
					{
						SetOneByteData(batt[i].B_Status);
					}
		   }
	     else if((command_group>0)&&(command_group<=16))
			{
					if(batt[command_group-1].Nexttime>1)
					{
						SetOneByteData(m_dataflag); 	
						SetOneByteData(batt[command_group-1].B_Status);
					}
					else
					{
						ReturnVailData(0xE6);return ;
					}
			}
			else
			{
				ReturnVailData(6);return ;
			}
	
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	 SetDataLen(m_sdatlen);
	 Set1363Packet();
	
	
	
	
}





void  CYDT1363::setE543(void)//空调状态量
{
	
			u8 command_group=0;

	
			if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(command_group==0xff)
			{
				 	
	
					SetOneByteData(m_dataflag); 
					
					SetOneByteData(8);//电池组数
					for(u8 i=0;i<8;i++)
					{
						SetOneByteData(g_astAIRpara[i].ast_workStatus);
						SetOneByteData(g_astAIRpara[i].ast_compressorstatus);
					}
		   }
	     else if((command_group>0)&&(command_group<=8))
			{
					
						SetOneByteData(m_dataflag); 	
				    SetOneByteData(g_astAIRpara[command_group-1].ast_workStatus);
						SetOneByteData(g_astAIRpara[command_group-1].ast_compressorstatus);
					
			}
			else
			{
				     ReturnVailData(6);return ;
			}
	
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	 SetDataLen(m_sdatlen);
	 Set1363Packet();
}


/////////////////获取告警量数据///////////////////////////////////////////
void CYDT1363::set4044(void)//交流屏获取告警量
{
	u8 command_group=0;
				SetOneByteData(m_dataflag);
	
	/////////////////////////////交流配电告警/////////////////////////////////////////////////////
											
	                    if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	                    if(command_group==0xff)
	                    SetOneByteData(1);//交流屏数量 M=1
											
				              SetOneByteData(1);//本屏交流配电数量 N
	                    SetOneByteData(acAlarmV[0]);//输入线/相电压 L1 L2/ L1
	                    SetOneByteData(acAlarmV[1]);//输入线/相电压 L2 L3/ L2
	                    SetOneByteData(acAlarmV[2]);//输入线/相电压 L3 L1/ L3
	                    SetOneByteData(acAlarmV[3]);//输入频率
	                    SetOneByteData(1);//监测熔丝/开关数量
	                    SetOneByteData(GetWarnInPut(LOAD_FUSE2,0x05));//交流输出熔丝/空开断 
	                    SetOneByteData(GetWarnInPut(SPD,0xE1));//交流防雷器异常 
	                    SetOneByteData(2);//铁塔自定义字段数量 a=2
	                    SetOneByteData(GetWarnInPut(ACINPUT,0x05));//交流输入主空开/熔丝断
	                    SetOneByteData(acAlarmV[7]);//交流停电
	                    SetOneByteData(0);//厂家自定义字段数量 b=0
	                    SetOneByteData(0);//输出电流 L1
	                    SetOneByteData(0);//输出电流 L2
	                    SetOneByteData(0);//输出电流 L3
											
///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
									 SetDataLen(m_sdatlen);
									 Set1363Packet();
}

void CYDT1363::set4144(void)//整流模块获取告警量
{
                    SetOneByteData(m_dataflag);
///////////////////////////////整流模块告警内容///////////////////////////////////////////////////////////////////////////////	
                     
                     u16 moduleNum=0;
										 u16  tmplen=0;
  									 u8 * pModuleWRANNING=(u8 *)pModuledata->GetDataAddr(WRANNING);	
	                   u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE)); 
										 
										 tmplen=m_sdatlen;//暂存模块数位置
	                    SetOneByteData(0);//临时写入整流模块数量 M
	          
										 for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历编所有在线的整流模块
											{
															if((moduletype[i]==ACDC)||(moduletype[i]==0x0F))
																{
																	moduleNum++;
																	SetOneByteData(pModuleWRANNING[i]&0x01);//整流模块故障
																	SetOneByteData(2);//铁塔自定义字段数量 a
																	
																	if(moduletype[i]==0x0F)//整流模块通讯断
																	{
																		SetOneByteData(1);//通讯断
																	}
																	else
																	{
																		SetOneByteData(0);//正常
																	}
																	
																	if(((pModuleWRANNING[i]&0xf0)>>4)>1) //整流模块未鉴权告警 0：未绑定，1：已绑定,2：锁定
																	{
																			SetOneByteData(1);//鉴权失败
																	}
																	else
																	{
																		  SetOneByteData(0);//鉴权成功
																	}
																	                   
																	
																	
											            SetOneByteData(0);//厂家自定义字段数量 b 
																}
											}
										HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置真实整流模块数量 M

///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
	
}

void CYDT1363::set4244(void)//直流瓶获取告警量
{
	  u8 command_group=0;
	  SetOneByteData(m_dataflag);
	
	///////////////////////////////直流配电告警内容///////////////////////////////////////////////////////////////////////////////		
	                       
		
											
																	if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
																	
																	if(command_group==0xff)	
																	SetOneByteData(1);//直流配电屏数量 M
																	
																	SetOneByteSwitchStatus(AlarmVolitage,1);//模块故障
		
																  SetOneByteData(1);
																  SetOneByteData(GetWarnInPut(BATTFUSR_BREAK,0x03));//电池1熔丝

											            SetOneByteData(5);//直流负载回路数
													        SetOneByteData(GetWarnInPut(LOAD_FUSE1,0x03));//本地熔丝 1个
																  for(u8 i=0;i<4;i++ ){//共4组
													        SetOneByteSwitchStatus(0x03,(gBattPara[i].battBreak1!=0));//扩展板负载熔丝
																  }
													      
													
																 SetOneByteData(0);//绝缘告警（未用项）	
																	
																 SetOneByteData(3);									//铁塔自定义字段数量 a=1+1+m m=1						
													       SetOneByteData(0);//电池下电
																 SetOneByteData(1);//电池组数量													
															   SetOneByteData(galram);//电池温度
																	
													       SetOneByteData(0);//a 厂家自定义字段数量 b=0
	
	
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}

void CYDT1363::setE244(void)//租户获取告警量
{
	 u8 command_group=0;
	  
	
	
	///////////////////////////////租户告警内容///////////////////////////////////////////////////////////////////////////////	
	                        	if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	                        if(command_group==0xff)	
													{
                              SetOneByteData(m_dataflag);
														  SetOneByteData(6);//租户数量 M
															for(u8 i=0;i<6;i++)
															{
//																 SetOneByteSwitchStatus(0x80,gDCdistribution.pst_userStatus[i+1]&0x01);//租户一次下电告警
//																 SetOneByteSwitchStatus(0x03,gDCdistribution.pst_userStatus[i+1]&0x40);//租户负载分路断告警
//																 SetOneByteData(0);//铁塔自定义字段数量 a=0
//																 SetOneByteData(0);//厂家自定义字段数量 b=0locdDownFlag
																  SetOneByteSwitchStatus(0x80,locdDownFlag&(1<<i));//租户一次下电告警
																  SetOneByteData(0);
																  SetOneByteData(0);//铁塔自定义字段数量 a=0
  																SetOneByteData(0);//厂家自定义字段数量 b=0locdDownFlag
																
																
																
																
															}
													}
													else if((command_group>0)&&(command_group<=6))
													{
														  SetOneByteData(m_dataflag);
//														  SetOneByteSwitchStatus(0x80,gDCdistribution.pst_userStatus[command_group]&0x01);//租户一次下电告警
//														  SetOneByteSwitchStatus(0x03,gDCdistribution.pst_userStatus[command_group]&0x40);//租户负载分路断告警
//															SetOneByteData(0);//铁塔自定义字段数量 a=0
//															SetOneByteData(0);//厂家自定义字段数量 b=0
														    SetOneByteSwitchStatus(0x80,locdDownFlag&(1<<command_group));//租户一次下电告警
															  SetOneByteData(0);
																SetOneByteData(0);//铁塔自定义字段数量 a=0
  															SetOneByteData(0);//厂家自定义字段数量 b=0locdDownFlag 
														
														
													}
	                        else
													{
														 ReturnVailData(6);return ;
													}
	
	
	
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}

void CYDT1363::set4344(void)//光伏模块获取告警量
{
                    SetOneByteData(m_dataflag);
///////////////////////////////整流模块告警内容///////////////////////////////////////////////////////////////////////////////	
                     
                     u16 moduleNum=0;
										 u16  tmplen=0;
  									 u8 * pModuleWRANNING=(u8 *)pModuledata->GetDataAddr(WRANNING);	
	                   u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE)); 
										 
										 tmplen=m_sdatlen;//暂存模块数位置
	                    SetOneByteData(0);//临时写入整流模块数量 M
	          
										 for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历编所有在线的整流模块
											{
															if((moduletype[i]==DCDC)||(moduletype[i]==0x0E))
																{
																	moduleNum++;
																	SetOneByteData(pModuleWRANNING[i]&0x01);//整流模块故障
																	SetOneByteData(1);//铁塔自定义字段数量 a
																	
																	if(moduletype[i]==0x0E)//整流模块通讯断
																	{
																		SetOneByteData(1);//通讯断
																	}
																	else
																	{
																		SetOneByteData(0);//正常
																	}
																	
																
																	
											            SetOneByteData(0);//厂家自定义字段数量 b 
																}
											}
										HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置真实整流模块数量 M

///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
	
}

void CYDT1363::setE2A4(void)//配电单元告警
{
	u8 command_group=0;
	
											  
	///////////////////////////////直流配电单元告警内容///////////////////////////////////////////////////////////////////////////////	
														u16 moduleNum=0;
														u16  tmplen=0;
														 
										        	moduleNum=0;
	                       if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	                       if(command_group==0xff)
												 {
													 
													  SetOneByteData(m_dataflag);
	                          tmplen=m_sdatlen;//暂存直流配电单元数量位置
											      SetOneByteData(0);//临时写入直流配电单元数量 M
													 
															for(u8 i=0;i<TOTAL_USER;i++)
															{
																if(SwitchOnlineCount[i]>0)
																{
																	 moduleNum++;
																
																	 SetOneByteSwitchStatus(0x80,gDCdistribution.pst_status[i]&0x01);//直流配电单元一次下电告警
																	 SetOneByteSwitchStatus(0x03,gDCdistribution.pst_status[i]&0x40);//直流配电单元断告警告警 
																	 SetOneByteData(0);//直流配电单元过温告警
																	 SetOneByteSwitchStatus(0x80,gDCdistribution.pst_status[i]&0x08);//直流配电单元断告警告警 
																	 SetOneByteSwitchStatus(0x80,gDCdistribution.pst_status[i]&0x80);//直流配电单元故障告警
																	 SetOneByteData(0);//直流配电单元反接告警
																	
																	SetOneByteData(i+1);//配电单元序号
																	
																	 SetOneByteData(0);//铁塔自定义字段数量 a=0
																	 SetOneByteData(0);//厂家自定义字段数量 b
																}
															
															}
															HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置配电单元数量 M
														}
												    else if((command_group>0)&&(command_group<=TOTAL_USER))
														{
																	u8 i=0;
																	 for( i=0;i<TOTAL_USER;i++)
																	{
																		 if(SwitchOnlineCount[i]>0)
																		{
																				 moduleNum++;
																			
																			if(command_group==moduleNum)
																			{
																				 SetOneByteData(m_dataflag);
																				 SetOneByteSwitchStatus(0x80,gDCdistribution.pst_status[i]&0x01);//直流配电单元一次下电告警
																				 SetOneByteSwitchStatus(0x03,gDCdistribution.pst_status[i]&0x40);//直流配电单元断告警告警 
																				 SetOneByteData(0);//直流配电单元过温告警
																				 SetOneByteSwitchStatus(0x80,gDCdistribution.pst_status[i]&0x08);//直流配电单元断告警告警 
																				 SetOneByteSwitchStatus(0x80,gDCdistribution.pst_status[i]&0x80);//直流配电单元故障告警
																				 SetOneByteData(0);//直流配电单元反接告警
																				
																				 SetOneByteData(i+1);//配电单元序号
																				
																				 SetOneByteData(0);//铁塔自定义字段数量 a=0
																				 SetOneByteData(0);//厂家自定义字段数量 b
																				 break;
																			 }
																		}
																			
																	}
																	if(i==TOTAL_USER)
																	{
																			ReturnVailData(0xE6);return ;//找不到对应分路
																	}
														}
														else
														{
																ReturnVailData(6);return ;
														}	

	
	
	
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}


void CYDT1363::setE144(void)//监控获取告警量
{
	SetOneByteData(m_dataflag);
	
	///////////////////////////////监控模块告警内容///////////////////////////////////////////////////////////////////////////////	
																  SetOneByteData(12);//输入干接点数量 M
																	 for(u8 i=0;i<12;i++)
																	 {
																			 SetOneByteData( GetDiInput(i,0x01));
																	 }
																
	
		///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	                     SetDataLen(m_sdatlen);
			                 Set1363Packet();
}
void CYDT1363::set4A44(void)////锂电模告警量
{
	   	u8 command_group=0;

	
			if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(command_group==0xff)
			{
	
					SetOneByteData(m_dataflag); 
					
					SetOneByteData(4);//电池组数
					for(u8 i=0;i<4;i++)
					{

						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_OVER_CHG);//电池过充
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_OVER_DHG);//电池过放
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_TEMP_HIGH);//电池温度高
						SetOneByteData(1);//电池单体数
						SetOneByteData(0);//电池1充电过压
						SetOneByteData(0);//电池1放电欠压	
						SetOneByteData(0);//电池1失效
						SetOneByteData(0);//电池单体数 (电池温度过高过低)
						SetOneByteData(9);//自定义
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_SHORT);//短路
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_VOLT_HIGH);//电池总体过压
					  SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_VOLT_LOW);//电池总体欠压
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_CELL_VOLT_HIGH);//电池单体过压
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_CELL_VOLT_LOW);//电池单体欠压
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_TEMP_LOW);//电池温度低
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&ENVTEMP_TEMP_HIGH);//环境温度高
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&ENVTEMP_TEMP_LOW);//环境温度低
						SetOneByteSwitchStatus(0x01,batt[i].prBattWarning&BATT_SOC_LOW);//SOC低

					}
			}	
			 else if((command_group>0)&&(command_group<=16))
		  {
			  if(batt[command_group-1].Nexttime>1)
				{
					  SetOneByteData(m_dataflag); 
					  SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_OVER_CHG);//过充
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_OVER_DHG);//过放
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_TEMP_HIGH);//电池温度高
						SetOneByteData(0);//电池单体数（过欠电压）
						SetOneByteData(0);//电池单体数 (电池温度过高过低)
						SetOneByteData(9);//自定义
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_SHORT);//短路
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_VOLT_HIGH);//电池总体过压
					  SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_VOLT_LOW);//电池总体欠压
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_CELL_VOLT_HIGH);//电池单体过压
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_CELL_VOLT_LOW);//电池单体欠压
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_TEMP_LOW);//电池温度低
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&ENVTEMP_TEMP_HIGH);//环境温度高
						SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&ENVTEMP_TEMP_LOW);//环境温度低
            SetOneByteSwitchStatus(0x01,batt[command_group-1].prBattWarning&BATT_SOC_LOW);//SOC低
					  
				}
				else
				{
					ReturnVailData(0xE6);return ;
				}
		}
		else
		{
			ReturnVailData(6);return ;
		}
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	 SetDataLen(m_sdatlen);
	 Set1363Packet();
	
	
}	







void  CYDT1363::setE544(void)
{
	  	u8 command_group=0;

	
			if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(command_group==0xff)
			{
	
					SetOneByteData(m_dataflag); 
					
					SetOneByteData(8);//电池组数
					for(u8 i=0;i<8;i++)
					{
						SetOneByteData((((g_astAIRpara[i].ast_warnning&0x01)==0x01)?1:0));//高温告警
						SetOneByteData((((g_astAIRpara[i].ast_warnning&0x02)==0x02)?1:0));//温度传感器故障
						SetOneByteData((((g_astAIRpara[i].ast_warnning&0x04)==0x04)?1:0));//高压力告警
						SetOneByteData((((g_astAIRpara[i].ast_warnning&0x08)==0x08)?1:0));//频繁高压力告警
						SetOneByteData((((g_astAIRpara[i].ast_warnning&0x10)==0x10)?1:0));//压缩机故障
						SetOneByteData((((g_astAIRpara[i].ast_warnning&0x20)==0x20)?1:0));//内风机故障
						SetOneByteData((((g_astAIRpara[i].ast_warnning&0x40)==0x40)?1:0));//外风机故障

					}
			}	
			 else if((command_group>0)&&(command_group<=8))
		  {
			  
					  SetOneByteData(m_dataflag); 
				
				    SetOneByteData((((g_astAIRpara[command_group-1].ast_warnning&0x01)==0x01)?1:0));//高温告警
						SetOneByteData((((g_astAIRpara[command_group-1].ast_warnning&0x02)==0x02)?1:0));//温度传感器故障
						SetOneByteData((((g_astAIRpara[command_group-1].ast_warnning&0x04)==0x04)?1:0));//高压力告警
						SetOneByteData((((g_astAIRpara[command_group-1].ast_warnning&0x08)==0x08)?1:0));//频繁高压力告警
						SetOneByteData((((g_astAIRpara[command_group-1].ast_warnning&0x10)==0x10)?1:0));//压缩机故障
						SetOneByteData((((g_astAIRpara[command_group-1].ast_warnning&0x20)==0x20)?1:0));//内风机故障
						SetOneByteData((((g_astAIRpara[command_group-1].ast_warnning&0x40)==0x40)?1:0));//外风机故障
				
		}
		else
		{
			ReturnVailData(6);return ;
		}
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	 SetDataLen(m_sdatlen);
	 Set1363Packet();
}


/////////////////获取参数数据///////////////////////////////////////////
void CYDT1363::set4046(void)//交流瓶获取参数
{
	     SetFourByteData((float)gInOverVLimit/100);//电压告警上限
	     SetFourByteData((float)gInOwrVLimit/100);//电压告警下限
	     SetFourByteData((float)gInOverILimit/100);//电流告警上限
	     SetFourByteData((float)gOverFLimit/100);//频率上限
	     SetFourByteData((float)gOwrFLimit/100);//频率下限
	      SetOneByteData(1);//铁塔自定义字段数量 a = 1 
	      SetFourByteData(0);//油机额定功率
	      SetOneByteData(0);//铁塔自定义字段数量 b = 0
	
	///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
			 SetDataLen(m_sdatlen);
			 Set1363Packet();
}

void CYDT1363::set4246(void)//直流瓶获取参数
{
	      SetFourByteData((float)gOutOverVLimit/100);//电压告警上限
	      SetFourByteData((float)goutPutOwrV/100);//电压告警下限
	
	
			// SetOneByteData(21+6+battGroud);//自定义P=21+6+m
	
				if(gbattGroud == 0)
				{
							SetOneByteData(21+1+battGroud);//自定义P=21+1+m
							SetFourByteData(battGroud);//电池组数量 m
							for(u8 i=0;i<battGroud;i++)
							SetFourByteData(gbranchBatt[i]);//电池容量
				}
				else if((gbattGroud>0)&&(gbattGroud<=6))
				{
							float tmpbatt[6]={0,0,0,0,0,0};
							u8 battCount = 0;
							
							for(u8 i=0;i<6;i++)
							{
								
								
									 if(gUseToBattBranch[i]<TOTAL_USER)//找到正在使用的电池分路
											 {
													
													 tmpbatt[battCount++] = gbranchBatt[i];//电池容量
											 }
								
								
							}
					
							SetOneByteData(21+1+battCount);
							SetFourByteData(battCount);
							for(u8 i=0;i<battCount;i++)
							SetFourByteData( tmpbatt[i]);			
					
				}
				else if(gbattGroud>6)
				{
					 SetOneByteData(21+1+0);//自定义P=21+1+m
				}
				
	
	
	
	
	
	
	      SetFourByteData((float)gSetfloatV/100);//浮充电压
	      SetFourByteData((float)gSetBattLimitPerC/100);//充电限流系数
				

        SetFourByteData((float)ghtemp/100);//电池温度高阈值
	      SetFourByteData((float)gltemp/100);//电池温度低阈值
				
	      SetFourByteData((float)gSleepOnoff);//休眠节能功能
	      SetFourByteData((float)gsleepMinCount);//最少开机模块数
	      SetFourByteData((float)gsleepTurnPeriod/24);//模块轮换周期 天
	      SetFourByteData((float)gsleepBattTestTime);//电池检测持续时间(分钟)
	     
			
				 
				 		 
				 float tmpf[4];
				 
				 
				 if(gbattGroud==0)
				 {
					 
					  tmpf[0]=(float)gDcParam0[0].st_downModle;//下电模式
					 
					   if(gDcParam0[0].st_downModle==0)
						 {
							 tmpf[1]=(float)0;//下电禁止
						 }
						 else
						 {
								tmpf[1]=(float)1;//下电使能
						 }
					 
				   tmpf[2]=(float)gDcParam0[0].st_LDVoltage/100;//下电电压
			    tmpf[3]=(float)gDcParam1[0].st_LDDelay/10;//下电时间（分钟）
					 
					 
				 }
					else if(gbattGroud>0)//配电单元用用电池
				 {
						 
						 tmpf[0]=(float)(gSwitchPara[TOTAL_USER+7].st_downMode&0x0f);
					 
						 if((gSwitchPara[TOTAL_USER+7].st_downMode&0x0f)==0)
						 {
							 tmpf[1]=(float)0;//下电禁止
						 }
						 else
						 {
								tmpf[1]=(float)1;//下电使能
						 }
						
							tmpf[2]=(float)gSwitchPara[TOTAL_USER+7].st_downV/100;
							tmpf[3]=(float)gSwitchPara[TOTAL_USER+7].st_downT;
						 
					} 
				 
				 
				  
				  SetFourByteData(tmpf[0]);//电池下电模式
					SetFourByteData(tmpf[1]);//电池下电使能
				  SetFourByteData(tmpf[2]);//电池下电电压
				  SetFourByteData(tmpf[3]);//电池下电时间
					
				  SetFourByteData((float)gAutoBootEnable);//周期均充使能
				  SetFourByteData((float)gSetequalV/100);//均充电压
				  SetFourByteData((float)gAutoPeriod);//均充周期
          SetFourByteData((float)gtestPeriodEnable);//周期测试使能
				  SetFourByteData((float)gtestV/100);//测试终止电压
				  SetFourByteData((float)gtestPeriod);//电池测试周期
				  SetFourByteData((float)gtestTimeout);//测试终止时间
				  SetFourByteData((float)gWorkShiftMode);//电池备电 //错峰使能
				  SetFourByteData(4);//负载下电模式
					SetOneByteData(0);
	

		///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
																 SetDataLen(m_sdatlen);
																 Set1363Packet();
}

void CYDT1363::setE246(void)//获取用租户参数
{
	
	u8 command_group=0;
	
	        if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	         if(command_group==0xff)
					 {						 
							SetOneByteData(6);//租户数量 n
							for(u8 i=0;i<6;i++)
						{
							
//							 u8 tmpdata=0;
//							 if((gSwitchPara[TOTAL_USER+1+i].st_downMode&0x0F)>0)
//							 {
//								 tmpdata=1;		 
//							 }
								SetFourByteData((float)gDcParam0[i].st_downModle);
								SetFourByteData((float)gDcParam1[i].st_LDDelay/10);
								SetFourByteData((float)gDcParam0[i].st_LDVoltage/100);
							  SetOneByteData(1);//铁塔自定义字段数量 a=1 1
								SetFourByteData(i+1);//租户名称
								SetOneByteData(0);//厂家自定义字段数量 b=0
						}
					}
					else if((command_group>0)&&(command_group<=6))
					{

						
						  	SetFourByteData((float)gDcParam0[command_group-1].st_downModle);
								SetFourByteData((float)gDcParam1[command_group-1].st_LDDelay/10);
								SetFourByteData((float)gDcParam0[command_group-1].st_LDVoltage/100);
						    SetOneByteData(1);//铁塔自定义字段数量 a=1 1
								SetFourByteData(command_group);//租户名称
								SetOneByteData(0);//厂家自定义字段数量 b=0
						
						
					}
					else
					{
						ReturnVailData(6);return ;
					}
			///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
	        SetDataLen(m_sdatlen);
				  Set1363Packet();
	
	
}
void CYDT1363::setE2A5(void)
{
	    u8 command_group=0; 
	   if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	   u8 branchCount=0,SearchCount=0;
			
		 if(command_group!=0xff){
				if((command_group>TOTAL_USER)||(command_group==0)){ReturnVailData(0xE6);return ;}
				
				
				
		  if(sysPara[PROL_OLD_FLAG] == 1)//按逻辑地址寻址
		  {

				 for( branchCount=0;branchCount<TOTAL_USER;branchCount++)
					 {
								if(SwitchOnlineCount[branchCount]>0)//查找已出现过的用户分路
								{
									 SearchCount++;
									if(SearchCount==command_group)
										break;
								}
					 }
					if(branchCount==TOTAL_USER){ReturnVailData(0xE6);return ;}//无数据;
				}
			else if(sysPara[PROL_OLD_FLAG] == 0)//按物理地址寻址
			{
				   	branchCount=command_group-1;
			    	if(SwitchOnlineCount[branchCount] == 0)
              {
								ReturnVailData(0xE6);
							  return ;
							}//无数据;	
			}
				       
				  
			     u8 tmpdata=0;
		   
				   if((gSwitchPara[branchCount].st_downMode&0x03)>0)
					 {
						 tmpdata=1;		 
					 }
				   SetFourByteData(tmpdata);//直流配电单元一次下电使能
					 SetFourByteData(gSwitchPara[branchCount].st_downT);//直流配电单元一次下电时间
					 SetFourByteData((float)gSwitchPara[branchCount].st_downV/100);//直流配电单元一次下电电压
					 
				   SetOneByteData(7);//铁塔自定义字段数量 a
					 
					  tmpdata=0;//直流配电单元名称
					  tmpdata=gSwitchPara[branchCount].st_userGroud&0x0F;
					  if(tmpdata==0x0B)
						tmpdata=0;
				    SetFourByteData(tmpdata);
						
						 	u32 tmp32=0;//配电单元供电截止日期
							u8 * ptmp=(u8 *)&tmp32;
							*(u16 *)&ptmp[2]=gSwitchPara[branchCount].st_stopTime.st_year;
							ptmp[1]=gSwitchPara[branchCount].st_stopTime.st_mon;
							ptmp[0]=gSwitchPara[branchCount].st_stopTime.st_day;
							SetFourByteU32Data(tmp32);
						
						 tmpdata=0;//配电上电授权
						// if((*(u32*)&gTimerNow<=*(u32*)&gSwitchPara[branchCount].st_stopTime)||(gSwitchData[ branchCount].authorizeflag==1))
						//if(*(u32*)&gTimerNow<=*(u32*)&gSwitchPara[branchCount].st_stopTime)
					  if(gSwitchData[ branchCount].authorizeflag==1)
						{
							tmpdata=1;
						}
						 SetFourByteData(tmpdata);
						
						 tmpdata=0;//直流配电单元用途
						if(gSwitchPara[branchCount].st_userGroud==0x7B)
						{
							 tmpdata=2;
						}
						 SetFourByteData(tmpdata);
						
						 SetFourByteData((float)gSwitchPara[branchCount].st_maxoverLoadI/10);//直流配电单元过流告警阀值
						
						///////////////////////////////配电单元下电类型/////////////////////////////////////////////							
						SetFourByteData(gSwitchPara[branchCount].st_downMode&0x03);
						
						
						
						
////////////////////////////////////////////直流配电单元序号/////////////////////////////////////////////					
						SetOneByteData(branchCount+1);
						 
						    	SetOneByteData(0);//厂家自定义字段数量 b=0
						     //gSwitchData[ branchCount].CmdStatus=1;//查询过载电流
					}
		     else if(command_group==0xFF)
				 {
					    u8 moduleNum=0;
					    u8  tmplen=m_sdatlen;//暂存模块数位置
	             SetOneByteData(0);//临时写入整流模块数量 M
					 
					 
					    for( branchCount=0;branchCount<TOTAL_USER;branchCount++)
							 {
										if(SwitchOnlineCount[branchCount]>0)//查找已出现过的用户分路
										{
											  u8 tmpdata=0;
											
											   moduleNum++;
											  if(moduleNum>=24)break;
		   
											 if((gSwitchPara[branchCount].st_downMode&0x03)>0)
											 {
												 tmpdata=1;		 
											 }
											 SetFourByteData(tmpdata);//直流配电单元一次下电使能
											 SetFourByteData(gSwitchPara[branchCount].st_downT);//直流配电单元一次下电时间
											 SetFourByteData((float)gSwitchPara[branchCount].st_downV/100);//直流配电单元一次下电电压
											 
											 SetOneByteData(7);//铁塔自定义字段数量 a
											 
												tmpdata=0;
												tmpdata=gSwitchPara[branchCount].st_userGroud&0x0F;
												if(tmpdata==0x0B)
												tmpdata=0;
												SetFourByteData(tmpdata);//直流配电单元名称
												
												u32 tmp32=0;//配电单元供电截止日期
												u8 * ptmp=(u8 *)&tmp32;
												*(u16 *)&ptmp[2]=gSwitchPara[branchCount].st_stopTime.st_year;
												ptmp[1]=gSwitchPara[branchCount].st_stopTime.st_mon;
												ptmp[0]=gSwitchPara[branchCount].st_stopTime.st_day;
												SetFourByteU32Data(tmp32);
												
												 tmpdata=0;//配电上电授权
												// if((*(u32*)&gTimerNow<=*(u32*)&gSwitchPara[branchCount].st_stopTime)||(gSwitchData[ branchCount].authorizeflag==1))
												//if(*(u32*)&gTimerNow<=*(u32*)&gSwitchPara[branchCount].st_stopTime)
											  if(gSwitchData[ branchCount].authorizeflag==1)
												{
													tmpdata=1;
												}
													SetFourByteData(tmpdata);
												
												 tmpdata=0;//直流配电单元用途
												if(gSwitchPara[branchCount].st_userGroud==0x7B)
												{
													 tmpdata=2;
												}
												 SetFourByteData(tmpdata);
												
												 SetFourByteData((float)gSwitchPara[branchCount].st_maxoverLoadI/10);//直流配电单元过流告警阀值
												
												///////////////////////////////配电单元下电类型/////////////////////////////////////////////							
												SetFourByteData(gSwitchPara[branchCount].st_downMode&0x03);
												
												
												
												
						////////////////////////////////////////////直流配电单元序号/////////////////////////////////////////////					
												SetOneByteData(branchCount+1);
												 
												SetOneByteData(0);//厂家自定义字段数量 b=0

										}
							 }
							 
							       HextoAscii(&m_senddata[13+tmplen],moduleNum);//设置真实整流模块数量 M
				 }
				
///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
																 SetDataLen(m_sdatlen);
																 Set1363Packet();				            
}


void CYDT1363::setE2AC(void)//获取配电单元扩展参数
{  u8 command_group=0; 
	   if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	   u8 branchCount=0,SearchCount=0;
			
				if((command_group>TOTAL_USER)||(command_group==0)){ReturnVailData(0xE6);return ;}

			 			
		  if(sysPara[PROL_OLD_FLAG] == 1)//按逻辑地址寻址
		  {

				 for( branchCount=0;branchCount<TOTAL_USER;branchCount++)
					 {
								if(SwitchOnlineCount[branchCount]>0)//查找已出现过的用户分路
								{
									 SearchCount++;
									if(SearchCount==command_group)
										break;
								}
					 }
					if(branchCount==TOTAL_USER){ReturnVailData(0xE6);return ;}//无数据;
				}
			else if(sysPara[PROL_OLD_FLAG] == 0)//按物理地址寻址
			{
				   	branchCount=command_group-1;
			    	if(SwitchOnlineCount[branchCount] == 0){ReturnVailData(0xE6);return ;}//无数据;	
			}
					
//////////////////////////////////////铁塔自定义字段数量 a = 24///////////////////////////////////////////					
					      SetOneByteData(24);
			   SetFourByteData((float)gSwitchPara[branchCount].st_downEnery/100);
///////////////////////////////////////直流配电单元定时下电使能	//////////////////////////////////////////						
								u8 tmpdata=0;
								 if((gSwitchPara[branchCount].st_downMode&0x10) !=0)
								 {
										tmpdata=1;
								 }
							 
									SetFourByteData(tmpdata);
/////////////////////////////////////////定时下电时段一起始时间	///////////////////////////////////////////							 
								 u32 tmp32=0;
								 tmp32=tmp32|((gSwitchPara[branchCount].startTime0/60)<<8);
								 tmp32=tmp32|(gSwitchPara[branchCount].startTime0%60);
								 SetFourByteU32Data(tmp32);
///////////////////////////////////////////////定时下电时段一时长/////////////////////////////////////////								 
								 SetFourByteData(gSwitchPara[branchCount].stopTime0);
///////////////////////////////////////////////其它时段不支持////////////////////////////////////////////								 
								 for(u8 i=0;i<8;i++)
								 {
									 SetFourByteData(0);
								 }
						 
////////////////////////////////////////直流配电单元定时免责使能//////////////////////////////////////////						 
									 tmpdata=0;
								 if((gSwitchPara[branchCount].st_downMode&0x20) !=0)
								 {
										tmpdata=1;
								 }
								 
									SetFourByteData(tmpdata);
/////////////////////////////////////////免责下电时段一起始时间//////////////////////////////////////////									 
									tmp32=0;
								 tmp32=tmp32|((gSwitchPara[branchCount].startTime1/60)<<8);
								 tmp32=tmp32|(gSwitchPara[branchCount].startTime1%60);
							 
								 SetFourByteU32Data(tmp32);
//////////////////////////////////////////免责下电时段一时长/////////////////////////////////////////////								 
								 SetFourByteData(gSwitchPara[branchCount].stopTime1);
							
/////////////////////////////////////////////其它时段不支持///////////////////////////////////////////////								 
								 for(u8 i=0;i<8;i++)
								 {
									 SetFourByteData(0);
								 }
////////////////////////////////////////////直流配电单元序号/////////////////////////////////////////////					
					      	SetOneByteData(branchCount+1);
////////////////////////////////////////////厂家自定义字段数量 b=0////////////////////////////////////////						 
						 
						    	SetOneByteData(0);
								 
								 
 ///////////////////////////////数据打包/////////////////////////////////////////////////////////////////
																 SetDataLen(m_sdatlen);
																 Set1363Packet();	
								 
								
}


void CYDT1363::set4A46(void)//设置锂电参数
{
	
		u8 command_group=0;

	
			if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(command_group==0xff)
			{
	
	
					SetOneByteData(4);//电池组数
					
					for(u8 i=0;i<4;i++)
					{
						SetFourByteData((float)batt[i].Set_BattV/100);
						SetFourByteData((float)batt[i].Set_Batt_ChargA/100);

					}
		
		 }
	    else if((command_group>0)&&(command_group<=16))
		  {
			  if(batt[command_group-1].Nexttime>1)
				{
					  SetFourByteData((float)batt[command_group-1].Set_BattV/100);
						SetFourByteData((float)batt[command_group-1].Set_Batt_ChargA/100);

					  
				}
				else
				{
					ReturnVailData(0xE6);return ;
				}
		}
		  else
			{
				ReturnVailData(6);return ;
			}
	
			///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
		   SetDataLen(m_sdatlen);
			 Set1363Packet();	
	
	
}

void CYDT1363::setE546(void)
{
		u8 command_group=0;

	
			if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(command_group==0xff)
			{
	
	
					SetOneByteData(8);//电池组数
					
					for(u8 i=0;i<8;i++)
					{
						SetFourByteData((float)g_astAIRpara[i].ast_airColdData/10);
						SetFourByteData((float)g_astAIRpara[i].ast_airTempData/10);
                
					}
		
		 }
	   else if((command_group>0)&&(command_group<=8))
		 {
			      SetFourByteData((float)g_astAIRpara[command_group-1].ast_airColdData/10);
						SetFourByteData((float)g_astAIRpara[command_group-1].ast_airTempData/10); 
		 }
		 else
		 {
				ReturnVailData(6);return ;
		 }
	
			///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
		   SetDataLen(m_sdatlen);
			 Set1363Packet();	
}




/////////////////设置参数数据///////////////////////////////////////////
void CYDT1363:: set4048(void)//设置交流瓶参数
{
	 u32 cmd_dat=0;
	   u8 cmd_type=0;
	
	  if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	  if(AsciitoHex32L(&cmd_dat ,&m_recvdata[2])==0){ReturnVailData(6);return ;};
		
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
		
		
			SetDataLen(m_sdatlen);
			 Set1363Packet();
		
}


void CYDT1363:: set4248(void)//设置直流瓶参数
{
		  u32 cmd_dat=0;
	  u8 cmd_type=0;
	
	  if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	  if(AsciitoHex32L(&cmd_dat ,&m_recvdata[2])==0){ReturnVailData(6);return ;}
		
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
							else if(( cmd_type>=0xA0) && ( cmd_type<=0xA5))
							{
								   if(gbattGroud == 0)
									 {
										  u8 id= cmd_type&0x0F;
										 if( id<=5)
										 {
										  	gbranchBatt[id]=*((float*)&cmd_dat);
											
												pgh52c0->savePara(& gbranchBatt[id]);
										 
											 gSetBattC=0;
												for(u8 i=0;i<battGroud;i++)
												{
														gSetBattC+=gbranchBatt[i];
												}
											 pbattCap->ReSetCap();
											}	
											
									 }
									 else if(gbattGroud > 0)
									 {
										 u8 id=(cmd_type&0x0F)+1;//设置第n路电池
										 if( id<=6)
										 { u8 countonline=0;
											 u8 j;
														 for(j=0;j<6;j++)//
															{
																	 if(gUseToBattBranch[j]<TOTAL_USER)
																	 {
																			
																				countonline++;
																				if(countonline == id)
																				{
																					break;
																				}
																			 
																	 }
																		
															}
												     if(j<6)
														 {
															  gbranchBatt[j]=*((float*)&cmd_dat);//找到第n在线的电池
											        	pgh52c0->savePara(& gbranchBatt[j]);
															 
															  gSetBattC=0;
																 for(u8 i=0;i<6;i++)
																{
																		 if(gUseToBattBranch[i]<TOTAL_USER)
																		 {
																				
																				 gSetBattC+=gbranchBatt[i];
																		 }
																			
																}
																 pbattCap->ReSetCap();
														 }
												
											}
										 
										 
										 
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
							else if(cmd_type==0xAA)//电流温度高
							{
									ghtemp=*((float*)&cmd_dat)*100;
								  pgh52c0->savePara(&ghtemp);
							}
							else if(cmd_type==0xAB)//电流温度低
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
								
								 gsleepTurnPeriod=(*((float*)&cmd_dat))*24;						
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

									 
											gDcParam0[0].st_downModle=(u8)(*((float*)&cmd_dat));
					
											pgh52c0->savePara((u16 *)&gDcParam0[0]);//指向st_downModle位置
									 
											 gDcParam0[1].st_downModle=(u8)(*((float*)&cmd_dat));
					
											pgh52c0->savePara((u16 *)&gDcParam0[1]);//指向st_downModle位置
									 
									   setPoint=4;
									   setBattParaflag=2;
									 
								 }
								 else
								 {
									 gSwitchPara[TOTAL_USER+7].st_downMode&=0xF0;
									 gSwitchPara[TOTAL_USER+7].st_downMode|=(u8)(*((float*)&cmd_dat));
									 gSetDownParaCode=12;//下电模式命令码
												gSetDownParaGroudType=0x52;//分组设置
												gSetDownParaAddr=7;//电池分组
										 
										 
													for(u8 i=0;i<TOTAL_USER;i++)
											 {
													if(gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)//同步所有电池分组数据
													{
																	 gSwitchPara[i].st_downMode=gSwitchPara[TOTAL_USER+7].st_downMode;
													}
											 }
									 
								 }
							}
							else if(cmd_type==0xB3)//下电使能
							{
								
								 
									 if(gbattGroud==0)
									 {   
										

										 
											 gDcParam0[0].st_downModle=(u8)(*((float*)&cmd_dat));
					
											 pgh52c0->savePara((u16 *)&gDcParam0[0]);//指向st_downModle位置
										 
										 
											gDcParam0[1].st_downModle=(u8)(*((float*)&cmd_dat));
					
											 pgh52c0->savePara((u16 *)&gDcParam0[1]);//指向st_downModle位置
										 
										  setPoint=4;
									    setBattParaflag=2;
															 
									 }
									 else 
									 {
												gSwitchPara[TOTAL_USER+7].st_downMode&=0xF0;
												gSwitchPara[TOTAL_USER+7].st_downMode|=(u8)(*((float*)&cmd_dat));
												gSetDownParaCode=12;//下电模式命令码
												gSetDownParaGroudType=0x52;//分组设置
												gSetDownParaAddr=7;//电池分组
										 
										 
													for(u8 i=0;i<TOTAL_USER;i++)
											 {
													if(gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)//同步所有电池分组数据
													{
																	 gSwitchPara[i].st_downMode=gSwitchPara[TOTAL_USER+7].st_downMode;
													}
											 }
										 
										 
										 
									 }
							 
								
									 
							}
							else if(cmd_type==0xB4)//下电电压
							{
								 if(gbattGroud==0)
								 {

									 
									 gDcParam0[0].st_LDVoltage=*((float*)&cmd_dat)*100;
									 pgh52c0->savePara((u16 *)&gDcParam0[0]+3);		//指st_LDVoltage位置			
									 
									 gDcParam0[1].st_LDVoltage=*((float*)&cmd_dat)*100;
									 pgh52c0->savePara((u16 *)&gDcParam0[1]+3);		//指st_LDVoltage位置		
									 
									 
									  setPoint=4;
									  setBattParaflag=2;
								 }
								 else
								 {
									 
										 gSwitchPara[TOTAL_USER+7].st_downV=(*((float*)&cmd_dat))*100;
											gSetDownParaCode=4;//下电模式命令码
												gSetDownParaGroudType=0x52;//分组设置
												gSetDownParaAddr=7;//电池分组
									 
										for(u8 i=0;i<TOTAL_USER;i++)
											 {
													if(gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)//同步所有电池分组数据
													{
																	 gSwitchPara[i].st_downV=gSwitchPara[TOTAL_USER+7].st_downV;
													}
											}
									 
								 }
								
							}
							else if(cmd_type==0xB5)//下电时间
							{
				
											if(gbattGroud==0)
											{

												
												 gDcParam1[0].st_LDDelay=(u16)(*((float*)&cmd_dat))*10;  
												 pgh52c0->savePara((u16 *)&gDcParam1[0]+2);	//指st_LDDelay位置	

												 gDcParam1[1].st_LDDelay=(u16)(*((float*)&cmd_dat))*10;  
												 pgh52c0->savePara((u16 *)&gDcParam1[1]+2);	//指st_LDDelay位置				

                          setPoint=4;
									        setBattParaflag=2;												
												
											}
											else
											{//电组数设置大于0时，使能在设项设置
												
														gSwitchPara[TOTAL_USER+7].st_downT=(u16)(*((float*)&cmd_dat));
															gSetDownParaCode=5;//时间下电模式命令码
															gSetDownParaGroudType=0x52;//分组设置
															gSetDownParaAddr=7;//电池分组
												 
													for(u8 i=0;i<TOTAL_USER;i++)
														 {
																if(gSwitchPara[i].st_userGroud>>4==gSetDownParaAddr)//同步所有电池分组数据
																{
																				 gSwitchPara[i].st_downT=gSwitchPara[TOTAL_USER+7].st_downT;
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
							else if(cmd_type==0xB8)//均充周期
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
							else if(cmd_type==0xBA)//测试终止电压
							{
								   gtestV=*((float*)&cmd_dat)*100;
								   pgh52c0->savePara(&gtestV);
							}
							
							else if(cmd_type==0xBB)//测试周期
							{    
								   gtestPeriod=(*((float*)&cmd_dat));
								   pgh52c0->savePara(&gtestPeriod);

							}
			        else if(cmd_type==0xBC)//测试终止时间
							{
								   gtestTimeout=*((float*)&cmd_dat); //页面以小时表示，因些"*60",变为分钟
									 pgh52c0->savePara(&gtestTimeout);
							}
							else if(cmd_type==0xBD)//电池备电模式
							{
								  gWorkShiftMode=*((float*)&cmd_dat);
								  if(gWorkShiftMode>1) gWorkShiftMode=1;
								   pgh52c0->savePara(&gWorkShiftMode);
							}
							
//							else if(cmd_type==0xBE)//负载下电模式
//							{
//							}
							else//无效命令
							{
								ReturnVailData(0x06);return ;
							}
							

              SetDataLen(m_sdatlen);
			        Set1363Packet();
}

void CYDT1363:: setE2A6(void)//设置配电单元参数
{
	     u32 cmd_dat=0;
	      u8 cmd_type=0;
	      u8 command_group=0; 
	
	    if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(AsciitoHex(&cmd_type,&m_recvdata[2])==0){ReturnVailData(6);return ;}//无效命令;
			if(AsciitoHex32L(&cmd_dat ,&m_recvdata[4])==0){ReturnVailData(6);return ;};
			
			u8 branchCount=0,SearchCount=0;
			if(command_group!=0xff)
			{
					if((command_group>TOTAL_USER)||(command_group==0)){ReturnVailData(0xE6);return ;}
					
							  if(sysPara[PROL_OLD_FLAG] == 1)//按逻辑地址寻址
								{

									 for( branchCount=0;branchCount<TOTAL_USER;branchCount++)
										 {
													if(SwitchOnlineCount[branchCount]>0)//查找已出现过的用户分路
													{
														 SearchCount++;
														if(SearchCount==command_group)
															break;
													}
										 }
										if(branchCount==TOTAL_USER){ReturnVailData(0xE6);return ;}//无数据;
								}
								else if(sysPara[PROL_OLD_FLAG] == 0)//按物理地址寻址
								{
											branchCount=command_group-1;
											if(SwitchOnlineCount[branchCount] == 0){ReturnVailData(0xE6);return ;}//无数据;	
								}
				
					

						 gSetDownParaGroudType=0x51;//分组设置 单用户
						gSetDownParaAddr=branchCount+1;//
			}
			else
			{
				    gSetDownParaGroudType=0x52;//分组设置 设置全部
						gSetDownParaAddr=0xf;//电池分组
				    branchCount=TOTAL_USER;
			}
			
			
			      if(cmd_type==0x80)//直流配电单元一次下电使能
						{
							 gSwitchPara[branchCount].st_downMode&=0xF0;
							 gSwitchPara[branchCount].st_downMode|=(u8)(*((float*)&cmd_dat));
								 gSetDownParaCode=12;//下电模式命令码
							 pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downMode,1);				
							if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
							{
								pdisDisPlayData[34]=1;
							}
							
							if(command_group==0xff)
							{
								 for(u8 i=0;i<TOTAL_USER;i++)
								{
									 gSwitchPara[i].st_downMode=gSwitchPara[TOTAL_USER].st_downMode;
								}
							}
							
										
						}
						
						else if(cmd_type==0x81)//直流配电单元一次下电时间
						{
								gSwitchPara[branchCount].st_downT=(u16)(*((float*)&cmd_dat));
								 gSetDownParaCode=5;//下电时间命令码
										
							    pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downT);
							if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
							{
								pdisDisPlayData[34]=1;
							}
							
							if(command_group==0xff)
							{
								 for(u8 i=0;i<TOTAL_USER;i++)
								{
									 gSwitchPara[i].st_downT=gSwitchPara[TOTAL_USER].st_downT;
								}
							}
										
						}
						else if(cmd_type==0x82)//直流配电单元一次下电电压
						{
								gSwitchPara[branchCount].st_downV=(*((float*)&cmd_dat))*100;
								 gSetDownParaCode=4;//下电电压命令码
								 pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downV);

							  *(((u16 *)&gsetTmpData)+1)=gSwitchPara[ branchCount].st_downV;
								*(((u8 *)&gsetTmpData)+0)=gSwitchPara[ branchCount].st_userGroud;
								gcmdAddr = branchCount+1;
								gSwitchData[branchCount].CmdStatus=8;
							
							if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
								{
									pdisDisPlayData[34]=1;
								}		
								
							if(command_group==0xff)
							{
								 for(u8 i=0;i<TOTAL_USER;i++)
								{
									 gSwitchPara[i].st_downV=gSwitchPara[TOTAL_USER].st_downV;
								}
							}	
								
						}
						else if(cmd_type==0xA0)//直流配电单元名称
						{
					
									 u8 usegroud[12]={0,1,1,2,2,3,3,4,4,6,5,7};//0无, 1移动 ，2联通，3电信，4广电，6铁塔，5行业外，7电池
									 u8 tmp8=(u8)(*((float*)&cmd_dat));
									 
									 if((command_group==0xff)&&(tmp8==0x0B)){	ReturnVailData(0xE2);return ;}//不能批量设电池，返回//绑定电池失败}
									 
									 if((gSwitchPara[branchCount].st_userGroud==0x7B)&&(tmp8!=0x0B))//若设置前为电池分路并且将设为非电池分路，则移除分路电池绑定
									 {
											for(u8 i=0;i<6;i++)
										 {
												if(gUseToBattBranch[i]==(branchCount))
												{
													
														 pgBattBranch[i]=NULL;//移除电池绑定
														 gUseToBattBranch[i]=0xff;//设置电池分路设为未定义
														 gbattGroud--;//电池分路数减一

													   pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
														 break;
												
												}
										 }
									 }
					
									 else if((gSwitchPara[branchCount].st_userGroud!=0x7B)&&(tmp8==0x0B))//若设置前为非电池分路并且将设为电池分路，则分路绑定电池
									 {
										 
											 if(gDCdistribution.pst_switchtype[branchCount]!=2)//若当前分路开关不是双向开关
											 {
													ReturnVailData(0xE2);return ;//不能设为电池，返回
													 //return ;
											 }
											 u8 i;
											 for(i=0;i<6;i++)
											 {
													if(gUseToBattBranch[i]==0xff)
													{
														
															 pgBattBranch[i]=&gDCdistribution.pst_I[branchCount];//电池绑定
															 gUseToBattBranch[i]=branchCount;//设置电池分路
															 gbattGroud++;//电池分路数加一
														
                                pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
																
															 break;
													
													}
											 }
												if(i==6)
											 {
													ReturnVailData(0xE2);return ;//不能设为电池，返回//绑定电池失败
											 }
									 }
								 
					         
							gSwitchPara[branchCount].st_userGroud=((usegroud[tmp8]<<4)|tmp8);
							gSetDownParaCode=1;//下电电压命令码
									 
						if(gSwitchPara[ branchCount].st_userGroud!=0x7B)//用户类型只存非电池的用户
							{								 
							 pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_userGroud,1);
							 	*(((u16 *)&gsetTmpData)+1)=gSwitchPara[ branchCount].st_downV;
								*(((u8 *)&gsetTmpData)+0)=gSwitchPara[ branchCount].st_userGroud;
								gcmdAddr = branchCount+1;
								gSwitchData[branchCount].CmdStatus=8;
							}	 
								 
								if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
								{
									pdisDisPlayData[34]=1;
								}		 
						

						 if(command_group==0xff)
						 {
								for(u8 i=0;i<TOTAL_USER;i++)
								{
									 gSwitchPara[i].st_userGroud=gSwitchPara[TOTAL_USER].st_userGroud;
								}
								
								
								for(u8 i=0;i<6;i++)//清除所有电池分路
								 {
												 pgBattBranch[i]=NULL;//移除电池绑定
												 gUseToBattBranch[i]=0xff;//设置电池分路设为未定义
												 gbattGroud=0;//电池分路数减一
												 break;
										
										
								 }
								
						 }
										
															
							
						}
            else if(cmd_type==0xA1)//配电单元供电截止日期
						{
										//u32 tmp32=AsciitoHex32(&m_recvdata[4]);
							u8 *ptmp=(u8 *)&cmd_dat;
										  gSwitchPara[branchCount].st_stopTime.st_year=(ptmp[0]<<8)|ptmp[1];//(u16)(cmd_dat>>16);
										  gSwitchPara[branchCount].st_stopTime.st_mon=ptmp[2];//.(u8)(cmd_dat>>8);
										  gSwitchPara[branchCount].st_stopTime.st_day=ptmp[3];//(u8)cmd_dat;
										  gSetDownParaCode=2;
										 pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_stopTime,4);
										if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
											pdisDisPlayData[34]=1;
										}
						}
						else if(cmd_type==0xA2)//配电上电授权
						{
							   
							
										 u8 tmp8=(u8)(*((float*)&cmd_dat));
										 if(tmp8==1)
										
										{
										  gSwitchPara[branchCount].st_stopTime.st_year=2099;
										  gSwitchPara[branchCount].st_stopTime.st_mon=12;
										  gSwitchPara[branchCount].st_stopTime.st_day=31;
											gSetDownParaCode=2;
											
											gSwitchData[branchCount].setAuthorize=1;
											gSwitchData[branchCount].CmdStatus=7;
										
										}
										else if(tmp8==0)
										{
											 gSwitchPara[branchCount].st_stopTime.st_year=1900;
										   gSwitchPara[branchCount].st_stopTime.st_mon=12;
										   gSwitchPara[branchCount].st_stopTime.st_day=31;
											 gSetDownParaCode=2;
											
											 gSwitchData[branchCount].setAuthorize=0;
											 gSwitchData[branchCount].CmdStatus=7;
										}
										
										 pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_stopTime,4);
										if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
												pdisDisPlayData[34]=1;
										}
										
										 if(command_group==0xff)
										 {
												for(u8 i=0;i<TOTAL_USER;i++)
												{
													gSwitchPara[i].st_stopTime.st_year=gSwitchPara[TOTAL_USER].st_stopTime.st_year;
										      gSwitchPara[i].st_stopTime.st_mon=gSwitchPara[TOTAL_USER].st_stopTime.st_mon;
										      gSwitchPara[i].st_stopTime.st_day=gSwitchPara[TOTAL_USER].st_stopTime.st_day;
												}
										}
		
						}

            else if(cmd_type==0xA3)//直流配电单元用途
						{		
							if(command_group==0xff){ReturnVailData(0xE2);return ;}
							
							  u8 tmp8=(u8)(*((float*)&cmd_dat));
										
										   if((tmp8==2)||(tmp8==3))
											 {
												 
												 if(gSwitchPara[branchCount].st_userGroud!=0x7B)//若设置前为非电池分路并且将设为电池分路
														 {
															 
																 if(gDCdistribution.pst_switchtype[branchCount]!=2)//若当前分路开关不是双向开关
																 {
																		
																			ReturnVailData(0xE2);return ;
																 }
																 u8 i;
															   for(i=0;i<6;i++)
																 {
																		if(gUseToBattBranch[i]==0xff)
																		{
																			
																				 pgBattBranch[i]=&gDCdistribution.pst_I[branchCount];//电池绑定
																				 gUseToBattBranch[i]=branchCount;//设置电池分路
																				 gbattGroud++;//电池分路数加一

																			   pgh52c0->SaveCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置写入
																			
																			   break;
																		
																		}
																 }
																 if(i==6)
																 {
																	  ReturnVailData(0xE2);return ;
																 }
																 
																 
																  gSwitchPara[branchCount].st_userGroud=0x7B;
										            	gSetDownParaCode=1;//用户命令码
																 	pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_userGroud,1);
																 
																  *(((u16 *)&gsetTmpData)+1)=gSwitchPara[ branchCount].st_downV;
																	*(((u8 *)&gsetTmpData)+0)=gSwitchPara[ branchCount].st_userGroud;
																	gcmdAddr = branchCount+1;
																	gSwitchData[branchCount].CmdStatus=8;
														 }
												 
												 
											 }
										
										   if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
												{
														pdisDisPlayData[34]=1;
												}
										
						}
            else if(cmd_type==0xA4)//直流配电单元过流告警阀值
						{	
							  gSwitchPara[branchCount].st_maxoverLoadI=(*((float*)&cmd_dat))*10;
											 gSetDownParaCode=11;//过载电流命令码
							
							       pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_maxoverLoadI);
							
							       gSwitchData[branchCount].CmdStatus=2;
										 gcmdAddr=branchCount+1;
											
											if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
												pdisDisPlayData[34]=1;
										}
										
										if(command_group==0xff)
										 {
												for(u8 i=0;i<TOTAL_USER;i++)
												{
													gSwitchPara[i].st_maxoverLoadI=gSwitchPara[TOTAL_USER].st_maxoverLoadI;
												}
										}
						}
            else if(cmd_type==0xA5)//下电模式命令码
						{
							 gSwitchPara[branchCount].st_downMode&=0xF0;
										  gSwitchPara[branchCount].st_downMode|=(u8)(*((float*)&cmd_dat));
											 gSetDownParaCode=12;//下电模式命令码
										 pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downMode,1);	
										
											if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
											{
												pdisDisPlayData[34]=1;
											}
						}
            else
						{
							    ReturnVailData(0x06);return ;
						}






              SetDataLen(m_sdatlen);
			        Set1363Packet();

}

void CYDT1363:: setE2AD(void)//设置配电单元扩展参数
{
	     u32 cmd_dat=0;
	      u8 cmd_type=0;
	      u8 command_group=0; 
	
	    if(AsciitoHex(&command_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
			if(AsciitoHex(&cmd_type,&m_recvdata[2])==0){ReturnVailData(6);return ;}//无效命令;
			if(AsciitoHex32L(&cmd_dat ,&m_recvdata[4])==0){ReturnVailData(6);return ;};
			
			u8 branchCount=0,SearchCount=0;
			if(command_group!=0xff)
			{
					if((command_group>TOTAL_USER)||(command_group==0)){ReturnVailData(0xE6);return ;}
					
					  
							  if(sysPara[PROL_OLD_FLAG] == 1)//按逻辑地址寻址
								{

									 for( branchCount=0;branchCount<TOTAL_USER;branchCount++)
										 {
													if(SwitchOnlineCount[branchCount]>0)//查找已出现过的用户分路
													{
														 SearchCount++;
														if(SearchCount==command_group)
															break;
													}
										 }
										if(branchCount==TOTAL_USER){ReturnVailData(0xE6);return ;}//无数据;
								}
								else if(sysPara[PROL_OLD_FLAG] == 0)//按物理地址寻址
								{
											branchCount=command_group-1;
											if(SwitchOnlineCount[branchCount] == 0){ReturnVailData(0xE6);return ;}//无数据;	
								}
				
					

						 gSetDownParaGroudType=0x51;//分组设置 单用户
						gSetDownParaAddr=branchCount+1;//
			}
			else
			{
				    gSetDownParaGroudType=0x52;//分组设置 设置全部
						gSetDownParaAddr=0xf;//电池分组
				    branchCount=TOTAL_USER;
			}
			
			
			      if(cmd_type==0xA0)//下电电量
						{	
							 
						  
								  gSwitchPara[branchCount].st_downEnery=(*((float*)&cmd_dat))*100;
								 gSetDownParaCode=3;//
								
								  pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downEnery);
									if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
									{
										pdisDisPlayData[34]=1;
									}
							
						}
			      else if(cmd_type==0xA1)//直流配电单元定时下电使能
						{	
							   if(*((float*)&cmd_dat)==1)
								 gSwitchPara[branchCount].st_downMode|=0x10;
								else
									gSwitchPara[branchCount].st_downMode&=0xEF;
								
								gSetDownParaCode=12;//
								  pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downMode,1);
								  if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
									{
											pdisDisPlayData[34]=1;
									}
									
									
									
									if(command_group==0xff)
										 {
												for(u8 i=0;i<TOTAL_USER;i++)
												{
													gSwitchPara[i].st_downMode=gSwitchPara[TOTAL_USER].st_downMode;
												}
										}
									
										
									
						}
						else if(cmd_type==0xA2)//定时下电时段一起始时间
						{	
							  	u8 tmp=(cmd_dat&0xFF000000)>>24;// 分
								  u8 tmp1=(cmd_dat&0x00FF0000)>>16;// 时
								
								
								  gSwitchPara[branchCount].startTime0=tmp1*60+tmp;
								  //gSwitchPara[branchCount].st_downMode&=0xEF;
								  gSetDownParaCode=7;//
								   pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].startTime0);
								    if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
												pdisDisPlayData[34]=1;
										}
										
										if(command_group==0xff)
										 {
												for(u8 i=0;i<TOTAL_USER;i++)
												{
													gSwitchPara[i].startTime0=gSwitchPara[TOTAL_USER].startTime0;
												}
										}
						}
						else if(cmd_type==0xA3)//定时下电时段一时长
						{
							    gSwitchPara[branchCount].stopTime0=(u16)(*((float*)&cmd_dat));
								 // gSwitchPara[branchCount].st_downMode&=0xEF;
								  gSetDownParaCode=8;//
								  pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].stopTime0);
								    if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
												pdisDisPlayData[34]=1;
										}
										
											if(command_group==0xff)
										 {
												for(u8 i=0;i<TOTAL_USER;i++)
												{
													gSwitchPara[i].stopTime0=gSwitchPara[TOTAL_USER].stopTime0;
												}
										}
						}
						 else if(cmd_type==0xAC)//直流配电单元免责下电使能
						{	
							   if(*((float*)&cmd_dat)==1)
								 gSwitchPara[branchCount].st_downMode|=0x20;
								else
									gSwitchPara[branchCount].st_downMode&=0xDF;
								
								gSetDownParaCode=12;//
								  pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].st_downMode,1);
								  if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
									{
											pdisDisPlayData[34]=1;
									}
									
									
									
									if(command_group==0xff)
										 {
												for(u8 i=0;i<TOTAL_USER;i++)
												{
													gSwitchPara[i].st_downMode=gSwitchPara[TOTAL_USER].st_downMode;
												}
										}
									
									
						}
						else if(cmd_type==0xAD)//免责时段一起始时间
						{	
							  u8 tmp=(cmd_dat&0xFF000000)>>24;// 分
								 u8 tmp1=(cmd_dat&0x00FF0000)>>16;// 时
								
								
								  gSwitchPara[branchCount].startTime1=tmp1*60+tmp;
								  //gSwitchPara[branchCount].st_downMode&=0xDF;
								  gSetDownParaCode=9;//
								  pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].startTime1);
								    if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
												pdisDisPlayData[34]=1;
										}
										
										if(command_group==0xff)
										 {
												for(u8 i=0;i<TOTAL_USER;i++)
												{
													gSwitchPara[i].startTime1=gSwitchPara[TOTAL_USER].startTime1;
												}
										}
						}
						else if(cmd_type==0xAE)//免责时段一时长
						{
							    gSwitchPara[branchCount].stopTime1=(u16)(*((float*)&cmd_dat));
								 // gSwitchPara[branchCount].st_downMode&=0xDF;
								  gSetDownParaCode=10;//
								  	pgh52c0->SaveSwitchPara(&gSwitchPara[ branchCount].stopTime1);
								    if( pScreen->m_layer[pScreen->m_LayerCount].st_ppItem==&pSetBranchParaMenu)
										{
												pdisDisPlayData[34]=1;
										}
										
											if(command_group==0xff)
										 {
												for(u8 i=0;i<TOTAL_USER;i++)
												{
													gSwitchPara[i].stopTime1=gSwitchPara[TOTAL_USER].stopTime1;
												}
										}
						}
						else
						{
							    ReturnVailData(0x06);return ;
						}
			
			
			
			
			
			


             SetDataLen(m_sdatlen);
			        Set1363Packet();
}
void CYDT1363:: set4A48(void)//设置锂电
{
	 u32 cmd_dat=0;
	 u8 cmd_type=0;
	u8 cmd_group=0;
	
	
	
	 if(AsciitoHex(&cmd_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	  if(AsciitoHex(&cmd_type,&m_recvdata[2])==0){ReturnVailData(6);return ;}//无效命令;
	  if(AsciitoHex32L(&cmd_dat ,&m_recvdata[4])==0){ReturnVailData(6);return ;};//无效命令;
	
	if(cmd_type==0xA5)//电压
	{

		 gsetbatt.Set_BattV = *((float*)&cmd_dat)*100;;
		 SetBatteryData(1,0,(u8 *)&gsetbatt.Set_BattV);
		 pgh52c0->savePara(&gsetbatt.Set_BattV );
		
		
		
	
	}
	else if(cmd_type==0xA6)//充电电流
	{
		 
		 gsetbatt.Set_Batt_ChargA= *((float*)&cmd_dat)*100;
		 SetBatteryData(2,0,(u8 *)&gsetbatt.Set_Batt_ChargA);
		 pgh52c0->savePara(&gsetbatt.Set_Batt_ChargA );
		
		
		
	} 
	
	else if(cmd_type==0xA7)//MOS充
	{


		
		    gsetbatt.MOS_Charg = *((float*)&cmd_dat);
		
		
		   SetBatteryData(3,0,(u8 *)&gsetbatt.MOS_Charg);
		   pgh52c0->savePara(&gsetbatt.MOS_Charg );
		
		
	} 
	else if(cmd_type==0xA8)//MOS放
	{

		   gsetbatt.MOS_DisCharg = *((float*)&cmd_dat);
		   
		   SetBatteryData(4,0,(u8 *)&gsetbatt.MOS_DisCharg);
		   pgh52c0->savePara(&gsetbatt.MOS_DisCharg );
		
	}  
	
	
	  SetDataLen(m_sdatlen);
		Set1363Packet();	
	
	
	
}
void CYDT1363::setE248(void)
{
	u32 cmd_dat = 0;
	u8 cmd_type = 0;
	u8 command_group = 0;

	if (AsciitoHex(&command_group, &m_recvdata[0]) == 0)
	{
		ReturnVailData(6);
		return;
	} // 无效命令;
	if (AsciitoHex(&cmd_type, &m_recvdata[2]) == 0)
	{
		ReturnVailData(6);
		return;
	} // 无效命令;
	if (AsciitoHex32L(&cmd_dat, &m_recvdata[4]) == 0)
	{
		ReturnVailData(6);
		return;
	} // 无效命令;

	if ((command_group >= 1) && (command_group <= 6)) // command_group有效取值 1--2（返回单分路）
	{

		if (cmd_type == 0x80) // 铁塔A协议 通道号2453 本地下电1使能
		{

			gDcParam0[command_group - 1].st_downModle = (u8)(*((float *)&cmd_dat)); // 设置下电使能   0，禁能，>0使能
			pgh52c0->savePara((u16 *)&gDcParam0[command_group - 1] + 0);			// 指向st_downModle位置//写入E2ROM
		}
		else if (cmd_type == 0x81)
		{

			downDelayTimeMin = (u16)(*((float *)&cmd_dat) * 10);
			gDcParam1[command_group - 1].st_LDDelay = downDelayTimeMin;	 // 设置下电时间
			pgh52c0->savePara((u16 *)&gDcParam1[command_group - 1] + 2); // 指st_LDDelay位置	//写入E2ROM
		}
		else if (cmd_type == 0x82)
		{

			downVoltageLimit = *((float *)&cmd_dat) * 100;

			gDcParam0[command_group - 1].st_LDVoltage = downVoltageLimit; // 设置下电电压
			pgh52c0->savePara((u16 *)&gDcParam0[command_group - 1] + 3);  // 指st_LDVoltage位置		//写入E2ROM
		}
		else // 无效命令
		{
			m_rtn = 5;
		}
	}
	else // 无效数据
	{
		m_rtn = 6;
	}

	SetDataLen(m_sdatlen);
	Set1363Packet();
}
void CYDT1363:: setE548(void)
{
	 u32 cmd_dat=0;
	 u8 cmd_type=0;
	u8 cmd_group=0;
	
	
	
	 if(AsciitoHex(&cmd_group,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	  if(AsciitoHex(&cmd_type,&m_recvdata[2])==0){ReturnVailData(6);return ;}//无效命令;
	  if(AsciitoHex32L(&cmd_dat ,&m_recvdata[4])==0){ReturnVailData(6);return ;};//无效命令;
		
		if(cmd_type==0x80)//压缩机制冷点
	  {
			    	g_AirInterfaceData[cmd_group-1].g_AirColdData  = *((float*)&cmd_dat)*10;
			      pgh52c0->savePara(&g_AirInterfaceData[cmd_group-1].g_AirColdData);//掉电保存
			      g_AirWriteFlag = 1;
			      airIdx = cmd_group-1;
			      airAddr = cmd_group+9;
		}
		else if(cmd_type==0x81)//高温告警点
		{
			      g_AirInterfaceData[cmd_group-1].g_AirTempData  = *((float*)&cmd_dat)*10;
			      pgh52c0->savePara(&g_AirInterfaceData[cmd_group-1].g_AirTempData);//掉电保存
			      g_AirWriteFlag = 2;
			      airIdx = cmd_group-1;
			      airAddr = cmd_group+9;
		}
		
		
	  SetDataLen(m_sdatlen);
		Set1363Packet();
		
}




void CYDT1363::set42AE(void)
{
	   SetOneByteData(60);
	
	   for(u8 i = 0 ; i<60 ;i++){//limitCap[5][12]
			 SetFourByteData( *((s16 *)limitCap+i));
			 
			 
		 }
	
	
///////////////////////////////数据打包///////////////////////////////////////////////////////////////////////////////
		   SetDataLen(m_sdatlen);
			 Set1363Packet();	
}

void CYDT1363::set42AF(void)
{
		      u32 cmd_dat=0;
          u8 cmd_type=0;
        
          if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
          if(AsciitoHex32L(&cmd_dat ,&m_recvdata[2])==0){ReturnVailData(6);return ;}
        
                float tmpdatfloat= *((float *)&cmd_dat);        
                u8 a=(cmd_type-1)/12;
                u8 b=(cmd_type-1)%12;                
                if (a >= 5 || b >= 12) {
                ReturnVailData(6); 
                return;
    }

                limitCap[a][b]=tmpdatfloat;
                pgh52c0->savePara(& limitCap[a][b]);
                
           SetDataLen(m_sdatlen);
           Set1363Packet();
	 
}



/////////////////////////遥控///////////////////////////////////////////
void CYDT1363:: set4145(void)
{
	  u8 cmd_type=0;
	  u8 cmd_id=0; 
	 
	
	     if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	     if(AsciitoHex(&cmd_id,&m_recvdata[2])==0){ReturnVailData(6);return ;}//无效命令;
	     
	
	       if(cmd_type==0x10)//转均充
				 {
					 pMonitor->m_workMode=EQUAL_MOD;	
				
				 }
				 else if(cmd_type==0x1F)//转浮充
				 {
					 pMonitor->m_workMode=FLOAT_MOD;	
				 
				 }
				 else if(cmd_type==0x11)//转测试
				 {
					 pMonitor->m_workMode=TEST_MOD;	
				
					 
					}
				 else if(cmd_type==0x20)//开机
				 {
						 
						
								if((cmd_id>ModuleOnlineMessage[2])||(cmd_id==0))
								{
									 m_rtn=6;
								}
								else
								{ u8 moduleNum=0;
									u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
										 m_rtn=0xE2;
										 for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历遍所有模块
											{
												if((moduletype[i]==ACDC)||(moduletype[i]==0x0F))//查找整流模块（在线与不在线）
													{
														 moduleNum++;
														
															if((moduleNum==cmd_id)&&(moduletype[i]==ACDC))//查找需要控制且在线的整流模块
															{
																	
																		 pcanruleln->SetModuleOnOff(i+1,0);//开关
																			m_rtn=0;

																	break;
															}
													}
											}
									

								}
					}
				 else if(cmd_type==0x2F)//关机
				 {
							 if((cmd_id>ModuleOnlineMessage[2])||(cmd_id==0))
								{
									 m_rtn=6;
								}
								else
								{
									u8 moduleNum=0;
									u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
										 m_rtn=0xE2;
										 for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历遍所有模块
											{
												if((moduletype[i]==ACDC)||(moduletype[i]==0x0F))//查找整流模块（在线与不在线）
													{
														 moduleNum++;
														
															if((moduleNum==cmd_id)&&(moduletype[i]==ACDC))//查找需要控制且在线的整流模块
															{
																	
																		 pcanruleln->SetModuleOnOff(i+1,1);//关机
																			m_rtn=0;

																	break;
															}
													}
											}
								}
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
					else
					{
						 ReturnVailData(6);return ;//无效命令;
					}
					/*
				 else if(cmd_type==0x40)//直流配电单元上电
				 {
						
								s8 branchCount=0,SearchCount=0;
						
										if((cmd_id>TOTAL_USER)||(cmd_id==0)){ReturnVailData(0xE6);return ;}
										
										
										branchCount = SwitchAddFromLogicToPysicl(cmd_id);
		
														if((branchCount == -1)||(branchCount>=TOTAL_BRANCH))
															{ReturnVailData(0xE6);return ;}//无数据;
											
											 gSetDownParaGroudType=0x51;//分组设置 单用户
											 gSetDownParaAddr=branchCount+1;//
							
							
												 gSwitchPara[SwitchSNtable[branchCount]].st_downMode&=0xF3;
												 gSwitchPara[SwitchSNtable[branchCount]].st_downMode|=0x08;
												 gSetDownParaCode=12;//下电模式命令码
							
							
							
							
						
						
					}
			   else if(cmd_type==0x4F)//直流配电单元下电
				 {
						
								s8 branchCount=0,SearchCount=0;
						
										if((cmd_id>TOTAL_USER)||(cmd_id==0)){ReturnVailData(0xE6);return ;}
										
												branchCount = SwitchAddFromLogicToPysicl(cmd_id);
		
														if((branchCount == -1)||(branchCount>=TOTAL_BRANCH))
															{ReturnVailData(0xE6);return ;}//无数据;
											
											
											
											 gSetDownParaGroudType=0x51;//分组设置 单用户
											 gSetDownParaAddr=branchCount+1;//
							
							
												 gSwitchPara[SwitchSNtable[branchCount]].st_downMode&=0xF3;
												 gSwitchPara[SwitchSNtable[branchCount]].st_downMode|=0x04;
												 gSetDownParaCode=12;//下电模式命令码
						
					}
				 else if(cmd_type==0x80)//重启
				 {
							SoftReset();//复位系统
					}
				 else if(cmd_type==0x8F)//恢复自动
				 {
					
							s8 branchCount=0,SearchCount=0;
					
									if((cmd_id>TOTAL_USER)||(cmd_id==0)){ReturnVailData(0xE6);return ;}
									
									
									   branchCount = SwitchAddFromLogicToPysicl(cmd_id);
	
	                   if((branchCount == -1)||(branchCount>=TOTAL_BRANCH))
										  {ReturnVailData(0xE6);return ;}//无数据;
										
										 gSetDownParaGroudType=0x51;//分组设置 单用户
										 gSetDownParaAddr=branchCount+1;//
						
						
						        	 gSwitchPara[SwitchSNtable[branchCount]].st_downMode&=0xF3;
								       gSetDownParaCode=12;//下电模式命令码
					
				}
         */

          SetDataLen(m_sdatlen);
			    Set1363Packet();

}
void CYDT1363:: set4345(void)
{
	

	  u8 cmd_type=0;
	  u8 cmd_id=0; 
	 
	
	     if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	     if(AsciitoHex(&cmd_id,&m_recvdata[2])==0){ReturnVailData(6);return ;}//无效命令;
	     
	
	      if(cmd_type==0x10)//开机
				 {
						 
						
								if((cmd_id>ModuleOnlineMessage[2])||(cmd_id==0))
								{
									 m_rtn=6;
								}
								else
								{ u8 moduleNum=0;
									u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
										 m_rtn=0xE2;
										 for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历遍所有模块
											{
												if((moduletype[i]==DCDC)||(moduletype[i]==0x0E))//查找整流模块（在线与不在线）
													{
														 moduleNum++;
														
															if((moduleNum==cmd_id)&&(moduletype[i]==DCDC))//查找需要控制且在线的整流模块
															{
																	
																		 pcanruleln->SetModuleOnOff(i+1,0);//开关
																			m_rtn=0;

																	break;
															}
													}
											}
									

								}
					}
				 else if(cmd_type==0x1F)//关机
				 {
							 if((cmd_id>ModuleOnlineMessage[2])||(cmd_id==0))
								{
									 m_rtn=6;
								}
								else
								{
									u8 moduleNum=0;
									u8  *moduletype=((u8*)pModuledata->GetDataAddr(MODULETYPE));
										 m_rtn=0xE2;
										 for(u8 i=0;i<ModuleOnlineMessage[2];i++)//历遍所有模块
											{
												if((moduletype[i]==DCDC)||(moduletype[i]==0x0E))//查找整流模块（在线与不在线）
													{
														 moduleNum++;
														
															if((moduleNum==cmd_id)&&(moduletype[i]==DCDC))//查找需要控制且在线的整流模块
															{
																	
																		 pcanruleln->SetModuleOnOff(i+1,1);//关机
																			m_rtn=0;

																	break;
															}
													}
											}
								}
					}
					else
					{
						 ReturnVailData(6);return ;//无效命令;
					}
		

          SetDataLen(m_sdatlen);
			    Set1363Packet();

}


void CYDT1363::setE1A0(void)
{
	
	  u8 cmd_type=0;
	  u8 cmd_id=0; 
	 
	
	     if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	     if(AsciitoHex(&cmd_id,&m_recvdata[2])==0){ReturnVailData(6);return ;}//无效命令;
			 
	       if(cmd_type==0x80)//重启
				 {
							SoftReset();//复位系统
					}
				 
					SetDataLen(m_sdatlen);
			    Set1363Packet();
}	
void CYDT1363::setE1A1(void)
{
	u8 str[32];
	  for(u8 i=0;i<32;i++)
	{
		  str[i]=0x20;
	}
	
	str[0]='H';
	str[1]='5';
	str[2]='2';
	str[3]='C';
	str[4]='0';
	
	for(u8 i=0;i<32;i++)
	{
		SetOneByteData(str[i]);
	}
	
	 float fdat=(float)(ModuleCap*ModuleOnlineMessage[1]);
	SetFourByteData(fdat);
	SetOneByteData(ModuleOnlineMessage[1]);
	 SetDataLen(m_sdatlen);
	 Set1363Packet();
}




void CYDT1363::set42A7(void)//错峰参数1获取
{
	 SetOneByteData(103);
	 SetFourByteData((float)*(s16 *)&gSetBattLimitPerCSpike/100);//尖峰时段充电系数 
	 SetFourByteData((float)*(s16 *)&gSetBattLimitPerCPeak/100);//高峰时段充电系数 
	 SetFourByteData(0);       //平段时段充电系数
	 SetFourByteData(0);       //低谷时段充电系数
	 SetFourByteData((float)eleRate[0]/10000);       //尖峰时段电价
	 SetFourByteData((float)eleRate[1]/10000);       //高峰时段电价
	 SetFourByteData((float)eleRate[2]/10000);       //平段时段电价
	 SetFourByteData((float)eleRate[3]/10000);       //低谷时段电价 
	 SetFourByteData(gWorkShiftMode);       //电池充电模式   //时段模块
	u32 tmp32=0;
	
    for(u8 i=0;i<5;i++)//起始时段 
	{
		  tmp32=0;
		  YeardayToDate(gsPeakTimeInteval[i].st_yday,(u8 *)&tmp32,1);
		  SetFourByteU32Data(tmp32);
	}
	
	for(u8 i=0;i<4;i++)//结束时段 
	 {
		  tmp32=0;
			YeardayToDate((gsPeakTimeInteval[i+1].st_yday-1),(u8 *)&tmp32,1);
			SetFourByteU32Data(tmp32);
				
	 }
	 
		tmp32=0;//结束时段 
		YeardayToDate((gsPeakTimeInteval[0].st_yday-1),(u8 *)&tmp32,1);
		SetFourByteU32Data(tmp32);
	 
	 
	    for(u8 j=0;j<5;j++)//时段类型选择  
			 {
							for(u8 i=0;i<12;i++)//时点
						{
								tmp32=0;
							  tmp32=GetSpeakFlag(j,i);    
								SetFourByteData(tmp32);			
						 }//32
			}//80
	 
	 
	 
	 
	     for(u8 i=0;i<12;i++)//时段1起点时间 81
			 {   	
				  tmp32=0;
					GetTimeInMin(gsPeakTimeInteval[0].st_time[i],(u8 *)&tmp32,1);
					SetFourByteU32Data(tmp32);		//时段1起点时间     
								
			 }
	 
	      for(u8 i=0;i<11;i++)
				{   
					  tmp32=0;
						GetTimeInMin(gsPeakTimeInteval[0].st_time[i+1]-1,(u8 *)&tmp32,1);
						SetFourByteU32Data(tmp32);//时段1起点时间     
									
				
				}
	 
	       tmp32=0;
				 GetTimeInMin(gsPeakTimeInteval[0].st_time[0]-1,(u8 *)&tmp32,1);
				 SetFourByteU32Data(tmp32);//时段1起点时间     
	
	
	
	 SetDataLen(m_sdatlen);
	 Set1363Packet();
	
}



void CYDT1363::set42A8(void)//错峰参数2获取
{
                 

                   u32 tmp32=0;
  
                           SetOneByteData(98);
                          
                        
                          for(u8 j=0;j<4;j++)//第2，3，4，5大段
                         { 
                                  for(u8 i=0;i<12;i++)//时段1起点时间 81
                                                                                {   
                                                                                        tmp32=0;
                                                                                        
                                                                                          GetTimeInMin(gsPeakTimeInteval[j+1].st_time[i],(u8 *)&tmp32,1);
                                                                                                SetFourByteU32Data(tmp32);//时段1起点时间     
                                                                                        
                                                                                 }
                                                

                                                                                  for(u8 i=0;i<11;i++)
                                                                                {   
                                                                                          tmp32=0;
                                                                                          GetTimeInMin(gsPeakTimeInteval[j+1].st_time[i+1]-1,(u8 *)&tmp32,1);
                                                                                                SetFourByteU32Data(tmp32);//时段1起点时间     
                                                                                                                
                                                                                
                                                                                }
                                                                        
                                                                                tmp32=0;
                                                                                   GetTimeInMin(gsPeakTimeInteval[j+1].st_time[0]-1,(u8 *)&tmp32,1);
                                                                                         SetFourByteU32Data(tmp32);//时段1起点时间  
                         }
                         
                         

                                        SetFourByteData((float)gsafedisV/100);
                                        SetFourByteData((float)gsafedisV/100);
                         
        
                         
                         SetDataLen(m_sdatlen);
             Set1363Packet(); 
                         

        }




void CYDT1363::set42A9(void)//获取削峰参数
{
        
                         SetOneByteData(8);
                   SetOneByteData(gPeakShaveOnoff);
                         SetOneByteData(0);
                         SetFourByteData(3000);// SetFourByteData((float)gSinglePhasePower/10);
                    SetFourByteData((float)gAPhasePower/10);
                    SetFourByteData((float)gBPhasePower/10);
                    SetFourByteData((float)gCPhasePower/10);
                    SetFourByteData(220);
                    SetFourByteData(1);
                    SetOneByteData(0);
                    
                   SetDataLen(m_sdatlen);
             Set1363Packet(); 
                        
}		
		
		

void CYDT1363:: set42AA(void)//设置错峰参数1
{
	    u32 cmd_dat=0;
	    u8 cmd_type=0;
	
	  if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	  if(AsciitoHex32L(&cmd_dat ,&m_recvdata[2])==0){ReturnVailData(6);return ;};
		
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
											  
											     SetSpeakFlag(0,cmd_type-0x23,(SpeakFlag)tmpdatfloat);
											  pgh52c0->savePara(& gSpeakFlag[0*6+(cmd_type-0x23)/2]);
										}
										else if((cmd_type>=0x2F)&&(cmd_type<=0x3A))//时段2选择
										{
											  
											     SetSpeakFlag(1,cmd_type-0x2F,(SpeakFlag)tmpdatfloat);
											  pgh52c0->savePara(& gSpeakFlag[1*6+(cmd_type-0x2F)/2]);
										}
											else if((cmd_type>=0x3B)&&(cmd_type<=0x46))//时段3选择
										{
											  
											     SetSpeakFlag(2,cmd_type-0x3B,(SpeakFlag)tmpdatfloat);
											  pgh52c0->savePara(& gSpeakFlag[2*6+(cmd_type-0x3B)/2]);
										}
										else if((cmd_type>=0x47)&&(cmd_type<=0x52))//时段4选择
										{
											  
											     SetSpeakFlag(3,cmd_type-0x47,(SpeakFlag)tmpdatfloat);
											  pgh52c0->savePara(& gSpeakFlag[3*6+(cmd_type-0x47)/2]);
										}	
						        else if((cmd_type>=0x53)&&(cmd_type<=0x5E))//时段5选择
										{
											  
											     SetSpeakFlag(4,cmd_type-0x53,(SpeakFlag)tmpdatfloat);
											  pgh52c0->savePara(& gSpeakFlag[4*6+(cmd_type-0x53)/2]);
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
						
		
	        SetDataLen(m_sdatlen);
			     Set1363Packet();
		
		
}

void CYDT1363:: set42AB(void)//设置错峰参数2
{
	   
	    u32 cmd_dat=0;
	    u8 cmd_type=0;
	
	  if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	  if((AsciitoHex32L(&cmd_dat ,&m_recvdata[2])==0)&&(cmd_type!=0x10)&&(cmd_type!=0x11)){ReturnVailData(6);return ;};
		
		float tmpdatfloat= *((float *)&cmd_dat);	
		u8  tmpdatu8= *((u8 *)&cmd_dat);	
		
		
		  if(cmd_type==0x10)//削峰开关 
								 {
									     if(AsciitoHex(&tmpdatu8,&m_recvdata[2])==0){ReturnVailData(6);return ;}
									     gPeakShaveOnoff=tmpdatu8;
									      pgh52c0->savePara(&gPeakShaveOnoff);
									 
								 }
								 else if(cmd_type==0x11)//削峰配置 
								 {

								 }
								 else if(cmd_type==0x12)//单相模式总功率 
								 {
//									    gSinglePhasePower=tmpdatfloat*10;
//									     pgh52c0->savePara(&gSinglePhasePower);
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
								 
								 
							SetDataLen(m_sdatlen);
			        Set1363Packet();
		
		
}


void CYDT1363::setXXD4F(void)//获取告警事件记录
{
	  u8 cmd_type=0;
	  u8 tmpwarn[12];
		
	  s16 getaddr=0;
	 if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	 
	 
	    if(cmd_type==0)
			{
				 m_warnCount[0]=0;//取第一条
			}
			else if(cmd_type==1)
			{
				 m_warnCount[0]++;//取下一条
			}
	    else if(cmd_type==2)
			{
				 if( m_warnCount[0]>0)
				  m_warnCount[0]--;//取上一条
				 
			}
			
			 RecordInfo Record={ &recordAlarmCount,&galarmRecordMax,&m_warnCount[0]};						
			 getaddr=RecordOrderNumToGetAddr(&Record,ALARM_RECORD_LEN);
		  	tmpwarn[0]=0;
			 
			 	 if(m_warnCount[0]==m_warnCount[1])//最后一条
			 {
				 tmpwarn[0]=1; 
				 m_warnCount[0]--;
				 
			 }
			 
			 pgh52c0->readI2C(ALARM_DATA_STAR+12*getaddr,&tmpwarn[6],6);//获取当记录时间
						      
						        tmpwarn[1]=((uTIME *)(&tmpwarn[6]))->mon;
						        tmpwarn[2]=((uTIME *)(&tmpwarn[6]))->day;
						        tmpwarn[3]=((uTIME *)(&tmpwarn[6]))->hour;
						        tmpwarn[4]=((uTIME *)(&tmpwarn[6]))->min;
						        tmpwarn[5]=((uTIME *)(&tmpwarn[6]))->sec;
			 
			  pgh52c0->readI2C(ALARM_DATA_STAR+6+12*getaddr,&tmpwarn[6],2);//获取告警数据
	           
			 
			            SetOneByteData(tmpwarn[0]);
			            SetTowByeteData(((uTIME *)(&tmpwarn[6]))->year);
									for(u8 i=1;i<8;i++)
								 {
									 SetOneByteData(tmpwarn[i]);
								 }
	 
	               SetDataLen(m_sdatlen);
			           Set1363Packet();
	         
	
}
void CYDT1363::setXXD8F(void)//模拟量校准
{
	   u8 cmd_type=0;
	   u8 cmd_ch=0;
	   u32 cmd_dat=0;
	  if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
		if(AsciitoHex(&cmd_ch,  &m_recvdata[2])==0){ReturnVailData(6);return ;}//无效命令;
	  if(AsciitoHex32L(&cmd_dat ,&m_recvdata[4])==0){ReturnVailData(6);return ;}
		if(cmd_type==0)//电压
		{
       

						 psysPara[VY]=*((float*)&cmd_dat)*100;
						 psysPara[VX]=Yout[2];
														 
//			       gyl[3]=0;
//						 gxl[3]=0;
//			

//			      pgh52c0->writeI2C(AD_PARA+2+3*12, (u8 *)& gyh[3], 2);
//						pgh52c0->writeI2C(AD_PARA+4+3*12, (u8 *)& gxh[3], 4);
			  
			
		}
		else if(cmd_type==1)//电流
		{
			
			      if(cmd_ch==1)     //通道2 电池电流1
						{
							  psysPara[BATT3_ZERO]=pgh52c0->setIvalue(*((float*)&cmd_dat)*10,0);
							
							  //pgh52c0->writeI2C(AD_PARA+10+2*12,(u8 *)&gxl[2], 4);
						}
						else if(cmd_ch==2)//通道4 电池电流2
						{
							
							    psysPara[BATT4_ZERO]=pgh52c0->setIvalue(*((float*)&cmd_dat)*10,1);
							  //pgh52c0->writeI2C(AD_PARA+10+4*12,(u8 *)&gxl[4], 4);
						}
						else if(cmd_ch==3)//通道0高点 用户电流1 
						{
//							   gyh[0]= *((float*)&cmd_dat)*10;
//								 gxh[0]=AD_channel[0];	
//							
//							   pgh52c0->writeI2C(AD_PARA+2+0*12, (u8 *)& gyh[0], 2);
//								pgh52c0->writeI2C(AD_PARA+4+0*12, (u8 *)& gxh[0], 4);
							
						}
						else if(cmd_ch==4)//通道1高点 用户电流2 
						{
//							  gyh[1]= *((float*)&cmd_dat)*10;
//								gxh[1]=AD_channel[1];	
//							
//							  pgh52c0->writeI2C(AD_PARA+2+1*12, (u8 *)& gyh[1], 2);
//								pgh52c0->writeI2C(AD_PARA+4+1*12, (u8 *)& gxh[1], 4);
							
						}
						else if(cmd_ch==13)//通道0低点 用户电流1 
						{
//							   gyl[0]= *((float*)&cmd_dat)*10;
//								 gxl[0]=AD_channel[0];	
//							
//							   pgh52c0->writeI2C(AD_PARA+8+0*12, (u8 *)& gyl[0], 2);
//								pgh52c0->writeI2C(AD_PARA+10+0*12, (u8 *)& gxl[0], 4);
							
						}
						else if(cmd_ch==14)//通道1低点 用户电流2 
						{
//							  gyl[1]= *((float*)&cmd_dat)*10;
//								gxl[1]=AD_channel[1];	
//							
//							  pgh52c0->writeI2C(AD_PARA+8+1*12, (u8 *)& gyl[1], 2);
//								pgh52c0->writeI2C(AD_PARA+10+1*12, (u8 *)& gxl[1], 4);
							
						}
						else if(cmd_ch==21)//通道2 分流器值
						{
							  psysPara[BATT3_RANGE]=*((float*)&cmd_dat)/100;
							  //pgh52c0->savePara(&range[0]);
						}
						else if(cmd_ch==22)//通道4 分流器值
						{
							  psysPara[BATT4_RANGE]=*((float*)&cmd_dat)/100;
							  //pgh52c0->savePara(&range[1]);
							
						}
						
							
			
		}
		
//		u8 adflag[2]={0};
//		if(*((u16 *)adflag)!=0xa5a5)
//		{
//				 adflag[0]=0xa5;
//				 adflag[1]=0xa5;
//			 pgh52c0->writeI2C(AD_PARA,adflag,2);//写入读写标志位
//			
//		}
		
		  SetDataLen(m_sdatlen);
		  Set1363Packet();
		
}

void CYDT1363::setXX4DF(void)//获取设备时间
{
	
	    SetOneByteData(*(((u8 *)&gTimer.year)+1));//年
	    SetOneByteData(*((u8 *)&gTimer.year));
	    SetOneByteData(gTimer.mon);
	    SetOneByteData(gTimer.day);
	    SetOneByteData(gTimer.hour);
	    SetOneByteData(gTimer.min);
	    SetOneByteData(gTimer.sec);
	
	    SetDataLen(m_sdatlen);
		  Set1363Packet();
}

void CYDT1363::setXX4EF(void)//设置设备时间
{
			 u8  Mon=0;
			 u8  Day=0;
			 u8  Hour=0;
			 u8  Min=0;
			 u8  Sec=0;
			 u16 Year=0;
	
	    if( AsciitoHex16(&Year,&m_recvdata[0]) ==0){ReturnVailData(6);return ;}
	    if(AsciitoHex(&Mon,&m_recvdata[4])==0){ReturnVailData(6);return ;}
		  if(AsciitoHex(&Day,&m_recvdata[6])==0){ReturnVailData(6);return ;}
			if(AsciitoHex(&Hour,&m_recvdata[8])==0){ReturnVailData(6);return ;}
			if(AsciitoHex(&Min,&m_recvdata[10])==0){ReturnVailData(6);return ;}
			if(AsciitoHex(&Sec,&m_recvdata[12])==0){ReturnVailData(6);return ;}
			
			if((Year<1900)&&(Year>2200)){ReturnVailData(6);return ;}
			if((Mon>12)||(Mon==0)){ReturnVailData(6);return ;}
			if((Day>31)||(Day==0)){ReturnVailData(6);return ;}
			if(Hour>23){ReturnVailData(6);return ;}
			if(Min>59){ReturnVailData(6);return ;}
			if(Sec>59){ReturnVailData(6);return ;}
			
			RTC_Set(Year,Mon,Day,Hour,Min,Sec);
			
			SetDataLen(m_sdatlen);
		  Set1363Packet();

}

void CYDT1363::setXXDDF(void)//设置设备信息
{
	 u8 cmd_type=0;
	 if(AsciitoHex(&cmd_type,&m_recvdata[0])==0){ReturnVailData(6);return ;}//无效命令;
	 
	 if(cmd_type==0x80)
	 {
					for(u8 i=0;i<16;i++)
				 {
						AsciitoHex(&gmSN[i],&m_recvdata[(i+1)*2]);
				 }
				 gmSN[16]=0;
				 pgh52c0->writeI2C(SYS_INFO_START,gmSN,17);
	 }
	 else if(cmd_type==0x81)
	 {
		 for(u8 i=0;i<16;i++)
					{
						
						AsciitoHex(&gsSN[i],&m_recvdata[(i+1)*2]);
					}
					gsSN[16]=0;
					pgh52c0->writeI2C(SYS_INFO_START+17,gsSN,17);
	 }
	 else if(cmd_type==0x82)
		{
					 for(u8 i=0;i<6;i++)
					{
				
						AsciitoHex(&gMAC[i],&m_recvdata[(i+1)*2]);
					}
					
					pgh52c0->writeI2C(SYS_INFO_START+34,gMAC,6);
		}
		else
		{m_rtn=6;}
			
			 SetDataLen(m_sdatlen);
	     Set1363Packet();
	
}

	u8 a1,a2,a3;

void CYDT1363::setXX51F(void)//工厂信息
{
	

	u8 moduleName[30];
	u8 factory[20];

	
	
	for(u8 i=0;i<30;i++)
	{
		  moduleName[i]=0x20;
	}
	
		
	for(u8 i=0;i<20;i++)
	{
		factory[i]=0x20;
	}
   
	sprintf((char *)moduleName,(const char *)"H52C0");
	
	
	if(sysPara[SELECT_FACT] == 0)
	{
	    sprintf((char *)factory,(const char *)"ZTKD");
	}
	else
	{
		 sprintf((char *)factory,(const char *)"Mentech");
	}
 
	
	moduleName[strlen((const char *)moduleName)]=0x20;
	factory[strlen((const char *)factory)]=0x20;	
	//gsw[strlen((const char *)gsw)]=0x20;//字符串无结束符，导致字符串长度获取函数返回错误，引起数组访问越界，引起重启
	
	
	for(u8 i=0;i<30;i++)//1、	采集器名称
	{
		SetOneByteData(moduleName[i]);
	}
	
	for(u8 i=0;i<7;i++)//版本号
	{
		SetOneByteData(gsw[i]);
	}
	

	for(u8 i=0;i<13;i++)//版本号
	{
		SetOneByteData(0x20);
	}
	
	
	for(u8 i=0;i<20;i++)//厂家名称
	{
		   SetOneByteData(factory[i]);
	}

	
   SetDataLen(m_sdatlen);
	 Set1363Packet();
}






