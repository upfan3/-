#ifndef __TCP_H
#define __TCP_H

#include "w5500.h"
#include <stdio.h>
#include <string.h>
#include "socket.h"

// #include "http_server.h"

typedef struct __rx_str
{
	u8 rx_buff[2048];
	u16 rx_len; // 接收的长度
	u16 tx_len; // 要发送的长度
} _RX_STR;

class tcp
{

public:
	_RX_STR m_rxinfo;
	// st_http_request m_http_request;
	u16 tcp_err;
	u8 m_lock;

	tcp();
	~tcp();

	void OnTick(uint16 port, _RX_STR *prxinfo);

	u8 checkConnet();
	void Init();
};

#endif
