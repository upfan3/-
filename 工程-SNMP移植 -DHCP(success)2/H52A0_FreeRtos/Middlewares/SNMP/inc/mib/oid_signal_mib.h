#ifndef OID_SIGNAL_MIB_H
#define OID_SIGNAL_MIB_H

#include "./core/snmp.h" // 你已有的 SNMP 框架头
#include <string.h>

/* 初始化函数：由 main 或 snmp_agent_init 调用 */
void oid_signal_init(void);

#endif
