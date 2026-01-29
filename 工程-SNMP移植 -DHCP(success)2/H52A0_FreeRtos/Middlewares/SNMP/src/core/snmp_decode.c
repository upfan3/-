// SNMP v3处理实现
// 实现v3版本的处理逻辑和增强功能

#include <string.h>
#include <stdlib.h>
#include "./core/snmp_crypto.h"
#include "./core/snmp_encode.h"
#include "./core/snmp_decode.h"
#include "./mib/mib.h"
#include "./asn1/asn.1.h"
#include "./core/snmp.h"

// 静态全局变量声明
extern _snmp_params *snmp_params;
extern snmp_stats_t *snmp_stats;
extern u32 gtimeCount1s;
extern unsigned char snmp_varbindings[];
extern _varbinding_data vb_data;
extern _cb_struct cb_struct;
extern const unsigned int rpt_err_oid[11];

// ==================== 辅助函数实现 ====================

// 安全内存拷贝函数
static void safe_memcpy(void *dest, const void *src, size_t dest_size, size_t copy_len) {
    if (!dest || !src || dest_size == 0) return;
    
    size_t actual_len = (copy_len < dest_size) ? copy_len : dest_size - 1;
    memcpy(dest, src, actual_len);
    
    // 确保字符串以null结尾
    if (actual_len < dest_size) {
        ((char*)dest)[actual_len] = '\0';
    }
}

// 验证用户索引有效性
static int is_valid_user_index(unsigned int user_index) {
    return (snmp_params && user_index < MAX_SNMPV3_USERS);
}

// 验证输入参数
static int validate_input_params(const unsigned char *packet_in, unsigned int length_in, const unsigned char *packet_out) {
    if (!packet_in || !packet_out || length_in == 0 || length_in > MAX_PACKET_LENGTH) {
        return SNMP_ERROR_INVALID_PARAM;
    }
    return SNMP_SUCCESS;
}

// ==================== V3处理子函数 ====================

