#include "canruleln.h"
#include "globalval.h"
#include "rtc.h"
#define USE_OPT 0//定议是否使用光伏

u32 romveModuleMasklist;
u32  moduleswitchstatus=0;
u32  modulelimitstatus=0;

u8 testaddr=0;
//u8 phudate=0;

u16 gVac[7]={0,0,0,0,0,0,0};
extern u16 gsafedisV;
extern u8 gmoduleInputLowV;
extern u8 genableTP;
extern u8 gmoduleSN[17];
extern u8 pronlineNum ;
extern u16 gOcrdiffV;  //光伏电压差

//extern u8 startCmdTP;
s8 gmoduleTPdeay=-1;//启动防盗延时，每发现一个新模块，延时值复位

u8 glockDelayTime=0;
CanRuleLn gCanRuleLn;
CanRuleLn* pcanruleln=&gCanRuleLn;

u32 gTotalOcrEnergy=0;
u32 gTotalMrEnergy=0;
u32 gTotalOilEnergy=0;
//u32 gDayOcrEnergy=0;
//u32 gMonOcrEnergy=0;
//u32 gYearOcrEnergy=0;


CanRuleLn::CanRuleLn()
{
	 m_psn=NULL;
	  m_MoudleCount=0;
	  m_MoudleAccCount=0;
	  m_ModuleAddrlist=0;
	  m_ModuleAddrlistOK=0;
	  m_WaitTimer=WAIT_TIME;
	m_isCmd=0;
	m_setAddr=0;
	m_setCmd=-1;
  m_WaitResponse=0;
//	m_setPowerLimitFlag=0;
	  m_cmdlist[0]= CMD_GET_OUT_V;
	  m_cmdlist[1]= CMD_GET_OUT_I;
	  m_cmdlist[2]= CMD_GET_MODULE_STATUS;
	  m_cmdlist[3]= CMD_GET_AC_V;
	  m_cmdlist[4]= CMD_GET_AC_C;
	  m_cmdlist[5]= CMD_GET_AC_F;
	  m_cmdlist[6]= CMD_SET_OUT_VOLTAGE;
	  m_cmdlist[7]= CMD_SET_CURRENT_LIMIT;
	  m_cmdlist[8]= CMD_GET_PS;
	  m_cmdlist[9]= CMD_GET_AC_F;
		m_cmdlist[10]= CMD_GET_TMP;
    m_cmdlist[11]= CMD_GET_SN;
		m_cmdlist[12]=CMD_GET_INFO;
		
		
		m_cmdlist[13]=CMD_GET_POWER;
		m_cmdlist[14]=CMD_GET_ENERGY;
		
    m_cmdlist[15]= CMD_SET_ONOFF;
    m_cmdlist[16]= CMD_SET_LED_FLASH;
		
	  m_setData=0;
	  m_floatorequal=FLOAT_MOD;
	
	 m_vfloat=53500;//浮充电压
	 m_vequal=54500;//均充电压
	 m_vshift=47500;//错峰安全电压
    m_vtest=47500;//测试电压
	 m_Ilimit=55000;//整流模块限流电流
	 m_OCIlimit=55000;//光伏模块限流电流
   m_prol=2;
	 m_cmd=0;
	 
	  m_ptotalBuf=(LeiNengSN *)pvPortMalloc (sizeof(LeiNengSN)*MODULE_NUM);
	 for(u8 i=0;i<MODULE_NUM;i++)
	 {
		 m_ptotalBuf->st_snH=0;
		 m_ptotalBuf->st_snL=0;
	 }
	 
}


void CanRuleLn::Init(u8 _prol)
{
//	 m_psn=NULL;
//	  m_MoudleCount=0;
//	  m_MoudleAccCount=0;
//	  m_ModuleAddrlist=0;
//	  m_ModuleAddrlistOK=0;
//	  m_WaitTimer=WAIT_TIME;
//	m_isCmd=0;
//	m_setAddr=0;
//	m_setCmd=-1;
//  m_WaitResponse=0;

//	  m_cmdlist[0]= CMD_GET_OUT_V;
//	  m_cmdlist[1]= CMD_GET_OUT_I;
//	  m_cmdlist[2]= CMD_GET_MODULE_STATUS;
//	  m_cmdlist[3]= CMD_GET_AC_V;
//	  m_cmdlist[4]= CMD_GET_AC_C;
//	  m_cmdlist[5]= CMD_GET_AC_F;
//	  m_cmdlist[6]= CMD_SET_OUT_VOLTAGE;
//	  m_cmdlist[7]= CMD_SET_CURRENT_LIMIT;
//	  m_cmdlist[8]= CMD_GET_PS;
//	  m_cmdlist[9]= CMD_GET_AC_F;
//		m_cmdlist[10]= CMD_GET_TMP;
//    m_cmdlist[11]= CMD_GET_SN;
//    m_cmdlist[12]= CMD_SET_ONOFF;
//    m_cmdlist[13]= CMD_SET_LED_FLASH;
//		
//	  m_setData=0;
//	  m_floatorequal=FLOAT_MOD;
//	
//	 m_vfloat=53500;//浮充电压
//	 m_vequal=54500;//均充电压
//     m_vtest=47500;//测试电压
//	 m_Ilimit=55000;//整流模块限流电流
//	 m_OCIlimit=55000;//光伏模块限流电流
//   m_prol=_prol;
//	 m_cmd=0;
}

//u8 test456;
CanRuleLn::~CanRuleLn()
{ LeiNengSN *ptem=NULL;
	
	   while(m_psn!=NULL)
		 {
			 ptem=m_psn;
			 m_psn=ptem->st_pNext;
			 vPortFree(ptem);
		 }
	
}







