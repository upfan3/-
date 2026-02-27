#ifndef SNMP_CRYPTO_H
#define SNMP_CRYPTO_H

// #include "snmp_types.h"
#include "snmp.h"

// 加密和认证函数声明
void snmp_init_keys(void);
void calculate_hmac(int auth_type, unsigned char *auth_key, unsigned char *data, unsigned int data_len, unsigned char *to);
void encrypt_pdu(int priv_type, unsigned char *priv_key, unsigned char *data, unsigned int data_len, unsigned char *salt, unsigned int engine_boots, unsigned int engine_time);
void decrypt_pdu(int priv_type, unsigned char *priv_key, unsigned char *data, unsigned int data_len, unsigned char *salt, unsigned int engine_boots, unsigned int engine_time);
void localize_key(unsigned int auth_type, unsigned char *key, unsigned char *lkey, unsigned char *engine_id, unsigned int engine_id_len);
void password_to_key(char *password, unsigned char *key, unsigned int auth_type);

#endif // SNMP_CRYPTO_H
