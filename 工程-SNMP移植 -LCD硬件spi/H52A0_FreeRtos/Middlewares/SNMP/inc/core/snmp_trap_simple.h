#ifndef SNMP_TRAP_SIMPLE_H
#define SNMP_TRAP_SIMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "snmp.h"

// 标准Trap OID定义
#define SNMPTRAP_COLDSTART_OID 1, 3, 6, 1, 6, 3, 1, 1, 5, 1
#define SNMPTRAP_WARMSTART_OID 1, 3, 6, 1, 6, 3, 1, 1, 5, 2
#define SNMPTRAP_LINKDOWN_OID 1, 3, 6, 1, 6, 3, 1, 1, 5, 3
#define SNMPTRAP_LINKUP_OID 1, 3, 6, 1, 6, 3, 1, 1, 5, 4
#define SNMPTRAP_AUTHFAIL_OID 1, 3, 6, 1, 6, 3, 1, 1, 5, 5

    // 简化的Trap发送函数，只包含sysUpTime和trapOID两个变量绑定
    int trap_send_simple(_oid *trap_oid);

    // 支持自定义变量绑定的Trap发送函数
    int trap_send(_trap_varbinding *var_bindings, unsigned int num_bindings, _oid *trap_oid);

    // 标准Trap发送函数
    void trap_send_coldStart(_snmp_params *snmp_params);
    void trap_send_warmStart(_snmp_params *snmp_params);
    void trap_send_linkDown(unsigned int if_index);
    void trap_send_linkUp(unsigned int if_index);
    void trap_send_authFailure(void);

    // 初始化函数
    void trap_init(void);

#ifdef __cplusplus
}
#endif

#endif // SNMP_TRAP_SIMPLE_H