// 解码V3全局头部
static int decode_v3_global_header(unsigned char **packet_ptr, unsigned int *remaining_len) {
    unsigned char *p1 = *packet_ptr;
    unsigned int len;
    int bytes_used;

    // 解码序列头
    if ((bytes_used = asn_decode_sequence(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    if (len != *remaining_len - bytes_used) {
        return SNMP_ERROR_ASN_PARSE;
    }

    *packet_ptr = p1;
    *remaining_len = len;
    return SNMP_SUCCESS;
}

// 解码V3消息头部
static int decode_v3_message_header(unsigned char **packet_ptr, unsigned int *remaining_len) {
    unsigned char *p1 = *packet_ptr;
    unsigned int len;
    int bytes_used;

    // 解码版本号
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.msg_version)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }

    if (in_data.v3.msg_version != SNMP_V3_VERSION) {
        if (snmp_stats) snmp_stats->badVersions++;
        return SNMP_ERROR_VERSION;
    }
    p1 += bytes_used;
    *remaining_len -= bytes_used;

    // 解码消息头部序列
    if ((bytes_used = asn_decode_sequence(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解码消息ID
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.msg_id)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解码最大消息大小
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.msg_max_size)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解码消息标志
    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    if (len != 1) {
        return SNMP_ERROR_ASN_PARSE;
    }
    in_data.v3.msg_flags = *p1++;

    // 解码安全模型
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.msg_security_model)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    if (in_data.v3.msg_security_model != SECURITY_MODEL_USM) {
        return SNMP_ERROR_SECURITY;
    }

    *packet_ptr = p1;
    *remaining_len -= (p1 - *packet_ptr);
    return SNMP_SUCCESS;
}

// 处理V3安全参数
static int process_v3_security_parameters(unsigned char **packet_ptr, unsigned int *remaining_len, unsigned int *user_index) {
    unsigned char *p1 = *packet_ptr;
    unsigned int len;
    int bytes_used;

    // 解码安全参数字符串
    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解码安全参数序列
    if ((bytes_used = asn_decode_sequence(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解码引擎ID
    if ((bytes_used = asn_decode_octet_string(p1, &in_data.v3.msg_authoritative_engine_id_len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    if (in_data.v3.msg_authoritative_engine_id_len > MAX_ENGINE_ID) {
        return SNMP_ERROR_BUFFER;
    }
    safe_memcpy(in_data.v3.msg_authoritative_engine_id, p1 + bytes_used, 
               MAX_ENGINE_ID, in_data.v3.msg_authoritative_engine_id_len);
    p1 += bytes_used + in_data.v3.msg_authoritative_engine_id_len;

    // 解码引擎启动次数
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.msg_authoritative_engine_boots)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解码引擎时间
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.msg_authoritative_engine_time)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解码用户名
    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    if (len >= MAX_USERNAME_LEN) {
        return SNMP_ERROR_BUFFER;
    }
    safe_memcpy(in_data.v3.msg_username, p1, MAX_USERNAME_LEN, len);
    p1 += len;

    // 查找用户索引
    *user_index = 0;
    if (len > 0 && snmp_params) {
        for (unsigned int i = 0; i < MAX_SNMPV3_USERS; i++) {
            if (memcmp(snmp_params->users[i].username, in_data.v3.msg_username, len) == 0) {
                *user_index = i;
                break;
            }
        }
    }
    
    if (snmp_params) {
        snmp_params->current_user_index = *user_index;
    }

    // 解码认证参数
    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;
    safe_memcpy(in_data.v3.msg_authentication_parameters, p1, MAX_MSG_AUTH_PARAM, len);
    p1 += len;

    // 解码加密参数
    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;
    safe_memcpy(in_data.v3.msg_privacy_parameters, p1, MAX_MSG_PRIV_PARAM, len);
    p1 += len;

    *packet_ptr = p1;
    *remaining_len -= (p1 - *packet_ptr);
    return SNMP_SUCCESS;
}

// 验证V3用户安全设置
static int validate_v3_user_security(unsigned int user_index) {
    if (!is_valid_user_index(user_index)) {
        return RPT_ERR_AUTH_FAIL;
    }

    // 验证认证标志匹配
    if (((snmp_params->users[user_index].auth_type != AUTH_NONE) && 
         ((in_data.v3.msg_flags & MSG_SEC_AUTH) == 0)) ||
        ((snmp_params->users[user_index].auth_type == AUTH_NONE) && 
         ((in_data.v3.msg_flags & MSG_SEC_AUTH) != 0))) {
        return RPT_ERR_AUTH_FAIL;
    }

    // 验证加密标志匹配
    if (((snmp_params->users[user_index].priv_type != PRIV_NONE) && 
         ((in_data.v3.msg_flags & MSG_SEC_PRIV) == 0)) ||
        ((snmp_params->users[user_index].priv_type == PRIV_NONE) && 
         ((in_data.v3.msg_flags & MSG_SEC_PRIV) != 0))) {
        return RPT_ERR_AUTH_FAIL;
    }

    return RPT_ERR_NONE;
}

// 验证V3引擎参数
static int validate_v3_engine_parameters(void) {
    if (!snmp_params) {
        return RPT_ERR_ENG_ID;
    }

    // 检查时间窗口
    if ((abs((int)(gtimeCount1s - in_data.v3.msg_authoritative_engine_time)) >= 150) ||
        (in_data.v3.msg_authoritative_engine_boots != snmp_params->boots)) {
        return RPT_ERR_TIME_WIN;
    }

    // 检查引擎ID
    if ((in_data.v3.msg_authoritative_engine_id_len != snmp_params->engine_id_len) || 
        (memcmp(in_data.v3.msg_authoritative_engine_id, snmp_params->engine_id, 
                snmp_params->engine_id_len) != 0)) {
        return RPT_ERR_ENG_ID;
    }

    return RPT_ERR_NONE;
}

// 检查V3用户权限
static int check_v3_user_permissions(unsigned int user_index, int request_type) {
    if (!is_valid_user_index(user_index)) {
        return RPT_ERR_AUTH_FAIL;
    }

    switch (request_type) {
        case GET_RESPONSE_PDU:
            if (snmp_stats) snmp_stats->in.getResponses++;
            // 继续检查GET权限
        case GET_REQUEST_PDU:
            if (snmp_stats) snmp_stats->in.getRequests++;
            // 继续检查GET权限
        case GET_NEXT_REQUEST_PDU:
            if (snmp_stats) snmp_stats->in.getNexts++;
            if (snmp_params->users[user_index].user_level == USER_LEVEL_NONE) {
                return RPT_ERR_AUTH_FAIL;
            }
            break;

        case SET_REQUEST_PDU:
            if (snmp_stats) snmp_stats->in.setRequests++;
            if (snmp_params->users[user_index].user_level != USER_LEVEL_READ_WRITE) {
                return RPT_ERR_AUTH_FAIL;
            }
            break;

        default:
            break;
    }

    return RPT_ERR_NONE;
}

// 处理V3加密PDU
static int process_v3_encrypted_pdu(unsigned char **packet_ptr, unsigned int user_index) {
    unsigned char *p1 = *packet_ptr;
    unsigned int len;
    int bytes_used;

    if (!(in_data.v3.msg_flags & MSG_SEC_PRIV)) {
        return SNMP_SUCCESS; // 未加密，直接返回
    }

    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解密PDU
    if (is_valid_user_index(user_index)) {
        localize_key(snmp_params->users[user_index].auth_type, 
                    snmp_params->users[user_index].priv_key, 
                    in_data.v3.lkey, 
                    in_data.v3.msg_authoritative_engine_id, 
                    in_data.v3.msg_authoritative_engine_id_len);
        decrypt_pdu(snmp_params->users[user_index].priv_type, 
                   in_data.v3.lkey, p1, len, 
                   in_data.v3.msg_privacy_parameters, 
                   in_data.v3.msg_authoritative_engine_boots, 
                   in_data.v3.msg_authoritative_engine_time);
    }

    *packet_ptr = p1;
    return SNMP_SUCCESS;
}

// 解码V3 PDU头部
static int decode_v3_pdu_header(unsigned char **packet_ptr) {
    unsigned char *p1 = *packet_ptr;
    unsigned int len;
    int bytes_used;

    // 解码PDU序列
    if ((bytes_used = asn_decode_sequence(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 跳过contextEngineID和contextName
    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used + len;

    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used + len;

    // 解码PDU类型
    if ((bytes_used = asn_decode_request_type(p1, &in_data.v3.request_type, &len)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 解码请求ID
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.request_id)) == 0) {
        return SNMP_ERROR_ASN_PARSE;
    }
    p1 += bytes_used;

    // 根据PDU类型解码不同字段
    if (in_data.v3.request_type == GET_BULK_PDU) {
        if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.non_repeaters)) == 0) {
            return SNMP_ERROR_ASN_PARSE;
        }
        p1 += bytes_used;

        if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.max_repetitions)) == 0) {
            return SNMP_ERROR_ASN_PARSE;
        }
        p1 += bytes_used;

        in_data.v3.error_index = 0;
        in_data.v3.error_status = 0;
    } else {
        if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.error_status)) == 0) {
            return SNMP_ERROR_ASN_PARSE;
        }
        p1 += bytes_used;

        if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v3.error_index)) == 0) {
            return SNMP_ERROR_ASN_PARSE;
        }
        p1 += bytes_used;

        in_data.v3.non_repeaters = 0;
        in_data.v3.max_repetitions = 0;
    }

    *packet_ptr = p1;
    return SNMP_SUCCESS;
}

