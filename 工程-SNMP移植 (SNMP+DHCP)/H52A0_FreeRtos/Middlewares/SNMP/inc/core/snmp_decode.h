#ifndef SNMP_DECODE_H
#define SNMP_DECODE_H

// #include "snmp_types.h"
#include "snmp.h"

#ifdef __cplusplus
extern "C"
{
#endif
    // 常量定义
    #define SNMP_V1_VERSION 0
    #define SNMP_V2C_VERSION 1
    #define SNMP_V3_VERSION 3
    #define SECURITY_MODEL_USM 0x03
    #define MAX_PACKET_LENGTH 1500
    
    // 错误码定义
    typedef enum {
        SNMP_SUCCESS = 0,
        SNMP_ERROR_INVALID_PARAM = -1,
        SNMP_ERROR_ASN_PARSE = -2,
        SNMP_ERROR_VERSION = -3,
        SNMP_ERROR_COMMUNITY = -4,
        SNMP_ERROR_SECURITY = -5,
        SNMP_ERROR_BUFFER = -6,
        SNMP_ERROR_DECODE = -7
    } snmp_error_t;

    int snmp_process(unsigned char *packet_in, unsigned int length_in, unsigned char *packet_out);
    int snmp_process_v1(unsigned char *packet_in, unsigned int in_length, unsigned char *packet_out);
    int process_varbindings(unsigned char *in_varbindings, int in_varbindings_len, unsigned char *out_varbindings, int msg_version, int request_type, int non_repeaters, int max_repetitions, int *error_status, int *error_index);
    int snmp_process_v3(unsigned char *packet_in, unsigned int in_length, unsigned char *packet_out);
#ifdef __cplusplus
}
#endif

#endif // SNMP_DECODE_H
