#ifndef HMAC_MD5_H
#define HMAC_MD5_H

#include <stdint.h>
#include <stddef.h>
#include "./crypto/md5/md5.h"

#define HMAC_MD5_BLOCK_SIZE 64
#define HMAC_MD5_OUTPUT_SIZE 16

/**
 * @brief 计算 HMAC-MD5
 * 
 * @param key 密钥
 * @param key_len 密钥长度
 * @param data 数据
 * @param data_len 数据长度
 * @param out 输出缓冲区(必须至少 16 字节)
 */
void hmac_md5(const uint8_t *key, size_t key_len,
              const uint8_t *data, size_t data_len,
              uint8_t out[HMAC_MD5_OUTPUT_SIZE]);

#endif /* HMAC_MD5_H */