// 处理V3报告请求
static int handle_v3_report_request(unsigned char *packet_out) {
    if (!cb_struct.wait_report) {
        return 0;
    }

    // 设置响应数据
    out_data.v3.msg_version = SNMP_V3_VERSION;
    out_data.v3.msg_id = in_data.v3.msg_id;
    out_data.v3.msg_max_size = SNMP_MAX_LEN;
    out_data.v3.msg_flags = MSG_SEC_REP;
    
    if (is_valid_user_index(cb_struct.user_index)) {
        if (snmp_params->users[cb_struct.user_index].auth_type != AUTH_NONE)
            out_data.v3.msg_flags |= MSG_SEC_AUTH;
        if (snmp_params->users[cb_struct.user_index].priv_type != PRIV_NONE)
            out_data.v3.msg_flags |= MSG_SEC_PRIV;
    }
    
    out_data.v3.msg_security_model = SECURITY_MODEL_USM;
    safe_memcpy(out_data.v3.msg_authoritative_engine_id, 
               in_data.v3.msg_authoritative_engine_id, 
               MAX_ENGINE_ID, in_data.v3.msg_authoritative_engine_id_len);
    out_data.v3.msg_authoritative_engine_id_len = in_data.v3.msg_authoritative_engine_id_len;
    out_data.v3.msg_authoritative_engine_boots = in_data.v3.msg_authoritative_engine_boots;
    out_data.v3.msg_authoritative_engine_time = in_data.v3.msg_authoritative_engine_time;
    
    if (is_valid_user_index(cb_struct.user_index)) {
        safe_memcpy(out_data.v3.msg_username, 
                   snmp_params->users[cb_struct.user_index].username, 
                   MAX_USERNAME_LEN, MAX_USERNAME_LEN - 1);
    }
    
    out_data.v3.request_type = GET_REQUEST_PDU;
    out_data.v3.request_id = in_data.v3.request_id;
    out_data.v3.error_status = 0;
    out_data.v3.error_index = 0;
    
    if (is_valid_user_index(cb_struct.user_index)) {
        out_data.v3.user = &snmp_params->users[cb_struct.user_index];
    }
    
    out_data.v3.var_bindings = snmp_varbindings;
    out_data.v3.non_repeaters = 0;
    out_data.v3.max_repetitions = 0;

    // 添加NULL var-binding
    out_data.v3.var_bindings_len = mib_add_var_binding(snmp_varbindings, 0, &cb_struct.oid, 0, ASN_NULL);

    cb_struct.wait_report = 0;
    cb_struct.wait_reply = 1;

    return build_snmp_v3(packet_out, &out_data);
}

