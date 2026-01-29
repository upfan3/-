#include "./network/network_interface.h"
#include "BSP/NET/w5500.h"
#include "BSP/NET/socket.h"

#define PHYCFGR_LNK_ON 0x01 // Link up status bit

static int w5500_init(uint8_t sock, uint16_t port)
{
    return socket(sock, Sn_MR_UDP, port, 0);
}

static int w5500_receive(uint8_t sock, uint8_t *buffer, uint16_t len, uint8_t *remote_ip, uint16_t *remote_port)
{
    uint16_t available = getSn_RX_RSR(sock);
    if (available > 0)
    {
        if (len > available)
            len = available;
        return recvfrom(sock, buffer, len, remote_ip, remote_port);
    }
    return 0;
}

static int w5500_send(uint8_t sock, uint8_t *buffer, uint16_t len, uint8_t *remote_ip, uint16_t remote_port)
{
    return sendto(sock, buffer, len, remote_ip, remote_port);
}

static int w5500_is_connected(uint8_t sock)
{
    return (getSn_SR(sock) != SOCK_CLOSED);
}

static int w5500_is_link_up(void)
{
    // 检测PHY链接状态
    return (getPHYCFGR() & PHYCFGR_LNK_ON);
}

// 统一网络状态检查函数
static int w5500_check_network_status(uint8_t sock, uint16_t port)
{
    // 首先检查物理链路状态
    if (!w5500_is_link_up())
    {
        // 链路断开
        return -1;
    }

    // 然后检查socket连接状态
    if (!w5500_is_connected(sock))
    {
        // 重新初始化socket
        if (socket(sock, Sn_MR_UDP, port, 0))
        {
            return 0; // 重新初始化成功
        }
        else
        {
            return -2; // 重新初始化失败
        }
    }

    return 0; // 网络状态正常
}

const network_interface_t w5500_network_interface = {
    .init = w5500_init,
    .receive = w5500_receive,
    .send = w5500_send,
    .is_connected = w5500_is_connected,
    .is_link_up = w5500_is_link_up,
    .check_network_status = w5500_check_network_status,
    .name = "W5500"};
