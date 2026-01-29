#include "./core/snmp_trap_simple.h"
#include "./core/snmp.h"
#include <string.h>
#include <stdio.h>

// 声明外部变量
extern _snmp_params snmp_parameters;
extern unsigned int gtimeCount10ms;

// 静态变量绑定
static _trap_varbinding sysuptime_vb;
static _trap_varbinding trap_oid_vb;

// 静态OID定义
static _oid sysuptime_oid = {
    9,
    {1, 3, 6, 1, 2, 1, 1, 3, 0}};

static _oid default_trap_oid = {
    11,
    {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0}};

/**
 * @brief 初始化预定义的变量绑定
 */
void trap_init(void)
{
    // 初始化sysUpTime变量绑定
    memcpy(&sysuptime_vb.oid, &sysuptime_oid, sizeof(_oid));
    // sysuptime_vb.data = &gtimeCount10ms;
    // sysuptime_vb.data_length = sizeof(gtimeCount10ms);
    sysuptime_vb.type = ASN_SNMP_TIMETICKS;

    // 初始化trap OID变量绑定（默认值）
    memcpy(&trap_oid_vb.oid, &default_trap_oid, sizeof(_oid));
    trap_oid_vb.data = NULL; // 将在发送时动态设置
    trap_oid_vb.data_length = 0;
    trap_oid_vb.type = ASN_OID;
}

/**
 * @brief 简化的Trap发送函数，只包含sysUpTime和trapOID两个变量绑定
 * @param trap_oid 要发送的trap OID，如果为NULL则使用默认值
 * @return 发送结果，0表示成功，其他值表示失败
 */
int trap_send_simple(_oid *trap_oid)
{
    // 准备变量绑定数据（只包含两个绑定）
    _trap_varbinding bindings[2];

    // 构造sysUpTime变量
    memcpy(&bindings[0], &sysuptime_vb, sizeof(_trap_varbinding));
    // 确保使用最新的gtimeCount10ms值
    uint32_t current_time = 100;
    bindings[0].data = &current_time;
    bindings[0].data_length = sizeof(current_time);

    // 构造trap OID变量绑定
    memcpy(&bindings[1], &trap_oid_vb, sizeof(_trap_varbinding));
    if (trap_oid != NULL)
    {
        // 使用传入的trap OID
        bindings[1].data = trap_oid;
        bindings[1].data_length = asn_bytes_to_encode_oid(trap_oid);
    }
    else
    {
        // 使用默认的trap OID (enterpriseSpecific.0)
        static _oid default_oid = {7, {1, 3, 6, 1, 4, 1, 0}};
        bindings[1].data = &default_oid;
        bindings[1].data_length = asn_bytes_to_encode_oid(&default_oid);
    }

    // 向所有启用的Trap目标发送SNMPv3 Trap
    int result = 0;
    int sent_count = 0;

    for (int i = 0; i < MAX_TRAP_TARGETS; i++)
    {
        if (snmp_parameters.trap_targets[i].enabled)
        {
            int target_result = snmp_send_trap(3,
                                               snmp_parameters.trap_targets[i].target_ip,
                                               snmp_parameters.trap_targets[i].target_port,
                                               &snmp_parameters.trap_targets[i].user_index,
                                               bindings, 2);
            if (target_result > 0)
            {
                sent_count++;
            }
            // 即使某个目标发送失败，也继续发送给其他目标
            result = result || target_result;
        }
    }

    // 如果至少有一个目标发送成功，返回成功
    return (sent_count > 0) ? sent_count : -1;
}

/**
 * @brief 支持自定义变量绑定的Trap发送函数
 * @param var_bindings 自定义变量绑定数组
 * @param num_bindings 自定义变量绑定数量
 * @param trap_oid 要发送的trap OID，如果为NULL则使用默认值
 * @return 发送结果，0表示成功，其他值表示失败
 */
// 为了解决并发发送陷阱时的问题，使用静态变量
static _trap_varbinding static_bindings[8];
static unsigned char trap_sending_in_progress = 0;

