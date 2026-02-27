#ifndef SNMP_SHA1_H
#define SNMP_SHA1_H

#include <stdint.h>
#include <stddef.h>

// SHA-1 定义
#define SHA1_BLOCK_SIZE  64   // SHA-1 的块大小 (512位)
#define SHA1_DIGEST_SIZE 20   // SHA-1 的摘要大小 (160位)

// SHA-1 上下文结构
typedef struct {
    uint32_t state[5];    // 当前哈希值
    uint64_t count;       // 已处理的位数
    uint8_t  buffer[64];  // 输入缓冲区
} SHA1_CTX;

// SHA-1 基本函数
void sha1_init(SHA1_CTX *ctx);
void sha1_update(SHA1_CTX *ctx, const uint8_t *data, size_t len);
void sha1_final(uint8_t *digest, SHA1_CTX *ctx);

// HMAC-SHA1 函数 (SNMPv3 认证所需)
void hmac_sha1(const uint8_t *key, size_t key_len,
               const uint8_t *data, size_t data_len,
               uint8_t *digest);

#endif /* SNMP_SHA1_H */
