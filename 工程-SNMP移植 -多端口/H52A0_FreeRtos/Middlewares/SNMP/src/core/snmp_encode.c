// SNMP v1/v2c处理实现
// 包含v1和v2c版本的处理逻辑
#include <string.h>
#include <stdlib.h>
#include "./mib/mib.h"
#include "./asn1/asn.1.h"
#include "./core/snmp.h"
#include "./core/snmp_crypto.h"

//                       AUTH_NONE  AUTH_MD5  AUTH_SHA -- both 96 bits
unsigned int auth_len[] = {0, 12, 12};
//                       PRIV_NONE  PRIV_DES   PRIV_AES -- both 64 bits
unsigned int priv_len[] = {0, 8, 8};

unsigned char trap_ip[4] = {0, 0, 0, 0}; // trap目标IP地址
// build a v1
int build_snmp_v1(unsigned char *out_buf, _snmp_data *data)
{

    unsigned char *p1;
    unsigned int bytes_used;

    p1 = out_buf;

    data->v1.generic_trap = TRAP_ENTERPRISE_SPEC;
    data->v1.specific_trap = 0;
    data->v1.enterprise.val[0] = 1;
    data->v1.enterprise.val[1] = 3;
    data->v1.enterprise.val[2] = 4;
    data->v1.enterprise.val[3] = 6;
    data->v1.enterprise.val[4] = 1234;
    data->v1.enterprise.len = 5;

    // *p1++=ASN_SEQUENCE;
    // *p1++=0x23;

    // packet length consists of
    // 1 + asn_bytes_length(strlen(community_name)) + strlen(community_name)
    // 1 + asn_bytes_length(6+3+3+1+asn_bytes_length(var_binding_bytes_used)+var_binding_bytes_used)
    // 1 + asn_bytes_length(&data->v1.specific_trap, ASN_INT, 0)
    // 1 + asn_bytes_length(&uptime_sec, ASN_INT, 0)
    // 1 + asn_bytes_length(&data->v1.enterprise)
    // 6 for agent-addr (IP)
    // 1 for request type (0xa4)
    // 1 for data length
    // 1 for end of sequence indicator
    // 1 for ending value of 0x00
    unsigned char length = asn_bytes_to_encode(&data->v1.community, ASN_OCTET_STRING, 0) + strlen((const char*)data->v1.community);
    length += 6 + 3 + 1 + 1 + 1 + asn_bytes_to_encode(&data->v1.specific_trap, ASN_INT, 0) + asn_bytes_to_encode(&gtimeCount1s, ASN_INT, 0);
    length += asn_bytes_to_encode(&data->v1.generic_trap, ASN_INT, 0);
    length += asn_bytes_to_encode_oid(&data->v1.enterprise) + data->v1.var_bindings_len + asn_bytes_length(data->v1.var_bindings_len);

    // debug_printf("Var length: %d\n", length);

    *p1++=ASN_SEQUENCE;
    *p1++=length;

    // Add version
    p1+=asn_encode_int(p1,ASN_INT, data->v1.msg_version);

    // add community
    *p1++=ASN_OCTET_STRING;
    if ((bytes_used=asn_encode_length(p1, strlen((const char*)data->v1.community)))==0) return 0;
    p1+=bytes_used;
    memcpy(p1, data->v1.community, strlen((const char*)data->v1.community));
    p1+=strlen((const char*)data->v1.community);

    // add packet_type
    *p1++=data->v1.request_type;

    unsigned int data_length = 6 + 1 + asn_bytes_to_encode(&data->v1.specific_trap, ASN_INT, 0);
    data_length += asn_bytes_to_encode(&data->v1.generic_trap, ASN_INT, 0) + asn_bytes_to_encode(&gtimeCount1s, ASN_INT, 0);
    data_length += asn_bytes_to_encode_oid(&data->v1.enterprise) + data->v1.var_bindings_len + asn_bytes_length(data->v1.var_bindings_len);

    *p1++=data_length;

    //Encode the enterprise OID
     p1+=asn_encode_oid(p1, &data->v1.enterprise);

    //Encode the agent address
    p1+=asn_encode_snmp_ipaddress(p1, trap_ip); //TODO: Need this to be specific to each interface

    //Encode Generic Trap
    p1+=asn_encode_int(p1, ASN_INT, data->v1.generic_trap);

    //Encode Specific Trap
    p1+=asn_encode_int(p1, ASN_INT, data->v1.specific_trap);

    //Encode time Stamp
    p1+=asn_encode_int(p1, ASN_SNMP_TIMETICKS, gtimeCount1s);

    // *p1++=0x30;
    // *p1++=0x00;


    // Add VAR_BINDING SEQUENCE
    *p1++=ASN_SEQUENCE;
    if ((bytes_used=asn_encode_length(p1, data->v1.var_bindings_len))==0) return 0;
    p1+=bytes_used;
    memcpy(p1, data->v1.var_bindings, data->v1.var_bindings_len);
    p1+=data->v1.var_bindings_len;

    return p1 - out_buf;
}

