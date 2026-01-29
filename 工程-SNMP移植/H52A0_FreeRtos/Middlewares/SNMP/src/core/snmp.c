// snmpv1.c

// #include "lwip/opt.h"

// Standard Includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// File Includes

#include "./core/snmp.h"
#include "./mib/mib.h"
#include "./mib/rfc_1066.h"
#include "./mib/oid_signal_mib.h"
#include "./network/network_interface.h"
#include "./core/snmp_crypto.h"
#include "./core/snmp_encode.h"
#include "./core/snmp_decode.h"

u32 gtimeCount1s = 10;
_snmp_params *snmp_params;

// 添加一个实际的统计结构体实例
static snmp_stats_t snmp_stats_instance = {0};
snmp_stats_t *snmp_stats = &snmp_stats_instance;

// NOTE: v1 and v2c are so similar that they are handled the same except when it
//       comes to getbulk, so when you see v1 you should also assume v2c

_snmp_data in_data;                  // used to construct and hold inbound snmp values
_snmp_data out_data;                 // used to construct and hold outbound snmp values
_cb_struct cb_struct = {0};          // used to contain callback for CLIENT SNMP requests
_varbinding_data vb_data;            // used to contain var-binding data for inbound processing
unsigned char snmp_buffer[512];      // used to hold complete snmp messages
unsigned char snmp_varbindings[512]; // used to hold complete constructed outbound varbinding messages

const unsigned int rpt_err_oid[] = {1, 3, 6, 1, 6, 3, 15, 1, 1, 0, 0};
unsigned int snmp_v3_oid_errors[RPT_ERR_LAST_ONE];
_mib_handler snmp_v3_error_oid_mib = {snmp_v3_error_oid_handler, 9, {1, 3, 6, 1, 6, 3, 15, 1, 1}};

// function definitions
int snmp_send_trap(int version, uint8_t *ip_addr, uint16_t port, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings)
{
    int ret = 0;
    int length;
    uint16_t sent_len;

    // 构建SNMP trap消息
    if (version == 0)
        length = snmp_build_trap_v1(snmp_buffer, auth, var_bindings, num_bindings);
    else if (version == 1)
        length = snmp_build_trap_v2c(snmp_buffer, auth, var_bindings, num_bindings);
    else if (version == 3)
        length = snmp_build_trap_v3(snmp_buffer, auth, var_bindings, num_bindings);
    else
        length = 0;

    if (length == 0)
    {
        return 0;
    }

    // 检查SNMP socket是否已打开
    extern const network_interface_t w5500_network_interface;
    if (w5500_network_interface.check_network_status(SOCK_SNMP_TRAP, SNMP_TRAP_PORT) != 0)
    {
        return 0;
    }

    // 发送SNMP trap数据
    sent_len = w5500_network_interface.send(SOCK_SNMP_TRAP, snmp_buffer, length, ip_addr, port);

    if (sent_len == length)
    {
        ret = 1;
    }

    // 短暂延时确保数据发送完成
    // vTaskDelay(1);

    return ret;
}

int snmp_build_trap(unsigned char *buffer, int version, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings)
{
    if (version == 0)
        return snmp_build_trap_v1(buffer, auth, var_bindings, num_bindings);
    else if (version == 1)
        return snmp_build_trap_v2c(buffer, auth, var_bindings, num_bindings);
    else if (version == 3)
        return snmp_build_trap_v3(buffer, auth, var_bindings, num_bindings);
    else
        return 0;
}

// handles keeping track of v3 errors, gets registered as a MIB handler for 1.3.6.1.6.3.15.1.1
int snmp_v3_error_oid_handler(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error)
{
    // debug_printf("snmp_v3_error_oid_handler called with :");
    // print_oid(oid);

    if (oid->val[RPT_ERR_INDEX] >= RPT_ERR_LAST_ONE)
        return ASN_NULL;

    snmp_v3_oid_errors[oid->val[RPT_ERR_INDEX]]++;
    *data = &snmp_v3_oid_errors[oid->val[RPT_ERR_INDEX]];
    return ASN_SNMP_COUNTER;
}

// setup snmp system by registering UDP socket, and registering some mib's
void snmp_init(_snmp_params *snmp_params_in)
{
    // debug_printf("SNMP init:\t\t");
    // debug_printf("done\r\n");
    snmp_setup(snmp_params_in);
}

void snmp_setup(_snmp_params *snmp_params_in)
{
    snmp_params = snmp_params_in;
    rfc_1066_init();
    oid_signal_init();
    mib_register((void *)&snmp_v3_error_oid_mib); // register v3 error OID handler
    snmp_init_keys();
}

