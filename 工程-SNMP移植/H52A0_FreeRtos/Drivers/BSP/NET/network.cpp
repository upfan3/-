#include "network.h"
#define FW_VER_HIGH 1
#define FW_VER_LOW 0

u8 gMAC[6] = {66, 43, 64, 65, 64, 67};
u8 gIP[4] = {192, 168, 12, 112};
u8 gGW[4] = {192, 168, 12, 1};
u8 gMask[4] = {255, 255, 255, 0};
u8 gDNS[4] = {114, 114, 114, 114};

uint8 txsize[MAX_SOCK_NUM] = {2, 2, 2, 2, 2, 2, 2, 2}; // 选择8个Socket每个Socket发送缓存的大小，在w5500.c的void sysinit()有设置过程
uint8 rxsize[MAX_SOCK_NUM] = {2, 2, 2, 2, 2, 2, 2, 2}; // 选择8个Socket每个Socket接收缓存的大小，在w5500.c的void sysinit()有设置过程

network gnetwork;

network::network()
{
	//	  WIZ_SPI_Init();
	//	  iinchip_init();
	//	  w5500_reset();
	//
	//	  setDefault();
	//	  setNetPara();
	//	  setKeepAlive(0);
}

void network::Init()
{
	WIZ_SPI_Init();
	iinchip_init();
	w5500_reset();

	setDefault();
	setNetPara();
	setKeepAlive(0);
	setKeepAlive(1);
	setKeepAlive(2);
	setKeepAlive(3);
	setKeepAlive(4);
	setKeepAlive(5);
	setKeepAlive(6);
	setKeepAlive(7);
	m_delaytimer = 0;
	m_constatus = 0;
}

void network::setIP(u8 *pIP)
{
	memcpy(m_ConfigMsg.lip, pIP, 4);
}

void network::setMac(u8 *pMac)
{
	memcpy(m_ConfigMsg.mac, pMac, 6);
}

void network::setSub(u8 *pSub)
{
	memcpy(m_ConfigMsg.sub, pSub, 4);
}

void network::setGw(u8 *pGw)
{
	memcpy(m_ConfigMsg.gw, pGw, 4);
}

void network::setDns(u8 *pDns)
{
	memcpy(m_ConfigMsg.dns, pDns, 4);
}

void network::setDefault(void) // 设置默认MAC、IP、GW、SUB、DNS
{
	// uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};
	// uint8 lip[4]={192,168,1,112};
	uint8 sub[4] = {255, 255, 255, 0};
	// uint8 gw[4]={192,168,13,1};
	uint8 dns[4] = {114, 114, 114, 114};
	memcpy(m_ConfigMsg.lip, gIP, 4);
	memcpy(m_ConfigMsg.sub, sub, 4);
	memcpy(m_ConfigMsg.gw, gGW, 4);
	memcpy(m_ConfigMsg.mac, gMAC, 6);
	memcpy(m_ConfigMsg.dns, dns, 4);

	m_ConfigMsg.dhcp = 0;
	m_ConfigMsg.debug = 1;
	m_ConfigMsg.fw_len = 0;

	m_ConfigMsg.state = NORMAL_STATE;
	m_ConfigMsg.sw_ver[0] = FW_VER_HIGH;
	m_ConfigMsg.sw_ver[1] = FW_VER_LOW;
}

void network::setNetPara(void)
{
	uint8 local_ip[6] = {0, 0, 0, 0, 0, 0};

	setSHAR(m_ConfigMsg.mac);
	setSUBR(m_ConfigMsg.sub);
	setGAR(m_ConfigMsg.gw);
	setSIPR(m_ConfigMsg.lip);

	getSIPR(local_ip);
	getSUBR(local_ip);
	getGAR(local_ip);
	getSHAR(local_ip);

	sysinit(txsize, rxsize); // 初始化8个socket
	setRTR(2000);			 // 设置超时时间
	setRCR(3);				 // 设置最大重新发送次数

	m_delaytimer = 0;
	m_constatus = 0;
}

void network::setKeepAlive(SOCKET s)
{
	setkeepalive(s);
}
void network::OsRest(void)
{
	iinchip_init();
	w5500_reset();
	setDefault();
	setNetPara();
	// sysinit(txsize, rxsize);// 初始化8个socket
	setKeepAlive(0);
	setKeepAlive(1);
	setKeepAlive(2);
	setKeepAlive(3);
	setKeepAlive(4);
	setKeepAlive(5);
	setKeepAlive(6);
	setKeepAlive(7);
	m_delaytimer = 0;
	m_constatus = 0;
}
void network::ScanState(u8 *ptcp_err)
{
	if (m_delaytimer > 3)
	{
		if (!(getPHYCFGR() & 0x01))
		{
			// printf("网络没有连接 \r\n");
			//  OsRest();
			m_constatus = 0;
		}
		else
		{
			// printf("网络已经连接 \r\n");
			m_constatus = 1;
			//						  if( *ptcp_err>100)//tcp错误超100次
			//							{
			//								 OsRest();//复位网络
			//								*ptcp_err=0;
			//							}
		}
	}
}

void network::onTick(void)
{
	if (m_delaytimer < 10)
	{
		m_delaytimer++;
	}
}
