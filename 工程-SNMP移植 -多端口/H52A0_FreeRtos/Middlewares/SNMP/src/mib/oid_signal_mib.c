#include "./mib/oid_signal_mib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "./asn1/asn.1.h"
#include "./mib/mib.h"

#define MAX_RECTIFIERS 16
#define ENTERPRISE_MIB_OID 61739
#define ENTERPRISE_PREFIX 1, 3, 6, 1, 4, 1, ENTERPRISE_MIB_OID // 企业 OID 前缀
#define ENTERPRISE_PREFIX_LEN 7

// ----------- 数据定义（略，保持原样） -----------
// 标识类
static char ident_model[] = "H52A0";
static char ident_sw_version[] = "V1.0.0";
static char ident_site_name[] = "SiteA";
static char ident_snmp_version[] = "SNMPv2c";
static int ident_rect_total_power = 12000;

// 系统类
static int sys_status = 1;
static int sys_voltage_mv = 54000;
static int sys_current_ma = 12000;
static int sys_comm_status = 0;
static int sys_batt_mode = 2;

// 电池类
static int ps_batt_current_ma = 500;
static int ps_batt1_current_ma = 250;
static int ps_batt2_current_ma = 250;
static int ps_batt1_capacity_pct = 80;
static int ps_batt2_capacity_pct = 75;

// 输入类
static int ps_ac_a_mv = 220000;
static int ps_ac_b_mv = 220000;
static int ps_ac_c_mv = 220000;

// 温度类
static int ps_temp1_mdegc = 25000;
static int ps_temp2_mdegc = 26000;
static int ps_temp3_mdegc = 25500;
static int ps_temp4_mdegc = 25800;
static int ps_env_hum_mrh = 45;

// 整流器表结构体
typedef struct
{
    int addr;
    int vin;
    int vout;
    int iout;
    int power;
    int temp;
    char onoff[8];
} rect_entry_t;

// 整流器表数据
static rect_entry_t rect_table[MAX_RECTIFIERS] = {
    {1, 220000, 54000, 1000, 54000, 25000, "ON"},
    {2, 220000, 54000, 1000, 54000, 25000, "ON"},
    // ...其余项初始化...
};

// ----------- OID匹配函数 -----------
static int oid_prefix_match(const uint32_t *oid, int oid_len, const uint32_t *prefix, int prefix_len)
{
    if (oid_len < prefix_len)
        return 0;
    return memcmp(oid, prefix, prefix_len * sizeof(uint32_t)) == 0;
}

// ----------- 大类处理函数声明 -----------
static int snmp_ident(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error);
static int snmp_system(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error);
static int snmp_battery(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error);
static int snmp_input(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error);
static int snmp_temperature(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error);
static int snmp_rectifier_table(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error);

// ----------- 分发数组 -----------
typedef int (*snmp_request)(void **, int *, _oid *, int, int, void *, int, int *);
static snmp_request oid_signal_handlers[] = {
    snmp_ident,
    snmp_system,
    snmp_battery,
    snmp_input,
    snmp_temperature,
    snmp_rectifier_table};

static const unsigned char valid_branch[] = {1, 2, 3, 4, 5, 8}; // 分发节点顺序
unsigned char indexmap[9] = {[1] = 0, [2] = 1, [3] = 2, [4] = 3, [5] = 4, [8] = 5};
static const unsigned int first_oid[] = {ENTERPRISE_PREFIX, 1, 1, 1, 1, 0};
static const unsigned int first_oid_len = 12;

