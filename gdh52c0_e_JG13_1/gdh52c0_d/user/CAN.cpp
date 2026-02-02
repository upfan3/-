#include "CAN.h"
//
#define CAN_ID_1 0
#define CAN_ID_2 1

#define CAN1_TX_BUF_SIZE 11
#define CAN1_RX_BUF_SIZE 64

#define CAN2_TX_BUF_SIZE 5
#define CAN2_RX_BUF_SIZE 40
#define CAN2_RX_BUF_SIZE1 40

#define XMODEM_DATA_SIZE 256

//---------------------------GD32F105 usart硬件相关配置----------------------------------------------

const Ts_gpio  canTxIO[2]={GPIOA,GPIO_PIN_12,GPIOB,GPIO_PIN_13};
const Ts_gpio  canRxIO[2]={GPIOA,GPIO_PIN_11,GPIOB,GPIO_PIN_12};
const uint32_t canPort[2]={CAN0,CAN1};
const rcu_periph_enum canRCUCom[2]={RCU_CAN0,RCU_CAN1};
const rcu_periph_enum canRCUTxRx[2]={RCU_GPIOA,RCU_GPIOB};
const Ts_nvic canTxNvic[2]={ CAN0_TX_IRQn,0,1, CAN1_TX_IRQn,1,1};
const Ts_nvic canRxNvic[2]={CAN0_RX0_IRQn,0,0, CAN1_RX0_IRQn,1,0};//Can fifo0接收中断
const Ts_nvic canRxfifo1Nvic[2]={CAN0_RX1_IRQn,0,0,CAN1_RX1_IRQn,1,0};//Can fifo1接收中断 与fifo0处于同一中断优先级。
//*************初始化CAN合局变量**********************************/

Queue<CanBuf> gCan1RecvBuf(CAN1_RX_BUF_SIZE),gCan2RecvBuf(CAN2_RX_BUF_SIZE),gCan1SendBuf(CAN1_TX_BUF_SIZE),gCan2SendBuf(CAN2_TX_BUF_SIZE),gCan2SendBuf2(CAN2_RX_BUF_SIZE1);

 CanPort gCan1(CAN_ID_1,125,&gCan1RecvBuf,&gCan1SendBuf),gCan2(CAN_ID_2,250,&gCan2RecvBuf,&gCan2SendBuf);
 CanPort *pcan1=&gCan1;
 CanPort *pcan2=&gCan2;


