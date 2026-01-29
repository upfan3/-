#ifndef SNMP_H
#define SNMP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../BSP/NET/types.h"
#include "../asn1/asn.1.h"
#include "../crypto/md5/md5.h"
#include "../crypto/des/des.h"
#include "../crypto/aes/snmp_aes.h"
#include "../crypto/des/snmp_des.h"
#include "../crypto/sha1/snmp_sha1.h"
#include "../crypto/sha1/sha256.h"
#include "../crypto/hmac/hmac_md5.h"
#define SHA_1

#define SNMP_PORT 161
#define SNMP_MAX_LEN 1000
#define SNMP_TRAP_PORT 162
#define SOCK_SNMP_V3 6  //3
#define SOCK_SNMP_TRAP 7 //4
#define SNMPV3_BUF_MAX 500
    typedef struct
    {
        _oid oid;
        unsigned int data_length;
        void *data;
        int type;
    } _trap_varbinding;

    typedef int (*snmp_cb)(_oid *oid, int type, void *val, int val_len);

    /******* SNMP CONFIGURATION **************/

#define AUTH_NONE 0
#define AUTH_MD5 1
#define AUTH_SHA 2

#define PRIV_NONE 0
#define PRIV_DES 1
#define PRIV_AES 2

#define NUM_SNMP_BUFFERS 5

#define MAX_COMMUNITY_NAME 20
#define MAX_ENGINE_ID 20
#define MAX_SNMPV3_USERS 6
#define MAX_USERNAME_LEN 20
#define MAX_PASSWORD_LEN 20

#define USER_LEVEL_NONE 0
#define USER_LEVEL_READ 1
#define USER_LEVEL_READ_WRITE 2

#define SNMP_LAN_ENABLED 0x01
#define SNMP_WAN_ENABLED 0x02

// SNMP v3 Security defines
#define MSG_SEC_AUTH 0x01
#define MSG_SEC_PRIV 0x02
#define MSG_SEC_REP 0x04

// v3 error OID stuff
// ERROR OID values for snmp v3
#define RPT_ERR_NONE 0
#define RPT_ERR_SEC_LVL 1
#define RPT_ERR_TIME_WIN 2
#define RPT_ERR_SEC_NAME 3
#define RPT_ERR_ENG_ID 4
#define RPT_ERR_AUTH_FAIL 5
#define RPT_ERR_DEC_ERR 6
#define RPT_ERR_LAST_ONE 7

#define RPT_ERR_INDEX 9

    // niver niver niver, oose de 1st ooser
    typedef struct
    {
        char username[MAX_PASSWORD_LEN];
        char auth_pass[MAX_PASSWORD_LEN];
        char priv_pass[MAX_PASSWORD_LEN];
        unsigned char auth_type;
        unsigned char priv_type;
        unsigned char auth_key[20];
        unsigned char priv_key[20];
        unsigned char user_level;
    } _snmpv3_user;

#define MAX_TRAP_TARGETS 5

    typedef struct
    {
        unsigned char target_ip[4]; // 目标IP地址
        unsigned short target_port; // 目标端口 (默认162)
        unsigned char enabled;      // 是否启用
        unsigned int user_index;    // 使用的用户索引
    } _trap_target;

    typedef struct
    {
        unsigned char enabled;
        unsigned char OLD1;
        unsigned char use_v3;
        char read_community[MAX_COMMUNITY_NAME];
        char write_community[MAX_COMMUNITY_NAME];
        unsigned char engine_id[MAX_ENGINE_ID];
        unsigned int engine_id_len;
        unsigned int boots;
        unsigned char traps_enabled;
        // 删除单一的trap_ip字段
        // unsigned char trap_ip[4];
        unsigned char current_user_index;

        // 添加Trap目标列表
        _trap_target trap_targets[MAX_TRAP_TARGETS];
        _snmpv3_user users[MAX_SNMPV3_USERS];
    } _snmp_params;