LeiNengSN *  CanRuleLn::DeletSNList( LeiNengSN *psn)
{
		
	
	 if(psn->st_pPre!=NULL)//对当前模块序列号信息节点进行移除
		 { 
			 psn->st_pPre->st_pNext=psn->st_pNext;//将当前节点的前向节点的后向节点指针改为，当前节点的后向节点。
			
			if(psn->st_pNext!=NULL)//当前节点后向节点不为空
			 psn->st_pNext->st_pPre=psn->st_pPre; //将当前节点的后向节点的前向节点指针改为，当前节点的前向节点。
			 
		
			 LeiNengSN *pdel=psn;//暂存当前节点指节以用删除，释放内存
				psn=psn->st_pNext;//将当前节点指针，指向其后向节点
				//vPortFree(pdel);//释放当前模块序列号信息节点内存
			   pdel->st_snH=0;//（节点，设为未使用）序列号字段为空表示该节点未使用
			   pdel->st_snL=0;//（节点，设为未使用）序列号字段为空表示该节点未使用
			
				pdel=NULL;
		 }
		 else//若前向指针为空，则表示当前为第一个,因此需修改m_psn
		 {
			
			 m_psn=psn->st_pNext;//将当前节点的后向节点，传给首节点 
			 if(psn->st_pNext!=NULL)//当前节点后向节点不为空
			 psn->st_pNext->st_pPre=NULL;//将当前节点的后向节点的前向节点清空，作为首节点
	
			 //vPortFree(psn);
			 psn->st_snH=0;//（节点，设为未使用）序列号字段为空表示该节点未使用
			 psn->st_snL=0;//（节点，设为未使用）序列号字段为空表示该节点未使用
		
			 psn=m_psn;//将当前节点指针，指向首节点
		 }
		 return psn;

}


//u8 addrtmp=0;

LeiNengSN *CanRuleLn::ApplySNList(void)
{
	   for(u8 i=0;i<MODULE_NUM;i++)
	   {
			  if((m_ptotalBuf[i].st_snH==0)&&(m_ptotalBuf[i].st_snL==0))
				{
					 return &m_ptotalBuf[i];
				}
		 }
		 return 0;
		 
}