// 处理V3普通请求
static int handle_v3_normal_request(unsigned char **packet_ptr, unsigned int remaining_len, 
                                   unsigned int user_index, unsigned char *packet_out) {
    unsigned char *p1 = *packet_ptr;
    
    // 处理var-bindings
    out_data.v3.var_bindings_len = process_varbindings(p1, remaining_len, 
                                                     snmp_varbindings, in_data.v3.msg_version, 
                                                     in_data.v3.request_type, in_data.v3.non_repeaters, 
                                                     in_data.v3.max_repetitions, 
                                                     (int *)&out_data.v3.error_status, 
                                                     (int *)&out_data.v3.error_index);

    if (out_data.v3.var_bindings_len <= 0) {
        return 0;
    }

    // 构建响应
    out_data.v3.msg_version = SNMP_V3_VERSION;
    out_data.v3.msg_id = in_data.v3.msg_id;
    out_data.v3.msg_max_size = SNMP_MAX_LEN;
    out_data.v3.msg_flags = 0;
    
    if (is_valid_user_index(user_index)) {
        if (snmp_params->users[user_index].auth_type != AUTH_NONE)
            out_data.v3.msg_flags |= MSG_SEC_AUTH;
        if (snmp_params->users[user_index].priv_type != PRIV_NONE)
            out_data.v3.msg_flags |= MSG_SEC_PRIV;
    }
    
    out_data.v3.msg_security_model = SECURITY_MODEL_USM;
    
    if (snmp_params) {
        safe_memcpy(out_data.v3.msg_authoritative_engine_id, snmp_params->engine_id, 
                   MAX_ENGINE_ID, snmp_params->engine_id_len);
        out_data.v3.msg_authoritative_engine_id_len = snmp_params->engine_id_len;
    }
    
    out_data.v3.msg_authoritative_engine_boots = in_data.v3.msg_authoritative_engine_boots;
    out_data.v3.msg_authoritative_engine_time = in_data.v3.msg_authoritative_engine_time;
    
    if (is_valid_user_index(user_index)) {
        safe_memcpy(out_data.v3.msg_username, snmp_params->users[user_index].username, 
                   MAX_USERNAME_LEN, MAX_USERNAME_LEN - 1);
    }
    
    out_data.v3.request_type = GET_RESPONSE_PDU;
    out_data.v3.request_id = in_data.v3.request_id;
    out_data.v3.user = is_valid_user_index(user_index) ? &snmp_params->users[user_index] : NULL;
    out_data.v3.var_bindings = snmp_varbindings;
    out_data.v3.non_repeaters = in_data.v3.non_repeaters;
    out_data.v3.max_repetitions = in_data.v3.max_repetitions;

    return build_snmp_v3(packet_out, &out_data);
}