#define MAX_MSG_AUTH_PARAM 20
#define MAX_MSG_PRIV_PARAM 20
#define OCTET_STRING_VB_LEN 100

    typedef union
    {
        struct
        {
            // AUTH data structures
            // Hmac hmac;
            unsigned char password_buf[72];
            union
            {
                md5_ctx md5;
#ifdef SHA_1
                SHA1_CTX sha;
#else
            SHA256_CTX sha256;
#endif
            } t;

            unsigned char digest[20];
        } auth;

        struct
        {
            // PRIV data structures
            union
            {
                // Des des;
                AES_CTX aes;
                // struct AES_ctx aes;
            } t;
            unsigned char key[16];
            unsigned char iv[16];
        } priv;
    } _auth_priv_str;

    typedef struct
    {
        unsigned int msg_version;
        unsigned char community[MAX_COMMUNITY_NAME];
        _oid enterprise;
        unsigned int generic_trap;
        unsigned int specific_trap;
        unsigned int request_type;
        unsigned int request_id;
        unsigned int error_status;
        unsigned int error_index;
        unsigned int non_repeaters;
        unsigned int max_repetitions;
        unsigned char *var_bindings;
        unsigned int var_bindings_len;
    } _v1_struct;

    typedef struct
    {
        unsigned int msg_version;
        unsigned int msg_id;
        unsigned int msg_max_size;
        unsigned int msg_flags;
        unsigned int msg_security_model;
        unsigned char msg_authoritative_engine_id[MAX_ENGINE_ID];
        unsigned int msg_authoritative_engine_id_len;
        unsigned int msg_authoritative_engine_boots;
        unsigned int msg_authoritative_engine_time;
        unsigned char msg_username[MAX_USERNAME_LEN];
        unsigned char msg_authentication_parameters[MAX_MSG_AUTH_PARAM];
        unsigned char msg_privacy_parameters[MAX_MSG_PRIV_PARAM];
        unsigned int request_type;
        unsigned int request_id;
        unsigned int error_status;
        unsigned int error_index;
        unsigned int non_repeaters;
        unsigned int max_repetitions;
        unsigned char *var_bindings;
        unsigned int var_bindings_len;
        unsigned char lkey[20];
        _snmpv3_user *user;
    } _v3_struct;

    typedef union
    {
        unsigned int msg_version;
        _v1_struct v1;
        _v3_struct v3;
    } _snmp_data;

    typedef struct
    {
        int wait_report;
        int wait_reply;
        int user_index;
        snmp_cb request_cb;
        char community[MAX_COMMUNITY_NAME];
        _oid oid;
    } _cb_struct;

    typedef union
    {
        _oid oid;
        char octet_string[OCTET_STRING_VB_LEN];
        unsigned int intv;
    } _varbinding_val;

    typedef struct
    {
        _oid oid;
        _varbinding_val val;
    } _varbinding_data;

    typedef struct
    {
        _snmp_data in_data;
        _snmp_data out_data;
        _cb_struct cb_struct;
        _varbinding_data vb_data;
        _auth_priv_str ap;
        unsigned char snmp_buffer[SNMP_MAX_LEN];
        unsigned char snmp_varbindings[SNMP_MAX_LEN];

    } _snmp_buffer;

    // SNMP统计结构体定义
    typedef struct
    {
        struct
        {
            int packets;       // 接收的数据包数
            int tooBigs;       // 数据包过大数
            int noSuchNames;   // 无此名称的请求数
            int badValues;     // 错误值的请求数
            int readOnlys;     // 只读请求数
            int generalErrors; // 一般错误数
            int getRequests;   // GET请求数
            int setRequests;   // SET请求数
            int getNexts;      // GET NEXT请求数
            int getResponses;  // GET RESPONSE请求数
            int traps;         // TRAP请求数
        } in;
        struct
        {
            int packets;       // 发送的数据包数
            int noSuchNames;   // 无此名称的请求数
            int tooBigs;       // 数据包过大数
            int badValues;     // 错误值的请求数
            int generalErrors; // 一般错误数
            int getRequests;   // GET请求数
            int setRequests;   // SET请求数
            int getNexts;      // GET NEXT请求数
            int getResponses;  // GET RESPONSE请求数
            int traps;         // TRAP请求数
        } out;
        int badVersions;       // 错误版本数
        int asnParseErrs;      // ASN.1解析错误数
        int badCommunityNames; // 错误社区名称数
        int badCommunityUses;  // 错误社区使用数
        int silentDrops;       // 静默丢弃数
        int proxyDrops;        // 代理丢弃数
    } snmp_stats_t;

    // 添加全局变量extern声明
    extern _snmp_params *snmp_params;
    extern snmp_stats_t *snmp_stats; // SNMP统计指针

    extern u32 gtimeCount1s;
    extern _snmp_data in_data; 
    extern _snmp_data out_data;
    extern _varbinding_data vb_data;
    extern _cb_struct cb_struct;
    extern const unsigned int rpt_err_oid[11];
    extern unsigned char snmp_buffer[512];     
    extern unsigned char snmp_varbindings[512];

    // static function
    // function prototypes
    int snmp_v3_error_oid_handler(void **data, int *data_len, _oid *oid, int request_type, int data_type, void *data_in, int data_in_len, int *error);

    // ONLY Call snmp_init OR snmp_setup since snmp_init calls snmp_setup
    // snmp_init  -- initializes UDP handler
    // snmp_setup -- sets up internal system

    void snmp_init(_snmp_params *snmp_params);
    void snmp_setup(_snmp_params *snmp_params);

    // int snmp_send_trap(int version, ip_addr_t *ipaddr, u16_t port, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings);
    int snmp_send_trap(int version, unsigned char *ip_addr, unsigned short port, void *auth, _trap_varbinding *var_bindings, unsigned int num_bindings);
// for version 1 and 2c, void *auth points to the community name
// for version 2, void *auth points to an integer representing the user_index
// int snmp_start_request(ip_addr_t *ipaddr, u16_t port, _oid *oid, int version, snmp_cb cb, void *auth);
#ifdef __cplusplus
}
#endif

#endif
/***   End Of File   ***/
