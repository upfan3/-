#include "USART.h"
#include "globalval.h"

#define COM1 0
#define COM2 1
#define COM3 2
#define COM4 3

#define COM1RECVBUF 1440
#define COM1SENDBUF 15

//---------------------------GD32F105 usart硬件相关配置----------------------------------------------
const Ts_gpio usartCtrIO[4]={ GPIOA,GPIO_PIN_8 ,GPIOD,GPIO_PIN_7 ,GPIOE,GPIO_PIN_15 ,GPIOA,GPIO_PIN_15};
const Ts_gpio  usartTxIO[4]={ GPIOA,GPIO_PIN_9 ,GPIOD,GPIO_PIN_5 ,GPIOB,GPIO_PIN_10 ,GPIOC,GPIO_PIN_10};
const Ts_gpio  usartRxIO[4]={GPIOA,GPIO_PIN_10 ,GPIOD,GPIO_PIN_6 ,GPIOB,GPIO_PIN_11 ,GPIOC,GPIO_PIN_11};
const uint32_t usartCom[4]={USART0,USART1,USART2,UART3};
const rcu_periph_enum usartRCUCom[4]  ={RCU_USART0,RCU_USART1,RCU_USART2,RCU_UART3};
const rcu_periph_enum usartRCUTxRx[4] ={RCU_GPIOA ,RCU_GPIOD ,RCU_GPIOB ,RCU_GPIOC};
const rcu_periph_enum usartRCUCTR[4]  ={RCU_GPIOA ,RCU_GPIOD ,RCU_GPIOE ,RCU_GPIOA};
const Ts_nvic usartNvic[4]={USART0_IRQn,0,2,USART1_IRQn,0,3,USART2_IRQn,1,2,UART3_IRQn,1,3};//中断优先级配置

//*************初始化USART合局变量*******************************************************************/
Queue<u8> com4rbuf(COM1RECVBUF),com4sbuf(COM1SENDBUF);
Queue<u8> com1rbuf(64),com1sbuf(16);
Queue<u8> com3rbuf(64),com3sbuf(16);
UsartPort gcom4(COM4,4800,&com4rbuf,&com4sbuf);
UsartPort gcom1(COM1,9600,&com1rbuf,&com1sbuf);  //背板RS485_1
UsartPort gcom3(COM3,9600,&com3rbuf,&com3sbuf);  //背板RS485_2
UsartPort *pusart4=&gcom4;
//UsartPort *pusartSwitch=&gcom1;
//UsartPort *pusartBatt=&gcom3;
UsartPort *pusartSwitch = &gcom3;
UsartPort *pusartBatt   = &gcom1;
//#pragma arm section code=".ARM.__at_0x8002440" 
extern u8 UpdateFlag;
extern u16 packlen;
#ifdef __cplusplus
 extern "C" {
#endif
	
/*usart时钟及GPIO配置*/
void usart_gpio_config(u8 Comid)
{
	         rcu_periph_clock_enable(RCU_AF);
	         rcu_periph_clock_enable( usartRCUCom[Comid]);//串口时钟使能
	         rcu_periph_clock_enable(usartRCUTxRx[Comid]);//收发引脚时钟使能
	         rcu_periph_clock_enable( usartRCUCTR[Comid]);//控制引脚时钟使能
	         gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
	         gpio_init(usartRxIO[Comid].gpio_periph , GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,usartRxIO[Comid].pin); //USART_RX
	         gpio_init(usartTxIO[Comid].gpio_periph , GPIO_MODE_AF_PP,       GPIO_OSPEED_50MHZ,usartTxIO[Comid].pin); //USART_TX
	         gpio_init(usartCtrIO[Comid].gpio_periph,GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,usartCtrIO[Comid].pin);//USART_485_CTR
					 gpio_bit_reset(usartCtrIO[Comid].gpio_periph, usartCtrIO[Comid].pin);//设置为接收模式
}
	


	
	 


/*usart寄存器配置*/
void usart_config(u8 Comid,uint16_t usart_baudrate)
{
	
	 /* USART configure */
    usart_deinit( usartCom[Comid]);
    usart_baudrate_set( usartCom[Comid], usart_baudrate);
    usart_word_length_set( usartCom[Comid], USART_WL_8BIT);
    usart_stop_bit_set( usartCom[Comid], USART_STB_1BIT);
    usart_parity_config( usartCom[Comid], USART_PM_NONE);
    usart_hardware_flow_rts_config( usartCom[Comid], USART_RTS_DISABLE);
    usart_hardware_flow_cts_config( usartCom[Comid], USART_CTS_DISABLE);
    usart_receive_config( usartCom[Comid], USART_RECEIVE_ENABLE);
    usart_transmit_config( usartCom[Comid], USART_TRANSMIT_ENABLE);
    usart_enable( usartCom[Comid]);
	
	
	
	
}



/*usart中断优先级配置*/
void usart_nvic_config(u8 Comid)
{
	
	nvic_irq_enable(usartNvic[Comid].nvic_irq,
	                usartNvic[Comid].nvic_irq_pre_priority,
	                usartNvic[Comid].nvic_irq_sub_priority);

}




/*Usart初始化*/
void Usart_Init(u8 Comid,uint16_t usart_baudrate)
{
	usart_gpio_config(Comid);
	usart_config(Comid,usart_baudrate);
	usart_nvic_config(Comid);
	usart_interrupt_enable(usartCom[Comid], USART_INT_RBNE);

}

void UART3_IRQHandler ()
{
	
	UsartPort::IRQpfn(gcom4);
}


void USART0_IRQHandler ()
{
	
	UsartPort::IRQpfn(gcom1);
}


void USART2_IRQHandler ()
{
	
	UsartPort::IRQpfn(gcom3);
}

#ifdef __cplusplus
}
#endif

