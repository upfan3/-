#include "./network/network_interface.h"
#include "BSP/NET/w5500.h"
#include "BSP/NET/socket.h"

#define PHYCFGR_LNK_ON 0x01 // Link up status bit
#define MAX_RETRY 3

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
    int retry = 0;

    // 先循环检查链路+重试socket初始化，直到成功或重试耗尽
    while (retry < MAX_RETRY)
    {
        // 1. 检查物理链路状态（链路断开时，重试等待恢复）
        if (!w5500_is_link_up())
        {
            retry++;

            continue; // 跳过后续逻辑，重新检查链路
        }

        // 2. 链路正常，检查socket连接状态
        if (w5500_is_connected(sock))
        {
            // 连接正常，直接返回成功
            return 0;
        }

        // 3. socket未连接，尝试重新初始化
        if (socket(sock, Sn_MR_UDP, port, 0))
        {
            // 初始化成功，返回成功
            return 0;
        }
        else
        {
            // 初始化失败，计数+1，延时后重试
            retry++;
        }
    }

    // 所有重试耗尽仍失败：区分最终错误原因
    if (!w5500_is_link_up())
    {
        return -1; // 最终是链路问题
    }
    else
    {
        return -2; // 链路正常，但socket初始化失败
    }
}

const network_interface_t w5500_network_interface = {
    .init = w5500_init,
    .receive = w5500_receive,
    .send = w5500_send,
    .is_connected = w5500_is_connected,
    .is_link_up = w5500_is_link_up,
    .check_network_status = w5500_check_network_status,
    .name = "W5500"};