// build a v1/v2c message
int build_snmp_v2c(unsigned char *out_buf, _snmp_data *data)
{
    unsigned char *p1;
    unsigned int bytes_used;

    p1 = out_buf;

    *p1++ = ASN_SEQUENCE;
    // packet length consists of
    // 3 byte for version
    // 1 + asn_bytes_length(strlen(community_name)) + strlen(community_name)
    // 1 + asn_bytes_length(6+3+3+1+asn_bytes_length(var_binding_bytes_used)+var_binding_bytes_used)
    // 6 for REQUEST_ID
    // 3 for ERROR_STATUS
    // 3 for ERROR_INDEX
    // 1 + asn_bytes_length(var_binding_bytes_used) + var_binding_bytes_used
    bytes_used = asn_encode_length(p1, 3 + 1 + asn_bytes_length(strlen((const char *)data->v1.community)) + strlen((const char *)data->v1.community) +
                                           1 + asn_bytes_length(asn_bytes_to_encode(&data->v1.request_id, ASN_INT, 0) + 3 + 3 + 1 + asn_bytes_length(data->v1.var_bindings_len) + data->v1.var_bindings_len) +
                                           asn_bytes_to_encode(&data->v1.request_id, ASN_INT, 0) + 3 + 3 +
                                           1 + asn_bytes_length(data->v1.var_bindings_len) + data->v1.var_bindings_len);
    if (bytes_used == 0)
        return 0;
    p1 += bytes_used;

    // Add version
    p1 += asn_encode_int(p1, ASN_INT, data->v1.msg_version);

    // add community
    *p1++ = ASN_OCTET_STRING;
    if ((bytes_used = asn_encode_length(p1, strlen((const char *)data->v1.community))) == 0)
        return 0;
    p1 += bytes_used;
    memcpy(p1, data->v1.community, strlen((const char *)data->v1.community));
    p1 += strlen((const char *)data->v1.community);

    // add packet_type
    *p1++ = data->v1.request_type;
    if ((bytes_used = asn_encode_length(p1, asn_bytes_to_encode(&data->v1.request_id, ASN_INT, 0) + 3 + 3 + 1 + asn_bytes_length(data->v1.var_bindings_len) + data->v1.var_bindings_len)) == 0)
        return 0;
    p1 += bytes_used;

    // Add request ID
    p1 += asn_encode_int(p1, ASN_INT, data->v1.request_id);

    // Add ERROR STATUS
    p1 += asn_encode_int(p1, ASN_INT, data->v1.error_status);

    // Add ERROR INDEX
    p1 += asn_encode_int(p1, ASN_INT, data->v1.error_status != snmp_NO_ERROR ? data->v1.error_index : 0);

    // Add VAR_BINDING SEQUENCE
    *p1++ = ASN_SEQUENCE;
    if ((bytes_used = asn_encode_length(p1, data->v1.var_bindings_len)) == 0)
        return 0;
    p1 += bytes_used;
    memcpy(p1, data->v1.var_bindings, data->v1.var_bindings_len);
    p1 += data->v1.var_bindings_len;

    return p1 - out_buf;
}