extern u32 limitIaddr;
u8 counterSn=0;
u8 CanRuleLn::DuleWithData(PCanBuf pCanbuf,DATABASE *pdatabase)
{
	LeiNengSN *pSn=m_psn;
   Lnid * pid=(Lnid *)&pCanbuf->id;
	 Lndata *plndat=(Lndata *)pCanbuf->data;
//	 u8 _ret;


	  
	
	      if((((pid->st_protocol)==NORMAL)&&((pid->st_systemType)==ACDC))||
					 (((pid->st_protocol)==NORMAL)&&((pid->st_systemType)==DCDC))
				  )
				{
					if(plndat->st_mType==READ_SERIAL_RESPONSE)//读模块序列号响应
					{
						
							if((plndat->st_cmdType==0)&&(plndat->st_cmdData==0)) 
							{	
							
								  return RECV_SN_RROR ;//排除序列号为0的情况
								
							}
						
					      
					     while(pSn!=NULL)
							 {	//testpoint=2;

								   if((pSn->st_snH==plndat->st_cmdType)&& //收到模块广播序列号已存在
									 (pSn->st_snL==plndat->st_cmdData))
									 { 
											u8 addr=pSn->st_addr;
										 
												if(
															( ((((u32)1<<(addr-1))&m_ModuleAddrlist)!=0)&& //已分配模块地址
																((((u32)1<<(addr-1))&m_ModuleAddrlistOK)==0) //模块不响应地址分配指令
															)
														//	||(pid->st_sAddr==0xEF)//模块对应分配地址掉失,地址恢复为默认值EF
													)
															
												{
														
																if(pid->st_sAddr==addr)//应对模块已成功分配，但监控收不到模块地址分配响应的情况
														 {
															 
															 m_ModuleAddrlistOK|=((u32)1<<(addr-1));//写入地址成功分配表对应位
															 romveModuleMasklist=m_ModuleAddrlistOK;
															 if(m_MoudleAccCount<m_MoudleCount)
																m_MoudleAccCount++;//模块有效数加1
															// break;
															 
																 u8 *	pv8dat=((u8 *)pdatabase->GetDataAddr(ONOFF));
																 pv8dat[addr-1]=0;//开机
															 
															 
														 }
														else//模块地址分配失败，无响应返回
														{
															//	testpoint=3;
															DeletSNList(pSn);
															 u8 *pv8dat_type=((u8 *)pdatabase->GetDataAddr(MODULETYPE));//将模块类型暂存OTHERS项中
															 pv8dat_type[addr-1]=0x0F;//将对应模块类型变为0，表示该模块无效
															 m_ModuleAddrlist&=~((u32)1<<(addr-1));//清除地址分配表对应位
															 m_ModuleAddrlistOK&=~((u32)1<<(addr-1));//清除OK列表对应位
															if(m_MoudleCount>0)
															 m_MoudleCount--;//模块存活数减1
															//break;
														}
													
												}
												else if( ((((u32)1<<(addr-1))&m_ModuleAddrlist)!=0)&& //已分配模块地址
																 ((((u32)1<<(addr-1))&m_ModuleAddrlistOK)!=0) //模块已响应地址分配指令
																)   
												{
														pSn->st_onlinetimer=ONLINE_TIME;//侦测到有模块，复位时间计数
														 if(pid->st_sAddr!=addr)//模块已存信息已存在，获取模块ID地址与记录地址不致
																{
																	 
																	//地址设置打包
																	// pSn->st_onlinetimer=ONLINE_TIME;//侦测到有模块，复位时间计数
																	 pid->st_dAddr=BROADCAST;
																	 pid->st_sAddr=MONITOR_S;
																	 pid->st_Snl=0;
																	 pid->st_rev=0;
																	 
																	 plndat->st_mType=SET_MODULE_ADDR;
																	 plndat->st_mAddrandFrame= addr;//重新设置模块地址
																	 //序列号已在pCanbuf->data中不用再打包
																		 return SET_ADDR ; 
																}
													
												}
												return MENERROR;
									 }//有相同序列号返回
									 if(pSn->st_pNext!=NULL)
									 {
										 pSn= pSn->st_pNext;//指向序列号列表的下一项	
									 }
									 else
									 {        //找到最后节点
										 break;//当前节点的后前节点为空，则当前节点为最后点节，
									 }
							 }
						  if(pSn==NULL)//第一次检测到模块
							{
								//pSn=(LeiNengSN *)pvPortMalloc (sizeof(LeiNengSN));//伸请内存
								pSn=ApplySNList();//伸请内存 静态
								if(pSn==NULL)return MENERROR;
								m_psn=pSn;
								pSn->st_pPre=NULL;
							}
						  else
							{
								//pSn->st_pNext=(LeiNengSN *)pvPortMalloc (sizeof(LeiNengSN));//伸请内存
								pSn->st_pNext=ApplySNList();//伸请内存 静态
								if(pSn->st_pNext==NULL)return MENERROR;
								pSn->st_pNext->st_pPre=pSn;
								pSn=pSn->st_pNext;
								//testpoint=9;
							}
							 if(pSn!=NULL)
							 {
								   pSn->st_snH=plndat->st_cmdType;//存入序列号高16位
								   pSn->st_snL=plndat->st_cmdData;//存入序列号低32位
								   pSn->st_pNext=NULL;//将指向下一序号表项指针清空
								   pSn->st_onlinetimer=FIRST_ONLINE_TIME;
								 
								 
								 for(u8 i=0;i<32;i++)
								 {  //testpoint=4;
									    if((((u32)1<<i)&m_ModuleAddrlist)==0)//找到未分配的地址
											{
												  pSn->st_addr=i+1;//设置地址
												if(m_MoudleCount<30)
												m_MoudleCount++; 
												 m_ModuleAddrlist|=((u32)1<<i);//将地址列表对应位置1，表示该位置已占用
												//testpoint=10;
												break;
											}
								 }
								 
								 
								 {//地址设置打包
									 pid->st_dAddr=BROADCAST;
									 pid->st_sAddr=MONITOR_S;
								   pid->st_Snl=0;
								   pid->st_rev=0;
									 
									 plndat->st_mType=SET_MODULE_ADDR;
									 plndat->st_mAddrandFrame= pSn->st_addr;
									 //序列号已在pCanbuf->data中不用再打包
	 
									 
								 }
								 								 
							 }
							  return SET_ADDR ; 
							 
					}
					
					else if(plndat->st_mType==SET_DATA_RESPONSE)//设置模块数据据响应
					{  //  testpoint=5;
						  if(bswap_16(plndat->st_cmdType)==CMD_SET_ADDR)
						  { testaddr=pid->st_sAddr-1;
														
								m_ModuleAddrlistOK|=((u32)1<<(bswap_32(plndat->st_cmdData)-1));//写入地址成功分配表对应位
								romveModuleMasklist=m_ModuleAddrlistOK;
				 
							//	testpoint=11;
								if(m_MoudleAccCount<m_MoudleCount)
								m_MoudleAccCount++;//模块有效数加1
								
								u8 *	pv8dat=((u8 *)pdatabase->GetDataAddr(ONOFF));
								pv8dat[testaddr]=0;//开机
								pv8dat=((u8 *)pdatabase->GetDataAddr(PHASEPOSITION));//获取物理地址
								pv8dat[testaddr]=0;//清除物理地址
								gmoduleTPdeay=5;
								 
	
					     }
          	 return _NONE_;
					}
					else if(plndat->st_mType==READ_DATA_RESPONSE)//读模块数据据响应
					{   
						
					    if(bswap_16(plndat->st_cmdType)==CMD_GET_OUT_V)//获得模块输出电压
						  {//testpoint=20;
								       testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
												if(pdatabase!=NULL)
												{
													
												     u16 *pvdat=((u16 *)pdatabase->GetDataAddr(VOLTAGE));
													   u32 tmpdat=bswap_32(plndat->st_cmdData);
													  pvdat[testaddr]=(u16)(tmpdat/10);
													
												
												
												     u8 *pv8dat=((u8 *)pdatabase->GetDataAddr(MODULETYPE));//将模块类型暂存OTHERS项中
												      pv8dat[testaddr]=pid->st_systemType;//记录模块类型
												
												}
								
												while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												 pSn=pSn->st_pNext;
											 }
											 
					     }
							else if(bswap_16(plndat->st_cmdType)==CMD_GET_OUT_I)//获得模块输出电流
							{//testpoint=21;
								       testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								        if(pdatabase!=NULL)
												{
													
												     u16 *pvdat=((u16 *)pdatabase->GetDataAddr(CURRENT));
													   u32 tmpdat=bswap_32(plndat->st_cmdData);
													   pvdat[testaddr]=(u16)(tmpdat/10);
													
												}
								
								       while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
							}
							else if(bswap_16(plndat->st_cmdType)==CMD_GET_AC_V)//获得交流电压
							{//testpoint=22;
								       testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								       
								         if(pdatabase!=NULL)
												{
													
												     u16 *pvdat=((u16 *)pdatabase->GetDataAddr(VOLTAGE_AC));
													   u32 tmpdat=bswap_32(plndat->st_cmdData);
													       tmpdat=tmpdat/10;
													  pvdat[testaddr]=(u16)tmpdat;
													

													    u8 phaseaddr;
													    u8 *pv8dat=((u8 *)pdatabase->GetDataAddr(PHASEPOSITION));//获取物理地址
													    
													    u8 *pv8dat_type=((u8 *)pdatabase->GetDataAddr(MODULETYPE));//将模块类型暂存OTHERS项中
													
													if(pv8dat_type[testaddr]==ACDC)//只有在整流模块类形下，才读取交流信息。
													{

														
														     if(pv8dat[testaddr]>0)//物理地址存在
																{
																			phaseaddr=pv8dat[testaddr]%3;//物理地址存在的取物理地址 0默认为c
																		 if((u16)tmpdat<8000) 
																				tmpdat=0;
																			gVac[phaseaddr]=(u16)tmpdat;  //0 C 1A 2B
													      }
																
													}
													
												}
								
								
								      while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
							}
							else if(bswap_16(plndat->st_cmdType)==CMD_GET_AC_C)//获得交流电流
							{//testpoint=23;
                        testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								
								        if(pdatabase!=NULL)
												{
													
												     u16 *pvdat=((u16 *)pdatabase->GetDataAddr(CURRENT_AC));
													   u32 tmpdat=bswap_32(plndat->st_cmdData);
													      // tmpdat=tmpdat;
													   if((s32)tmpdat<80000)//单模块输入交流电流不超过25A
													  pvdat[testaddr]=(u16)tmpdat;
														 if((s32)tmpdat<0)// 负数设为0
													  pvdat[testaddr]=0;
													
												}
								
								
								        while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
							} 
					
              else if(bswap_16(plndat->st_cmdType)==CMD_GET_MODULE_STATUS)//获得模块状态
							{//testpoint=24;
								
								        testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								         if(pdatabase!=NULL)
												{
													

												   u32 tmpdat=bswap_32(plndat->st_cmdData);

													
													 u8 *pModuleWRANNING=((u8 *)pdatabase->GetDataAddr(WRANNING));
													    if((tmpdat&0xF4F9FFFF)!=0)//有告警 
															{  
																 pModuleWRANNING[testaddr]|=0x01;//告警

																
															}
															else
																 pModuleWRANNING[testaddr]&=0xFE;//告警
															
															
														  u8 *	 pv8dat=((u8 *)pdatabase->GetDataAddr(ONOFF));//开关状态
															         if(((tmpdat>>25)&0x01)!=0)//开关机状态
																			 {
																				 moduleswitchstatus|=((u32)1<<testaddr);
																				  pv8dat[testaddr]=1;
																			 }
																			 else
																			 {
																				 moduleswitchstatus&=(~((u32)1<<testaddr));
																				 pv8dat[testaddr]=0;
																			 }  
															
													    
															
															 pv8dat=((u8 *)pdatabase->GetDataAddr(LIMIT_CURR_STATUS));
															         if(((tmpdat>>17)&0x03)!=0)//限流
																			 {
																				 modulelimitstatus|=((u32)1<<testaddr);
																				 pv8dat[testaddr]=0;
																				
																			 }
															         else
																			 {
																				 modulelimitstatus&=(~((u32)1<<testaddr));
																           pv8dat[testaddr]=1;
																			 }
																			 
																
												              	pv8dat=((u8 *)pdatabase->GetDataAddr(PHASEPOSITION));			 
																				 
															         if((((u8)(tmpdat>>4))&0x01)!=0)//模块低压
																			 {
																				
																						
																				    if( ( pv8dat[testaddr]%3==1))
																						{
																							  gmoduleInputLowV|=0x01;
																							  
																						}
																				    else if( ( pv8dat[testaddr]%3==2))
																						{
																							  gmoduleInputLowV|=0x02;
																							  
																						}
																						else if( ( pv8dat[testaddr]%3==0)&&( pv8dat[testaddr]!=0))
																						{
																							 gmoduleInputLowV|=0x04;
																							
																						}
																							
																				 
																			 }
																			 else
																			 {
																				  // gmoduleInputLowV&=~((u8)1<<testaddr);
																				   if( ( pv8dat[testaddr]%3==1))
																						{
																							  gmoduleInputLowV&=0xFE;
																							 
																						}
																				    else if( ( pv8dat[testaddr]%3==2))
																						{
																							  gmoduleInputLowV&=0xFD;
																							 
																						}
																						else if( ( pv8dat[testaddr]%3==0)&&( pv8dat[testaddr]!=0))
																						{
																							 gmoduleInputLowV&=0xFB;
																							 
																						}
																			 }
															
															
															
																		// }
													
												}
								      while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
							}
               else if(bswap_16(plndat->st_cmdType)==CMD_GET_PS)//获得模块物理地址
							{//testpoint=25;
								        testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								     if(pdatabase!=NULL)
												{
													
												     u8 *pvdat=((u8 *)pdatabase->GetDataAddr(PHASEPOSITION));//模块状态取用4个字节长度存放
													   u32 tmpdat=bswap_32(plndat->st_cmdData);
													      // tmpdat=tmpdat;
													  pvdat[testaddr]=(u8)tmpdat;
													
												}
								    while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
											// phudate=1;
							}			
							else if(bswap_16(plndat->st_cmdType)==CMD_GET_AC_F)//获得模块频率
							{//testpoint=26;
								    testaddr=pid->st_sAddr-1;
								    if(testaddr>30) return RULEERROR;
								 if(pdatabase!=NULL)
									{

										u16 *pvdat=((u16 *)pdatabase->GetDataAddr(FREQUENCY));
													   u32 tmpdat=bswap_32(plndat->st_cmdData);
													  pvdat[testaddr]=(u16)tmpdat;
										       
										 u8 *pv8dat_type=((u8 *)pdatabase->GetDataAddr(MODULETYPE));//将模块类型暂存OTHERS项中
													
													if(pv8dat_type[testaddr]==ACDC)//只有在整流模块类形下，才读取交流信息。
													{
										        gVac[3]=(u16)tmpdat/10;
													}
										
										
										
									}
								    while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
							}
							else if(bswap_16(plndat->st_cmdType)==CMD_GET_TMP)//获得模块温度
							{//testpoint=27;
								       testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								 if(pdatabase!=NULL)
									{
										u16 *pvdat=((u16 *)pdatabase->GetDataAddr(TEMP));
													   s32 tmpdat=bswap_32(plndat->st_cmdData);
										         
										          
													  pvdat[testaddr]=(u16)(tmpdat/10);
									}
									
									
									
									 //  testpoint=29;
									
								    while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
											 
											// 	testpoint=30;
											 
							}
							
							 
				      else if(bswap_16(plndat->st_cmdType)==CMD_GET_INFO)//获得模块信息
							{
								    testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								
								      if(pdatabase!=NULL)
									{
										u8 *pvdat=((u8 *)pdatabase->GetDataAddr(MODULE_RATE_I));
													   u32 tmpdat=bswap_32(plndat->st_cmdData);
										         // testpoint=28;
										          
													  pvdat[testaddr]=(u8)(tmpdat>>16);
										   pvdat=((u8 *)pdatabase->GetDataAddr(MODULE_EFFICIEN));
										         pvdat[testaddr]=(u8)tmpdat;
										
									}
									
									
								
							}
							 else if(bswap_16(plndat->st_cmdType)==CMD_GET_POWER)//获得模块功率
							{
								    testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								u8 *pv8dat_type=((u8 *)pdatabase->GetDataAddr(MODULETYPE));//模块类型
							 
								   if(pdatabase!=NULL)
									{
										u16 *pvdat=((u16 *)pdatabase->GetDataAddr(POWER));
													   u32 tmpdat=bswap_32(plndat->st_cmdData);
								
										          
													  pvdat[testaddr]=tmpdat/1000;
										  
										
									}
									
						
									
									 while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
									
									
								
							}
							
							
							
							
							else if(bswap_16(plndat->st_cmdType)==CMD_GET_ENERGY)//获得模块电能
							{
								    testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								
								      if(pdatabase!=NULL)
									{
										u8 *pv8dat_type=((u8 *)pdatabase->GetDataAddr(MODULETYPE));//模块类型
										if(pv8dat_type[testaddr]==DCDC)//光伏时才统计总光伏电能
										{ 
										
														u32 *pvdat=((u32 *)pdatabase->GetDataAddr(ENERGY1));//OLD
													
										 
													   u32 tmpdat=bswap_32(plndat->st_cmdData);//NEW 取0.001kwh为单位
								               
										          if((tmpdat>=pvdat[testaddr])&&pvdat[testaddr]!=0)//新值大于旧值 pvdat[testaddr]!=0排除监控重启时，光伏电能非零时，重复计入初始值
															 {
																 
																 
																   
																 
																 	
																				 gTotalOcrEnergy += (tmpdat-pvdat[testaddr]);//光伏总电能  
																           
																 
																					 pvdat[testaddr]=tmpdat;
																						 bkp_data_write(BKP_DATA_16, *(u16 *)&gTotalOcrEnergy);
																						 bkp_data_write(BKP_DATA_17,*((u16 *)&gTotalOcrEnergy+1));
																		
																  
															 } 
															 else
															 {
																   pvdat[testaddr]=tmpdat;
															 }
										
										
										}
										else if(pv8dat_type[testaddr]==ACDC)
										{
											
											  u32 *pvdat=((u32 *)pdatabase->GetDataAddr(ENERGY1));//OLD
														
										 
													   u32 tmpdat=bswap_32(plndat->st_cmdData);//NEW   取0.1KWH为单位
								               
										          if((tmpdat>=pvdat[testaddr])&&pvdat[testaddr]!=0)//新值大于旧值 pvdat[testaddr]!=0排除监控重启时，整流电能非零时，重复计入初始值
															 {
																 
																 
																   
																        
																 	      if(GetWarnInPut(OILINPUT,1)==1)
					                              {
																					 gTotalOilEnergy += (tmpdat-pvdat[testaddr]);//统计油机电能
																				}
																				else
																				{
																				   gTotalMrEnergy += (tmpdat-pvdat[testaddr]);//整流模块电能  
																				}
																 
																					 pvdat[testaddr]=tmpdat;
																						 bkp_data_write(BKP_DATA_12, *(u16 *)&gTotalMrEnergy);
																						 bkp_data_write(BKP_DATA_13,*((u16 *)&gTotalMrEnergy+1));
																		
																  
															 } 
															 else
															 {
																   pvdat[testaddr]=tmpdat;
															 }
											
										}
										          
													 
										  
										
									}
									
									 while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
									
									
								
							}
							else if(bswap_16(plndat->st_cmdType)==100)//获得绑定状态
							{
								     testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								
								        if(pdatabase!=NULL)
												{
													    u32 tmpdat=bswap_32(plndat->st_cmdData);
													    u8 *pModuleWRANNING=((u8 *)pdatabase->GetDataAddr(WRANNING));
													
													  pModuleWRANNING[testaddr]&=0x0F;       
													  pModuleWRANNING[testaddr]|=( tmpdat<<4);//保存绑定状态
													
												}
												 while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
							}
							else if(bswap_16(plndat->st_cmdType)==101)//获得绑定信息
							{
								
								      								     testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								
								        if(pdatabase!=NULL)
												{
													  if(plndat->st_mAddrandFrame<=3)//电源模块流水号
														{
													     memcpy(&gmoduleSN[plndat->st_mAddrandFrame*4],&plndat->st_cmdData,4);
														}
													  else if((plndat->st_mAddrandFrame>3)&&(plndat->st_mAddrandFrame<=7))//客户标识
														{
													     memcpy(&gmoduleSN[(plndat->st_mAddrandFrame-4)*4],&plndat->st_cmdData,4);
														}
														else if(plndat->st_mAddrandFrame>7)//位监控模块流水号
														{
													     memcpy(&gmoduleSN[(plndat->st_mAddrandFrame-8)*4],&plndat->st_cmdData,4);
														}
												
												}
												while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
							}
							else if(bswap_16(plndat->st_cmdType)==105)//取得模块锁定延时
							{
								         testaddr=pid->st_sAddr-1;
								       if(testaddr>30) return RULEERROR;
								
								
								      glockDelayTime=*((u8 *)&plndat->st_cmdData+3);
								
								
								
								       while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }
							}
							
							
							while(pSn!=NULL)//模块在线定时器复位
											 {
												 if(pid->st_sAddr==pSn->st_addr)
												 {
													 pSn->st_onlinetimer=ONLINE_TIME;
													 break;
												 }
												  pSn=pSn->st_pNext;
											 }




					}
				
					
					return RULEERROR;
				}	
  
				  return _NONE_;	
	          
}	

