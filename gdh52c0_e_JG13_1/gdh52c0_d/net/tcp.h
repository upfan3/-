#ifndef  __TCP_H
#define  __TCP_H

#include "w5500.h"
#include <stdio.h>
#include <string.h>
#include "socket.h"
#include "http_server.h"



class tcp{
	
public:
	_RX_STR m_rxinfo;
   st_http_request m_http_request;
   u16  tcp_err;
	
 tcp();
~tcp();
	
void OnTick(uint16 port,_RX_STR  *prxinfo);
u8 httpOnTick(uint16 port, _RX_STR  *prxinfo,uint8 ch);
void Init();	


};


extern tcp *ptcp;



















#endif
