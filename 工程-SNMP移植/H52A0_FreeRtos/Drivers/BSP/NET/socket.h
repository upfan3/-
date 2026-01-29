
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "types.h"

/*Socket 端口选择，可按自的习惯定义*/
#define SOCK_TCPS 0 // TCP SERVER
// #define SOCK_PING			  0 //PING
// #define SOCK_TCPC             1 //TCP client
// #define SOCK_UDPS             2 //UDP
// #define SOCK_WEIBO      	  2 //微博
// #define SOCK_DHCP             3 //DHCP 路由自动分配ip
// #define SOCK_HTTPS            4 //httpserver
// #define SOCK_HTTPC			  4 //http client
// #define SOCK_NETBIOS     	  6		/* 定义NetBIOS采用的socket*/
// #define SOCK_DNS              5  //dns解释
// #define SOCK_SMTP             6  //邮件
// #define SOCK_NTP              7  //NTP服务器 获取网络时间
// #define SOCK_WEBSOCKET        6  //web 服务器

#ifdef __cplusplus
extern "C"
{
#endif

    extern uint8 socket(SOCKET s, uint8 protocol, uint16 port, uint8 flag);                 // Opens a socket(TCP or UDP or IP_RAW mode)
    extern void close(SOCKET s);                                                            // Close socket
    extern uint8 connect(SOCKET s, uint8 *addr, uint16 port);                               // Establish TCP connection (Active connection)
    extern void disconnect(SOCKET s);                                                       // disconnect the connection
    extern uint8 listen(SOCKET s);                                                          // Establish TCP connection (Passive connection)
    extern uint16 send(SOCKET s, const uint8 *buf, uint16 len);                             // Send data (TCP)
    extern uint16 recv(SOCKET s, uint8 *buf, uint16 len);                                   // Receive data (TCP)
    extern uint16 sendto(SOCKET s, const uint8 *buf, uint16 len, uint8 *addr, uint16 port); // Send data (UDP/IP RAW)
    extern uint16 recvfrom(SOCKET s, uint8 *buf, uint16 len, uint8 *addr, uint16 *port);    // Receive data (UDP/IP RAW)
    void send_ka(SOCKET s);

#ifdef __cplusplus
}
#endif

#ifdef __MACRAW__
void macraw_open(void);
uint16 macraw_send(const uint8 *buf, uint16 len); // Send data (MACRAW)
uint16 macraw_recv(uint8 *buf, uint16 len);       // Recv data (MACRAW)
#endif

#endif
/* _SOCKET_H_ */