void  CanRuleLn::ModuleOnLineDetect(DATABASE *pdatabase)
{
	
	 LeiNengSN *psn=m_psn;    
      u8 addr;

	    while(psn!=NULL)
			{      addr=psn->st_addr;
				   if((((u32)1<<(addr-1))&m_ModuleAddrlistOK)!=0)//检测对应模块地址是否有效，若已在OK列表（m_ModuleAddrlistOK）登记（对应位置1）
					 {
  						 psn->st_onlinetimer--;        // 则进行存活倒计时(若收到模块序号广播，则存活倒计数复位)   
					 }	

					 //移除掉线模块的信息
					 if(psn->st_onlinetimer==(u16)0)//掉线超时
					 {
						 
						 u8 *pv8dat_type=((u8 *)pdatabase->GetDataAddr(MODULETYPE));//将模块类型暂存OTHERS项中
						 //将模块所有相位的的交流电压清零
						   u8 *pv8dat=((u8 *)pdatabase->GetDataAddr(PHASEPOSITION));//获取物理地址
						   u8 *pv8dat_limit=((u8 *)pdatabase->GetDataAddr(LIMIT_CURR_STATUS));//获取物理地址
						    u16 * Vdata=(u16 *)pdatabase->GetDataAddr(VOLTAGE);
						   // u16 *pvdat=((u16 *)pdatabase->GetDataAddr(CURRENT_AC));
						 if(pv8dat_type[addr-1]==ACDC)//只有在整流模块类形下，才读取交流信息。
						{

									 if(pv8dat[addr-1]>0)//物理地址存在
									{
											u8	phaseaddr=pv8dat[addr-1]%3;//物理地址存在的取物理地址 0默认为c
												gVac[phaseaddr]=0;  
											 // pv8dat[addr-1]=0;//清除物理地址
									}
									
									gVac[3]=0;
							pv8dat_type[addr-1]=0x0F;//将对应模块类型变为0，表示该模块无效
									
						}
						else if(pv8dat_type[addr-1]==DCDC)
						{
							pv8dat_type[addr-1]=0x0E;
						}
						 
                  //  pvdat[addr-1]=0;//将对应模块交流电流清零
						  
							pv8dat_limit[addr-1]=1;	//清零限流标志		,不限流为1	
              Vdata[addr-1]=0;
              						
						 
						  m_ModuleAddrlistOK&=~((u32)1<<(addr-1));//清除OK列表对应位
						  m_ModuleAddrlist&=~((u32)1<<(addr-1));//清除地址分配表对应位
						 
							if(m_MoudleCount>0)
						   m_MoudleCount--;//模块存活数减1
                psn=DeletSNList(psn);		 
					 }
					 else
					 {
						 psn=psn->st_pNext;
					 }
			}



  
	
	
}