int trap_send(_trap_varbinding *var_bindings, unsigned int num_bindings, _oid *trap_oid)
{
    // 检查是否正在发送
    if (trap_sending_in_progress)
    {
        return -1; // 发送正在进行中
    }

    trap_sending_in_progress = 1;

    // 使用静态变量来构造变量绑定数据
    // 构造sysUpTime变量
    memcpy(&static_bindings[0], &sysuptime_vb, sizeof(_trap_varbinding));
    // 确保使用最新的gtimeCount10ms值
    uint32_t current_time = 100;
    static_bindings[0].data = &current_time;
    static_bindings[0].data_length = sizeof(current_time);

    // 构造trap OID变量绑定
    memcpy(&static_bindings[1], &trap_oid_vb, sizeof(_trap_varbinding));
    if (trap_oid != NULL)
    {
        // 使用传入的trap OID
        static_bindings[1].data = trap_oid;
        static_bindings[1].data_length = asn_bytes_to_encode_oid(trap_oid);
    }
    else
    {
        // 使用默认的trap OID (enterpriseSpecific.0)
        static _oid default_oid = {7, {1, 3, 6, 1, 4, 1, 0}};
        static_bindings[1].data = &default_oid;
        static_bindings[1].data_length = asn_bytes_to_encode_oid(&default_oid);
    }

    // 构造自定义变量绑定
    for (unsigned int i = 0; i < num_bindings && i < 6; i++)
    {
        memcpy(&static_bindings[i + 2], &var_bindings[i], sizeof(_trap_varbinding));
    }

    // 向所有启用的Trap目标发送SNMPv3 Trap
    int result = 0;
    int sent_count = 0;

    for (int i = 0; i < MAX_TRAP_TARGETS; i++)
    {
        if (snmp_parameters.trap_targets[i].enabled)
        {
            int target_result = snmp_send_trap(3,
                                               snmp_parameters.trap_targets[i].target_ip,
                                               snmp_parameters.trap_targets[i].target_port,
                                               &snmp_parameters.trap_targets[i].user_index,
                                               static_bindings, 2 + num_bindings);
            if (target_result > 0)
            {
                sent_count++;
            }
            // 即使某个目标发送失败，也继续发送给其他目标
            result = result || target_result;
        }
    }

    // 注意：因为snmp_send_trap是异步的，所以需要在发送完成回调中清除标志
    trap_sending_in_progress = 0;

    // 如果至少有一个目标发送成功，返回成功发送的目标数
    return (sent_count > 0) ? sent_count : -1;
}

/**
 * @brief 发送coldStart Trap (1.3.6.1.6.3.1.1.5.1)
 * @param snmp_params SNMP参数指针
 */
void trap_send_coldStart(_snmp_params *snmp_params)
{
    static _oid coldStart_oid = {10, {SNMPTRAP_COLDSTART_OID}};
    // 向所有启用的Trap目标发送coldStart Trap
    trap_send_simple(&coldStart_oid);
}

/**
 * @brief 发送warmStart Trap (1.3.6.1.6.3.1.1.5.2)
 * @param snmp_params SNMP参数指针
 */
void trap_send_warmStart(_snmp_params *snmp_params)
{
    static _oid warmStart_oid = {10, {SNMPTRAP_WARMSTART_OID}};
    // 向所有启用的Trap目标发送warmStart Trap
    trap_send_simple(&warmStart_oid);
}

/**
 * @brief 发送linkDown Trap (1.3.6.1.6.3.1.1.5.3)
 * @param if_index 接口索引
 */
void trap_send_linkDown(unsigned int if_index)
{
    static _oid linkDown_oid = {10, {SNMPTRAP_LINKDOWN_OID}};
    // 构造ifIndex变量绑定 (示例OID: 1.3.6.1.2.1.2.2.1.1.x)
    static _oid ifIndex_oid = {11, {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 1}};
    ifIndex_oid.val[10] = if_index; // 设置接口索引

    // 构造变量绑定
    _trap_varbinding var_bindings[1];
    memcpy(&var_bindings[0].oid, &ifIndex_oid, sizeof(_oid));
    var_bindings[0].data = &if_index;
    var_bindings[0].data_length = sizeof(if_index);
    var_bindings[0].type = ASN_INT;

    trap_send(var_bindings, 1, &linkDown_oid);
}

/**
 * @brief 发送linkUp Trap (1.3.6.1.6.3.1.1.5.4)
 * @param if_index 接口索引
 */
void trap_send_linkUp(unsigned int if_index)
{
    static _oid linkUp_oid = {10, {SNMPTRAP_LINKUP_OID}};
    // 构造ifIndex变量绑定 (示例OID: 1.3.6.1.2.1.2.2.1.1.x)
    static _oid ifIndex_oid = {11, {1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 1}};
    ifIndex_oid.val[10] = if_index; // 设置接口索引

    // 构造变量绑定
    _trap_varbinding var_bindings[1];
    memcpy(&var_bindings[0].oid, &ifIndex_oid, sizeof(_oid));
    var_bindings[0].data = &if_index;
    var_bindings[0].data_length = sizeof(if_index);
    var_bindings[0].type = ASN_INT;

    trap_send(var_bindings, 1, &linkUp_oid);
}

/**
 * @brief 发送authenticationFailure Trap (1.3.6.1.6.3.1.1.5.5)
 */
void trap_send_authFailure(void)
{
    static _oid authFailure_oid = {10, {SNMPTRAP_AUTHFAIL_OID}};
    trap_send_simple(&authFailure_oid);
}
