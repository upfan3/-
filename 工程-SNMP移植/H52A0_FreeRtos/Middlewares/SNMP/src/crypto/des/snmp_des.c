#include "./crypto/des/snmp_des.h"
#include <string.h>

// DES 初始置换表 IP
static const uint8_t ip_table[64] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9,  1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};

// DES 逆初始置换表 IP^-1
static const uint8_t ip_inv_table[64] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9,  49, 17, 57, 25
};

// DES E 扩展表
static const uint8_t e_table[48] = {
    32, 1,  2,  3,  4,  5,
    4,  5,  6,  7,  8,  9,
    8,  9,  10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32, 1
};

// DES P 置换表
static const uint8_t p_table[32] = {
    16, 7,  20, 21, 29, 12, 28, 17,
    1,  15, 23, 26, 5,  18, 31, 10,
    2,  8,  24, 14, 32, 27, 3,  9,
    19, 13, 30, 6,  22, 11, 4,  25
};

// DES S 盒
static const uint8_t s_boxes[8][64] = {
    {   // S1
        14, 4,  13, 1,  2,  15, 11, 8,  3,  10, 6,  12, 5,  9,  0,  7,
        0,  15, 7,  4,  14, 2,  13, 1,  10, 6,  12, 11, 9,  5,  3,  8,
        4,  1,  14, 8,  13, 6,  2,  11, 15, 12, 9,  7,  3,  10, 5,  0,
        15, 12, 8,  2,  4,  9,  1,  7,  5,  11, 3,  14, 10, 0,  6,  13
    },
    {   // S2
        15, 1,  8,  14, 6,  11, 3,  4,  9,  7,  2,  13, 12, 0,  5,  10,
        3,  13, 4,  7,  15, 2,  8,  14, 12, 0,  1,  10, 6,  9,  11, 5,
        0,  14, 7,  11, 10, 4,  13, 1,  5,  8,  12, 6,  9,  3,  2,  15,
        13, 8,  10, 1,  3,  15, 4,  2,  11, 6,  7,  12, 0,  5,  14, 9
    },
    {   // S3
        10, 0,  9,  14, 6,  3,  15, 5,  1,  13, 12, 7,  11, 4,  2,  8,
        13, 7,  0,  9,  3,  4,  6,  10, 2,  8,  5,  14, 12, 11, 15, 1,
        13, 6,  4,  9,  8,  15, 3,  0,  11, 1,  2,  12, 5,  10, 14, 7,
        1,  10, 13, 0,  6,  9,  8,  7,  4,  15, 14, 3,  11, 5,  2,  12
    },
    {   // S4
        7,  13, 14, 3,  0,  6,  9,  10, 1,  2,  8,  5,  11, 12, 4,  15,
        13, 8,  11, 5,  6,  15, 0,  3,  4,  7,  2,  12, 1,  10, 14, 9,
        10, 6,  9,  0,  12, 11, 7,  13, 15, 1,  3,  14, 5,  2,  8,  4,
        3,  15, 0,  6,  10, 1,  13, 8,  9,  4,  5,  11, 12, 7,  2,  14
    },
    {   // S5
        2,  12, 4,  1,  7,  10, 11, 6,  8,  5,  3,  15, 13, 0,  14, 9,
        14, 11, 2,  12, 4,  7,  13, 1,  5,  0,  15, 10, 3,  9,  8,  6,
        4,  2,  1,  11, 10, 13, 7,  8,  15, 9,  12, 5,  6,  3,  0,  14,
        11, 8,  12, 7,  1,  14, 2,  13, 6,  15, 0,  9,  10, 4,  5,  3
    },
    {   // S6
        12, 1,  10, 15, 9,  2,  6,  8,  0,  13, 3,  4,  14, 7,  5,  11,
        10, 15, 4,  2,  7,  12, 9,  5,  6,  1,  13, 14, 0,  11, 3,  8,
        9,  14, 15, 5,  2,  8,  12, 3,  7,  0,  4,  10, 1,  13, 11, 6,
        4,  3,  2,  12, 9,  5,  15, 10, 11, 14, 1,  7,  6,  0,  8,  13
    },
    {   // S7
        4,  11, 2,  14, 15, 0,  8,  13, 3,  12, 9,  7,  5,  10, 6,  1,
        13, 0,  11, 7,  4,  9,  1,  10, 14, 3,  5,  12, 2,  15, 8,  6,
        1,  4,  11, 13, 12, 3,  7,  14, 10, 15, 6,  8,  0,  5,  9,  2,
        6,  11, 13, 8,  1,  4,  10, 7,  9,  5,  0,  15, 14, 2,  3,  12
    },
    {   // S8
        13, 2,  8,  4,  6,  15, 11, 1,  10, 9,  3,  14, 5,  0,  12, 7,
        1,  15, 13, 8,  10, 3,  7,  4,  12, 5,  6,  11, 0,  14, 9,  2,
        7,  11, 4,  1,  9,  12, 14, 2,  0,  6,  10, 13, 15, 3,  5,  8,
        2,  1,  14, 7,  4,  10, 8,  13, 15, 12, 9,  0,  3,  5,  6,  11
    }
};