u8 prol=ACDC;

bool  CanRuleLn::PollingModuleCmd(PCanBuf pCanbuf)
{
    //static	LeiNengSN *psn=m_psn;
	  Lnid * pid=(Lnid *)&pCanbuf->id;
		Lndata *plndat=(Lndata *)pCanbuf->data;  
	
	      

	      if(m_WaitTimer>0)
					m_WaitTimer--;//让出总线时间倒计时
				
		
			if((m_ModuleAddrlistOK!=0)&&(m_WaitTimer==0))
			{
	       
#ifdef USE_OPT				
					  prol++;
				   if(prol==OVER_DET)
					 {
#endif
						 prol=ACDC;
						 m_cmd++;
				     if(m_cmd==CMD_END)m_cmd=0;

#ifdef USE_OPT						 
					 }
#endif
					 

            m_WaitTimer=WAIT_REC_TIME;//所有指令轮询一轮后，等待一段时间，让出总线
				  {//设置打包

						 pid->st_protocol=NORMAL;
						 pid->st_systemType=prol;
						 pid->st_dAddr=BROADCAST;//addr+1;
						
			
						 pid->st_sAddr=MONITOR_S;
						 pid->st_Snl=0;
						 pid->st_rev=0;
						 
						 plndat->st_eType=NO_ERROR;
						 plndat->st_mType=READ_DATA;
						 plndat->st_mAddrandFrame=0;
						 plndat->st_cmdType=bswap_16((u16) m_cmdlist[m_cmd]);//数据储存以小端模式，需转为大端模式发送
						 plndat->st_cmdData=0;	 
						
						  //if(cmd==SET_OUT_VOLTAGE)
						   if(m_cmd==SET_OUT_VOLTAGE)
							{												
								plndat->st_mType=SET_DATA;
								if(m_floatorequal==(u8)FLOAT_MOD)
								{
									if(prol==DCDC)
						      { 
										plndat->st_cmdData=bswap_32(m_vfloat+gOcrdiffV);//浮充电压
									}
									else
									{
										plndat->st_cmdData=bswap_32(m_vfloat);//浮充电压
										
									}
											
								}
								if(m_floatorequal==(u8)EQUAL_MOD)
								{
									if(prol==DCDC)
						      { 
										plndat->st_cmdData=bswap_32(m_vequal+gOcrdiffV);//均充电压
									}
									else
									{
										plndat->st_cmdData=bswap_32(m_vequal);//均充电压
										
									}
											
								}
							  else if(m_floatorequal==(u8)TEST_MOD)
								{
									  
								  plndat->st_cmdData=bswap_32(m_vtest);	//测试电压	
								}
								else if(m_floatorequal==(u8)TEST_MOD2)
								{
									 
									 plndat->st_cmdData=bswap_32(48000);//休眠时，调低电压，检测电池		
								}
								else if(m_floatorequal==(u8)PEAK_MOD)//错峰峰值时，调低模块电压
								{
									
									
									if(prol==ACDC)
									{
										
										 plndat->st_cmdData=bswap_32(m_vshift);	
										
									}
                  else if(prol==DCDC)
									{
										 
										 plndat->st_cmdData=bswap_32(m_vfloat+gOcrdiffV);
									}										
								}
								
								
							}
						  
							 
//							if(m_cmd==SET_CURRENT_LIMIT)
//							{
//								 if( setPowerLimitFlag>0) 
//								  return false;
//								 
//									plndat->st_mType=SET_DATA;
//								 if(prol==ACDC)
//								 {
//								    plndat->st_cmdData=bswap_32(m_Ilimit);	
//									 
//								 }
//								 else if(prol==DCDC)
//									 plndat->st_cmdData=bswap_32(m_OCIlimit);	 
//							}
							
				  }
				
				

						 return true;
					
			}

				
	
	
			return false;
	
}


