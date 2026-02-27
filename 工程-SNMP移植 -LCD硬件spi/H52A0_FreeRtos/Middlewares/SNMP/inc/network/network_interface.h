#ifndef _NETWORK_INTERFACE_H
#define _NETWORK_INTERFACE_H

#include <stdint.h>

typedef struct
{
    int (*init)(uint8_t socket, uint16_t port);
    int (*receive)(uint8_t socket, uint8_t *buffer, uint16_t len, uint8_t *remote_ip, uint16_t *remote_port);
    int (*send)(uint8_t socket, uint8_t *buffer, uint16_t len, uint8_t *remote_ip, uint16_t remote_port);
    int (*is_connected)(uint8_t socket);
    int (*is_link_up)(void);                                    // 添加链路检测功能
    int (*check_network_status)(uint8_t socket, uint16_t port); // 统一网络状态检查
    const char *name;
} network_interface_t;

#endif // _NETWORK_INTERFACE_H
