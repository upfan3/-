//#include "globalval.h"
#include "cydt1363.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//const u8 strDevice[]="H52C0\0";
//#define DEBUG_1363

#define FONT_DATA_BASE 0x08040000
extern u16 FONT_LEN;//读取汉字长度

extern u32 ASC2_1206;//ASCII字模起始位置 固定为偏地址后8个字节
extern u32 FONT_1206;//汉字模起始位置 //总共95个ascii码，每个占9个字节 再加上8字节信息码
extern u32 FONT_TXT;//起始位置第4个字节，指定了汉字内码的偏移地址

extern u8 send1363buf[2048];


const char updatahead[]="MNC-EN";
const char CurrVer[]="0.0";
const char updataOK[]="{\"reply\":\"OK\"}";
const char updataErr[]="{\"reply\":\"ERROR\",\"type\":";
const char updataCRCErr[]="\"CheckSum Error\"}";
const char updataLen[]="\"Length Error\"}";
const char updataPacketErr[]="\"Current Packet Error\"}";
const char updataCmdErr[]="\"Command Error\"}";
u8 *strDatatype=NULL;
u8 *strVer=NULL;
u8 *strCmd=NULL;
u32 updataflashaddr;
UPDATA_LOG_ST updatalog;	
s8 getInfodata(u8 *str)
{  u8 *tmphead=NULL;
	   tmphead = (u8 *)strstr((const char *)str,"H52C0");
	  if(tmphead==NULL) return -1;//不是本监控型号
      tmphead = (u8 *)strstr((const char *)str,"Data_Type");

	                    strtok(( char *)tmphead ,":\"");//Data_Type
	  strDatatype=(u8 *)strtok(NULL ,":\", ");////Data_Type内容
	                    strtok(NULL ,":\", ");//Ver
	       strVer=(u8 *)strtok(NULL ,":\", ");//Ver内容
	                    strtok(NULL ,":\", ");//Command
	       strCmd=(u8 *)strtok(NULL ,":\", ");//Command内容
	
	      return 0;
	
}
	
s8 CheckCrc16(u8 *addr,u32 num,u8 *pcrc)
{
	unsigned short CRC_Temp = 0xFFFF; 
    u8 i;
    //u16 num=1043;
    
    while (num--) 
    {
      CRC_Temp ^= *addr++; 
      for (i = 0; i < 8; i++) 
      { 
          if (CRC_Temp & 1) 
          { 
              CRC_Temp >>= 1; 
              CRC_Temp ^= 0xA001; 
          } 
          else
          { 
              CRC_Temp >>= 1; 
          } 
       } 
     } 
    
		 if(CRC_Temp==*((u16 *)pcrc))
		 {
			   return 0;
			 
		 }
		 else
		 {
			   *((u16 *)pcrc)=CRC_Temp;
			   return -1;
		 }
 
		 
		 
		 
		 

}

void SaveDataToFlash(u16 num,u8 *pdata)
{ 
	 
	   //fmc_unlock();
	if(updataflashaddr>=0x8080000)//超过512K
	{
	   if((num-1)%4==0)//每4K擦除一次
	   {
			  fmc_page_erase(updataflashaddr+((num-1)*1024));
		 }
	}
	else 
	{
		  if((num-1)%2==0)//每2K擦除一次
	   {
			  fmc_page_erase(updataflashaddr+((num-1)*1024));
		 }
	}
		 
		 for( u16 i=0;i<256;i++)
		 {
			 fmc_word_program((updataflashaddr+((num-1)*1024)+i*4),*((u32 *)pdata+i));
		 }
		 
	
}


