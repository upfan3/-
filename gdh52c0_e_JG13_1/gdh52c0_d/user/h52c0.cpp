#include "h52c0.h"
#include "globalval.h"
#include <math.h>
# define FLASH_PARA_START (0x8000000+510*1024)

//u16 gyh[6]={1,2,3,4,5,6};
//u16 gyl[6]={1,2,3,4,5,6};
//u32 gxh[6]={1,2,3,4,5,6};
//u32 gxl[6]={1,2,3,4,5,6};

u8 sysPara[72];
u16 *psysPara=(u16 *)&sysPara[0];
s16 gMaxBattI=0;
u8  gMaxBattCap=0;
u8  gsynflag=0;


u16 tempV;
u16 tempI;
u16 tempAD;

h52c0 gh52c0;
h52c0 *pgh52c0=&gh52c0;
u8 saveEnergyOnSaveFlag=0;
s16 gbattRateI[10];
u8 pregbattGroud = 0;//记录电池变化数


extern s16 totalBattI ; 


h52c0::h52c0(void)
{ 
	  m_do._doData=0;
		m_di._diData=0;
	
	  m_pdata8head=&gAutoBootEnable;//单字节首地址
    m_pdata16head=&gSetfloatV;//双字节首地址
    m_pdatatail=&EndParaData;	//参数结束地址
	
	
	m_lenth16=(u32)m_pdata8head-(u32)m_pdata16head;
  m_lenth8=(u32)m_pdatatail-(u32)m_pdata8head;
	
	
//	m_paraAddr=SYS_PARA_FLAG;
	
}


void h52c0::Init(void)
{ 
			if( GetFlashData16(sysPara,72)!=0)
			{ 
				    psysPara[BATT1_ZERO]=10000;//电池电流1，传感器
				    psysPara[BATT1_RANGE]=0;
				
				    psysPara[BATT2_ZERO]=10000;//电池电流2，传感器
				    psysPara[BATT2_RANGE]=0;
				
				    psysPara[VY]=5350;
				    psysPara[VX]=3416;
				
				    psysPara[BATT3_ZERO]=2342;//电池电流3，分流器
				    psysPara[BATT3_RANGE]=0;
				
				    psysPara[BATT4_ZERO]=2377;//电池电流4，分流器
				    psysPara[BATT4_RANGE]=0;
				    
				    sysPara[SWITCH_FACT]=1;//默认为泰永
				    sysPara[PROL_OLD_FLAG]=0;//默认为NJP
				    sysPara[BATT_MOS_OFF]=0;//默认不关闭
				    sysPara[DELAY_VOLT_UP]=0;//默认不起用
				    sysPara[SELECT_FACT]=1;//默认中天
				    sysPara[LIDIAN_MODEL]=0;
				    //*(u16 *)&sysPara[TEMP_CORR]=0; //温度修正
				
				    SaveFlashData16(sysPara,72);
				
			}
			 
		
	 
}


h52c0::~h52c0(void)
{
}

void h52c0:: setdo(u8 _No)
{u16 tmp=1;
	m_do._doData=m_do._doData|(tmp<<_No);
}
void h52c0:: clrdo(u8 _No)
{
	u16 tmp=1;
	m_do._doData=m_do._doData&(~(tmp<<_No));

}

void h52c0:: portOntick(void)
{  
	
	  	   
	   if(m_do._doBit.output0)
      {
			    m_io.set(OC9,ON);  //用户1下电
		  }
	    else
			{
				   m_io.set(OC9,OFF);//用户1下电
			}
					
			if(m_do._doBit.output1)
			{
			    m_io.set(OC10,ON);//用户2下电
		  }
	    else
			{
				   m_io.set(OC10,OFF);//用户2下电
  		}		
				if(m_do._doBit.output2)
     {
			    m_io.set(OC11,ON);//电池下电
		  }
	    else
			{
				   m_io.set(OC11,OFF);//电池下电
			}
			

  
			
		
			
     if(m_do._doBit.output5)
		  {
			    m_io.set(OC6,ON);
		  }
	    else
			{
				   m_io.set(OC6,OFF);
			}

				 if(m_do._doBit.output6)
			{
			    m_io.set(OC7,ON);
		  }
	    else
			{
				   m_io.set(OC7,OFF);
			}
			
			    	if(m_do._doBit.output7)
      {
			    m_io.set(OC8,ON);
		  }
	    else
			{
				  m_io.set(OC8,OFF);
			}
			
			
				if(m_do._doBit.output8)
				{
						m_io.set(Relayer0,ON);
				}
				else
				{
						 m_io.set(Relayer0,OFF);
				}	

		
				if(m_do._doBit.output9)
				{
						m_io.set(Relayer1,ON);
				}
				else
				{
						 m_io.set(Relayer1,OFF);
				}	
	
		
				if(m_do._doBit.output10)
			  {
						m_io.set(Relayer2,ON);
				}
				else
				{
						 m_io.set(Relayer2,OFF);
				}	

			
				if(m_do._doBit.output11)
				{
						m_io.set(Relayer3,ON);
				}
				else
				{
						 m_io.set(Relayer3,OFF);
				}	
	
		
				if(m_do._doBit.output12)
			  {
						m_io.set(Relayer4,ON);
				}
				else
				{
						 m_io.set(Relayer4,OFF);
				}	
	
	
				if(m_do._doBit.output13)
				{
						m_io.set(Relayer5,ON);
				}
				else
				{
						 m_io.set(Relayer5,OFF);
				}	
	

		

			if(	m_io.get(READ_DI0)==Bit_SET)
			{
				m_di._diBit.input0=1;
			}
			else{
				m_di._diBit.input0=0;
			}
						
			if(	m_io.get(READ_DI1)==Bit_SET)
			{
				m_di._diBit.input1=1;
			}
			else{
				m_di._diBit.input1=0;
			}
			
			if(	m_io.get(READ_DI2)==Bit_SET)
			{
				m_di._diBit.input2=1;
			}
			else{
				m_di._diBit.input2=0;
			}
			
			if(	m_io.get(READ_DI3)==Bit_SET)
			{
				m_di._diBit.input3=1;
			}
			else{
				m_di._diBit.input3=0;
			}
			
			if(	m_io.get(READ_DI4)==Bit_SET)
			{
				m_di._diBit.input4=1;
			}
			else{
				m_di._diBit.input4=0;
			}
						
			if(	m_io.get(READ_DI5)==Bit_SET)
			{
				m_di._diBit.input5=1;
			}
			else{
				m_di._diBit.input5=0;
			}
			
			if(	m_io.get(READ_DI6)==Bit_SET)
			{
				m_di._diBit.input6=1;
			}
			else{
				m_di._diBit.input6=0;
			}
	
			if(	m_io.get(READ_DI7)==Bit_SET)
			{
				m_di._diBit.input7=1;
			}
			else{
				m_di._diBit.input7=0;
			}
			
			if(	m_io.get(READ_DI8)==Bit_SET)
			{
				m_di._diBit.input8=1;
			}
			else{
				m_di._diBit.input8=0;
			}
			
			if(	m_io.get(READ_DI9)==Bit_SET)
			{
				m_di._diBit.input9=1;
			}
			else{
				m_di._diBit.input9=0;
			}
			
			if(	m_io.get(READ_DI10)==Bit_SET)
			{
				m_di._diBit.input10=1;
			}
			else{
				m_di._diBit.input10=0;
			}
	
			if(	m_io.get(READ_DI11)==Bit_SET)
			{
				m_di._diBit.input11=1;
			}
			else{
				m_di._diBit.input11=0;
			}
			
			
			m_io.scankey(0);//按键扫描
}



