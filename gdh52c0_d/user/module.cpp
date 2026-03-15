#include "module.h"
#include "globalval.h"
u8 ModuleOnlineMessage[6]={1,0,0,0,1,0};;// 0: 1：模志实际在线数 2：已发现模块数 3：开关机状态 4：遍历模块序号 5：整流或光伏数目
u16 tatalRateI=0;
u8 ModuleCap;
s8 gmoduleTPdeay2=-1;
u8 TPaddr=0;
u16 gpower[3]={0,0,0};
u16 gDcpower=0;
u16 testI[32]={0};

extern u8 gmoduleSN[17];
Module gModule(ModuleOnlineMessage);

Module *pModule=&gModule;
Module::Module(u8 *paddr)
{
	m_pmoduleaddr=paddr;

}

Module::~Module()
{


}


void Module::SetPara(u16 *_pcurr,u8 * _pworkstatus,u8 *_psetflag,u8  *_prfreshdata)
{
	   m_pcurr=_pcurr;//总电流信息
	   m_pworkstatus=_pworkstatus;//限流信息
	   m_psetflag=_psetflag;//是否刷新模块数据
	   m_prfreshdata=_prfreshdata;//更新模块显示数据
} 

void Module::AssignAndGetData(CanRuleLn *pruleCtrl,DATABASE *pdata)
{
	static u16 lastaddr=0;
	static u8 lastlight=0,lightstatus=1;
	 u16 addr=0;
	   m_scrCount=0;
	   m_smrCount=0;
     tatalRateI=0;
	
	    	if(m_pmoduleaddr[4]>m_pmoduleaddr[2])
							       m_pmoduleaddr[4]=1;//当查找模块大于最大存在数时，变为第一个模块
										 addr=m_pmoduleaddr[4]-1;//模块地址与储存空间地址变换
	
	     m_ModuleCommStatus=((CanRuleLn *)pruleCtrl)->m_ModuleAddrlistOK;
	
	       u16 * vpdata=(u16 *)pdata->GetDataAddr(CURRENT);
				 u16 * vAccpdata=(u16 *)pdata->GetDataAddr(CURRENT_AC);
				 u16 * vfrqdata=(u16 *)pdata->GetDataAddr(FREQUENCY);//改为显示频率
				 u16 * vpowrdata=(u16 *)pdata->GetDataAddr(POWER);
					
				 u8 * typedata=(u8 *)pdata->GetDataAddr(MODULETYPE);
				 u8 *phadat=((u8 *)pdata->GetDataAddr(PHASEPOSITION));//获取物理地址
				 u8 *plimitdat=((u8 *)pdata->GetDataAddr(LIMIT_CURR_STATUS));
				 u8 *prateI=((u8 *)pdata->GetDataAddr(MODULE_RATE_I));
	
	     m_pcurr[2]=0;
			 m_pcurr[3]=0;
			 m_pcurr[4]=0;
			 gpower[0]=0;
			 gpower[1]=0;
			 gpower[2]=0;
			  gDcpower=0;
	     for(u8 i=0;i< ((CanRuleLn *)pruleCtrl)->m_MoudleAccCount;i++)
			 {
								     if((((u32)1<<i)&m_ModuleCommStatus)!=0)//只统计在线模块的电流
										 {
											    // (*m_poutI)+=vpdata[i];//统计总电流
											     if(typedata[i]==DCDC)//统计光伏电流
													 {
														   gDcpower+=vpowrdata[i];
															 m_pcurr[1]+=vpdata[i]/10;
															 m_scrCount++;
														}
														
														
													 if(typedata[i]==ACDC)//在整流模块条件下才计算，输入电流
													 {   
												     m_pcurr[0]+=vpdata[i]/10;
												     m_smrCount++;
												 
														  if((phadat[i]%3==0)&&(phadat[i]>0))//C相电流
															{
																m_pcurr[2]+=vAccpdata[i]/10;
																gpower[0]+=vpowrdata[i];//c相功率
															}
														   
															if(phadat[i]%3==1)//A相电流
															{
																m_pcurr[3]+=vAccpdata[i]/10;
																	gpower[1]+=vpowrdata[i];//A相功率
															}
															
															if(phadat[i]%3==2)//B相电流
															{
																m_pcurr[4]+=vAccpdata[i]/10;
																	gpower[2]+=vpowrdata[i];//B相功率
															}

													
												}
	
												tatalRateI+=prateI[i];
												

										 }
										 
										  *m_pworkstatus=0;//
											if(plimitdat[i]==0)//设置限流标志，只要有一个模块限流，就设为限流
											{
											
														*m_pworkstatus=1;//
											}
										 
											 
				}
////////////取每个模块的输出电流和的平均值作为每个模块的输出电流（每个模块的输出电流看起都一致）
		/*	     
			
			 for(u8 i=0;i< ((CanRuleLn *)pruleCtrl)->m_MoudleAccCount;i++)//统计模块类型及数量
			{   
          if(typedata[i]==ACDC)
					{
							float tmpfloat=(float)*((s16*)&gpSysData[LOADCURR])/m_smrCount;
						if((((u32)1<<i)&m_ModuleCommStatus)!=0)//查找有效的模块
						 {
							   testI[i]=tmpfloat*10;
							 
									 if(i%3==0)
									 {
										 testI[i]+=0;
									 }
									 else if(i%3==1) 
									 {
										 testI[i]+=1;
									 }
									 else if(i%3==2)
									 {
											testI[i]-=1;
									 }
								 

						 }
						 else
						 {
							   testI[i]=0;
						 }
				   }
					 else if(typedata[i]==DCDC)
					 { 
						 	float tmpfloat=(float)*((s16*)&gpSysData[LOADCURR])/m_scrCount;
						 if((((u32)1<<i)&m_ModuleCommStatus)!=0)//查找有效的模块
						 {
							   testI[i]=tmpfloat*10;
							 
									 if(i%3==0)
									 {
										 testI[i]+=0;
									 }
									 else if(i%3==1) 
									 {
										 testI[i]+=1;
									 }
									 else if(i%3==2)
									 {
											testI[i]-=1;
									 }
								 

						 }
						 else
						 {
							   testI[i]=0;
						 }
					 }
			}
			*/
				
////////////////////////////////////////////////////////////////////////////////////////////				
				
				
				
				
				
	              ModuleCap=tatalRateI/(m_scrCount+m_smrCount);//计算模块平均容量
			          if(ModuleCap>0)
	              gMaxModuleCurr=ModuleCap*gRateCurr/10;
	
	     if((*m_psetflag&0x01)==0x01)
			 {
	        *m_psetflag&=0xF0;
				 
				
				 
				     u8 type=0;

									        if((*m_psetflag&0x10)==0x10)									 
												{
													type=2;
													m_pmoduleaddr[5]=m_smrCount;
												}
												else if((*m_psetflag&0x20)==0x20)	//if(&pSCRMenu==m_messageItem) 
												{
													type=3;
													m_pmoduleaddr[5]=m_scrCount;
												}
												
									if(m_pmoduleaddr[4]>m_pmoduleaddr[5]) m_pmoduleaddr[4]=1;
												
									u8 counter=0;	
                 											
												for(addr=0;addr<m_pmoduleaddr[2];addr++)
												{
													
													  if(typedata[addr]==type)
														{
															 counter++;
														}
														if(counter==m_pmoduleaddr[4])
														{
															break;
														}
												}
												
												
												if((addr==m_pmoduleaddr[2])||(counter==0))
												{
													   m_pmoduleaddr[1]=((CanRuleLn *)pruleCtrl)->m_MoudleCount;//更新模块实时数量
				                     m_pmoduleaddr[2]=((CanRuleLn *)pruleCtrl)->m_MoudleAccCount;//已获取模块数量	
													return ;
												}
	
	/*				
				 
				 
				      if(addr>=lastaddr)//查找模块加时
							 {
									 while((((u32)1<<addr)&m_ModuleCommStatus)==0)//跳过不在线的模块
									 {
														addr++;
												if(addr==32)
												{
													addr=0;
													
													 lastaddr=addr;//保存当次模块查找位，用以下一次查找
		                       m_pmoduleaddr[4]=addr+1;//当前模块
													if(((CanRuleLn *)pruleCtrl)->m_MoudleCount==0)//模块数为0时
												  {
													   m_pmoduleaddr[5]=0;
														 m_pmoduleaddr[4]=0;
															for(u8 i=0;i<12;i++)
															{
																m_prfreshdata[i]=0;

															}
															m_prfreshdata[0]=3;
														  m_prfreshdata[1]=3;
													}
													return;
												}
									 }
									 
							         
									 u8 type=0;

                  if((*m_psetflag&0x10)==0x10)									 
									{
										type=2;
										m_pmoduleaddr[5]=m_smrCount;
									}
				          else if((*m_psetflag&0x20)==0x20)	
									{
										type=3;
									  m_pmoduleaddr[5]=m_scrCount;
									}
									   while(typedata[addr]!=type)//查找整流模块,跳过光伏模块
									 {
														
														addr++;
												if(addr==32)
												{
													addr=0;
													 lastaddr=addr;//保存当次模块查找位，用以下一次查找
		                       m_pmoduleaddr[4]=addr+1;//当前模块
													return ;
												}
									 }
									 
							 }
							 else if(addr<lastaddr)//查找模块减时
							 {
								 while((((u32)1<<addr)&m_ModuleCommStatus)==0)//跳过不在线的模块
									 {
														addr--;
														if(addr>=32)
														{
															addr=m_pmoduleaddr[2]-1;
															break;
														}
									 }
									 
									 

							   				
									    u8 type=0;

									        if((*m_psetflag&0x10)==0x10)									 
												{
													type=2;
													m_pmoduleaddr[5]=m_smrCount;
												}
												else if((*m_psetflag&0x20)==0x20)	//if(&pSCRMenu==m_messageItem) 
												{
													type=3;
													m_pmoduleaddr[5]=m_scrCount;
												}
									 
									     while( typedata[addr]!=type)//查找整流模块,跳过光伏模块
									   {
														
														addr--;
														if(addr>=32)
														{
															addr=m_pmoduleaddr[2]-1;
															break;
														}
									   }
									 
									 
							 }
	*/			 
				  *m_psetflag=0;
				 		 
							         
							 // 模块限流与故障
							               u8 * pModuleWRANNING=(u8 *)pdata->GetDataAddr(WRANNING);  
							 
							           if((pModuleWRANNING[addr]&0x01)==0)//无告警时显示限流状态，是否正常
												 {
							               m_prfreshdata[0]=plimitdat[addr];
												 }
												 else
												 {
													  m_prfreshdata[0]=2;//有告警时显示故障
												 }
							 
							 //输出电压
							 
							         u16 *v16pdata=(u16 *)pdata->GetDataAddr(VOLTAGE);
							             if(v16pdata!=NULL)
													 {
							              (* (u16 *)(&m_prfreshdata[2]))=v16pdata[addr];
													 }
													 
								//输出电流			 
													 (* (u16 *)(&m_prfreshdata[4]))=vpdata[addr];
													 //(* (u16 *)(&m_prfreshdata[4]))=testI[addr];
													 
													 
													 
							  //输入电压
							 
							           v16pdata=(u16 *)pdata->GetDataAddr(VOLTAGE_AC);//
							             if(v16pdata!=NULL)
													 {
							              (* (u16 *)(&m_prfreshdata[6]))=v16pdata[addr]/10;
													 }
													 
							    //输入电流  
													if( typedata[addr]==ACDC)//整流模块
													{
															(* (u16 *)(&m_prfreshdata[8]))=vfrqdata[addr]/10;//	改为频率	
													}
													else if ( typedata[addr]==DCDC)//整流模块
													{
														  (* (u16 *)(&m_prfreshdata[8]))= vAccpdata[addr]/10;
													}
													 
													 
													 
				                   
													 
													 
													 
									 //温度			 
									       v16pdata=(u16 *)pdata->GetDataAddr(TEMP);//温度
													 if(v16pdata!=NULL)
												  (* (u16 *)(&m_prfreshdata[10]))=(v16pdata[addr]);	

                 //温度	
													u8 * vonoff=(u8 *)pdata->GetDataAddr(ONOFF);
													 if(vonoff!=NULL)
													 {
														
														 if(vonoff[addr]==0)//开关
														 {
															               m_prfreshdata[1]=1;//关机状态 开机 0：休眠 1：开机 2：关机
															               m_prfreshdata[12]=1;//关机 动作显示  0：开机 1：关机
															 
															              if(pdisDisPlayData[13]==1)//检测是否有关机动作
																						{
																							pdisDisPlayData[13]=0;
																							pruleCtrl->SetModuleOnOff(addr+1,1);
																						}
															 
														 }
														 else
														 {
															    if( typedata[addr]==ACDC)//整流模块
																	{
																		 if(gsleepSucceedFlag==1)//处于休眠
																		 {
																			              m_prfreshdata[1]=0;//关机状态 休眠
																		 }
																		 else
																		 {
																			              m_prfreshdata[1]=2;//关机状态 关机
																		 }
																	}
																	else//光伏
																	{
																		                m_prfreshdata[1]=2;//关机状态 关机
																	}
																	
																	                  m_prfreshdata[12]=0;//开机 动作显示
																	                 if(pdisDisPlayData[13]==1)//检测是否有开机动作
																										{
																											pdisDisPlayData[13]=0;
																											pruleCtrl->SetModuleOnOff(addr+1,0);
																										}
															 
														 }
													 }
													 
													//防盗锁定状态
													  if(genableTP==1)
														{

															  pdisDisPlayData[31]=(pModuleWRANNING[addr]&0xf0)>>4;
														}
													 
													 
													  //闪灯
							       
								    if(lastlight!=addr)//当前的位置不等于上一个位置
									  {
										
										      lightstatus=1;
										
                          //上一个位置的常亮灯	
                         ((CanRuleLn *)pruleCtrl)->m_setAddr=lastlight+1;
												 ((CanRuleLn *)pruleCtrl)->m_setCmd=SET__LED_FLASH;
												 ((CanRuleLn *)pruleCtrl)->m_isCmd=1;
												 ((CanRuleLn *)pruleCtrl)->m_setData=0;//常亮											
										}
										else
										{    
											   //开当前位置的开闪灯
											 if(lightstatus==1)
											 {
												  lightstatus=2;
												 ((CanRuleLn *)pruleCtrl)->m_setAddr=addr+1;
												 ((CanRuleLn *)pruleCtrl)->m_setCmd=SET__LED_FLASH;
												 ((CanRuleLn *)pruleCtrl)->m_isCmd=1;
												 ((CanRuleLn *)pruleCtrl)->m_setData=2;//闪灯
												 
												  if(genableTP==1)//使能防盗
													{
														 
														  memset(gmoduleSN,0x20,16);
														  gmoduleSN[16]='\0';
														  gmoduleTPdeay2=5;//设置模块闪灯５秒后查询模块流水号
													}
												 
												 
												 
											 }												 
											
										}
													 
													 
													 
													 
				 
			 }
			 else
			 {
				  if(lightstatus==2)//退出模块界面时，使闪灯常亮
				{
					lightstatus=1;
					((CanRuleLn *)pruleCtrl)->m_setAddr=lastlight+1;
					((CanRuleLn *)pruleCtrl)->m_setCmd=SET__LED_FLASH;
					((CanRuleLn *)pruleCtrl)->m_isCmd=1;
					((CanRuleLn *)pruleCtrl)->m_setData=0;//常亮	
				}
				 
			 }

          lastlight=addr;
				// m_pmoduleaddr[4]=addr+1;//当前模块
				 m_pmoduleaddr[1]=((CanRuleLn *)pruleCtrl)->m_MoudleCount;//更新模块实时数量
				 m_pmoduleaddr[2]=((CanRuleLn *)pruleCtrl)->m_MoudleAccCount;//已获取模块数量	
				 //pdisDisPlayData[30]=addr+1;//当前模块号，防盗用
			   TPaddr=addr+1;//当前模块号，防盗用
		
}



