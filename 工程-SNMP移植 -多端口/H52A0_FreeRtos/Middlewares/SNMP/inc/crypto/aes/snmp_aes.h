#ifndef SNMP_AES_H
#define SNMP_AES_H

#include <stdint.h>

// AES-128-CFB 参数定义
#define AES_BLOCK_SIZE 16  // AES 块大小 (128位)
#define AES_KEY_SIZE   16  // AES-128 密钥长度 (128位)

// AES 上下文结构
typedef struct {
    uint8_t round_key[176];  // 密钥扩展后的轮密钥
    uint8_t iv[AES_BLOCK_SIZE];  // CFB 模式的初始化向量
} AES_CTX;

// 接口函数
void aes_init(AES_CTX *ctx, const uint8_t *key, const uint8_t *iv);
void aes_encrypt_cfb(AES_CTX *ctx, const uint8_t *plaintext, uint32_t length, uint8_t *ciphertext);
void aes_encrypt_cfb8(AES_CTX *ctx, const uint8_t *plaintext, uint32_t length, uint8_t *ciphertext);
void aes_decrypt_cfb(AES_CTX *ctx, const uint8_t *ciphertext, uint32_t length, uint8_t *plaintext);

#endif /* SNMP_AES_H */