// build a v3 message
int build_snmp_v3(unsigned char *out_buf, _snmp_data *data)
{
    unsigned char *p1;
    unsigned char *pdu_ptr;
    unsigned char *auth_param_ptr;
    unsigned char *priv_param_ptr;
    int bytes_used;

    // At this point we have the var-bindings and we need to construct a reply packet
    // we need to calculate how large the entire sequence is
    // VERSION
    // HEADER
    // SEC PARAMS
    // PDU
    int version_len;
    version_len = asn_bytes_to_encode(&data->v3.msg_version, ASN_INT, 0);

    int header_len;
    header_len = asn_bytes_to_encode(&data->v3.msg_id, ASN_INT, 0);              // msgID
    header_len += asn_bytes_to_encode(&data->v3.msg_max_size, ASN_INT, 0);       // msgMaxSize
    header_len += asn_bytes_to_encode(&data->v3.msg_flags, ASN_OCTET_STRING, 1); // msg_flags
    header_len += asn_bytes_to_encode(&data->v3.msg_security_model, ASN_INT, 0); // msg_security_model

    int security_param_len;
    security_param_len = asn_bytes_to_encode(data->v3.msg_authoritative_engine_id, ASN_OCTET_STRING, data->v3.msg_authoritative_engine_id_len);
    security_param_len += asn_bytes_to_encode(&data->v3.msg_authoritative_engine_boots, ASN_INT, 0);
    security_param_len += asn_bytes_to_encode(&data->v3.msg_authoritative_engine_time, ASN_INT, 0);
    security_param_len += asn_bytes_to_encode(0, ASN_OCTET_STRING, strlen((const char *)data->v3.user->username));
    security_param_len += asn_bytes_to_encode(0, ASN_OCTET_STRING, auth_len[data->v3.user->auth_type]);
    security_param_len += asn_bytes_to_encode(0, ASN_OCTET_STRING, priv_len[data->v3.user->priv_type]);

    int pdu_len;
    pdu_len = asn_bytes_to_encode(&data->v3.request_id, ASN_INT, 0);
    pdu_len += asn_bytes_to_encode(&data->v3.error_status, ASN_INT, 0);
    pdu_len += asn_bytes_to_encode(&data->v3.error_index, ASN_INT, 0);
    pdu_len += asn_bytes_to_encode(0, ASN_SEQUENCE, data->v3.var_bindings_len);

    int pdu_seq_len;
    pdu_seq_len = asn_bytes_to_encode(data->v3.msg_authoritative_engine_id, ASN_OCTET_STRING, data->v3.msg_authoritative_engine_id_len);
    pdu_seq_len += asn_bytes_to_encode(0, ASN_OCTET_STRING, 0);
    pdu_seq_len += asn_bytes_to_encode(0, data->v3.request_type, pdu_len);

    int pdu_encryption_len = 0;
    int pad_bytes = 0;
    // check to see if we're encrypting, we'll have an octet_string containing the encrypted data
    // and the PDU packet needs to be padded to a multiple of 8 bytes
    if (data->v3.msg_flags & MSG_SEC_PRIV) // doing encryption
    {
        pdu_encryption_len = asn_bytes_to_encode(0, ASN_SEQUENCE, pdu_seq_len);
        if ((data->v3.user->priv_type == PRIV_DES) && (pdu_encryption_len % 8))
        {
            pad_bytes = (8 - (pdu_encryption_len % 8));
            pdu_encryption_len += pad_bytes;
        }
    }

    // we know how big everything is, now we start constructing the packet
    // the Sequence is first, that contains everything
    p1 = out_buf;
    p1 += asn_encode_sequence(p1, version_len +
                                      asn_bytes_to_encode(0, ASN_SEQUENCE, header_len) +
                                      asn_bytes_to_encode(0, ASN_OCTET_STRING, asn_bytes_to_encode(0, ASN_SEQUENCE, security_param_len)) +
                                      ((data->v3.msg_flags & MSG_SEC_PRIV) ? asn_bytes_to_encode(0, ASN_OCTET_STRING, pdu_encryption_len) : asn_bytes_to_encode(0, ASN_SEQUENCE, pdu_seq_len)));

    // add the version
    p1 += asn_encode_int(p1, ASN_INT, data->v3.msg_version);

    // add the header
    p1 += asn_encode_sequence(p1, header_len);
    p1 += asn_encode_int(p1, ASN_INT, data->v3.msg_id);
    p1 += asn_encode_int(p1, ASN_INT, data->v3.msg_max_size);
    p1 += asn_encode_octet_string(p1, (unsigned char *)&data->v3.msg_flags, 1);
    p1 += asn_encode_int(p1, ASN_INT, data->v3.msg_security_model); // USM

    // add the security parameters, this is tricky since the parameters are stored in an octet_string instead of a sequence
    // the first part is the octet_string token and length, then a sequence, this is a semi-hack job to do but it works
    // the first step is just to get the token and length then we'll manually copy the data in
    asn_encode_octet_string(p1, 0, asn_bytes_to_encode(0, ASN_SEQUENCE, security_param_len));
    bytes_used = 1 + asn_bytes_length(asn_bytes_to_encode(0, ASN_SEQUENCE, security_param_len));
    p1 += bytes_used;

    // now the octet_string token and length are encoded and we add the sequence
    p1 += asn_encode_sequence(p1, security_param_len);
    p1 += asn_encode_octet_string(p1, data->v3.msg_authoritative_engine_id, data->v3.msg_authoritative_engine_id_len);
    p1 += asn_encode_int(p1, ASN_INT, data->v3.msg_authoritative_engine_boots);
    p1 += asn_encode_int(p1, ASN_INT, data->v3.msg_authoritative_engine_time);
    p1 += asn_encode_octet_string(p1, (unsigned char *)data->v3.user->username, strlen(data->v3.user->username));

    // clear the auth_param and priv_parm structures, set them and then set the pointers, we'll fill this data in
    // later after it's calculated, it's fixed size so we can do this
    memset(data->v3.msg_authentication_parameters, 0, MAX_MSG_AUTH_PARAM);
    memset(data->v3.msg_privacy_parameters, 0, MAX_MSG_PRIV_PARAM);
    p1 += asn_encode_octet_string(p1, data->v3.msg_authentication_parameters, auth_len[data->v3.user->auth_type]);
    auth_param_ptr = p1 - auth_len[data->v3.user->auth_type];
    p1 += asn_encode_octet_string(p1, data->v3.msg_privacy_parameters, priv_len[data->v3.user->priv_type]);
    priv_param_ptr = p1 - priv_len[data->v3.user->priv_type];

    // if we're encrypting the packet we need to add an OCTET_STRING, pad the data and we'll encrypt at the end
    if (data->v3.msg_flags & MSG_SEC_PRIV) // encryption
    {
        // we fake it here since the encrypted data is stored as an octet_string and the asn_encode_octet_string tries to actually encode it
        asn_encode_octet_string(p1, 0, pdu_encryption_len);
        p1 += (1 + asn_bytes_length(pdu_encryption_len));
    }

    pdu_ptr = p1;

    // add the PDU sequence stuffs
    p1 += asn_encode_sequence(p1, pdu_seq_len);
    p1 += asn_encode_octet_string(p1, data->v3.msg_authoritative_engine_id, data->v3.msg_authoritative_engine_id_len);
    p1 += asn_encode_octet_string(p1, 0, 0); // context name, which is not supported/used in this implementation

    // now the pdu
    p1 += asn_encode_pdu(p1, data->v3.request_type, pdu_len); // this just adds the token and length
    p1 += asn_encode_int(p1, ASN_INT, data->v3.request_id);
    p1 += asn_encode_int(p1, ASN_INT, data->v3.error_status);
    p1 += asn_encode_int(p1, ASN_INT, data->v3.error_status != snmp_NO_ERROR ? data->v3.error_index : 0);
    p1 += asn_encode_sequence(p1, data->v3.var_bindings_len);
    memcpy(p1, data->v3.var_bindings, data->v3.var_bindings_len);
    p1 += data->v3.var_bindings_len;

    // now we encrypt and/or calculate HMAC
    if (data->v3.user->priv_type != PRIV_NONE)
    {
        localize_key(data->v3.user->auth_type, data->v3.user->priv_key, data->v3.lkey, data->v3.msg_authoritative_engine_id, data->v3.msg_authoritative_engine_id_len);
        encrypt_pdu(data->v3.user->priv_type, data->v3.lkey, pdu_ptr, pdu_encryption_len, priv_param_ptr, data->v3.msg_authoritative_engine_boots, data->v3.msg_authoritative_engine_time);
        p1 += pad_bytes;
    }

    if (data->v3.user->auth_type != AUTH_NONE)
    {
        localize_key(data->v3.user->auth_type, data->v3.user->auth_key, data->v3.lkey, data->v3.msg_authoritative_engine_id, data->v3.msg_authoritative_engine_id_len);
        calculate_hmac(data->v3.user->auth_type, data->v3.lkey, out_buf, p1 - out_buf, auth_param_ptr);
    }

    return p1 - out_buf;
}

