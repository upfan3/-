#include "./crypto/hmac/hmac_md5.h"
#include <string.h>

// HMAC 内部使用的填充常量
#define IPAD 0x36
#define OPAD 0x5C

void hmac_md5(const uint8_t *key, size_t key_len,
              const uint8_t *data, size_t data_len,
              uint8_t out[HMAC_MD5_OUTPUT_SIZE])
{
    md5_ctx context;
    uint8_t k_ipad[HMAC_MD5_BLOCK_SIZE];    // 内部填充
    uint8_t k_opad[HMAC_MD5_BLOCK_SIZE];    // 外部填充
    uint8_t tk[HMAC_MD5_OUTPUT_SIZE];       // 临时密钥
    size_t i;

    // 如果密钥长度大于区块大小,先对密钥进行哈希
    if (key_len > HMAC_MD5_BLOCK_SIZE) {
        md5_ctx tctx;
        md5_init(&tctx);
        md5_update(&tctx, (uint8_t *)key, key_len);
        md5_final(tk, &tctx);
        key = tk;
        key_len = HMAC_MD5_OUTPUT_SIZE;
    }

    // 复制密钥到内部/外部填充缓冲区
    memset(k_ipad, 0, HMAC_MD5_BLOCK_SIZE);
    memset(k_opad, 0, HMAC_MD5_BLOCK_SIZE);
    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);

    // 对填充缓冲区进行XOR运算
    for (i = 0; i < HMAC_MD5_BLOCK_SIZE; i++) {
        k_ipad[i] ^= IPAD;
        k_opad[i] ^= OPAD;
    }

    // 内部哈希
    md5_init(&context);
    md5_update(&context, k_ipad, HMAC_MD5_BLOCK_SIZE);
    md5_update(&context, (uint8_t *)data, data_len);
    md5_final(out, &context);

    // 外部哈希
    md5_init(&context);
    md5_update(&context, k_opad, HMAC_MD5_BLOCK_SIZE);
    md5_update(&context, out, HMAC_MD5_OUTPUT_SIZE);
    md5_final(out, &context);
}