void UsartPort::EnableRecv(void)
{
	usart_interrupt_enable(usartCom[m_Comid], USART_INT_RBNE);
}

void UsartPort::DisableRecv(void)
{
	usart_interrupt_disable(usartCom[m_Comid], USART_INT_RBNE);
}


UsartPort::UsartPort(u8 Comid,u16 bouad,Queue<u8> *rdatbuf,Queue<u8> *sdatbuf)
{
	   Usart_Init(Comid,bouad);
	         m_precvdat=rdatbuf;
	         m_psenddat=sdatbuf;
	         m_pCtrIo=(Ts_gpio *)&usartCtrIO[Comid];
	         m_Comid=Comid;
	         m_rendflag=0;
	         m_tail=0x0d;

}



void UsartPort::SendData(u8 *pbuf,u16 len)
{
	u16 i=0;

		
			 if(m_psenddat==NULL) return;
			 if(m_pCtrIo!=NULL)
			  gpio_bit_set(m_pCtrIo->gpio_periph, m_pCtrIo->pin);//设置为发送模式
				while(len--)
				{
					m_psenddat->push(pbuf[i++]);
		
				}

				 usart_interrupt_enable(usartCom[m_Comid], USART_INT_TBE); 
		
}

void UsartPort::SendDataEx(u8 *pbuf,u16 len)
{

	
	    m_psenddat->base=pbuf;
	    m_psenddat->write=len;
	    m_psenddat->maxsize=4096;
	    m_psenddat->read=0;
	
	   gpio_bit_set(m_pCtrIo->gpio_periph, m_pCtrIo->pin);//设置为发送模式
     usart_interrupt_enable(usartCom[m_Comid], USART_INT_TBE); 
	
}
void UsartPort::IRQpfn(UsartPort &s)
{u8 tmp;
	bool flag;
	
	if( usart_flag_get(usartCom[s.m_Comid], USART_FLAG_PERR) != RESET)
	{
		  usart_data_receive(usartCom[s.m_Comid]);
	}
	else if(usart_flag_get(usartCom[s.m_Comid], USART_FLAG_ORERR) != RESET)
	{
		   usart_data_receive(usartCom[s.m_Comid]);
	}
	else if(usart_flag_get(usartCom[s.m_Comid], USART_FLAG_FERR) != RESET)
	{
		   usart_data_receive(usartCom[s.m_Comid]);
	}
	
	if(usart_interrupt_flag_get(usartCom[s.m_Comid], USART_INT_FLAG_RBNE) != RESET)
	{
		   tmp=(u8)usart_data_receive(usartCom[s.m_Comid]);
		    
		    if(s.m_precvdat==NULL)  //接收指针为空,返回
							return ; 
            flag=s.m_precvdat->push(tmp)	;					
						if(!flag)//接收队列满，接收失败，返回
							return ;
						
						  if(s.m_tail==tmp)
						 {
							 s.m_rendflag=1;
						 }
	}
	else if(usart_interrupt_flag_get(usartCom[s.m_Comid], USART_INT_FLAG_TBE) != RESET)
	{
		       if(s.m_psenddat->IsEmpty())//发送队列为空,返回
						 {
							  usart_interrupt_disable(usartCom[s.m_Comid],USART_INT_TBE);//关发送缓冲空中断
							  usart_interrupt_enable(usartCom[s.m_Comid], USART_INT_TC);//打开发送完成中断
							 
						 }
						 else //发送队列不为空,
						 {
							     tmp =s.m_psenddat->top();
		            
							    usart_data_transmit(usartCom[s.m_Comid], tmp);
		               s.m_psenddat->pop();
						 }
	}else if(usart_interrupt_flag_get(usartCom[s.m_Comid], USART_INT_FLAG_TC) != RESET)
	{
		      usart_interrupt_flag_clear(usartCom[s.m_Comid], USART_INT_FLAG_TC);//清除收送完成中断标专
		      if(s.m_pCtrIo!=NULL)//若485控制脚存在
			    gpio_bit_reset(s.m_pCtrIo->gpio_periph, s.m_pCtrIo->pin);//设置为接收模式
				 usart_interrupt_disable(usartCom[s.m_Comid],USART_INT_TC);//关发送完成中断
	}
	
	
}