// ----------- 主分发函数 -----------
int oid_signal_handler(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error)
{
    int reply;
    int branch_idx = -1;
    *error = snmp_NO_ERROR;
    // 找到当前分支在 valid_branch 中的索引
    for (int i = 0; i < sizeof(valid_branch); ++i)
    {
        if (oid->len >= 8 && oid->val[7] == valid_branch[i])
        {
            branch_idx = i;
            break;
        }
    }

    // GET-NEXT分支
    if (request_type == GET_NEXT_REQUEST_PDU)
    {
        // 如果OID长度不足，初始化到第一个分支
        if (oid->len < 8)
        {
            memcpy(oid->val, first_oid, first_oid_len * sizeof(unsigned int));
            oid->len = first_oid_len;
            branch_idx = 0;
        }
        // 如果当前分支无效，则跳到第一个分支
        else if (branch_idx == -1)
        {
            memcpy(oid->val, first_oid, first_oid_len * sizeof(unsigned int));
            oid->len = first_oid_len;
            branch_idx = 0;
        }

        // 遍历所有分支
        do
        {
            reply = oid_signal_handlers[branch_idx](data, data_len, oid, request_type, data_type, data_in, data_in_len, error);
            if (*error != snmp_NO_SUCH_NAME)
            {
                break;
            }
            branch_idx++;
        } while (branch_idx < sizeof(valid_branch));

        return reply;
    }

    // GET/SET分支
    if (branch_idx == -1)
    {
        // *error = snmp_NO_SUCH_NAME;
        // return ASN_NULL;
        return NO_SUCH_OBJECT;
    }
    if (branch_idx >= sizeof(oid_signal_handlers) / sizeof(snmp_request))
    {
        // *error = snmp_NO_SUCH_NAME;
        // return ASN_NULL;
        return NO_SUCH_OBJECT;
    }
    return oid_signal_handlers[branch_idx](data, data_len, oid, request_type, data_type, data_in, data_in_len, error);
}