#ifdef __cplusplus
 extern "C" {
#endif
	
/*CAN时钟及GPIO配置*/
void can_gpio_config(u8 canId)
{
	
	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(canRCUCom[canId]);
	rcu_periph_clock_enable(canRCUTxRx[canId]);
	
	gpio_init(canRxIO[canId].gpio_periph,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,canRxIO[canId].pin); //CAN_RX
	gpio_init(canTxIO[canId].gpio_periph,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,canTxIO[canId].pin); //CAN_TX

}


/*CAN寄存器配置*/
void can_config(u8 canId,uint16_t cand_baudrate)
{
  can_parameter_struct can_parameter;
	can_filter_parameter_struct can_filter;
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_INIT_STRUCT, &can_filter);
    /* initialize CAN register */

	 can_deinit(canPort[canId]);
	    
    /* initialize CAN parameters */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.no_auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_9TQ;//CAN_BT_BS1_5TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_8TQ;//CAN_BT_BS2_3TQ;


/*波特率计算方法*/
	 can_parameter.prescaler=2000/cand_baudrate;
    /* initialize CAN */
    can_init(canPort[canId], &can_parameter);

    /* initialize filter */ 
		
		
    can_filter.filter_number=0;
		if(canPort[canId]==CAN1)
	  {	
		  can_filter.filter_number=15;
	  }
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    
    can_filter_init(&can_filter);

	/*
	else if(canPort[canId]==CAN1)
	{

		//滤波参数配置
		//------------------------------滤波组1--设备参数 --------------------------------------------------
		
		{
		u32 FilterID=0;
			         ((PCid*)(&FilterID))->st_CID1=DEVICE_PARA; //设备参数 
			             FilterID=FilterID<<3;//左对齐
			              FilterID= FilterID+4;//扩展帧，数据帧
			  u32 FilterMasK=0;          
			         ((PCid*)(&FilterMasK))->st_CID1=CID1_MASK; //CID1为5位，b(11111)=31 关注CID1所有位
		                     FilterMasK=FilterMasK<<3;//左对齐
			                   FilterMasK= FilterMasK+6;//将扩展帧，数据帧过滤出来，不过滤远程帧
		
								
						can_filter.filter_number=15;
						can_filter.filter_mode = CAN_FILTERMODE_MASK;
						can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		
						can_filter.filter_list_high = FilterID>>16;
						can_filter.filter_list_low = (u16)FilterID;
						can_filter.filter_mask_high =  FilterMasK>>16;
						can_filter.filter_mask_low = (u16)FilterMasK;
						can_filter.filter_fifo_number = CAN_FIFO0;
						can_filter.filter_enable = ENABLE;
						can_filter_init(&can_filter);
			}
		//------------------------------滤波组2--时间同步 --------------------------------------------------		
		{
			u32 FilterID=0;
			         ((PCid*)(&FilterID))->st_CID1=REMOTE_CONTROL; //设备参数 
							 ((PCid*)(&FilterID))->st_CID2=0x080;//抓取时间同步帧
			             FilterID=FilterID<<3;//左对齐
			              FilterID= FilterID+4;//扩展帧，数据帧
			 u32 FilterMasK=0;          
			         ((PCid*)(&FilterMasK))->st_CID1=CID1_MASK; //CID1为5位，b(11111)=31 关注CID1所有位
							 ((PCid*)(&FilterMasK))->st_CID2=CID2_MASK;//关注CID2所有位
		                     FilterMasK=FilterMasK<<3;//左对齐
			                   FilterMasK= FilterMasK+4;//将扩展帧，数据帧过滤出来，不过滤远程帧
						
		
		      	can_filter.filter_number=16;
						can_filter.filter_mode = CAN_FILTERMODE_MASK;
						can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		
						can_filter.filter_list_high = FilterID>>16;
						can_filter.filter_list_low = (u16)FilterID;
						can_filter.filter_mask_high =  FilterMasK>>16;
						can_filter.filter_mask_low = (u16)FilterMasK;
						can_filter.filter_fifo_number = CAN_FIFO0;
						can_filter.filter_enable = ENABLE;
						can_filter_init(&can_filter);
					}		
		//------------------------------滤波组3--统计帧参数 --------------------------------------------------	
    {					
			u32 FilterID=0;
			         ((PCid*)(&FilterID))->st_CID1=TOTAL; //统计帧
			             FilterID=FilterID<<3;//左对齐
			              FilterID= FilterID+4;//扩展帧，数据帧
			u32 FilterMasK=0;          
			         ((PCid*)(&FilterMasK))->st_CID1=CID1_MASK; //CID1为5位，b(11111)=31 关注CID1所有位
		                     FilterMasK=FilterMasK<<3;//左对齐
			                   FilterMasK= FilterMasK+6;//将扩展帧，数据帧过滤出来，不过滤远程帧
		
		
		
		   	can_filter.filter_number=17;
						can_filter.filter_mode = CAN_FILTERMODE_MASK;
						can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		
						can_filter.filter_list_high = FilterID>>16;
						can_filter.filter_list_low = (u16)FilterID;
						can_filter.filter_mask_high =  FilterMasK>>16;
						can_filter.filter_mask_low = (u16)FilterMasK;
						can_filter.filter_fifo_number = CAN_FIFO0;
						can_filter.filter_enable = ENABLE;
						can_filter_init(&can_filter);
		    
			 
		 } 
		 
		//------------------------------滤波组4--非过滤（基本数据帧） --------------------------------------------------			 
		{	 
			 can_filter.filter_number=18;
						can_filter.filter_mode = CAN_FILTERMODE_MASK;
						can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		
						can_filter.filter_list_high =0;
						can_filter.filter_list_low = 0;
						can_filter.filter_mask_high = 0;
						can_filter.filter_mask_low = 0;
						can_filter.filter_fifo_number = CAN_FIFO1;
						can_filter.filter_enable = ENABLE;
						can_filter_init(&can_filter);
			 
		 } 
		
	}

		*/
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
}