u16 gsendlen;
u8 * gpsend;
void  UsartPort::onRecvFrame(PTRFUN1 pFun)
{size_t _size,tmp;
	u8 *pdat=NULL;
	u8 *psend=NULL;
	u16 i=0;
   
	 
	  if(UpdateFlag==0)//非升级状态情况下
		{
	    if(m_rendflag==0) //等待0x0D
			{
				if(m_precvdat->IsFull())
				{
					  m_precvdat->write=0;
						m_precvdat->read=0;
				}
				
				return;
				
			}
	      m_rendflag=0;
			_size=m_precvdat->size();
			
			 tmp=_size;
	     if(_size==0)
				 return;
		 DisableRecv();//关串口接收中断
			 

			 	for(u16 t=m_precvdat->read;t<m_precvdat->write;t++)
				{    if(((m_precvdat->base[t]=='M')&&(m_precvdat->base[t+1]=='N')&&(m_precvdat->base[t+2]=='C'))||(m_precvdat->base[t]==0x7E))
						{

							m_precvdat->read=t;
							 break;
						}
			  }
			 
			  pdat=&m_precvdat->base[m_precvdat->read];
			 
			 
			 
			 
			 
			 
			 

			if(pFun==NULL)
			{
			  SendData(pdat,(u8)_size);
			}
			else{
				
				 u16 datlen=0;
				psend=(u8 *)pFun((u8 *)pdat,&datlen);
				
				if(psend!=NULL)
				{
					
				
					if(datlen==0)
					datlen= strlen((const char *)psend);//获取数据段长度
					
					if(datlen>0)
					{
						 SendDataEx(psend,datlen);//响应请求
					}
					
          	psend=NULL;				
					
				}

			}
			
				m_precvdat->read=0;
				m_precvdat->write=0;
				EnableRecv();//打开串口接收中断
			  
	
		}
		else if(UpdateFlag==1) //处理在线升级情况
		{	
			 u8 _ERR=0;
			 _size=m_precvdat->size();
				//taskENTER_CRITICAL();
			 if(_size< packlen)//接收数据包时为1046 接收校验包长为22
			 {
				 
				 

				 if(m_precvdat->IsFull())//接收缓冲满
				{
					  m_precvdat->write=0;
						m_precvdat->read=0;
					 _ERR=1;
				}
				else
				{
				  return;
				}
			 }
			 
			 
		DisableRecv();//关串口接收中断
			 
			 if(_ERR==1)//接收缓冲满错误处理
			 {
				   
				    pdat[7]=UPDATE_ERR;//返回错
			 }
			 else//无错误，复制接收数据
			{
   						tmp=_size;
      ////////////通信错误，开头多了几个无关字节/////////////////////

				for(u16 t=m_precvdat->read;t<m_precvdat->write;t++)
				{    if((m_precvdat->base[t]=='M')&&(m_precvdat->base[t+1]=='N')&&(m_precvdat->base[t+2]=='C'))
						{

							m_precvdat->read=t;
							 break;
						}
			  }
			////////////////////////////////////////////////////////////////////	
				
				

				 pdat=&m_precvdat->base[m_precvdat->read];
				
				
				

		    }
			
			        u16 datlen=0;
			        psend=(u8 *)pFun((u8 *)pdat,&datlen);//调用接收处理函数
								
								if(psend!=NULL)
								{
									
								 
									
									if(datlen>0)
									{
										 SendDataEx(psend,datlen);//响应请求
									}
									//vPortFree(psend);//释放发送数据内存
								
										psend=NULL;				
									
							}

								
							m_precvdat->read=0;
							m_precvdat->write=0;
							EnableRecv();//打开串口接收中断
	
    }
}


void UsartPort::CRCcheck(u8 *pdat,u8 len)
{	//计算CRC校验值
		u16 regcrc=0xFFFF;
			for( u8 i=0;i<len-2;i++)
			{
				regcrc=regcrc^(u16)pdat[i];
				for(u8 j=0;j<8;j++)
				{
					   if((regcrc&0x0001)==0x0001)
						 {
							  regcrc=regcrc>>1;
							 regcrc=regcrc^0xA001;
					
							
						 }
             else
						 {
							 regcrc=regcrc>>1;
						 }
						 
							
				}
				
				
				
			}
			
     *(u16 *)&pdat[len-2]=regcrc;
		}

void  UsartPort::onRecvModulebus(PTRFUN1 pFun)
 { size_t _size;
	 u8 *pdat=NULL,i=0;
	 
	  _size=m_precvdat->size();
	  pdat=m_precvdat->base;
	   
	     if(_size==0)
				 return;



			u16 getcrc,regcrc;
			getcrc=*((u16*)(&pdat[_size-2]));//获取CRC校验值
			*((u16*)(&pdat[_size-2]))=0;//清零CRC位置，待新计算，以验以接收数据是否正确
			 CRCcheck( pdat, _size);
			regcrc=*((u16*)(&pdat[_size-2]));//获取CRC校验值
			
				
    
			 if(getcrc==regcrc)//验以接收数据是否正确
			 {
				 if(pFun!=NULL)
			   pFun((u8 *)pdat,NULL);
				
			 }
			  m_precvdat->write=0;
			  m_precvdat->read=0;
		
       pdat	= NULL;	
	 
 }
//#pragma arm section