u8 * ReplyUpData(u8 cmd, u8* asw,u16 * _plen)
{ u8 *psend=NULL,tt_size,_lenth,tt_size1=0;
	   *_plen=0;
	     if(asw==NULL)//返回0K
			 {
				  if((cmd==UPDATE_INIT)||(cmd==UPDATE_DOING))
					{tt_size=strlen(updataOK);
						_lenth=19+tt_size+3;
//				    psend=(u8*)pvPortMalloc(_lenth);//获取空间					
//						if(psend==NULL) return 0;
						psend=send1363buf;//数据发送缓冲
					 memset(psend,0,_lenth);	//清零
					 memcpy(psend,updatahead,6);//写入头
					 memcpy(&psend[19],updataOK,tt_size);//回复 "reply":"Ok"
						
						
					}
				 
			 }
			 else//返回Error
			 {
				 
				   tt_size=strlen(updataErr);
				   tt_size1=strlen((const char *)asw);
						_lenth=19+tt_size+tt_size1+3;
//				   psend=(u8*)pvPortMalloc(_lenth);//获取空间
//				   if(psend==NULL) return 0;
				   psend=send1363buf;//数据发送缓冲
					 memset(psend,0,_lenth);	//清零
					 memcpy(psend,updatahead,6);//写入头
					 memcpy(&psend[19],updataErr,tt_size);//回复 "reply":"ERROR"
				   strcat((char *)&psend[19],(const char *)asw);//写入错误类型
				 
				 
				 
				 
			 }
	
	
	
	    *((u16*)&psend[6])=1;//填充版本号
				psend[8]=cmd;//命令
			*((u16*)&psend[9])=_lenth;//长度
			  CheckCrc16(psend,_lenth-3,&psend[19+tt_size+tt_size1]);//设算校验和，_lenth-3扣除两字节CRC及0D，校验和数据回写到&psend[19+tt_size+tt_size1]
		  *((u16*)&psend[_lenth-1])=0x0D;
	    *_plen=_lenth;//数据长度
			 return psend;
}





u8 isTieta=0;
u8 UpdateFlag=0;
u16 PackNum;
u16 packlen=0;
u8 upDataErrCount=0;
u8 isUnlockFlash=0;
u8 isUpDataTimeOut=10;


void UplogGenerate(u8 _time,u8 _err)
{
	
	if(_time==0)
	{
		     updatalog.stime.st_year=gTimer.year;
				 updatalog.stime.st_mon=gTimer.mon;
				 updatalog.stime.st_day=gTimer.day;
				 updatalog.stime.st_hour=gTimer.hour;
				 updatalog.stime.st_min=gTimer.min;
				 updatalog.stime.st_sec=gTimer.sec;
	}
	else if(_time==1)
	{
		     updatalog.etime.st_year=gTimer.year;
				 updatalog.etime.st_mon=gTimer.mon;
				 updatalog.etime.st_day=gTimer.day;
				 updatalog.etime.st_hour=gTimer.hour;
				 updatalog.etime.st_min=gTimer.min;
				 updatalog.etime.st_sec=gTimer.sec;
	}
	
	
	if(_err==0){
		
		updatalog.verOle=VerNB;
	}
	else if(_err==1)
	{
		updatalog.verOle=0xE000;
	}
	
	
	
	
}

void SaveUpDataLog(void)
{
	
	u16 _flag=0;
		pgh52c0->readI2C2(UPDATA_LOG,(u8 *)&_flag,2);
		if(_flag==0x5a5a)
		{
			u16 len=0;
			pgh52c0->readI2C2(UPDATA_LOG+2,(u8 *)&len,2);
			
		//	if(len>=5)len=0;
			pgh52c0->writeI2C2(UPDATA_LOG_START+14*(len%5),(u8 *)&updatalog,14);
			
			len++;
			pgh52c0->writeI2C2(UPDATA_LOG+2,(u8 *)&len,2);
			
		}
		else
		{ _flag=0x5a5a;
			u16 len=1;
			  pgh52c0->writeI2C2(UPDATA_LOG,(u8 *)&_flag,2);
			  pgh52c0->writeI2C2(UPDATA_LOG+2,(u8 *)&len,2);
			  pgh52c0->writeI2C2(UPDATA_LOG_START,(u8 *)&updatalog,14);
			
		}
}