/*CAN中断优先级配置*/
void nvic_config(u8 canId)
{
	
	nvic_irq_enable(canTxNvic[canId].nvic_irq,
	                canTxNvic[canId].nvic_irq_pre_priority,
	                canTxNvic[canId].nvic_irq_sub_priority);
	
	
	nvic_irq_enable(canRxNvic[canId].nvic_irq,
	                canRxNvic[canId].nvic_irq_pre_priority,
	                canRxNvic[canId].nvic_irq_sub_priority);
	
}

/*CAN初始化*/
void CAN_Init(u8 canId,uint16_t cand_baudrate)
{
	can_gpio_config(canId);
	nvic_config(canId);
	can_config(canId,cand_baudrate);
	can_interrupt_enable(canPort[canId], CAN_INT_RFNE0);
  can_interrupt_enable(canPort[canId], CAN_INTEN_TMEIE); 
	

}


void CAN0_RX0_IRQHandler(void)
{
	CanPort::rIRQpfn(gCan1,0);//fifo0数据
}

void CAN0_RX1_IRQHandler(void)
{
	CanPort::rIRQpfn(gCan1,1);//fifo1数据
}

void CAN1_RX0_IRQHandler(void)
{
	CanPort::rIRQpfn(gCan2,0);//fifo0数据
}

void CAN1_RX1_IRQHandler(void)
{
	CanPort::rIRQpfn(gCan2,1);//fifo1数据
}


void CAN0_TX_IRQHandler(void)
{
	CanPort::sIRQpfn(gCan1);
}

void CAN1_TX_IRQHandler(void)
{
	CanPort::sIRQpfn(gCan2);
}

#ifdef __cplusplus
}
#endif



//*************Funtion of Can Class*******************************/

CanPort:: CanPort(u8 canId,u16 bouad,Queue<CanBuf> *rdatbuf,Queue<CanBuf> *sdatbuf)
{
	CAN_Init(canId,bouad);
	m_canId=canId;
	m_bouad=bouad;
	m_precvdat=rdatbuf;
	m_psenddat=sdatbuf;

}


u8 CanPort::SendCanMessage(PCanBuf data)
{
	can_trasnmit_message_struct TxMessage;
    u8 TransmitMailbox = 0;
	u32 i;
	if(data->ide == 1)    // 扩展帧
    {
        TxMessage.tx_ff = CAN_FF_EXTENDED;
        TxMessage.tx_efid = data->id;
    }
    else                                    // 标准帧
    {
        TxMessage.tx_ff = CAN_FF_STANDARD;
        TxMessage.tx_sfid = data->id;
    }
	if(data->rtr == 1)   // 远程帧
    {
        TxMessage.tx_ft = CAN_FT_REMOTE;
    }
    else
    {
       TxMessage.tx_ft = CAN_FT_DATA;       // 数据帧
       TxMessage.tx_dlen = data->dlc;   //数据长度
		for(i=0;i<8;i++)
			TxMessage.tx_data[i] =data->data[i];
   }
	// 发送数据，返回此次发送所使用的邮箱号码，如果没有空邮箱，则返回CAN_NO_MB
        TransmitMailbox = can_message_transmit(canPort[m_canId], &TxMessage);
		if(CAN_NOMAILBOX==TransmitMailbox)
			return FALSE;
 	return TRUE;
}

u8 CanPort:: SendData(PCanBuf pdat)
{     
	 if(m_psenddat==NULL) return 0;//发送队列为空时，返回0
	   if(m_psenddat->IsEmpty())
		 { 
			 can_interrupt_enable(canPort[m_canId], CAN_INT_TME);//发送队列空，开中断
			  if(SendCanMessage(pdat)==TRUE)
			 { 
				 return 2;//成功发送返回2
				} 
			 else
			 {
				  if(m_psenddat->push(* pdat))
			       return 1;//成功写入发送队列返回1
			    else
				     return 0;//写入发送队列失败返回0
			 }
				 
		 }
		 else{
			 if(m_psenddat->push(* pdat))
			       return 1;//成功写入发送队列返回1
			 else
				     return 0;//写入发送队列失败返回0
		 }
			 
}