int snmp_build_trap_v1(unsigned char *buffer, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings)
{
    int i;

    out_data.v1.var_bindings_len = 0;

    for (i = 0; i < num_bindings; i++)
    {
        out_data.v1.var_bindings_len += mib_add_var_binding(snmp_varbindings + out_data.v1.var_bindings_len, var_bindings[i].data_length, &var_bindings[i].oid, var_bindings[i].data, var_bindings[i].type);
    }

    // configure packet
    out_data.v1.msg_version = 0;
    strcpy((char *)out_data.v1.community, (const char *)auth);
    out_data.v1.request_type = TRAP_PDU;
    out_data.v1.request_id = rand();
    out_data.v1.error_status = 0;
    out_data.v1.error_index = 0;
    out_data.v1.non_repeaters = 0;
    out_data.v1.max_repetitions = 0;
    out_data.v1.var_bindings = snmp_varbindings;

    // build packet
    return build_snmp_v1(buffer, &out_data);
}

int snmp_build_trap_v2c(unsigned char *buffer, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings)
{
    int i;

    out_data.v1.var_bindings_len = 0;

    for (i = 0; i < num_bindings; i++)
    {
        out_data.v1.var_bindings_len += mib_add_var_binding(snmp_varbindings + out_data.v1.var_bindings_len, var_bindings[i].data_length, &var_bindings[i].oid, var_bindings[i].data, var_bindings[i].type);
    }

    // configure packet
    out_data.v1.msg_version = 1;
    strcpy((char *)out_data.v1.community, (const char *)auth);
    out_data.v1.request_type = TRAPv2C_PDU;
    out_data.v1.request_id = rand();
    out_data.v1.error_status = 0;
    out_data.v1.error_index = 0;
    out_data.v1.non_repeaters = 0;
    out_data.v1.max_repetitions = 0;
    out_data.v1.var_bindings = snmp_varbindings;

    // build packet
    return build_snmp_v2c(buffer, &out_data);
}