// 密钥置换PC1表
static const uint8_t pc1_table[56] = {
    57, 49, 41, 33, 25, 17, 9,  1,
    58, 50, 42, 34, 26, 18, 10, 2,
    59, 51, 43, 35, 27, 19, 11, 3,
    60, 52, 44, 36, 63, 55, 47, 39,
    31, 23, 15, 7,  62, 54, 46, 38,
    30, 22, 14, 6,  61, 53, 45, 37,
    29, 21, 13, 5,  28, 20, 12, 4
};

// 密钥置换PC2表
static const uint8_t pc2_table[48] = {
    14, 17, 11, 24, 1,  5,  3,  28,
    15, 6,  21, 10, 23, 19, 12, 4,
    26, 8,  16, 7,  27, 20, 13, 2,
    41, 52, 31, 37, 47, 55, 30, 40,
    51, 45, 33, 48, 44, 49, 39, 56,
    34, 53, 46, 42, 50, 36, 29, 32
};

static uint64_t permute_bits(uint64_t input, const uint8_t *table, int table_len, int in_bits)
{
    uint64_t output = 0;
    for (int i = 0; i < table_len; ++i) {
        int from = in_bits - table[i];
        uint64_t bit = (input >> from) & 1ULL;
        output = (output << 1) | bit;
    }
    return output;
}

static uint32_t left_rotate_28(uint32_t v, int n)
{
    v &= 0x0FFFFFFF;
    return ((v << n) | (v >> (28 - n))) & 0x0FFFFFFF;
}

static void des_key_schedule(const uint8_t key[8], uint64_t subkeys[16])
{
    uint64_t k = 0;
    for (int i = 0; i < 8; ++i) {
        k = (k << 8) | key[i];
    }
    uint64_t perm56 = permute_bits(k, pc1_table, 56, 64);
    uint32_t C = (uint32_t)((perm56 >> 28) & 0x0FFFFFFF);
    uint32_t D = (uint32_t)(perm56 & 0x0FFFFFFF);

    for (int round = 0; round < 16; ++round) {
        int rot = (round < 2 || round == 8 || round == 15) ? 1 : 2;
        C = left_rotate_28(C, rot);
        D = left_rotate_28(D, rot);
        uint64_t cd = (((uint64_t)C) << 28) | (uint64_t)D;
        subkeys[round] = permute_bits(cd, pc2_table, 48, 56);
    }
}

static uint32_t feistel(uint32_t r, uint64_t subkey48)
{
    uint64_t ER = 0;
    for (int i = 0; i < 48; ++i) {
        int from = 32 - e_table[i];
        uint64_t bit = (uint64_t)((r >> from) & 1U);
        ER = (ER << 1) | bit;
    }
    ER ^= subkey48;
    uint32_t out32 = 0;
    for (int box = 0; box < 8; ++box) {
        uint8_t six = (uint8_t)((ER >> (42 - 6 * box)) & 0x3F);
        int row = ((six & 0x20) >> 4) | (six & 0x01);
        int col = (six >> 1) & 0x0F;
        uint8_t s = s_boxes[box][row * 16 + col];
        out32 = (out32 << 4) | s;
    }
    uint32_t p_out = 0;
    for (int i = 0; i < 32; ++i) {
        int from = 32 - p_table[i];
        uint32_t bit = (out32 >> from) & 1U;
        p_out = (p_out << 1) | bit;
    }
    return p_out;
}