bool  CanRuleLn::SendCmd(PCanBuf pCanbuf,DATABASE *pdatabase)//模块地址从1开始
{
		Lnid * pid=(Lnid *)&pCanbuf->id;
		Lndata *plndat=(Lndata *)pCanbuf->data;
	   u8 *pv8dat=((u8 *)pdatabase->GetDataAddr(MODULETYPE));//
	       if(m_setAddr>0)
          m_prol=pv8dat[m_setAddr-1];
				 else
					m_prol=ACDC;
	     if(m_isCmd==0)return false;
	       if(m_isCmd<0)return false;
	        if(m_setAddr>0)
					{
					 
	          if((((u32)1<<(m_setAddr-1))&m_ModuleAddrlistOK)==0)
							  return false;
					}
					else
					{
						;
					}
				  {//设置打包

						 pid->st_protocol=NORMAL;
						 pid->st_systemType=m_prol;//ACDC;
						
						 pid->st_dAddr=m_setAddr;
						
			
						 pid->st_sAddr=MONITOR_S;
						 pid->st_Snl=0;
						 pid->st_rev=0;
						 
						 plndat->st_eType=NO_ERROR;
						 plndat->st_mType=SET_DATA;
						 plndat->st_mAddrandFrame=0;
						 plndat->st_cmdType=bswap_16((u16) m_cmdlist[m_setCmd]);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_cmdData=bswap_32(m_setData);
           		
						
						m_setAddr=0;
						m_setCmd=-1;
						m_isCmd=0;
						m_setData=0;
             				
				  }
	        return true;		
	
}	