int snmp_build_trap_v3(unsigned char *buffer, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings)
{
    int i;
    unsigned int user_index = *(unsigned int *)auth;

    // 验证用户索引
    if (user_index >= MAX_SNMPV3_USERS)
    {
        return 0;
    }

    // 清空变量绑定长度
    out_data.v3.var_bindings_len = 0;

    // 添加变量绑定
    for (i = 0; i < num_bindings; i++)
    {
        out_data.v3.var_bindings_len += mib_add_var_binding(
            snmp_varbindings + out_data.v3.var_bindings_len,
            var_bindings[i].data_length,
            &var_bindings[i].oid,
            var_bindings[i].data,
            var_bindings[i].type);
    }

    // 配置SNMPv3 Trap数据
    out_data.v3.msg_version = 3;
    out_data.v3.msg_id = rand();
    out_data.v3.msg_max_size = SNMP_MAX_LEN;

    // 设置消息标志
    out_data.v3.msg_flags = 0;
    if (snmp_params->users[user_index].auth_type != AUTH_NONE)
    {
        out_data.v3.msg_flags |= MSG_SEC_AUTH; // 需要认证
    }
    if (snmp_params->users[user_index].priv_type != PRIV_NONE)
    {
        out_data.v3.msg_flags |= MSG_SEC_PRIV; // 需要加密
    }

    out_data.v3.msg_security_model = 3; // USM

    // 设置引擎ID
    memcpy(out_data.v3.msg_authoritative_engine_id,
           snmp_params->engine_id,
           snmp_params->engine_id_len);
    out_data.v3.msg_authoritative_engine_id_len = snmp_params->engine_id_len;
    out_data.v3.msg_authoritative_engine_boots = snmp_params->boots;
    out_data.v3.msg_authoritative_engine_time = gtimeCount1s;

    // 设置用户名
    strcpy((char *)out_data.v3.msg_username,
           (const char *)snmp_params->users[user_index].username);

    // 设置PDU类型为SNMPv2 Trap
    out_data.v3.request_type = TRAPv2C_PDU;
    out_data.v3.request_id = rand();
    out_data.v3.error_status = 0;
    out_data.v3.error_index = 0;
    out_data.v3.user = &snmp_params->users[user_index];
    out_data.v3.var_bindings = snmp_varbindings;
    out_data.v3.non_repeaters = 0;
    out_data.v3.max_repetitions = 0;

    // 构建SNMPv3消息
    return build_snmp_v3(buffer, &out_data);
}