static void des_encrypt_block(const uint8_t in[8], uint8_t out[8], const uint64_t subkeys[16])
{
    uint64_t m = 0;
    for (int i = 0; i < 8; ++i) m = (m << 8) | in[i];
    uint64_t ip = permute_bits(m, ip_table, 64, 64);
    uint32_t L = (uint32_t)(ip >> 32);
    uint32_t R = (uint32_t)(ip & 0xFFFFFFFFU);

    for (int round = 0; round < 16; ++round) {
        uint32_t prevR = R;
        R = L ^ feistel(R, subkeys[round]);
        L = prevR;
    }
    uint64_t pre = (((uint64_t)R) << 32) | (uint64_t)L;
    uint64_t c = permute_bits(pre, ip_inv_table, 64, 64);
    for (int i = 7; i >= 0; --i) {
        out[i] = (uint8_t)(c & 0xFF);
        c >>= 8;
    }
}

static void des_decrypt_block(const uint8_t in[8], uint8_t out[8], const uint64_t subkeys[16])
{
    uint64_t rev[16];
    for (int i = 0; i < 16; ++i) rev[i] = subkeys[15 - i];
    des_encrypt_block(in, out, rev);
}

// 初始化DES上下文
void des_init(DES_CTX *ctx, const uint8_t *key, const uint8_t *iv)
{
    des_key_schedule(key, ctx->subkeys);
    if (iv) memcpy(ctx->iv, iv, DES_BLOCK_SIZE);
    else memset(ctx->iv, 0, DES_BLOCK_SIZE);
}

// CBC模式加密
void des_encrypt_cbc(DES_CTX *ctx, const uint8_t *plaintext, uint32_t length, uint8_t *ciphertext)
{
    uint8_t block[8];
    uint8_t prev[8];
    memcpy(prev, ctx->iv, 8);

    for (uint32_t i = 0; i < length; i += 8) {
        for (int j = 0; j < 8; ++j)
            block[j] = plaintext[i + j] ^ prev[j];
        des_encrypt_block(block, &ciphertext[i], ctx->subkeys);
        memcpy(prev, &ciphertext[i], 8);
    }
}

// CBC模式解密
// void des_decrypt_cbc(DES_CTX *ctx, const uint8_t *ciphertext, uint32_t length, uint8_t *plaintext)
// {
//     uint8_t block[8];
//     uint8_t prev[8];
//     memcpy(prev, ctx->iv, 8);

//     for (uint32_t i = 0; i < length; i += 8) {
//         des_decrypt_block(&ciphertext[i], block, ctx->subkeys);
//         for (int j = 0; j < 8; ++j)
//             plaintext[i + j] = block[j] ^ prev[j];
//         memcpy(prev, &ciphertext[i], 8);
//     }
// }

void des_decrypt_cbc(DES_CTX *ctx, const uint8_t *ciphertext, uint32_t length, uint8_t *plaintext) {
    uint8_t block[DES_BLOCK_SIZE];
    uint8_t temp[DES_BLOCK_SIZE];
    
    for (uint32_t i = 0; i < length; i += DES_BLOCK_SIZE) {
        // 保存当前块用于更新IV
        memcpy(temp, ciphertext + i, DES_BLOCK_SIZE);
        
        // DES解密
        des_decrypt_block(temp, block, ctx->subkeys);
        
        // XOR与IV
        for (int j = 0; j < DES_BLOCK_SIZE; j++) {
            block[j] ^= ctx->iv[j];
        }
        
        // 保存明文
        memcpy(plaintext + i, block, DES_BLOCK_SIZE);
        
        // 更新IV为原始密文块
        memcpy(ctx->iv, temp, DES_BLOCK_SIZE);
    }
}
