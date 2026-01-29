
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "types.h"

/*Socket 端口选择，可按自的习惯定义*/
#define SOCK_UPDATE             0 
//#define SOCK_PING			  0 //PING
#define SOCK_1363             1 //
#define SOCK_MODBUS             2 //
//#define SOCK_WEIBO      	  2 //微博
#define SOCK_dhcp             3 //DHCP 路由自动分配ip
#define SOCK_SNMP_V3 6  //3
#define SOCK_SNMP_TRAP 7 //4
//#define SOCK_HTTPS            4 //httpserver 
//#define SOCK_HTTPC			  4 //http client
//#define SOCK_NETBIOS     	  6		/* 定义NetBIOS采用的socket*/
//#define SOCK_DNS              5  //dns解释
//#define SOCK_SMTP             6  //邮件
//#define SOCK_NTP              7  //NTP服务器 获取网络时间
//#define SOCK_WEBSOCKET        6  //web 服务器

#define SOCK_OK               1        ///< Result is OK about socket process.
#define SOCK_BUSY             0        ///< Socket is busy on processing the operation. Valid only Non-block IO Mode.
#define SOCK_FATAL            -1000    ///< Result is fatal error about socket process.

#define SOCK_ERROR            0        
#define SOCKERR_SOCKNUM       (SOCK_ERROR - 1)     ///< Invalid socket number
#define SOCKERR_SOCKOPT       (SOCK_ERROR - 2)     ///< Invalid socket option
#define SOCKERR_SOCKINIT      (SOCK_ERROR - 3)     ///< Socket is not initialized or SIPR is Zero IP address when Sn_MR_TCP
#define SOCKERR_SOCKCLOSED    (SOCK_ERROR - 4)     ///< Socket unexpectedly closed.
#define SOCKERR_SOCKMODE      (SOCK_ERROR - 5)     ///< Invalid socket mode for socket operation.
#define SOCKERR_SOCKFLAG      (SOCK_ERROR - 6)     ///< Invalid socket flag
#define SOCKERR_SOCKSTATUS    (SOCK_ERROR - 7)     ///< Invalid socket status for socket operation.
#define SOCKERR_ARG           (SOCK_ERROR - 10)    ///< Invalid argument.
#define SOCKERR_PORTZERO      (SOCK_ERROR - 11)    ///< Port number is zero
#define SOCKERR_IPINVALID     (SOCK_ERROR - 12)    ///< Invalid IP address
#define SOCKERR_TIMEOUT       (SOCK_ERROR - 13)    ///< Timeout occurred
#define SOCKERR_DATALEN       (SOCK_ERROR - 14)    ///< Data length is zero or greater than buffer max size.
#define SOCKERR_BUFFER        (SOCK_ERROR - 15)    ///< Socket buffer is not enough for data communication.

#define SOCKFATAL_PACKLEN     (SOCK_FATAL - 1)     ///< Invalid packet length. Fatal Error.

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
    extern int32  sendto(SOCKET s, const uint8 * buf, uint16 len, uint8 * addr, uint16 port);// Send data (UDP/IP RAW)
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