// 构建V3错误报告
static int build_v3_error_report(unsigned char *packet_out, unsigned int report_error) {
    // 设置响应数据
    out_data.v3.msg_version = SNMP_V3_VERSION;
    out_data.v3.msg_id = in_data.v3.msg_id;
    out_data.v3.msg_max_size = SNMP_MAX_LEN;
    out_data.v3.msg_flags = 0;
    out_data.v3.msg_security_model = SECURITY_MODEL_USM;
    
    if (snmp_params) {
        safe_memcpy(out_data.v3.msg_authoritative_engine_id, snmp_params->engine_id,
                   MAX_ENGINE_ID, snmp_params->engine_id_len);
        out_data.v3.msg_authoritative_engine_id_len = snmp_params->engine_id_len;
        out_data.v3.msg_authoritative_engine_boots = snmp_params->boots;
        out_data.v3.user = &snmp_params->users[0];
    }
    
    out_data.v3.msg_authoritative_engine_time = gtimeCount1s;
    out_data.v3.msg_username[0] = '\0';
    out_data.v3.request_type = REPORT_PDU;
    out_data.v3.request_id = in_data.v3.request_id;
    out_data.v3.error_status = 0;
    out_data.v3.error_index = 0;

    // 设置错误OID
    memcpy(vb_data.oid.val, rpt_err_oid, sizeof(rpt_err_oid));
    vb_data.oid.len = sizeof(rpt_err_oid) / sizeof(unsigned int);
    vb_data.oid.val[RPT_ERR_INDEX] = report_error;

    // 添加报告var-binding
    out_data.v3.var_bindings = snmp_varbindings;
    out_data.v3.var_bindings_len = mib_process_oid(out_data.v3.var_bindings, &vb_data.oid, 
                                                 REPORT_PDU, 0, 0, 0, 
                                                 (int *)&out_data.v3.error_status);
    if (out_data.v3.var_bindings_len == 0) {
        return 0;
    }

    return build_snmp_v3(packet_out, &out_data);
}

// ==================== 主处理函数 ====================

// 优化后的V3消息处理函数（彻底模块化）
static int snmp_process_v3(unsigned char *packet_in, unsigned int in_length, unsigned char *packet_out) {
    unsigned char *p1;
    unsigned int remaining_len;
    unsigned int user_index;
    unsigned int report_error;
    int result;

    // 1. 输入参数验证
    if ((result = validate_input_params(packet_in, in_length, packet_out)) != SNMP_SUCCESS) {
        return 0;
    }

    p1 = packet_in;
    remaining_len = in_length;

    // 2. 解码全局头部
    if ((result = decode_v3_global_header(&p1, &remaining_len)) != SNMP_SUCCESS) {
        return 0;
    }

    // 3. 解码消息头部
    if ((result = decode_v3_message_header(&p1, &remaining_len)) != SNMP_SUCCESS) {
        return 0;
    }

    // 4. 处理安全参数
    if ((result = process_v3_security_parameters(&p1, &remaining_len, &user_index)) != SNMP_SUCCESS) {
        return 0;
    }

    // 5. 安全验证
    report_error = validate_v3_user_security(user_index);
    if (report_error != RPT_ERR_NONE) {
        return build_v3_error_report(packet_out, report_error);
    }

    // 6. 引擎参数验证
    report_error = validate_v3_engine_parameters();
    if (report_error != RPT_ERR_NONE) {
        return build_v3_error_report(packet_out, report_error);
    }

    // 7. 用户索引验证
    if (user_index == 0) {
        return build_v3_error_report(packet_out, RPT_ERR_SEC_NAME);
    }

    // 8. 处理加密PDU
    if ((result = process_v3_encrypted_pdu(&p1, user_index)) != SNMP_SUCCESS) {
        return 0;
    }

    // 9. 解码PDU头部
    if ((result = decode_v3_pdu_header(&p1)) != SNMP_SUCCESS) {
        return 0;
    }

    // 10. 请求类型验证
    if ((in_data.v3.request_type != REPORT_PDU) && (in_data.v3.request_type != GET_RESPONSE_PDU)) {
        // 检查用户权限
        report_error = check_v3_user_permissions(user_index, in_data.v3.request_type);
        if (report_error != RPT_ERR_NONE) {
            return build_v3_error_report(packet_out, report_error);
        }

        // 验证请求字段
        if (in_data.v3.error_status != 0 || in_data.v3.error_index != 0) {
            return 0;
        }
    }

    // 11. 根据请求类型处理
    if (in_data.v3.request_type == REPORT_PDU) {
        return handle_v3_report_request(packet_out);
    } else {
        return handle_v3_normal_request(&p1, remaining_len - (p1 - packet_in), user_index, packet_out);
    }
}