float b1;
float k;

u16 testt=0;


extern u16 Yout[7];
extern u32 acDCEnergydata[6];
extern u32 gTotalMrEnergy;
extern u32 gTotalOcrEnergy;
void h52c0:: ad_calibration(u8 ch)
{

/*高低点校准*/
	

	 if(ch==0)//电池电流1
	{ 

		m_result[0] = ((u32)( 3300 * Yout[0] )) >> 12;


		s32 tmpresult;
//		tmpresult =m_result[0] *5- gxl[0];
//		//tmpresult = tmpresult * 2.5*range[2];
//		tmpresult = tmpresult * 2.5*gxh[0];
		 tmpresult =m_result[0] *5- psysPara[BATT1_ZERO];
		 tmpresult = tmpresult * 2.5*psysPara[BATT1_RANGE];
		
		m_result[ch]=tmpresult/10;
		
  }
	else if(ch==1)//电池电流2
	{
		

		

    m_result[1] = ((u32)( 3300 * Yout[1] )) >> 12;
		s32 tmpresult;
//		tmpresult =m_result[1] *5- gxl[1];
//		//tmpresult = tmpresult * 2.5*range[3];
//		tmpresult = tmpresult * 2.5*gxh[1];
//		
		  tmpresult =m_result[1] *5- psysPara[BATT2_ZERO];
		  tmpresult = tmpresult * 2.5*psysPara[BATT2_RANGE];
		  m_result[ch]=tmpresult/10;
		
	}
  else	if(ch==2)//母排电压
	{
		
//		  if(gxh[3]!=0)
//		  k=(float)(*((s16 *)&gyh[3]))/(float)gxh[3];
//			b1=*((s16*)&gyh[3])-k*gxh[3];
			  if(psysPara[VX]!=0)
				k=(float)(*((s16 *)&psysPara[VY]))/(float)psysPara[VX];
			  b1=*((s16*)&psysPara[VY])-k*psysPara[VX];
			
			
			
					
					 m_result[3]=Yout[2] *k+b1;
		

	}
	else if(ch==3)//温度
	{
		 float ftmp,ftmp2;//,ln,A;
													
															ftmp=(float)3.38* Yout[3] /4096;	
                              tempAD =	 Yout[3];	
		                          tempV = ftmp*1000;
														  tempI = (3380-tempV)*1000/47;
		
//		                           ftmp2=(float)(*(u16 *)&sysPara[TEMP_CORR])/1000;//温度修正值计算；
//		                                ftmp=ftmp+ftmp2;
		
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
	else if(ch==4)//分流器1
	{
		
		/*电池分流器电流算法
		//3300/4096=0.805664mv/bit
		//(91/10.2)*(47/10)=41.931373 硬件电阻增益
		//200/(25*41.931373)=0.1907879A/mv
		//0.805664*0.1907879=0.1537A/bit
		
		将分流器量程取出作参数  2*100
		/k2用去增益调整
   //25*41.931373=1048.284325，取单位为0.1A, 结果放大10倍，则这缩小10倍 104.8284325
		gk取100
		
		*/
		
	 // m_result[2]=(s32)((s32)(Yout[4]-gxl[2])*0.805664*range[0]*100/104.8284);
		 // m_result[2]=(s32)((s32)(Yout[4]-gxl[2])*80.5664*range[0]/104.8284);
		  // m_result[2]=(s32)((s32)(Yout[4]-gxl[2])*80.5664*gxh[2]/104.8284);
		      m_result[2]=(s32)((s32)(Yout[4]-psysPara[BATT3_ZERO])*80.5664*psysPara[BATT3_RANGE]/104.8284);
	}
	else if(ch==5)//分流器1
	{
		//m_result[4]=(s32)((s32)(Yout[5]-gxl[4])*0.805664*range[1]*100/104.8284);
		//  m_result[4]=(s32)((s32)(Yout[5]-gxl[4])*80.5664*range[1]/104.8284);
		 // m_result[4]=(s32)((s32)(Yout[5]-gxl[4])*80.5664*gxh[4]/104.8284);
		     m_result[4]=(s32)((s32)(Yout[5]-psysPara[BATT4_ZERO])*80.5664*psysPara[BATT4_RANGE]/104.8284);
	}
	else if(ch==6)
	{
		m_result[5]=Yout[6];
	}
	
	
	
	
	
	

}


u32 W100ms[2]={0,0};


u32 h52c0::setIvalue(s16 valuI,u8 ch)
{
	u8 range=0;
	if(ch==0)range=psysPara[BATT3_RANGE];
	else if(ch==1) range=psysPara[BATT4_RANGE];
	
	float tmpf=0.805664*range*100/104.8284;

	if(ch==0)ch=4;
	else if(ch==1) ch=5;
	
	u32 tmpu32=Yout[ch]-(float)valuI/tmpf;
	return tmpu32;
}


u8 ghumi;//湿度值 
u32 resetADCount=0;
s16 battI2=0;
void h52c0::  GetAdc(void)
{u8  i;
	
	

	
	
	//padc->ReadADResult();
	for(i=0;i<7;i++)
	ad_calibration(i);
	
	
	if(genableTimeCount>0) return ;

	
	
	if((m_result[2]<(psysPara[BATT3_RANGE]*5))&&(m_result[2]>(psysPara[BATT3_RANGE]*5)*-1))// 限制电池电流，在 正负0.5%量程下清零
	          	m_result[2]=0;
	
	
	if((m_result[4]<(psysPara[BATT4_RANGE]*5))&&(m_result[4]>(psysPara[BATT4_RANGE]*5)*-1))// 限制电池电流，在 正负0.5%量程下清零
	          	m_result[4]=0;
	
	
	if((m_result[0]<(psysPara[BATT1_RANGE]*5))&&(m_result[0]>(psysPara[BATT1_RANGE]*5)*-1))// 限制电池电流，在 正负0.5%量程下清零
	          	m_result[0]=0;
	
	
	if((m_result[1]<(psysPara[BATT2_RANGE]*5))&&(m_result[1]>(psysPara[BATT2_RANGE]*5)*-1))// 限制电池电流，在 正负0.5%量程下清零
	          	m_result[1]=0;

	



	      //if(m_result[3]<5900)
      	//*((s16 *)&gpSysData[DCVOLTAGE])=m_result[3]; //获取本地母线电压（ AD3）
	
	      *((s16 *)&gpSysData[BATT_CURR1])=m_result[0];//（传感器1 AD0）				
				*((s16 *)&gpSysData[BATT_CURR2])=m_result[1]; //（ 传感器1 AD1） 
	
	      *((s16 *)&gpSysData[BATT_CURR3])=m_result[2];//（分流器1 ）				
				*((s16 *)&gpSysData[BATT_CURR4])=m_result[4];// （分流器2 ） 
	
        *((s16 *)&gpSysData[HUMIDITY])=(m_result[5]-885)/27+20;;
     

       	s32 tmpVoltage=0;
													 s16 * Vdata=(s16 *)pModuledata->GetDataAddr(VOLTAGE);
													 for(u8 i=0;i<ModuleOnlineMessage[2];i++)
													 {
														 
															 tmpVoltage+=Vdata[i];
													 }

				                   tmpVoltage=tmpVoltage/ModuleOnlineMessage[1];
													 
													 //在模块输出正常的情况下，防以由于硬件故障引起的母线电压显示异常情况。
													 *((s16 *)&gpSysData[DCVOLTAGE])=m_result[3]; //获取本地母线电压（ AD3）
													 if(m_result[3]>5700)
													 {
														 if ((m_result[3]-tmpVoltage)>100 && tmpVoltage>4200)
														 {
															  *((s16 *)&gpSysData[DCVOLTAGE])=tmpVoltage;
														 }
													 }
													 else if(m_result[3]<4700)
													 {
														 if((tmpVoltage-m_result[3])>100)
														 {
															  *((s16 *)&gpSysData[DCVOLTAGE])=tmpVoltage;
														 }
													 }
													 



	       if(gbattGroud==0)  //非智能空开电池分路
				 {

					/*
					 if((m_result[0]<1200)&&(m_result[1]<1200))
					 {
						 *((s16 *)&gpSysData[TOTAL_BATTI])=m_result[0];//使用传感器1电流
						 *((s16 *)&gpSysData[TOTAL_BATTI])+=m_result[1];//使用传感器2电流
						 *((s16 *)&gpSysData[TOTAL_BATTI])+=m_result[2];//使用分流器1电流
						 *((s16 *)&gpSysData[TOTAL_BATTI])+=m_result[4];//使用分流器2电流
						  *((s16 *)&gpSysData[TOTAL_BATTI])+= (*((s16 *)&gpSysData[BATT_CURR5]))*sysPara[BATT5_EN];//使用分流器1电流
						 *((s16 *)&gpSysData[TOTAL_BATTI])+= (*((s16 *)&gpSysData[BATT_CURR6]))*sysPara[BATT6_EN];//使用分流器2电流
						 *((s16 *)&gpSysData[TOTAL_BATTI])+=totalBattI;
						 
						 
					 }
					 else
					 {
						    *((s16 *)&gpSysData[TOTAL_BATTI])= *((s16*)&gpSysData[LOADCURR])-totalUserI;
					 }
					 */
					      
      					 *((s16 *)&gpSysData[TOTAL_BATTI])=0;
					      
								 *((s16 *)&gpSysData[TOTAL_BATTI])+=m_result[2];//使用分流器1电流
						     *((s16 *)&gpSysData[TOTAL_BATTI])+=m_result[4];//使用分流器2电流
					    
					 
					 
					 	if(m_result[0]>0)	//传感器1 对应电池容量5  比值 1
						{							
						   u16	tmplimit=(gbranchBatt[4]/1)*((s16)gSetBattLimitPerC)/10;
                 if(tmplimit>0)
								 {
									    gbattRateI[0]= m_result[0]*100/tmplimit;
									   
								 }	
                 else
								 {
									     gbattRateI[0]= 0;
									    
								 }									 
						}
						else
						{
							  gbattRateI[0]= 0;
							 
						}
						
					 				  
            if(m_result[1]>0) //传感器2 对应电池容量6  比值 2
						{							
						   u16	tmplimit=(gbranchBatt[5]/1)*((s16)gSetBattLimitPerC)/10;
                 if(tmplimit>0)
								 {
									    gbattRateI[1]= m_result[1]*100/tmplimit;
									   
								 }	
                 else
								 {
									     gbattRateI[1]= 0;
									  
								 }									 
						}
						else
						{
							  gbattRateI[1]= 0;
						}
						
						
		
						
						if(m_result[2]>0) //分流器1 对应电池容量3  比值 3	
						{							
						   u16	tmplimit=(gbranchBatt[2]/1)*((s16)gSetBattLimitPerC)/10;
                 if(tmplimit>0)
								 {
									   
									    gbattRateI[2]= m_result[2]*100/tmplimit;
								 }	
                 else
								 {
									    
									     gbattRateI[2]= 0;
								 }									 
						}
						else
						{
							
							  gbattRateI[2]= 0;
						}
						  
            if((m_result[4]>0))//分流器2 对应电池容量4  比值 4	
						{							
						   u16	tmplimit=(gbranchBatt[3]/1)*((s16)gSetBattLimitPerC)/10;
                 if(tmplimit>0)
								 {
									   
									    gbattRateI[3]= m_result[4]*100/tmplimit;
								 }	
                 else
								 {
									    
									     gbattRateI[3]= 0;
								 }									 
						}
						else
						{
							 
							  gbattRateI[3]= 0;
						}
						
						
						
						
            for(u8 i=0;i<4;i++)//锂电1 2 3 4       对应双值 5 6 7 8
						{
							if((batt[i].Set_Batt_ChargA>0)&&(batt[i].Ibus>0))
							gbattRateI[4+i]=batt[i].Ibus*100/batt[i].Set_Batt_ChargA;
							else
						  gbattRateI[4+i]=0;
						}	
						
						
						
						if(sysPara[BATT5_EN]==1) //铅配单元1   对应电池容量1 比值 9
						{
							 u16	tmplimit=(gbranchBatt[0]/1)*((s16)gSetBattLimitPerC)/10;
							if(tmplimit>0)
							gbattRateI[8]=(*((s16 *)&gpSysData[BATT_CURR5]))*100/tmplimit;
						}
						else
						{
							  gbattRateI[8]= 0;
						}
						
						if(sysPara[BATT6_EN]==1) //铅配单元2   对应电池容量2 比值 10
						{
							 u16	tmplimit=(gbranchBatt[1]/1)*((s16)gSetBattLimitPerC)/10;
							if(tmplimit>0)
							gbattRateI[9]=(*((s16 *)&gpSysData[BATT_CURR6]))*100/tmplimit;
						}
						else
						{
							  gbattRateI[9]= 0;
						}
						
					     
						 gMaxBattI=0;
						for(u8 i=0;i<10;i++)
						{
							    // gMaxBattI=0;
							    if(gbattRateI[i]>gMaxBattI)
									gMaxBattI=gbattRateI[i];
						}
					 
					 
					 
				 }
				 else if(gbattGroud>0)//智能空开电池分路 
				 {
					 
					  
					 *((s16 *)&gpSysData[TOTAL_BATTI])=0;
			
					 for(u8 i=0;i<6;i++)//使用分路电池电流
					 {
					    if( pgBattBranch[i]!=NULL)
					     *((s16 *)&gpSysData[TOTAL_BATTI])+=*pgBattBranch[i];
							
				   }
					  *((s16 *)&gpSysData[TOTAL_BATTI])+=totalBattI;
					 
					 
					 
					 
					 
					 
					 
					 
					  
						 
						  u16	tmplimit;
								
								
								for(u8 i=0;i<6;i++)
								{
   									tmplimit=(gbranchBatt[i]/1)*((s16)gSetBattLimitPerC)/10;
									   
									   if((tmplimit>0)&&(pgBattBranch[i]!=NULL))
									   gbattRateI[i]=((s16)(*pgBattBranch[i]))*100/tmplimit;
										 else
										 gbattRateI[i]=0;
								}
								
					 
					       for(u8 i=0;i<4;i++)
								{
									if(batt[i].Set_Batt_ChargA>0) 
									gbattRateI[6+i]=batt[i].Ibus*100/batt[i].Set_Batt_ChargA;
									else
									gbattRateI[6+i]=0;
								}	
					 
					 
					   gMaxBattI=0;
						for(u8 i=0;i<10;i++)
						{
							    
							    if(gbattRateI[i]>gMaxBattI)
									gMaxBattI=gbattRateI[i];
						}
					 
				 }
				
        
        
			
				 gsynflag++;
				 
				 if( pregbattGroud != gbattGroud)
				 {
					   pregbattGroud = gbattGroud;
					 
					    gSetBattC = 0; 
							if(gbattGroud>0)
							{
								
								 
									 for(u8 i=0;i<6;i++)
										{
												 if(gUseToBattBranch[i]<TOTAL_USER)
												 {
														
														 gSetBattC+=gbranchBatt[i];
												 }
													
										}
							}
							else if(gbattGroud == 0)
							{
									for(u8 i=0;i<battGroud;i++)
										{
												
														
											 gSetBattC+=gbranchBatt[i];
											
													
										}
							}
						 pbattCap->ReSetCap();

				 }
				 
}


void h52c0::readI2C(u16 nAddr, u8* pDat, u16 nLen)
{
		taskENTER_CRITICAL();
	   gEprom.ReadString(nAddr, pDat,  nLen);	
	  taskEXIT_CRITICAL();
}




void h52c0::writeI2C(u16 nAddr, u8* pDat, u16 nLen)
{
		taskENTER_CRITICAL();
	   gEprom.WriteString(nAddr, pDat,  nLen);	
	  taskEXIT_CRITICAL();
}


void h52c0::readI2C2(u16 nAddr, u8* pDat, u16 nLen)
{
		taskENTER_CRITICAL();
	   gEprom1.ReadString(nAddr, pDat,  nLen);	
	  taskEXIT_CRITICAL();
}




void h52c0::writeI2C2(u16 nAddr, u8* pDat, u16 nLen)
{
		taskENTER_CRITICAL();
	   gEprom1.WriteString(nAddr, pDat,  nLen);	
	  taskEXIT_CRITICAL();
}




/*
1，先读取记录标记，若标记存在，则读取最后记录位置，若记录不存在，则以0为读录起始位置，并设置记录标记
2，设置记录信息，最大条数 _pReMax，最总条数_pReTotal 均为当前记录位置加1
3，最大条数 _pReMax 只增加到ReLen，就不再增加，并将该值写入储存器
4，返回记录信息的最后一条的位置
*/

u16 h52c0::SaveRecordInfo(u16 BaseAddr,void * pRecord,u16 ReLen,u8 ch)
{
	RecordInfo  *pRecInfo=(RecordInfo *)pRecord;
	u16 flag,count=0;
	
	if(ch==1)
	{
	  	readI2C(BaseAddr,(u8 *)&flag,2);
										if(flag!=0xA5A5)
										{   flag=0xA5A5;
												 writeI2C(BaseAddr,(u8 *)&flag,2);//作第一次记录标记
										}
										else
										{
												 readI2C(BaseAddr+2,(u8 *)&count,2);//非第一次记录，读取存储记录数
										}
	}
	else if(ch==2)
	{
		  readI2C2(BaseAddr,(u8 *)&flag,2);
										if(flag!=0xA5A5)
										{   flag=0xA5A5;
												 writeI2C2(BaseAddr,(u8 *)&flag,2);//作第一次记录标记
										}
										else
										{
												 readI2C2(BaseAddr+2,(u8 *)&count,2);//非第一次记录，读取存储记录数
										}
	}
	
										
							 
										        if(*(pRecInfo->_pReMax)<ReLen)
														 {
															 
					                      *(pRecInfo->_pReMax)=count+1;
															 if(*(pRecInfo->_pReMax)==ReLen)
															 {
																 savePara(pRecInfo->_pReMax);
															 }
														 }
					                  
														 *(pRecInfo->_pReTotal)=count+1;
														 
														 return count;
}








void h52c0::savePara(void *paddr)
{ u16 offset;
	u8 datatype;
	     if(paddr<m_pdata16head) return;//参数地址无效
	     if(paddr>m_pdatatail)  return;//参数地址无效
       if(paddr>=m_pdata16head)//保存双字节数据
	     {
				   datatype=2;//双字节类型
				   offset=(u32)paddr-(u32)m_pdata16head;//计算偏移量
				   writeI2C(SYS_PARA+offset,(u8 *)paddr, datatype);
				 
			 }
//	     else//保单字节数据
//			 {
//				    datatype=1;//单字节类型
//				    offset=(u32)paddr-(u32)m_pdata8head;//计算偏移量
//				    writeI2C(SYS_PARA+m_lenth16+offset, (u8 *)paddr, datatype);
//			 }
	
	
	
}



void h52c0::InitPara(void)
{
	u16 _flag,_len8,_len16;
	readI2C(SYS_PARA_FLAG,(u8 *)&_flag,2);
	   if(_flag!=PARA_FLAG)
		 {
			    writeI2C(SYS_PARA, (u8 *)m_pdata16head, m_lenth8+m_lenth16);
			    _flag=PARA_FLAG;
			    writeI2C(SYS_PARA_FLAG, (u8 *)&_flag, 2);
			    writeI2C(SYS_PARA_LEN8, (u8 *)&m_lenth8, 2);
			    writeI2C(SYS_PARA_LEN16, (u8 *)&m_lenth16, 2);
		 }
		 else
		 {
			   readI2C(SYS_PARA_LEN8,(u8 *)&_len8,2);//读出单字节保存长度
			   readI2C(SYS_PARA_LEN16,(u8 *)&_len16,2);//读出双字节保存长度
			  if((_len8==m_lenth8)&&(_len16==m_lenth16))//参数储存空间未发生变化
				{
			      readI2C(SYS_PARA,(u8 *)m_pdata16head,m_lenth8+m_lenth16);//加载I2C中的参数
				}
				else//参数储存空间已发生变化
				{
					
					u16 tmplen8;
				  u16 tmplen16;
					
					if(_len16<m_lenth16)
					{tmplen16=_len16;}
					else
					{tmplen16=m_lenth16;}
					
					
					if(_len8<m_lenth8)
				  {tmplen8=_len8;}
					else
					{tmplen8=m_lenth8;}
					
					readI2C(SYS_PARA,(u8 *)m_pdata16head,tmplen16);//读取原双字节参数
					readI2C(SYS_PARA+_len16,(u8 *)m_pdata8head,tmplen8);//读取原单字节参数
					

            //双字节在前，解决单字节，奇数时增量时，单字节长度有可能不增加的问题。
            if(_len16<m_lenth16)//只处理增加的变量，减少变量不处理
						{

							  writeI2C(SYS_PARA+_len16,(u8 *)m_pdata16head+_len16,m_lenth16-_len16);//将增加变量回写到I2C	
							  writeI2C(SYS_PARA+m_lenth16,(u8 *)m_pdata8head,_len8);//将取原单字节参数写到I2C
							  writeI2C(SYS_PARA_LEN16, (u8 *)&m_lenth16, 2);
						}
						else if(_len16>m_lenth16)//处理减少变量
						{
							writeI2C(SYS_PARA_LEN16, (u8 *)&m_lenth16, 2);//回写正确参数长度
							writeI2C(SYS_PARA+m_lenth16,(u8 *)m_pdata8head,_len8);
							
							
						}

            
					  if(_len8<m_lenth8)//只处理增加的变量，减少变量不处理
						{
							 writeI2C(SYS_PARA+m_lenth16+_len8    //基地址+新双字节长+单字节原有长度
							             ,(u8 *)m_pdata16head+m_lenth16+_len8//基地址+新双字节长+单节原有长度
							                   ,m_lenth8-_len8);//将增加变量回写到I2C			
                writeI2C(SYS_PARA_LEN8, (u8 *)&m_lenth8, 2);							
						}
						else if(_len8>m_lenth8)//参数长度错误，（可能引起其它内存内间被改写，更正长度以免出错）
						{
							writeI2C(SYS_PARA_LEN8, (u8 *)&m_lenth8, 2);//回写正确参数长度
						}

						
						
				}
		 }
        for(u8 i=0;i<12;i++)//初始化告警号列表（同一类输入告警信息列表）
		 {
		     bitAlarmTypeList[gInPutAlarm[i].st_AlarmType]|=1<<(gInPutAlarm[i].st_InputNb%8);
		 
		 }
		 
		 
	 //初始化告警记录长度信息
        pgh52c0->readI2C(ALARM_DATA_FlAG,(u8 *)&_flag,2);//读取记录位置
				if(_flag==0xA5A5)
				{ 
					pgh52c0->readI2C(ALARM_DATA_LEN,(u8 *)&recordAlarmCount,2);//读取最后一条告警记录位置
			 
					//recordAlarmCount++;//加1换算成当前记录数 如位置0，有一条记录，最后位置1，有两条记录
					if(galarmRecordMax<ALARM_RECORD_LEN)
					galarmRecordMax=recordAlarmCount;
					
					pgh52c0->readI2C(ALARM_DATA_HEAD,(u8 *)&alarmhead[0],12);
					pgh52c0->readI2C(ALARM_DATA_TAIL,(u8 *)&alarmtail[0],12);
					
					
				}
		//初始化抄表记录长度信息	
				
    readI2C(ENERGY_DATA_FLAG,(u8 *)&_flag,2);
					if(_flag==0xA5A5)
					{
						readI2C(ENERGY_DATA_LEN,	(u8 *)&EnergyDataWriteConunt,1);//读出抄表记录指针
						//EnergyDataWriteConunt++;//加1换算成当前记录数 如位置0，有一条记录，最后位置1，有两条记录
						if(genergyDateRecordMax<ENERGY_DATE_RECORD_LEN)
					    genergyDateRecordMax=EnergyDataWriteConunt;
						
					}
					
	 //初始化用户配电记录长度信息			
				readI2C2(ENERGY_90DAY_DATA_FLAG,(u8 *)&_flag,2);
					if(_flag==0xA5A5)
					{
						readI2C2(ENERGY_90DAY_DATA_LEN,	(u8 *)&EnergyDataWriteConunt90,1);//读出抄表记录指针
						//EnergyDataWriteConunt++;//加1换算成当前记录数 如位置0，有一条记录，最后位置1，有两条记录
						if(genergyDateRecord90Max<ENERGY_DATE_90_RECORD_LEN)
					    genergyDateRecord90Max=EnergyDataWriteConunt90;
						
					}	
					
					
					
					
		//初始化电池测试记录长度
				readI2C(BATT_TEST_DATA_FLAG,(u8 *)&_flag,2);			
				if(_flag==0xA5A5)
					{
						readI2C(BATT_TEST_DATA_LEN,	(u8 *)&gbattTestCount,1);//读出抄表记录指针
						//gbattTestCount++;//加1换算成当前记录数 如位置0，有一条记录，最后位置1，有两条记录
						if(gdisBattRecordMax<DIS_BATT_RECORD_LEN)
					    gdisBattRecordMax=gbattTestCount;
						
					}
					
					

					
					totalEnergy[5]=0;//错峰总电能
					powerRate[5]=0;//错峰总电费
					
					for(u8 i=0;i<5;i++)
			  {
					   GetACDataFromBuckUpReg(i, totalEnergy);
					   GetACDataFromBuckUpReg(i, powerRate);
					   if(i>0)//累加出错峰总电能，及错峰总电费
						 {
					     totalEnergy[5]+=totalEnergy[i];
							 powerRate[5]+=powerRate[i]; 
						 }
					
					
				}
				
				//6个租户电能初始化 BKP_DATA_0~BKP_DATA_11
        for(u8 i=0;i<6;i++)	
			 {				
						 *((u16 *)&acDCEnergydata[i])   = bkp_data_read((bkp_data_register_enum)(2*i+1));
						 *((u16 *)&acDCEnergydata[i]+1) = bkp_data_read((bkp_data_register_enum)(2*i+2)); 
			 }
					
					
					*((u16 *)&gTotalMrEnergy)  =bkp_data_read(BKP_DATA_12);
					*((u16 *)&gTotalMrEnergy+1)=bkp_data_read(BKP_DATA_13);	

         	*((u16 *)&gTotalOcrEnergy)  =bkp_data_read(BKP_DATA_16);
					*((u16 *)&gTotalOcrEnergy+1)=bkp_data_read(BKP_DATA_17);				 
					
					
					
			//初始序列号信息
			readI2C(SYS_INFO_FLAG,(u8 *)&_flag,2);		
			if(_flag==0xA5A5)
			{
				   	readI2C(SYS_INFO_START,gmSN,48);
			}	
      else
			{
				  _flag=0xA5A5;
				 	writeI2C(SYS_INFO_FLAG,(u8 *)&_flag,2);
				 	writeI2C(SYS_INFO_START,gmSN,48);
			
			}				
					



      readI2C2(SWITCH_PARA,(u8 *)&_flag,2);	
			if(_flag==0xA5A5)
			{
				      readI2C2(SWITCH_PARA_STATR ,(u8 *)&gSwitchPara,(TOTAL_USER+8)*sizeof(SwitchPara));	
			}
			else
			{
				      for(u8 i=0;i<TOTAL_USER+8;i++)
							{
								  gSwitchPara[i].st_stopTime.st_year=1900;
								  gSwitchPara[i].st_stopTime.st_mon=1;
								  gSwitchPara[i].st_stopTime.st_day=1;
								  gSwitchPara[i].st_stopTime.st_hour=12;
								  gSwitchPara[i].st_stopTime.st_min=1;
								  gSwitchPara[i].startTime0=0;
								  gSwitchPara[i].startTime1=0;
								  gSwitchPara[i].stopTime0=0;
								  gSwitchPara[i].stopTime1=0;
								  gSwitchPara[i].st_downV=4600;
								  gSwitchPara[i].st_recoverV=5000;
								  gSwitchPara[i].st_downT=180;
								  gSwitchPara[i].st_downMode=1;
								  gSwitchPara[i].st_downEnery=1000;
								  gSwitchPara[i].st_maxoverLoadI=640;
								  gSwitchPara[i].authorizeflag=0;
           		}
							
							
							for(u8 i=0;i<6;i++)
							{
								gSwitchPara[i].st_maxoverLoadI=1250;
							}
							
							for(u8 i=28;i<TOTAL_USER;i++)
							{
								gSwitchPara[i].st_maxoverLoadI=1250;
								gSwitchPara[i].st_downV=4320;
								
							}
							
							
							
							
							
							 _flag=0xA5A5;
							 writeI2C2(SWITCH_PARA,(u8 *)&_flag,2);	
							 SaveSwitchPara( &gSwitchPara[0],(TOTAL_USER+8)*sizeof(SwitchPara));
							
			}




       GetCRCData( 10240,gUseToBattBranch,6); //在储存器10K位置读取数据,读取电池分路配电设置


}

s16 h52c0::GetOffSet(void * paddr)
{
	   if(paddr<m_pdata16head) return -1;//参数地址无效
	    if(paddr>m_pdatatail)  return -1;//参数地址无效
	
	      return (u32)paddr-(u32)m_pdata16head;//计算偏移量
	
}


/*
//void h52c0::SaverUserEenergy(void)
//{
////	 u16 *ptmp= (u16*)(&gpEnergy_100Wh[0]); 
////	 bkp_data_write(BKP_DATA_11,ptmp[0]);//本地支路电流1
////	 bkp_data_write(BKP_DATA_12,ptmp[1]);
////	 
////	    
////    	ptmp= (u16*)(&gpEnergy_100Wh[1]);
////	   bkp_data_write(BKP_DATA_13,ptmp[0]);//本地支路电流2
////	   bkp_data_write(BKP_DATA_14,ptmp[1]);
//}

//
//将32位交流电能数据保存到BKP_DATA_22及以后的备份寄存器中
*/
void h52c0::SaveACDataToBuckUpReg(u8 n, void * pdat)
{
	u8  data_num;
	   u16 *ptmp=(u16*)pdat;
	
	       if((u32*)pdat==totalEnergy)
				 {
					   data_num=23+2*n;//从 BKP_DATA_22 开始到 BKP_DATA_31
				 }
				 else if((u32*)pdat==powerRate)
				 {
					  data_num=33+2*n;//从 BKP_DATA_32 开始到 BKP_DATA_41
				 }
	  
	     bkp_data_write((bkp_data_register_enum)data_num,ptmp[2*n]);//本地支路电流1
	     bkp_data_write((bkp_data_register_enum)(data_num+1),ptmp[2*n+1]);
	
}

//从BKP_DATA_22及以后的备份寄存器中取出交流电能数据放到内存中
void h52c0::GetACDataFromBuckUpReg(u8 n, void * pdat)
{
	u8  data_num;
	   u16 *ptmp=(u16*)pdat;
	
	       if((u32*)pdat==totalEnergy)
				 {
					   data_num=23+2*n;//从 BKP_DATA_22 开始到 BKP_DATA_31
				 }
				 else if((u32*)pdat==powerRate)
				 {
					  data_num=33+2*n;//从 BKP_DATA_32 开始到 BKP_DATA_41
				 }
	  

	       ptmp[2*n]=bkp_data_read((bkp_data_register_enum)data_num);
	       ptmp[2*n+1]=bkp_data_read((bkp_data_register_enum)(data_num+1));
	
}

 void h52c0::SaveEnergyOnTick(void)
{
	 
	  u16 count;
	
	//抄表日电量记录
	    if(gtestSaveFlag==1)//电池测试记录共10条
			{u16 flag=0;
				  gtestSaveFlag=0;

				     
				     	RecordInfo Record={ &gbattTestCount,&gdisBattRecordMax,NULL};	
							
							 count=pgh52c0->SaveRecordInfo(BATT_TEST_DATA_FLAG,&Record,DIS_BATT_RECORD_LEN,1);
							  
							  writeI2C(BATT_TEST_DATA_STAR +0+(count%DIS_BATT_RECORD_LEN)*14,(u8 *)&gtestStarTimer,6);
							  writeI2C(BATT_TEST_DATA_STAR +6+(count%DIS_BATT_RECORD_LEN)*14,(u8 *)&gtestStopTimer,6);
							  writeI2C(BATT_TEST_DATA_STAR +12+(count%DIS_BATT_RECORD_LEN)*14,(u8 *)&pMonitor->m_saveDischarge,2);
							     count++;
							   if(count>=DIS_BATT_RECORD_LEN)
									  count=0;
							  writeI2C(BATT_TEST_DATA_LEN,(u8 *)&count,2);
							
							
			}
/*	
			if((gTimer.min==50)&&(gTimer.hour ==23))
			{
				 gGetACEnergy=1;//去获取交流电量
			}
*/

			if(gTimer.day==gGetEnergyDay)//抄表日电量保存
			{
						 
						 
						 if((gTimer.min==53)&&(gTimer.hour ==23))
							{		
								saveEnergyOnSaveFlag=1;
							}
	
						 
						 if((gTimer.min==54)&&(gTimer.hour ==23))
						 {
							 
							 if(saveEnergyOnSaveFlag==1)
							 {  
								 saveEnergyOnSaveFlag=0;


								  RecordInfo Record={  &EnergyDataWriteConunt,&genergyDateRecordMax,NULL};	
							
							    count=pgh52c0->SaveRecordInfo(ENERGY_DATA_FLAG,&Record,ENERGY_DATE_RECORD_LEN,1);
								 							 
									
									writeI2C(ENERGY_DATA_STAR+(count%48)*TOTAL_DATA_LEN,(u8 *)&gTimer,6);//写入存诸时间
						      writeI2C(ENERGY_DATA_STAR+(count%48)*TOTAL_DATA_LEN+TIME_LEN,(u8 *)gDCdistribution.pst_userEnerqy,32);//8个租户电能
						      writeI2C(ENERGY_DATA_STAR+(count%48)*TOTAL_DATA_LEN+TIME_LEN+USER_ENERGY_DATA_LEN,(u8 *)totalEnergy,AC_ENERGYDATA_LEN);//交流电能及电费  powerRate 直接跟着totalEnergy 后面
							
																		count++;
																		if(count>=48) //总空间占用为50*48=2400byte
																		{
																			count=0;//总共存48个月	
																		}
																	 writeI2C(ENERGY_DATA_LEN,	(u8 *)&count,2);
																
								}
																		
																
	
	
            }
			}
 
	
	


				
	
 //90天用户电量记录			

				
		 if((gTimer.min==58)&&(gTimer.hour ==23))
		 {		
				saveEnergyOnSaveFlag=1;
			}
				
	

		 if((gTimer.min==59)&&(gTimer.hour ==23))
		 {
				 if(saveEnergyOnSaveFlag==1)
				 {
						saveEnergyOnSaveFlag=0;
					 u16 flag=0;
					 
					
					 
						 RecordInfo Record={&EnergyDataWriteConunt90,&genergyDateRecord90Max,NULL};	
								
					count=pgh52c0->SaveRecordInfo(ENERGY_90DAY_DATA_FLAG,&Record,ENERGY_DATE_90_RECORD_LEN,2);
					 
					 
					 
	   				
						 
						  writeI2C2(ENERGY_90DAY_DATA_START+(count%90)*TOTAL_DATA_LEN,(u8 *)&gTimer,6);//写入存诸时间
						  writeI2C2(ENERGY_90DAY_DATA_START+(count%90)*TOTAL_DATA_LEN+TIME_LEN,(u8 *)gDCdistribution.pst_userEnerqy,32);//8个租户电能
						  writeI2C2(ENERGY_90DAY_DATA_START+(count%90)*TOTAL_DATA_LEN+TIME_LEN+USER_ENERGY_DATA_LEN,(u8 *)totalEnergy,AC_ENERGYDATA_LEN);//交流电能及电费  powerRate 直接跟着totalEnergy 后面
						 
						 
						 

					 
								 count++;
						 if(count>=90)  //每天存入24条，共存保存最
							 {
									count=0;
							 }
							 
									writeI2C2(ENERGY_90DAY_DATA_LEN,(u8 *)&count,2);//写入位置
					 
							
				 }
		}
		 
		 
}


/**********************************************
将智知开关参数写到E2PROM2中
pdat：智参开关参数指针
_type:参数默认为2字节，可以写入多字节
***********************************************/
void h52c0::SaveSwitchPara(void *pdat,u16 _type)
{
	u16 offset ;
	void *sw_addr=&gSwitchPara[0];
	    offset = (u32)pdat - (u32)sw_addr;
	
	
	     if(pdat<sw_addr) return;
	     if(pdat>&gSwitchPara[TOTAL_USER+8-1].st_userGroud) return;
	
	     
	
	
	       writeI2C2(SWITCH_PARA_STATR+offset,(u8 *)pdat, _type);
	
}




s8 h52c0::SaveFlashData16(u8 *pdat,u16 datlen)
{
	 u16 crcflag=0;     
	 u16 crcflag2=*(u16 *)FLASH_PARA_START; 
	  
	CheckCrc16(pdat,datlen,(u8*)&crcflag);
	
	if((crcflag==crcflag2)&&(crcflag2!=0xFFFF))
	 return -2;
	
	
	
	fmc_unlock();//解锁flash
	
	 fmc_page_erase(FLASH_PARA_START);
	
	 fmc_halfword_program(FLASH_PARA_START, crcflag);//写CRC校验标志
	
	u16 *pdat16=(u16 *)pdat;
	
	 for(u8 i=0;i<datlen/2;i++)
	{
		  fmc_halfword_program(FLASH_PARA_START+2+i*2, pdat16[i]);
	}
	
	fmc_lock();//上锁flash
	
	
	u8 * pdat8=(u8 *)(FLASH_PARA_START+2);
	
	crcflag=*(u16 *)FLASH_PARA_START;
  return	CheckCrc16(pdat8,datlen,(u8*)&crcflag);
	
}

s8 h52c0::GetFlashData16(u8 *pdat,u16 datlen)
{
	  u16 crcflag=*(u16 *)FLASH_PARA_START;  
	  u8 * pdat8=(u8 *)(FLASH_PARA_START+2);
	
	 if(CheckCrc16(pdat8,datlen,(u8*)&crcflag)!=0)
	     return -1;
	
	    for(u8 i=0;i<datlen;i++)
		{
			     pdat[i]=pdat8[i];
		}
	
	 return CheckCrc16(pdat ,datlen,(u8*)&crcflag);
	
	
}



//保存带有CRC校检的数据
s8 h52c0:: SaveCRCData(u16 addr,u8 *pdat,u8 len)
{
	u16 checkcrc=0; 
	
	CheckCrc16(pdat,len,(u8 *)&checkcrc);
	writeI2C2(addr, (u8 *)&checkcrc, 2);
	writeI2C2(addr+2, pdat, len);
	
	
	writeI2C2(addr+10240, (u8 *)&checkcrc, 2);
	writeI2C2(addr+10240+2, pdat, len);
	

}

//读取带有CRC校检的数据
s8 h52c0:: GetCRCData(u16 addr,u8 *pdat,u8 len)
{
	u16 checkcrc=0; 
	
	
	readI2C2(addr, (u8 *)&checkcrc, 2);
	readI2C2(addr+2, pdat, len);
	
	
	 if( CheckCrc16(pdat,len,(u8 *)&checkcrc)==-1)
	 {
		   
        checkcrc=0; 
        readI2C2(addr+10240, (u8 *)&checkcrc, 2);
	      readI2C2(addr+10240+2, pdat, len);

        if( CheckCrc16(pdat,len,(u8 *)&checkcrc)==-1)
				{
					 for(u8 i=0;i<len;i++)
						 {
								pdat[i]=0xff;
						 }
				}
	 }
 
	  

}






