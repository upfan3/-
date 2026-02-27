#ifndef __TCP_H
#define __TCP_H

#include "w5500.h"
#include <stdio.h>
#include <string.h>
#include "socket.h"

// 协议类型枚举（区分TCP/UDP）
typedef enum {
    PROTOCOL_TCP = 0,
    PROTOCOL_UDP = 1
} ProtocolType;

typedef struct __rx_str
{
    u8 rx_buff[2048];
    u16 rx_len;//接收的长度
    u16 tx_len;//要发送的长度
    // UDP专用 - 远端IP和端口（接收数据时记录）
    uint8 remote_ip[4];
    uint16 remote_port;
}_RX_STR;

class tcp{
    
public:
    _RX_STR m_rxinfo;
    u16  tcp_err[MAX_SOCK_NUM];
    u8   m_lock;
    // 当前Socket使用的协议类型（TCP/UDP）
    ProtocolType proto_type;

    tcp();
    ~tcp();
    
    void maintainConnection(uint8 ch, uint16 port, ProtocolType proto = PROTOCOL_TCP);
    uint16 receiveData(uint8 ch, _RX_STR *prxinfo,ProtocolType proto);
    void processAndSendData(uint8 ch, const u8 *data, uint16 len, _RX_STR *prxinfo,ProtocolType proto);

    void OnTick(uint16 ch, uint16 port, _RX_STR *prxinfo, ProtocolType proto = PROTOCOL_TCP);

    u8 checkConnet();
    void Init();	

    // UDP专用发送函数（指定远端IP和端口）
    void udpSendData(uint8 ch,  u8 *data, uint16 len,  uint8 *remote_ip, uint16 remote_port);
};

u8 *HandleFlashUpdateData(u8 *pdat, u16 *plen);
u8 *Handle1363NetData(u8 *pdat);
u8 *HandleModbusData(u8 *pdat, u16 *plen);
u8* HandleTestNetData(const u8 *data, u16 *send_len);
u8* Handle1363TestData(const u8 *data, u16 *send_len);

#endif