void SetLimitI(CanPort *pcan,u8 addr,float val)
{
			CanBuf canbuf;
			canbuf.ide = 1;
			canbuf.rtr = 0;
			canbuf.dlc = 8;
                 Lnid * pid=(Lnid *)&canbuf.id;
                Lndata *plndat=(Lndata *)canbuf.data;
        
          if(addr>0 && addr < 31){
           u8 *pv8dat=((u8 *)pModuledata->GetDataAddr(MODULETYPE));//
                 u8 *prateI=((u8 *)pModuledata->GetDataAddr(MODULE_RATE_I));
                        
     u8 prol=pv8dat[addr-1];
                        
                         if(val>(prateI[addr-1]*1100)){
                                 
                                  val = prateI[addr-1]*1100;
                         }
                        
            
           if(prol<2||prol>3) return;//非光伏整流模块不设置
                }
                else if(addr == 0){ 
                prol = ACDC;

      if(val>82500) val = 82500;                        
                        
                }//整流模块广播指令下发
                else if(addr == 255){ 
                        prol = DCDC;//光伏模块广播指令下发
                        addr = 0;
                        if(val>82500) val = 82500;
                        
                }
                
                
                
                
          
		{//设置打包

			 pid->st_protocol=NORMAL;
			 pid->st_systemType=prol;
			
			 pid->st_dAddr=addr;
			

			 pid->st_sAddr=MONITOR_S;
			 pid->st_Snl=0;
			 pid->st_rev=0;
			 
			 plndat->st_eType=NO_ERROR;
			 plndat->st_mType=SET_DATA;
			 plndat->st_mAddrandFrame=0;
			 plndat->st_cmdType=bswap_16(CMD_SET_CURRENT_LIMIT);//数据储存以小端模式，需转为大端模式发送        
			 plndat->st_cmdData=bswap_32(val);
		}                             
    
		pcan->SendData(&canbuf);
}










void CanRuleLn::OnTick(PCanBuf pCanbuf,DATABASE *pdatabase)
{
	
}


void  CanRuleLn::SetModuleOnOff(u8 sn,u8 act)
{
	m_setAddr=sn;
	m_setCmd=SET_ONOFF;
	m_isCmd=1;
	m_setData=act;//开机
}

extern u8 gCustom[17];
extern u8 gsSN[17];
u8 CanRuleLn::ModudleTP(PCanBuf pCanbuf,u8 n)//激活
{
	  Lnid * pid=(Lnid *)&pCanbuf->id;
		Lndata *plndat=(Lndata *)pCanbuf->data;
	
	      pid->st_protocol=NORMAL;
						 pid->st_systemType=m_prol;//ACDC;
						
	           
						 
						 //pid->st_dAddr=pid->st_sAddr;
	           pid->st_dAddr=0;
			
						 pid->st_sAddr=MONITOR_S;
						 pid->st_Snl=0;
						 pid->st_rev=0;
						 
						 plndat->st_eType=NO_ERROR;
						 plndat->st_mType=SET_DATA;
	         if(n==0)//同步帧1
					 {
						
						 plndat->st_mAddrandFrame=0;
						 plndat->st_cmdType=bswap_16((u16)104);//数据储存以小端模式，需转为大端模式发送	
						
						 u8 *tmp=(u8 *)&plndat->st_cmdData;
						     tmp[0]=0;
						     tmp[1]=gTimer.sec;
						     tmp[2]=gTimer.min;
						     tmp[3]=gTimer.hour;
						    
						 
					 }
					 else if(n==1)//同步帧2
					 {
						 plndat->st_mAddrandFrame=1;
						 plndat->st_cmdType=bswap_16((u16)104);//数据储存以小端模式，需转为大端模式发送	
						  u8 *tmp=(u8 *)&plndat->st_cmdData;
						     tmp[0]=gTimer.day;
						     tmp[1]=gTimer.mon;
						     *(u16 *)&tmp[2]=bswap_16(gTimer.year);
					 }
					 else if(n==2)//激活帧1
					 {
						 plndat->st_mAddrandFrame=0;
						 plndat->st_cmdType=bswap_16((u16) 102);//数据储存以小端模式，需转为大端模式发送	
					   plndat->st_cmdData=*(u32 *)&gCustom[0];
						    
					 }
					 else if(n==3)//激活帧2
					 {
						 plndat->st_mAddrandFrame=1;
						 plndat->st_cmdType=bswap_16((u16) 102);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_cmdData=*(u32 *)&gCustom[4];;
					 }
					  else if(n==4)//激活帧3
					 {
						 plndat->st_mAddrandFrame=2;
						 plndat->st_cmdType=bswap_16((u16)102);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_cmdData=*(u32 *)&gCustom[8];;
					 }
					 else if(n==5)//激活帧4
					 {
						 plndat->st_mAddrandFrame=3;
						 plndat->st_cmdType=bswap_16((u16) 102);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_cmdData=*(u32 *)&gCustom[12];;
					 }
					  else if(n==6)//激活帧5
					 {
						 plndat->st_mAddrandFrame=4;
						 plndat->st_cmdType=bswap_16((u16) 102);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_cmdData=*(u32 *)&gsSN[0];
					 }
					 else if(n==7)//激活帧6
					 {
						 plndat->st_mAddrandFrame=5;
						 plndat->st_cmdType=bswap_16((u16) 102);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_cmdData=*(u32 *)&gsSN[4];
					 }
					  else if(n==8)//激活帧7
					 {
						 plndat->st_mAddrandFrame=6;
						 plndat->st_cmdType=bswap_16((u16)102);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_cmdData=*(u32 *)&gsSN[8];
					 }
					 else if(n==9)//激活帧8
					 {
						 plndat->st_mAddrandFrame=7;
						 plndat->st_cmdType=bswap_16((u16) 102);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_cmdData=*(u32 *)&gsSN[12];
					 }
					  else if(n==10)//查询帧
					 {
						 plndat->st_mAddrandFrame=0;
						 plndat->st_cmdType=bswap_16((u16) 100);//数据储存以小端模式，需转为大端模式发送	
						 plndat->st_mType=READ_DATA;
						 plndat->st_cmdData=0;
						 
					 }
	
	
	
	return 0;
}
 
