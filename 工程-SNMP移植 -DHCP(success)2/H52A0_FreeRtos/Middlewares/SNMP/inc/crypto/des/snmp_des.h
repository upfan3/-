#ifndef SNMP_DES_H
#define SNMP_DES_H

#include <stdint.h>

#define DES_BLOCK_SIZE 8
#define DES_KEY_SIZE   8

typedef struct {
    uint64_t subkeys[16];      // 16轮子密钥，每轮48位，低位有效
    uint8_t iv[DES_BLOCK_SIZE];
} DES_CTX;

void des_init(DES_CTX *ctx, const uint8_t *key, const uint8_t *iv);
void des_encrypt_cbc(DES_CTX *ctx, const uint8_t *plaintext, uint32_t length, uint8_t *ciphertext);
void des_decrypt_cbc(DES_CTX *ctx, const uint8_t *ciphertext, uint32_t length, uint8_t *plaintext);

#endif /* SNMP_DES_H */
