#include "tcp.h"
#include "ydt1363.h"

tcp gtcp;
tcp *ptcp=&gtcp;
//u32 tcp_err=0;
//u8 tcp_star=0;

//extern u8 nettest1,nettest3;
//extern u8 nettest2[6];

tcp::tcp(){};
tcp::~tcp(){};	
void tcp::Init(void)
{
	 gprxinfo=&m_rxinfo;
	 gphttp_request=&m_http_request;
	 tcp_err=0;
}
	
void tcp::OnTick(uint16 port, _RX_STR  *prxinfo)
{
	uint8 ch=SOCK_TCPS;																		/*定义一个socket*/
//	uint16 len;
	//st_http_request *http_request;	/*定义一个结构指针*/
		switch(getSn_SR(ch))		/*获取socket状态*/
		{
				case SOCK_CLOSED:  /*socket处于关闭状态*/
			   socket(ch, Sn_MR_TCP, port, 0x00);   									/*打开socket*/
			break;
		
	    case SOCK_INIT:/*socket处于初始化状态*/			
			listen(ch);
			break;	
			case SOCK_LISTEN:			/*socket处于监听状态*/
		
			break;		
			case SOCK_ESTABLISHED:		/*socket处于连接状态*/

			if(getSn_IR(ch) & Sn_IR_CON)
			{
				setSn_IR(ch, Sn_IR_CON); /*清除接收中断标志位*/
			}
			prxinfo->rx_len=getSn_RX_RSR(ch);	    /*定义len为已接收数据的长度*/
			if(prxinfo->rx_len>0)
			{
				recv(ch,prxinfo->rx_buff,prxinfo->rx_len);      /*接收来自Client的数据*/
				prxinfo->rx_buff[prxinfo->rx_len]=0x00; 			  /*添加字符串结束符*/
					
				send(SOCK_TCPS,prxinfo->rx_buff,prxinfo->rx_len);									              /*向Client发送数据*/
		 	 }
		  break;
			case SOCK_CLOSE_WAIT: /*socket处于等待关闭状态*/ 
			 disconnect(ch);	
      // tcp_star=0;			
			break;
	}
}	

u8 tcp::httpOnTick(u16 port,_RX_STR  *prxinfo,uint8 ch)
{
																		/*定义一个socket*/
	uint16 len;
	u8 ret=0;
	//st_http_request *http_request;/*定义一个结构指针*/
		switch(getSn_SR(ch))		/*获取socket状态*/
		{
				case SOCK_CLOSED:  /*socket处于关闭状态*/
			   socket(ch, Sn_MR_TCP, 80, 0x00);   									/*打开socket*/
			break;
		
	    case SOCK_INIT:/*socket处于初始化状态*/	
         			
			     listen(ch);
			     tcp_err=100;
			break;	
			case SOCK_LISTEN:			/*socket处于监听状态*/
				    if(!(getPHYCFGR()&0x01))
								{
									
									close(ch);
								}
								if(tcp_err>0)
									tcp_err--;
								if(tcp_err==0)
								{
									close(ch);
								}

			break;		
			case SOCK_ESTABLISHED:		/*socket处于连接状态*/
				     tcp_err=500;
			memset(prxinfo->rx_buff,0x00,MAX_URI_SIZE);
			//http_request = (st_http_request*)prxinfo->rx_buff;	
			
						if(!(getPHYCFGR()&0x01))
								{
									
									
									//网络已断开
											disconnect(ch);
									   // close(ch);
								}
								else
								{
									//printf("网络已经连接 \r\n");
									
											if(getSn_IR(ch) & Sn_IR_CON)
											{
												setSn_IR(ch, Sn_IR_CON);/*清除中断标志位*/
											}
											if ((len = getSn_RX_RSR(ch)) > 0)		
											{
												len = recv(ch, prxinfo->rx_buff, len); 	/*接收http请求*/

											
												prxinfo->rx_buff[len] = 0;
												

												
												
												
												
												proc_http(ch, prxinfo->rx_buff,NULL);		/*接收http请求并发送http响应*/
												
										 
                       
												
											}
											else
											{
												disconnect(ch);
												close(ch);
												ret=1;
											}
											
								}
			
			
			
			
      
		
//			else
//			{
//				disconnect(ch);
//			}
		
			  	   
		  break;
			case SOCK_CLOSE_WAIT: /*socket处于等待关闭状态*/ 
			 disconnect(ch);
      // tcp_star=0;			
			break;
			default :break;
	}
		
	

	
	
	 
	
	
	
	
	
	
	
	return ret;
	
	
	
	
	
}	