extern u8 *pdisDisPlayData;
extern u8  TPaddr;
u8 CanRuleLn::ModudleTP2(PCanBuf pCanbuf,u8 n)//解绑
{
	  
	  Lnid * pid=(Lnid *)&pCanbuf->id;
		Lndata *plndat=(Lndata *)pCanbuf->data;
	
	      pid->st_protocol=NORMAL;
				pid->st_systemType=m_prol;//ACDC;
						
						
	           pid->st_dAddr= TPaddr;//pdisDisPlayData[30];
	
	           pid->st_sAddr=MONITOR_S;
						 pid->st_Snl=0;
						 pid->st_rev=0;
						 
						 plndat->st_eType=NO_ERROR;
						 plndat->st_mType=SET_DATA;
	
	          if(n==0)
						{
							  plndat->st_mAddrandFrame=0;
						    plndat->st_cmdType=bswap_16((u16) 103);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=*(u32 *)&gCustom[0];
						}
						else if(n==1)
						{
							  plndat->st_mAddrandFrame=1;
						    plndat->st_cmdType=bswap_16((u16) 103);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=*(u32 *)&gCustom[4];
						}
	          	else if(n==2)
						{
                 plndat->st_mAddrandFrame=2;
						    plndat->st_cmdType=bswap_16((u16) 103);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=*(u32 *)&gCustom[8];
						}
	          	else if(n==3)
						{
							  plndat->st_mAddrandFrame=3;
						    plndat->st_cmdType=bswap_16((u16) 103);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=*(u32 *)&gCustom[12];
						}
	          	else if(n==4)//解绑口令1
						{
							  plndat->st_mAddrandFrame=4;
						    plndat->st_cmdType=bswap_16((u16) 103);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=*(u32 *)&pdisDisPlayData[22];
						}
							else if(n==5)//解绑口令1
						{   
							  plndat->st_mAddrandFrame=5;
						    plndat->st_cmdType=bswap_16((u16) 103);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=*(u32 *)&pdisDisPlayData[26];
						}
}

u8 CanRuleLn::ModudleTP3(PCanBuf pCanbuf,u8 n)//查询模块信息
{
	       Lnid * pid=(Lnid *)&pCanbuf->id;
		     Lndata *plndat=(Lndata *)pCanbuf->data;
	
	      pid->st_protocol=NORMAL;
				pid->st_systemType=m_prol;//ACDC;
	
	
	      pid->st_dAddr= TPaddr;//pdisDisPlayData[30];
	
	           pid->st_sAddr=MONITOR_S;
						 pid->st_Snl=0;
						 pid->st_rev=0;
						 
						 plndat->st_eType=NO_ERROR;
						 plndat->st_mType=READ_DATA;
	
	        
	          if(n==0)//查询模块流水号
						{
							  plndat->st_mAddrandFrame=0;
						    plndat->st_cmdType=bswap_16((u16) 101);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=0;
						}
						else if(n==1)//查询客户标识
						{
							  plndat->st_mAddrandFrame=4;
						    plndat->st_cmdType=bswap_16((u16) 101);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=0;
						}
	          	else if(n==2)//查询监控流水号
						{
                 plndat->st_mAddrandFrame=8;
						    plndat->st_cmdType=bswap_16((u16) 101);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=0;
						}
						else if(n==3)//查询模块延时
						{
							  plndat->st_mAddrandFrame=0;
						    plndat->st_cmdType=bswap_16((u16) 105);//数据储存以小端模式，需转为大端模式发送	
					      plndat->st_cmdData=0;
						}
						else if(n==4)//查询模块绑定状态
						{
							 plndat->st_mAddrandFrame=0;
						   plndat->st_cmdType=bswap_16((u16) 100);//数据储存以小端模式，需转为大端模式发送	
						  // plndat->st_mType=READ_DATA;
						   plndat->st_cmdData=0;
						}
             else if(n==5)//询置模块锁定延时
						{
							 plndat->st_mAddrandFrame=0;
						    plndat->st_cmdType=bswap_16((u16) 105);//数据储存以小端模式，需转为大端模式发送	
							  plndat->st_mType=SET_DATA;
							  m_setData=glockDelayTime;
					      plndat->st_cmdData=bswap_32(m_setData);
							    pid->st_dAddr= 0;//以广播型式设置所有模块
						}
	





}




