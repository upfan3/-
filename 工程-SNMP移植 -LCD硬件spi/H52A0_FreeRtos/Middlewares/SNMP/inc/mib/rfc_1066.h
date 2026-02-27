#ifndef RFC_1066_H
#define RFC_1066_H

#include <stdint.h>
#include "./core/snmp.h"

// 系统站点信息结构体
typedef struct
{
    char sys_contact[256];  // 系统联系人
    char sys_name[256];     // 系统名称
    char sys_location[256]; // 系统位置
} site_info_t;

extern site_info_t *site_info; // 全局站点信息指针
void save_site_info(void);     // 保存站点信息的函数声明
// extern uint32_t uptime_sec;  // 系统运行时间(秒)

// 网络接口信息结构体
struct netif
{
    char name[2];        // 接口名称，如"en"、"lo"等
    uint8_t link_type;   // 链路类型
    uint32_t mtu;        // 最大传输单元
    uint32_t link_speed; // 链路速度
    uint32_t ts;         // 最后变更时间
    uint8_t hwaddr[6];   // 硬件地址(MAC地址)

    // 为支持 MIB-2 接口组而添加的计数器
    struct
    {
        uint32_t ifinoctets;        // 接收字节数
        uint32_t ifoutoctets;       // 发送字节数
        uint32_t ifinucastpkts;     // 接收到的单播包
        uint32_t ifoutucastpkts;    // 发送的单播包
        uint32_t ifindiscards;      // 接收丢弃包数
        uint32_t ifoutdiscards;     // 发送丢弃包数
        uint32_t ifinerrors;        // 接收错误数
        uint32_t ifouterrors;       // 发送错误数
        uint32_t ifinnucastpkts;    // 接收的非单播包
        uint32_t ifoutnucastpkts;   // 发送的非单播包
        uint32_t ifinunknownprotos; // 未知协议数据包
        uint32_t ifoutqlen;         // 发送队列长度
    } mib2_counters;

    // 用于链表连接的指针
    struct netif *next;
};

// ARP表项结构体
struct etharp_entry
{
    uint8_t state;      // ARP表项状态
    uint8_t ethaddr[6]; // MAC地址
    uint32_t ipaddr;    // IP地址
};

#define ARP_TABLE_SIZE 10

#define ETHARP_STATE_STABLE 1

extern _snmp_params snmp_parameters;

int netif_is_up(struct netif *netif);

int netif_is_link_up(struct netif *netif);

void rfc_1066_init(void);

#endif
/***   End Of File   ***/