void ErrEscUpData(void)
{
	   if(isUnlockFlash==1)
		{
				fmc_lock();//上锁flash
		}

		   UplogGenerate(1,1);
		   SaveUpDataLog();
		
		
	
		
		
		
		

		UpdateFlag=0;
}




u8 * UpDateFlash(u8 *pdat,u16 * _plen,u8 ch)
{
	u8 *psend=NULL;
	UpDateInfo *upInfo;
	upInfo=(UpDateInfo *)&pdat[7];//由于字节对齐，要取命令字段的前一个字节

	isUpDataTimeOut=10;//升级通信超时
	if(upInfo->upcmd==UPDATE_INIT)//dat的内存空间由外部函数释放
	{
		u8 *pdata=&pdat[19];

		   if(getInfodata(pdata)==-1)return 0;//型号不对结束升级
		
			 if(strDatatype==NULL) return 0;//无效类型结束升级
		   if(strcmp((const char *)strDatatype,"Code")==0)
			 {
				  UplogGenerate(0,2);
				 updataflashaddr=0x8080000;//程序备份空间
			 }
			 else if(strcmp((const char *)strDatatype,"HTML")==0)
			 {
				 updataflashaddr=0x8045000;//网页空间
			 }
		    else if(strcmp((const char *)strDatatype,"Font")==0)
			 {
				 updataflashaddr=0x8040000;//字库空间,若20K
				  UplogGenerate(0,2);
			 }
			 else
			 {
				  return 0;//无效类型结束升级
			 }
			if(strVer==NULL) return 0;//无效版本号结束升级
			 
      if(strcmp((const char *)strVer,CurrVer)<=0)	return 0;	//若升级版本少于等于当前版本，结束本次升级
			 
				if(strCmd==NULL) return 0;   //无效命令号结束升级 
			 if(strcmp((const char *)strCmd,"Upgrade")==0)
			 {
				      UpdateFlag=ch;//1;//将通信设置为升级模式（数字节结束一帧），非升级模式为0D结束一帧


						  packlen=1046;//设置接收包长为1046
				     psend= ReplyUpData(UPDATE_INIT, NULL, _plen);//回复 "reply":"Ok"
				      upDataErrCount=0;//超时清零
				      PackNum=1;//数据包序号初始化
				     fmc_unlock();//解锁flash
			
				 
				     
			 }	
       else
			 {
				 
				      
				       psend= ReplyUpData(UPDATE_INIT, (u8 *)updataCmdErr, _plen);//类型,校验错误 回复 "reply":"ERROR","Command Error"
				    //无效命令号结束升级 
			 }				 
				 
			 
		
		
		
		
		
	}
	else if((upInfo->upcmd==UPDATE_START)&&(UpdateFlag>0))
	{
	}
	else if((upInfo->upcmd==UPDATE_DOING)&&(UpdateFlag>0))
	{
		
		    if(CheckCrc16(&pdat[19],1024,&pdat[1043])==0)//升级帧每帧长度为1045
				{
					
					    if(PackNum==upInfo->cpcount)//目标包正确
							{
								   
								   SaveDataToFlash(PackNum,&pdat[19]);
                    isUnlockFlash=1;//已经开始升级，已解锁flash,并擦除改写部分flash
								    psend= ReplyUpData(UPDATE_DOING, NULL, _plen);//回复 "reply":"Ok"
								    upDataErrCount=0;//超时清零
								 
										if(upInfo->tpcount==PackNum)//若当前包为最后一包，更改接收包长为22
										{
											 packlen=22;//校验包长为22
										}	
								    PackNum++;
							}
							else//目标包不正确
							{
                     upDataErrCount++;
								    if(upDataErrCount>10)//错误数超限
										{
											     ErrEscUpData();
											     psend[0]='\0';
													
										}
										else//未超限，则正常返回
										{
								       psend=     ReplyUpData(UPDATE_DOING, (u8 *)updataPacketErr, _plen);//类型,校验错误 回复 "reply":"ERROR","Current Packet Error"
										}
							}
					
					
				}
				else
				{
                     upDataErrCount++;
								    if(upDataErrCount>10)//错误数超限
										{
											    ErrEscUpData();
											    psend[0]='\0';
													
										}
										else//未超限，则正常返回
										{
					            psend=  ReplyUpData(UPDATE_INIT, (u8 *)updataCRCErr, _plen);//类型,校验错误 回复 "reply":"ERROR","CheckSum Error"
										}
				}
		
	}
	else if((upInfo->upcmd==UPDATE_STOP)&&(UpdateFlag>0))
	{
		if((PackNum-1)==upInfo->tpcount)//目标总包数正确  (（PackNum-1）:因为最后一包PackNum多加一）
				{
					   
					 if(CheckCrc16((u8 *)updataflashaddr,(PackNum-1)*1024,&pdat[19])==0)//升级帧每帧长度为1045
					 {
					    fmc_lock();//上锁flash

						 UpdateFlag=0;
						 if(updataflashaddr==0x8080000)//执行区的升级才重启
						 {
						    bkp_data_write(BKP_DATA_20,0x5A5A);//在备份寄存，设置标志位
								bkp_data_write(BKP_DATA_21,upInfo->tpcount);//在备份寄存器，保存总包数  
							   UplogGenerate(1,0);
							  SaveUpDataLog();
							 SoftReset();//重启CPU
						 }
						 else if(updataflashaddr==0x8040000)//字库
						 {
							//#define FONT_DATA_BASE 0x08040000
									FONT_LEN=*(u16 *)FONT_DATA_BASE;//读取汉字长度

									ASC2_1206=FONT_DATA_BASE+8;//ASCII字模起始位置 固定为偏地址后8个字节
									FONT_1206=FONT_DATA_BASE+8+95*9;//汉字模起始位置 //总共95个ascii码，每个占9个字节 再加上8字节信息码
									FONT_TXT=FONT_DATA_BASE+(*((u16 *)FONT_DATA_BASE+3));//起始位置第4个字节，指定了汉字内码的偏移地址
							  UplogGenerate(1,0);
							  SaveUpDataLog();
							 
						 }

					 }
				}
				
		
	}
	else if(upInfo->upcmd==UPDATE_ERR)
	{
            
		                upDataErrCount++;
								    if(upDataErrCount>10)//错误数超限
										{
											    ErrEscUpData();
											     psend[0]='\0';
													
										}//未超时，则正常返回错误
										else
										{
		                     psend= ReplyUpData(UPDATE_INIT, (u8 *)updataLen, _plen);//类型,长度错误   回复 "reply":"ERROR","Length Error"
										}

		
	}
	
	return psend;
}



void * DuleWithData(u8 *pdat,u16 *plen)
{

	u8 *psend;
	
	if(UpdateFlag==1)
	{
		psend= UpDateFlash(pdat,plen,1);
		return psend;
		
	}	
	else if(pdat[0]==0x7E)
	{
		if(com1363.Get1363Data(pdat)==0)
			return com1363.m_senddata;//返回错误信息(长度或校验错误）
		com1363.DuleWith1363Data();
		  return com1363.m_senddata;//返回打包信息
	}
	else if((pdat[0]=='M')&&(pdat[1]=='N')&&(pdat[2]=='C')&&(pdat[3]=='-')&&(pdat[4]=='E')&&(pdat[5]=='N'))
	{
		if(UpdateFlag==0)//未执行升级时才执行
		{
			psend=UpDateFlash(pdat,plen,1);
			return psend;
		}
		else
		{
				psend[0]='\0';//使数据长度为0
				return psend;
		}
	}

	return 0;
}



