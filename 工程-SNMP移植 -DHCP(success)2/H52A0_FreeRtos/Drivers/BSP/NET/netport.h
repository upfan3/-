#ifndef _NET_PORT_H_
#define _NET_PORT_H_

#include "Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void net_port_init(void);
    uint8 net_read_write(uint8 tx, uint8 *rx);
    void net_scs_ctrl(uint8 val);
    void net_reset(void);

#ifdef __cplusplus
}
#endif

#endif