void CanPort:: sIRQpfn(CanPort &s)
{

	
	if(s.m_psenddat->IsEmpty())
				 {
					
					 //CAN_ITConfig(_CANx,CAN_IT_TME, DISABLE);//发送队列空，关中断
					 can_interrupt_disable(canPort[s.m_canId], CAN_INT_TME);//发送队列空，关中断
				 }
				 else{
					
	        //SendCanMessage( &psenddat->top());//不为空，发送队列中的，数据
					 if(s.SendCanMessage(&s.m_psenddat->top())==TRUE)
					   {
						      s.m_psenddat->pop();//弹出已发送数据
						 }
					   else
						 {
							 //发送失败，不弹出数据
						 }
				 }
	
	
}

void CanPort::WriteToRecvBuf(can_receive_message_struct* RxMessage,u8 fifo)
{
	CanBuf rbuf;
	u32 i;
   Queue<CanBuf> *_precvdat=NULL; 
	
	  if(fifo==0)
			_precvdat=m_precvdat;  //选择fifo0缓冲队列
		else
			_precvdat=m_precvdat1; //选择fifo1缓冲队列
		
	  if( _precvdat==NULL)return ;
	  if( _precvdat->IsFull())return ;
		
	 	if(RxMessage->rx_ff == CAN_FF_STANDARD)
	 	{
	 		rbuf.ide=0;
			rbuf.id=RxMessage->rx_sfid;
	 	}
		else
		{
			rbuf.ide=1;
			rbuf.id=RxMessage->rx_efid;
		}
		if( CAN_FT_REMOTE==RxMessage->rx_ft)
			{
				rbuf.rtr=1;
			}
		else
			{
				rbuf.rtr=0;
				rbuf.dlc=RxMessage->rx_dlen;
				for(i=0;i<8;i++)
					rbuf.data[i]=RxMessage->rx_data[i];
			}
		
        _precvdat->push(rbuf);
}




void CanPort::rIRQpfn(CanPort &s,u8 fifo)
{
	can_receive_message_struct RxMessage;
  
    RxMessage.rx_sfid=0x00;       // 标准帧ID
    RxMessage.rx_efid=0x00;       // 扩展帧ID
    RxMessage.rx_ff=0;            // 帧类型 CAN_ID_STD为标准帧，CAN_ID_EXT为扩展帧
    RxMessage.rx_dlen=0;            // 数据长度
    RxMessage.rx_fi=0;            // 过滤器
    RxMessage.rx_data[0]=0x00;     // 数据内容
    RxMessage.rx_data[1]=0x00;

    
	  if(fifo==0)
		{
	    can_message_receive(canPort[s.m_canId], CAN_FIFO0, &RxMessage);
		}
		else
		{
		  can_message_receive(canPort[s.m_canId], CAN_FIFO1, &RxMessage);
		}			
	  s._rendflag=1;
    s.WriteToRecvBuf(&RxMessage,fifo); 
	  

}


bool CanPort::onRecvFrame(u8 _fifo,CanBuf *prbuf)
{
	
	     Queue<CanBuf> *_precvdat;
	  if(_fifo==0)
		{
			 _precvdat=m_precvdat;
		}
		else
		{
			_precvdat=m_precvdat1;
		}
		
	  if( _precvdat==NULL) return false ;
	  if( _precvdat->IsEmpty())return false;
	  if(prbuf==NULL)return false;
	   *prbuf=_precvdat->top();
	   _precvdat->pop();
	       return true;
	
	
}