// varbindings between v1, v2c and v3 are the same so we can process them all in the same function
int process_varbindings(unsigned char *in_varbindings, int in_varbindings_len, unsigned char *out_varbindings, int msg_version, int request_type, int non_repeaters, int max_repetitions, int *error_status, int *error_index)
{
    unsigned char *p1;
    unsigned int bytes_used;
    unsigned int len;
    int i;
    int temp;
    int set_type;
    void *set_value;
    int out_varbindings_len = 0;

    // 检查输入参数
    if (!in_varbindings || !out_varbindings || !error_status || !error_index || in_varbindings_len <= 0) {
        return 0;
    }

    p1 = in_varbindings;
    *error_status = 0;
    *error_index = 0;

    // varbindings are encapsulated in a sequence, remove it first
    if ((bytes_used = asn_decode_sequence(p1, &len)) == 0)
        return 0;
    p1 += bytes_used;

    // now we process each var-binding
    while ((in_varbindings_len - (p1 - in_varbindings)) > 0)
    {
        // each var-binding is encapsulated in a sequence
        // and are comprised of an OID and a value
        if ((bytes_used = asn_decode_sequence(p1, &len)) == 0)
            break;
        p1 += bytes_used;

        // get oid
        if ((bytes_used = asn_decode_oid(p1, &vb_data.oid)) == 0)
            break;
        p1 += bytes_used;

        // get value
        if ((bytes_used = asn_decode_null(p1)) != 0)
        {
            set_type = ASN_NULL;
            set_value = NULL;
            p1 += bytes_used;
        }
        else if ((bytes_used = asn_decode_octet_string(p1, &len)) != 0)
        {
            set_type = ASN_OCTET_STRING;
            // 确保不会缓冲区溢出
            if (len >= OCTET_STRING_VB_LEN) {
                len = OCTET_STRING_VB_LEN - 1;
            }
            memcpy(vb_data.val.octet_string, p1 + bytes_used, len);
            vb_data.val.octet_string[len] = '\0';
            set_value = vb_data.val.octet_string;
            p1 += bytes_used + len;
        }
        else if ((bytes_used = asn_decode_int_type(p1, (unsigned int *)&set_type, &len)) != 0)
        {
            memcpy(&vb_data.val.intv, &len, sizeof(len));
            set_value = &vb_data.val.intv;
            p1 += bytes_used;
        }
        else if ((bytes_used = asn_decode_oid(p1, &vb_data.val.oid)) != 0)
        {
            set_value = &vb_data.val.oid;
            set_type = ASN_OID;
            p1 += bytes_used;
        }
        else
        {
            // debug_printf("snmp: Can't decode var binding type 0x%x\n", *p1);
            return 0;
        }

        if (*error_status == snmp_NO_ERROR)
        {
            if (((msg_version == 1) || (msg_version == 3)) && (request_type == GET_BULK_PDU))
            {
                if (non_repeaters > 0)
                {
                    if ((temp = mib_process_oid(out_varbindings + out_varbindings_len, &vb_data.oid, GET_NEXT_REQUEST_PDU, set_type, set_value, len, error_status)) == 0)
                        return 0;
                    out_varbindings_len += temp;
                    (*error_index)++;
                    non_repeaters--;
                }
                else
                {
                    for (i = 0; (i < max_repetitions) && (*error_status == snmp_NO_ERROR); i++)
                    {
                        if ((temp = mib_process_oid(out_varbindings + out_varbindings_len, &vb_data.oid, GET_NEXT_REQUEST_PDU, set_type, set_value, len, error_status)) == 0)
                            return 0;
                        out_varbindings_len += temp;
                        (*error_index)++;
                        non_repeaters--;
                    }
                }
            }
            else if (request_type == GET_RESPONSE_PDU)
            {
                // call call back function
                if ((cb_struct.wait_reply) && (cb_struct.request_cb != NULL))
                {
                    if ((in_data.msg_version == 0) || (in_data.msg_version == 1))
                    {
                        // check community
                        if (strcmp((const char *)cb_struct.community, (const char *)in_data.v1.community) != 0)
                        {
                            snmp_stats->badCommunityNames++;
                            return 0;
                        }
                    }
                    cb_struct.request_cb(&vb_data.oid, set_type, set_value, len);
                }
                return 0; // no reply to a GET_REQUEST_PDU
            }
            else
            {
                if ((temp = mib_process_oid(out_varbindings + out_varbindings_len, &vb_data.oid, request_type, set_type, set_value, len, error_status)) == 0)
                    return 0;
                out_varbindings_len += temp;
                (*error_index)++;
            }
        }
        else // had an error, just add NULL's for each additional var-binding
        {
            // debug_printf("Error: SNMP.c process_varbindings, one varbinding had a null value.\r\n");
            if ((temp = mib_add_null_oid(out_varbindings + out_varbindings_len, &vb_data.oid)) == 0)
                return 0;
            out_varbindings_len += temp;
        }
    }
    return out_varbindings_len;
}
// process a v1/v2c message
static int snmp_process_v1(unsigned char *packet_in, unsigned int in_length, unsigned char *packet_out)
{
    unsigned char *p1;
    unsigned int len;
    int bytes_used;

    // Check input parameters
    if (!packet_in || !packet_out || in_length == 0) {
        return 0;
    }

    // The header consists of an ASN.1 sequence containing:
    // INTEGER: Version
    // OCTET STRING: Community
    // ANY: Data

    p1 = packet_in;

    if ((bytes_used = asn_decode_sequence(p1, &len)) == 0)
        return 0;
    p1 += bytes_used;

    if (len != in_length - bytes_used)
    {
        // debug_printf("snmp: length not equal to packet length\n");
        snmp_stats->asnParseErrs++;
        return 0;
    }

    // the next 2 bytes should be a type2 with a length of 1 and a value of 0
    // to indicate a primitive integer of length 1 and value 0 for the SNMP
    // version.  Anything else is wrong
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v1.msg_version)) == 0)
    {
        snmp_stats->asnParseErrs++;
        return 0;
    }

    if ((in_data.v1.msg_version != 0) && (in_data.v1.msg_version != 1))
    {
        // debug_printf("snmp: bad version number: %d\n", in_data.v1.msg_version);
        snmp_stats->badVersions++;
        return 0;
    }
    p1 += bytes_used;

    // the next value should be a type 0x04 for the OCTET STRING
    // followed by our community name
    if ((bytes_used = asn_decode_octet_string(p1, &len)) == 0)
    {
        snmp_stats->asnParseErrs++;
        return 0;
    }
    p1 += bytes_used;

    // set community name
    if (len >= MAX_COMMUNITY_NAME) {
        len = MAX_COMMUNITY_NAME - 1;
    }
    memcpy(in_data.v1.community, p1, len);
    in_data.v1.community[len] = '\0';
    p1 += len; // take up slack from community name

    // get PDU packet type
    if ((bytes_used = asn_decode_request_type(p1, &in_data.v1.request_type, &len)) == 0)
    {
        snmp_stats->asnParseErrs++;
        return 0;
    }
    p1 += bytes_used;

    if ((in_data.v1.request_type != GET_RESPONSE_PDU) && (in_data.v1.request_type != GET_REQUEST_PDU) && (in_data.v1.request_type != SET_REQUEST_PDU) && (in_data.v1.request_type != GET_NEXT_REQUEST_PDU))
    {
        if ((in_data.v1.msg_version == 1) && (in_data.v1.request_type != GET_BULK_PDU))
        {
            // debug_printf("snmp: Unsupported or unknown RequestPDU %x\n", in_data.v1.request_type);
            return 0;
        }
    }

    // Verify Community
    switch (in_data.v1.request_type)
    {
    case GET_RESPONSE_PDU:
        snmp_stats->in.getResponses++;
        // Fall through
    case GET_REQUEST_PDU:
        snmp_stats->in.getRequests++;
        // Fall through
    case GET_NEXT_REQUEST_PDU:
        snmp_stats->in.getNexts++;
        if (strcmp((char *)in_data.v1.community, (char *)snmp_params->read_community) != 0)
        {
            // debug_printf("snmp: Invalid Read Community: Got '%s' need '%s'\r\n", in_data.v1.community, snmp_params->read_community);
            snmp_stats->badCommunityNames++;
            return 0;
        }
        break;

    case SET_REQUEST_PDU:
        snmp_stats->in.setRequests++;
        if (strcmp((char *)in_data.v1.community, (char *)snmp_params->write_community) != 0)
        {
            // debug_printf("snmp: Invalid Write Community: Got '%s' need '%s'\r\n", in_data.v1.community, snmp_params->write_community);
            return 0;
        }
        break;

    default:
        // debug_printf("snmp: Unchecked request type %d\r\n", in_data.v1.request_type);
        break;
    }

    // check the PDU length against the bytes left in the packet
    if ((in_length - (p1 - packet_in)) != len)
    {
        // debug_printf("snmp: invalid byte count, bytes_left=%d, pdu_bytes=%d\n", (in_length-(p1-packet_in)), len);
        snmp_stats->asnParseErrs++;
        return 0;
    }

    // get request id
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v1.request_id)) == 0)
    {
        snmp_stats->asnParseErrs++;
        return 0;
    }
    p1 += bytes_used;

    if ((in_data.v1.msg_version == 1) && (in_data.v1.request_type == GET_BULK_PDU)) // get other vals
    {
        // get non-repeaters
        if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v1.non_repeaters)) == 0)
        {
            snmp_stats->asnParseErrs++;
            return 0;
        }
        p1 += bytes_used;

        // get max-repetitions
        if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v1.max_repetitions)) == 0)
        {
            snmp_stats->asnParseErrs++;
            return 0;
        }
        p1 += bytes_used;
    }
    else // error number and index not in version 2c getBulkPDU
    {
        // get error number
        if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v1.error_status)) == 0)
        {
            snmp_stats->asnParseErrs++;
            return 0;
        }
        p1 += bytes_used;

        // get error index
        if ((bytes_used = asn_decode_int(p1, ASN_INT, &in_data.v1.error_index)) == 0)
        {
            snmp_stats->asnParseErrs++;
            return 0;
        }
        p1 += bytes_used;
    }

    out_data.v1.var_bindings = snmp_varbindings;
    out_data.v1.var_bindings_len = process_varbindings(p1, in_length - (p1 - packet_in), out_data.v1.var_bindings, in_data.v1.msg_version, in_data.v1.request_type, in_data.v1.non_repeaters, in_data.v1.max_repetitions, (int *)&out_data.v1.error_status, (int *)&out_data.v1.error_index);
    // we've process all the varbindings, now to build the reply packet and send it
    if (out_data.v1.var_bindings_len > 0)
    {
        // build reply packet
        out_data.v1.msg_version = in_data.v1.msg_version;
        strncpy((char *)out_data.v1.community, (const char *)in_data.v1.community, MAX_COMMUNITY_NAME - 1);
        out_data.v1.community[MAX_COMMUNITY_NAME - 1] = '\0';
        out_data.v1.request_type = GET_RESPONSE_PDU;
        out_data.v1.request_id = in_data.v1.request_id;
        out_data.v1.non_repeaters = 0;
        out_data.v1.max_repetitions = 0;

        return build_snmp_v2c(packet_out, &out_data);
    }

    return 0;
}

