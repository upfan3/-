#include "tcp.h"

tcp gtcp;
tcp *ptcp = &gtcp;

u8 netdatbuf[2048];

tcp::tcp() {};
tcp::~tcp() {};
void tcp::Init(void)
{
	tcp_err = 0;
	m_lock = 0;
	// net1363.Init(netdatbuf);
}

u8 *DuleWithNetData(u8 *pdat, u16 *plen)
{

	u8 *psend;
	//
	//  if(UpdateFlag==2)
	//	{
	//		  psend=UpDateFlash(pdat,plen,2);
	//			return psend;
	//	}
	//	else if(pdat[0]==0x7E)
	//	{
	//		if(net1363.Get1363Data(pdat)==0)
	//			return net1363.m_senddata;//返回错误信息(长度或校验错误）
	//		net1363.DuleWith1363Data();
	//		  return net1363.m_senddata;//返回打包信息
	//	}
	//	else if((pdat[0]=='M')&&(pdat[1]=='N')&&(pdat[2]=='C')&&(pdat[3]=='-')&&(pdat[4]=='E')&&(pdat[5]=='N'))
	//	{
	//		  if(UpdateFlag==0)
	//			{
	//			  psend=UpDateFlash(pdat,plen,2);
	//			  return psend;
	//			}
	//			else
	//			{
	//				psend[0]='\0';//使数据长度为0
	//				return psend;
	//			}
	//
	//	}
	//	else if((pdat[2]==0)&&(pdat[3]==0))//Modbus Tcp
	//	{
	//		psend=ModbusDulewithData(pdat,plen);
	//	}
}

void tcp::OnTick(uint16 port, _RX_STR *prxinfo)
{

	uint8 ch = SOCK_TCPS; /*定义一个socket*/
						  //	uint16 len;
						  // st_http_request *http_request;	/*定义一个结构指针*/
	switch (getSn_SR(ch)) /*获取socket状态*/
	{
	case SOCK_CLOSED:					   /*socket处于关闭状态*/
		socket(ch, Sn_MR_TCP, port, 0x00); /*打开socket*/
		break;

	case SOCK_INIT: /*socket处于初始化状态*/
		listen(ch);
		tcp_err = 100;
		break;
	case SOCK_LISTEN: /*socket处于监听状态*/
		if (!(getPHYCFGR() & 0x01))
		{

			disconnect(ch);
			close(ch);
		}

		break;
	case SOCK_ESTABLISHED: /*socket处于连接状态*/

		if (!(getPHYCFGR() & 0x01))
		{

			disconnect(ch);
			close(ch);
			return;
		}

		if (getSn_IR(ch) & Sn_IR_CON)
		{
			setSn_IR(ch, Sn_IR_CON); /*清除接收中断标志位*/
		}
		prxinfo->rx_len = getSn_RX_RSR(ch); /*定义len为已接收数据的长度*/
		if (prxinfo->rx_len > 0)
		{
			tcp_err = 100;
			recv(ch, prxinfo->rx_buff, prxinfo->rx_len); /*接收来自Client的数据*/
			prxinfo->rx_buff[prxinfo->rx_len] = 0x00;	 /*添加字符串结束符*/

			u16 len = 0;
			u8 *psend = NULL;
			psend = (u8 *)DuleWithNetData(prxinfo->rx_buff, &len);

			if (len == 0)
				len = strlen((const char *)psend); // 获取数据段长度

			if (len > 0)
				send(SOCK_TCPS, psend, len); /*向Client发送数据*/
		}
		else
		{

			if (tcp_err > 0)
				tcp_err--;
			if (tcp_err == 0)
			{
				disconnect(ch);
				close(ch);
			}
		}

		break;
	case SOCK_CLOSE_WAIT: /*socket处于等待关闭状态*/
		disconnect(ch);
		// tcp_star=0;
		break;
	}
}

u8 tcp::checkConnet()
{
	if (!(getPHYCFGR() & 0x01))
	{

		return 0; // 断网
	}
	else
	{
		return 1; // 连接上
	}
}