void CanPort::SetCanFilterAndFIFO1buf(Queue<CanBuf> *_precvdat1)
{
	  m_precvdat1=_precvdat1;
	
  can_interrupt_disable(canPort[m_canId], CAN_INT_RFNE0);//先关闭已打开的fifo0中断
	
	
	nvic_irq_enable(canRxfifo1Nvic[m_canId].nvic_irq,
	                canRxfifo1Nvic[m_canId].nvic_irq_pre_priority,
	                canRxfifo1Nvic[m_canId].nvic_irq_sub_priority);
    
	
	can_filter_parameter_struct can_filter;
	 	//滤波参数配置
		//------------------------------滤波组1--设备参数 --------------------------------------------------
		
		{
		u32 FilterID=0;
			         ((PCid*)(&FilterID))->st_CID1=DEVICE_PARA; //设备参数 
			             FilterID=FilterID<<3;//左对齐
			              FilterID= FilterID+4;//扩展帧，数据帧
			  u32 FilterMasK=0;          
			         ((PCid*)(&FilterMasK))->st_CID1=CID1_MASK; //CID1为5位，b(11111)=31 关注CID1所有位
		                     FilterMasK=FilterMasK<<3;//左对齐
			                   FilterMasK= FilterMasK+6;//将扩展帧，数据帧过滤出来，不过滤远程帧
		
								
						can_filter.filter_number=15;
						can_filter.filter_mode = CAN_FILTERMODE_MASK;
						can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		
						can_filter.filter_list_high = FilterID>>16;
						can_filter.filter_list_low = (u16)FilterID;
						can_filter.filter_mask_high =  FilterMasK>>16;
						can_filter.filter_mask_low = (u16)FilterMasK;
						can_filter.filter_fifo_number = CAN_FIFO0;
						can_filter.filter_enable = ENABLE;
						can_filter_init(&can_filter);
			}
		//------------------------------滤波组2--时间同步 --------------------------------------------------		
		{
			u32 FilterID=0;
			         ((PCid*)(&FilterID))->st_CID1=REMOTE_CONTROL; //设备参数 
							 ((PCid*)(&FilterID))->st_CID2=0x080;//抓取时间同步帧
			             FilterID=FilterID<<3;//左对齐
			              FilterID= FilterID+4;//扩展帧，数据帧
			 u32 FilterMasK=0;          
			         ((PCid*)(&FilterMasK))->st_CID1=CID1_MASK; //CID1为5位，b(11111)=31 关注CID1所有位
							 ((PCid*)(&FilterMasK))->st_CID2=CID2_MASK;//关注CID2所有位
		                     FilterMasK=FilterMasK<<3;//左对齐
			                   FilterMasK= FilterMasK+4;//将扩展帧，数据帧过滤出来，不过滤远程帧
						
		
		      	can_filter.filter_number=16;
						can_filter.filter_mode = CAN_FILTERMODE_MASK;
						can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		
						can_filter.filter_list_high = FilterID>>16;
						can_filter.filter_list_low = (u16)FilterID;
						can_filter.filter_mask_high =  FilterMasK>>16;
						can_filter.filter_mask_low = (u16)FilterMasK;
						can_filter.filter_fifo_number = CAN_FIFO0;
						can_filter.filter_enable = ENABLE;
						can_filter_init(&can_filter);
					}		
		//------------------------------滤波组3--统计帧参数 --------------------------------------------------	
    {					
			u32 FilterID=0;
			         ((PCid*)(&FilterID))->st_CID1=TOTAL; //统计帧
			             FilterID=FilterID<<3;//左对齐
			              FilterID= FilterID+4;//扩展帧，数据帧
			u32 FilterMasK=0;          
			         ((PCid*)(&FilterMasK))->st_CID1=CID1_MASK; //CID1为5位，b(11111)=31 关注CID1所有位
		                     FilterMasK=FilterMasK<<3;//左对齐
			                   FilterMasK= FilterMasK+6;//将扩展帧，数据帧过滤出来，不过滤远程帧
		
		
		
		   	can_filter.filter_number=17;
						can_filter.filter_mode = CAN_FILTERMODE_MASK;
						can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		
						can_filter.filter_list_high = FilterID>>16;
						can_filter.filter_list_low = (u16)FilterID;
						can_filter.filter_mask_high =  FilterMasK>>16;
						can_filter.filter_mask_low = (u16)FilterMasK;
						can_filter.filter_fifo_number = CAN_FIFO0;
						can_filter.filter_enable = ENABLE;
						can_filter_init(&can_filter);
		    
			 
		 } 
		 
		//------------------------------滤波组4--非过滤（基本数据帧） --------------------------------------------------			 
		{	 
			 can_filter.filter_number=18;
						can_filter.filter_mode = CAN_FILTERMODE_MASK;
						can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		
						can_filter.filter_list_high =0;
						can_filter.filter_list_low = 0;
						can_filter.filter_mask_high = 0;
						can_filter.filter_mask_low = 0;
						can_filter.filter_fifo_number = CAN_FIFO1;
						can_filter.filter_enable = ENABLE;
						can_filter_init(&can_filter);
			 
		 } 
	
	
	  can_interrupt_enable(canPort[m_canId], CAN_INT_RFNE0);
	  can_interrupt_enable(canPort[m_canId], CAN_INT_RFNE1);
	
	
}