int snmp_process(unsigned char *packet_in, unsigned int length_in, unsigned char *packet_out)
{
    int bytes_used;
    unsigned int len;
    unsigned int version_number;
    unsigned char *p1;

    // Check input parameters
    if (!packet_in || !packet_out || length_in == 0) {
        snmp_stats->asnParseErrs++;
        return 0;
    }

    // The header consists of an ASN.1 sequence containing:
    // INTEGER: Version
    // OCTET STRING: Community
    // ANY: Data

    p1 = packet_in;

    if ((bytes_used = asn_decode_sequence(p1, &len)) == 0)
    {
        snmp_stats->asnParseErrs++;
        return 0;
    }
    p1 += bytes_used;

    if (len != length_in - bytes_used)
    {
        // debug_printf("snmp: length not equal to packet length\n");
        snmp_stats->asnParseErrs++;
        return 0;
    }

    // the next 2 bytes should be a type2 with a length of 1 and a value of 0
    // to indicate a primitive integer of length 1 and value 0 for the SNMP
    // version.  Anything else is wrong
    if ((bytes_used = asn_decode_int(p1, ASN_INT, &version_number)) == 0)
    {
        snmp_stats->asnParseErrs++;
        return 0;
    }

    if ((version_number == 0) || (version_number == 1))
    {
        return snmp_process_v1(packet_in, length_in, packet_out);
    }
    else if (version_number == 3)
    {
        if (snmp_params->use_v3)
        {
            return snmp_process_v3(packet_in, length_in, packet_out);
        }
        else
        {
            // debug_printf("Received V3 SNMP, but V3 is disabled\r\n");
            return 0;
        }
    }
    else
    {
        snmp_stats->badVersions++;
        return 0;
    }
    
    return 0;
}