// ----------- SNMP请求处理函数实现 -----------
static int snmp_ident(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error)
{
    int leaf = oid->val[10];
    switch (request_type)
    {
    case GET_REQUEST_PDU:
        switch (leaf)
        {
        case 2:
            *data = ident_model;
            *data_len = strlen(ident_model);
            return ASN_OCTET_STRING;
        case 3:
            *data = ident_sw_version;
            *data_len = strlen(ident_sw_version);
            return ASN_OCTET_STRING;
        case 4:
            *data = ident_site_name;
            *data_len = strlen(ident_site_name);
            return ASN_OCTET_STRING;
        case 5:
            *data = ident_snmp_version;
            *data_len = strlen(ident_snmp_version);
            return ASN_OCTET_STRING;
        case 6:
            *data = &ident_rect_total_power;
            return ASN_INT;
        // default: *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        default:
            return NO_SUCH_OBJECT;
        }
    case GET_NEXT_REQUEST_PDU:
        if (leaf < 6)
        {
            oid->val[10] = leaf + 1;
            return snmp_ident(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
        else
        {
            oid->val[7] = 2;
            oid->val[8] = 1;
            oid->val[9] = 1;
            oid->val[10] = 1;
            return snmp_system(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
    case SET_REQUEST_PDU:
        switch (leaf)
        {
        case 2:
            if (data_type != ASN_OCTET_STRING || data_in_len >= sizeof(ident_model))
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            strncpy(ident_model, (char *)data_in, data_in_len);
            ident_model[data_in_len] = '\0';
            *data = ident_model;
            *data_len = strlen(ident_model);
            return ASN_OCTET_STRING;
        case 3:
            if (data_type != ASN_OCTET_STRING || data_in_len >= sizeof(ident_sw_version))
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            strncpy(ident_sw_version, (char *)data_in, data_in_len);
            ident_sw_version[data_in_len] = '\0';
            *data = ident_sw_version;
            *data_len = strlen(ident_sw_version);
            return ASN_OCTET_STRING;
        case 4:
            if (data_type != ASN_OCTET_STRING || data_in_len >= sizeof(ident_site_name))
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            strncpy(ident_site_name, (char *)data_in, data_in_len);
            ident_site_name[data_in_len] = '\0';
            *data = ident_site_name;
            *data_len = strlen(ident_site_name);
            return ASN_OCTET_STRING;
        case 5:
            if (data_type != ASN_OCTET_STRING || data_in_len >= sizeof(ident_snmp_version))
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            strncpy(ident_snmp_version, (char *)data_in, data_in_len);
            ident_snmp_version[data_in_len] = '\0';
            *data = ident_snmp_version;
            *data_len = strlen(ident_snmp_version);
            return ASN_OCTET_STRING;
        case 6:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ident_rect_total_power = *(int *)data_in;
            *data = &ident_rect_total_power;
            return ASN_INT;
        default:
            *error = snmp_READ_ONLY;
            return ASN_NULL;
        }
    default:
        // *error = snmp_NO_SUCH_NAME;
        // return ASN_NULL;
        return NO_SUCH_OBJECT;
    }
}

static int snmp_system(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error)
{
    int leaf = oid->val[10];
    switch (request_type)
    {
    case GET_REQUEST_PDU:
        switch (leaf)
        {
        case 1:
            *data = &sys_status;
            return ASN_INT;
        case 2:
            *data = &sys_voltage_mv;
            return ASN_INT;
        case 3:
            *data = &sys_current_ma;
            return ASN_INT;
        case 4:
            *data = &sys_comm_status;
            return ASN_INT;
        case 5:
            *data = &sys_batt_mode;
            return ASN_INT;
        // default: *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        default:
            return NO_SUCH_OBJECT;
        }
    case GET_NEXT_REQUEST_PDU:
        if (leaf < 5)
        {
            oid->val[10] = leaf + 1;
            return snmp_system(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
        else
        {
            oid->val[7] = 3;
            oid->val[8] = 1;
            oid->val[9] = 1;
            oid->val[10] = 1;
            return snmp_battery(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
    case SET_REQUEST_PDU:
        switch (leaf)
        {
        case 1:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            sys_status = *(int *)data_in;
            *data = &sys_status;
            return ASN_INT;
        case 2:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            sys_voltage_mv = *(int *)data_in;
            *data = &sys_voltage_mv;
            return ASN_INT;
        case 3:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            sys_current_ma = *(int *)data_in;
            *data = &sys_current_ma;
            return ASN_INT;
        case 4:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            sys_comm_status = *(int *)data_in;
            *data = &sys_comm_status;
            return ASN_INT;
        case 5:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            sys_batt_mode = *(int *)data_in;
            *data = &sys_batt_mode;
            return ASN_INT;
        default:
            *error = snmp_READ_ONLY;
            return ASN_NULL;
        }
    default:
        // *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        return NO_SUCH_OBJECT;
    }
}

static int snmp_battery(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error)
{
    int leaf = oid->val[10];
    switch (request_type)
    {
    case GET_REQUEST_PDU:
        switch (leaf)
        {
        case 1:
            *data = &ps_batt_current_ma;
            return ASN_INT;
        case 2:
            *data = &ps_batt1_current_ma;
            return ASN_INT;
        case 3:
            *data = &ps_batt2_current_ma;
            return ASN_INT;
        case 4:
            *data = &ps_batt1_capacity_pct;
            return ASN_INT;
        case 5:
            *data = &ps_batt2_capacity_pct;
            return ASN_INT;
        // default: *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        default:
            return NO_SUCH_OBJECT;
        }
    case GET_NEXT_REQUEST_PDU:
        if (leaf < 5)
        {
            oid->val[10] = leaf + 1;
            return snmp_battery(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
        else
        {
            oid->val[7] = 4;
            oid->val[8] = 1;
            oid->val[9] = 1;
            oid->val[10] = 1;
            return snmp_input(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
    case SET_REQUEST_PDU:
        switch (leaf)
        {
        case 1:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_batt_current_ma = *(int *)data_in;
            *data = &ps_batt_current_ma;
            return ASN_INT;
        case 2:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_batt1_current_ma = *(int *)data_in;
            *data = &ps_batt1_current_ma;
            return ASN_INT;
        case 3:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_batt2_current_ma = *(int *)data_in;
            *data = &ps_batt2_current_ma;
            return ASN_INT;
        case 4:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_batt1_capacity_pct = *(int *)data_in;
            *data = &ps_batt1_capacity_pct;
            return ASN_INT;
        case 5:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_batt2_capacity_pct = *(int *)data_in;
            *data = &ps_batt2_capacity_pct;
            return ASN_INT;
        default:
            *error = snmp_READ_ONLY;
            return ASN_NULL;
        }
    default:
        // *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        return NO_SUCH_OBJECT;
    }
}

static int snmp_input(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error)
{
    int leaf = oid->val[10];
    switch (request_type)
    {
    case GET_REQUEST_PDU:
        switch (leaf)
        {
        case 1:
            *data = &ps_ac_a_mv;
            return ASN_INT;
        case 2:
            *data = &ps_ac_b_mv;
            return ASN_INT;
        case 3:
            *data = &ps_ac_c_mv;
            return ASN_INT;
        // default: *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        default:
            return NO_SUCH_OBJECT;
        }
    case GET_NEXT_REQUEST_PDU:
        if (leaf < 3)
        {
            oid->val[10] = leaf + 1;
            return snmp_input(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
        else
        {
            oid->val[7] = 5;
            oid->val[8] = 1;
            oid->val[9] = 1;
            oid->val[10] = 1;
            return snmp_temperature(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
    case SET_REQUEST_PDU:
        switch (leaf)
        {
        case 1:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_ac_a_mv = *(int *)data_in;
            *data = &ps_ac_a_mv;
            return ASN_INT;
        case 2:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_ac_b_mv = *(int *)data_in;
            *data = &ps_ac_b_mv;
            return ASN_INT;
        case 3:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_ac_c_mv = *(int *)data_in;
            *data = &ps_ac_c_mv;
            return ASN_INT;
        default:
            *error = snmp_READ_ONLY;
            return ASN_NULL;
        }
    default:
        // *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        return NO_SUCH_OBJECT;
    }
}

static int snmp_temperature(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error)
{
    int leaf = oid->val[10];
    switch (request_type)
    {
    case GET_REQUEST_PDU:
        switch (leaf)
        {
        case 1:
            *data = &ps_temp1_mdegc;
            return ASN_INT;
        case 2:
            *data = &ps_temp2_mdegc;
            return ASN_INT;
        case 3:
            *data = &ps_temp3_mdegc;
            return ASN_INT;
        case 4:
            *data = &ps_temp4_mdegc;
            return ASN_INT;
        case 5:
            *data = &ps_env_hum_mrh;
            return ASN_INT;
        // default: *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        default:
            return NO_SUCH_OBJECT;
        }
    case GET_NEXT_REQUEST_PDU:
        if (leaf < 5)
        {
            oid->val[10] = leaf + 1;
            return snmp_temperature(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
        else
        {
            // 移除硬编码的跳转逻辑，让oid_signal_handler统一处理
            *error = snmp_NO_SUCH_NAME;
            return ASN_NULL;
        }
    case SET_REQUEST_PDU:
        switch (leaf)
        {
        case 1:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_temp1_mdegc = *(int *)data_in;
            *data = &ps_temp1_mdegc;
            return ASN_INT;
        case 2:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_temp2_mdegc = *(int *)data_in;
            *data = &ps_temp2_mdegc;
            return ASN_INT;
        case 3:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_temp3_mdegc = *(int *)data_in;
            *data = &ps_temp3_mdegc;
            return ASN_INT;
        case 4:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_temp4_mdegc = *(int *)data_in;
            *data = &ps_temp4_mdegc;
            return ASN_INT;
        case 5:
            if (data_type != ASN_INT)
            {
                *error = snmp_WRONG_TYPE;
                return ASN_NULL;
            }
            ps_env_hum_mrh = *(int *)data_in;
            *data = &ps_env_hum_mrh;
            return ASN_INT;
        default:
            *error = snmp_READ_ONLY;
            return ASN_NULL;
        }
    default:
        // *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        return NO_SUCH_OBJECT;
    }
}

static int snmp_rectifier_table(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error)
{
    static const uint32_t tbl_base[] = {1, 3, 6, 1, 4, 1, 61739, 8, 1, 1};
    static const int tbl_len = 10;
    *error = snmp_NO_ERROR;

    // SET只读
    if (request_type == SET_REQUEST_PDU)
    {
        *error = snmp_READ_ONLY;
        return ASN_NULL;
    }

    // GET-NEXT请求需要推进OID
    if (request_type == GET_NEXT_REQUEST_PDU)
    {
        // 如果OID是表前缀（长度为8或9），自动推进到第一个表项
        if (!oid_prefix_match(oid->val, oid->len, tbl_base, tbl_len))
        {
            memcpy(oid->val, tbl_base, tbl_len * sizeof(uint32_t));
        }

        if (oid_prefix_match(oid->val, oid->len, tbl_base, tbl_len))
        {
            int col = 1;
            int idx = 1;
            if (oid->len == tbl_len + 1 && oid->val[tbl_len] >= 1 && oid->val[tbl_len] <= 7)
            {
                col = oid->val[tbl_len];
                idx = 1;
            }
            else if (oid->len == tbl_len + 2)
            {
                col = oid->val[tbl_len];
                idx = oid->val[tbl_len + 1];
                if (col == 7 && idx == MAX_RECTIFIERS)
                {
                    // 已到达表格末尾
                    // *error = snmp_NO_SUCH_NAME;
                    // return ASN_NULL;
                    return END_OF_MIB_VIEW;
                }
                idx++;
                if (idx > MAX_RECTIFIERS)
                {
                    col++;
                    idx = 1;
                    if (col > 7)
                    {
                        // *error = snmp_NO_SUCH_NAME;
                        // return ASN_NULL;
                        return NO_SUCH_OBJECT;
                    }
                }
            }
            else
            {
                col = 1;
                idx = 1;
            }
            oid->val[tbl_len] = col;
            oid->val[tbl_len + 1] = idx;
            oid->len = tbl_len + 2;
            // 递归调用以返回OID对应的数据
            return snmp_rectifier_table(data, data_len, oid, GET_REQUEST_PDU, data_type, data_in, data_in_len, error);
        }
        // *error = snmp_NO_SUCH_NAME;
        // return ASN_NULL;
        return NO_SUCH_OBJECT;
    }

    // GET请求，返回表格数据
    if (oid_prefix_match(oid->val, oid->len, tbl_base, tbl_len))
    {
        if (oid->len != tbl_len + 2)
        {
            *error = snmp_NO_SUCH_NAME;
            return ASN_NULL;
        }
        int col = oid->val[tbl_len];
        int idx = oid->val[tbl_len + 1];
        if (col < 1 || col > 7)
        {
            // *error = snmp_NO_SUCH_NAME; return ASN_NULL;
            return NO_SUCH_OBJECT;
        }
        if (idx < 1 || idx > MAX_RECTIFIERS)
        {
            // *error = snmp_NO_SUCH_NAME; return ASN_NULL;
            return NO_SUCH_INSTANCE;
            ;
        }
        rect_entry_t *r = &rect_table[idx - 1];
        switch (col)
        {
        case 1:
            *data = &r->addr;
            return ASN_INT;
        case 2:
            *data = &r->vin;
            return ASN_INT;
        case 3:
            *data = &r->vout;
            return ASN_INT;
        case 4:
            *data = &r->iout;
            return ASN_INT;
        case 5:
            *data = &r->power;
            return ASN_INT;
        case 6:
            *data = &r->temp;
            return ASN_INT;
        case 7:
            *data = r->onoff;
            *data_len = strlen(r->onoff);
            return ASN_OCTET_STRING;
        // default: *error = snmp_NO_SUCH_NAME; return ASN_NULL;
        default:
            return NO_SUCH_OBJECT;
        }
    }
    // *error = snmp_NO_SUCH_NAME;
    // return ASN_NULL;
    return NO_SUCH_OBJECT;
}

// ----------- 注册到 MIB 中 -----------
static _mib_handler oid_signal_mib = {
    oid_signal_handler,
    ENTERPRISE_PREFIX_LEN,
    {ENTERPRISE_PREFIX}};

void oid_signal_init(void)
{
    mib_register(&oid_signal_mib);
}
