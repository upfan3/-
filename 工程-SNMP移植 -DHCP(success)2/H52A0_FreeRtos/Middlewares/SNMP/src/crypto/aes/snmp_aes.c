#include "./crypto/aes/snmp_aes.h"
#include <string.h>

// AES S-box
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// 轮常量
static const uint8_t Rcon[11] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

// AES 密钥扩展
static void aes_key_expansion(const uint8_t *key, uint8_t *round_key) {
    uint32_t i, j;
    uint8_t temp[4];

    // 第一轮密钥就是输入密钥
    memcpy(round_key, key, AES_KEY_SIZE);

    // 生成剩余的轮密钥
    for(i = 4; i < 44; i++) {
        for(j = 0; j < 4; j++) {
            temp[j] = round_key[(i-1) * 4 + j];
        }
        
        if(i % 4 == 0) {
            // 字循环
            uint8_t k = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = k;
            
            // S-box 替换
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
            
            // 轮常量异或
            temp[0] ^= Rcon[i/4];
        }
        
        for(j = 0; j < 4; j++) {
            round_key[i * 4 + j] = round_key[(i-4) * 4 + j] ^ temp[j];
        }
    }
}

// 单块加密
static void aes_encrypt_block(const uint8_t *round_key, uint8_t *block) {
    uint8_t state[4][4];
    uint32_t i, j;

    // 将输入数据转换为状态矩阵
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            state[j][i] = block[i * 4 + j];
        }
    }

    // 轮密钥加
    for(i = 0; i < 16; i++) {
        state[i % 4][i / 4] ^= round_key[i];
    }

    // 9轮变换
    uint32_t round;
    for(round = 1; round < 10; round++) {
        // SubBytes
        for(i = 0; i < 4; i++) {
            for(j = 0; j < 4; j++) {
                state[i][j] = sbox[state[i][j]];
            }
        }

        // ShiftRows
        uint8_t temp;
        temp = state[1][0];
        state[1][0] = state[1][1];
        state[1][1] = state[1][2];
        state[1][2] = state[1][3];
        state[1][3] = temp;

        temp = state[2][0];
        state[2][0] = state[2][2];
        state[2][2] = temp;
        temp = state[2][1];
        state[2][1] = state[2][3];
        state[2][3] = temp;

        temp = state[3][3];
        state[3][3] = state[3][2];
        state[3][2] = state[3][1];
        state[3][1] = state[3][0];
        state[3][0] = temp;

        // MixColumns
        uint8_t t, tmp, tm;
        for(i = 0; i < 4; i++) {
            t   = state[0][i];
            tmp = state[0][i] ^ state[1][i] ^ state[2][i] ^ state[3][i];
            
            tm  = state[0][i] ^ state[1][i];
            tm = (tm << 1) ^ (tm >> 7 ? 0x1b : 0);
            state[0][i] ^= tm ^ tmp;
            
            tm  = state[1][i] ^ state[2][i];
            tm = (tm << 1) ^ (tm >> 7 ? 0x1b : 0);
            state[1][i] ^= tm ^ tmp;
            
            tm  = state[2][i] ^ state[3][i];
            tm = (tm << 1) ^ (tm >> 7 ? 0x1b : 0);
            state[2][i] ^= tm ^ tmp;
            
            tm  = state[3][i] ^ t;
            tm = (tm << 1) ^ (tm >> 7 ? 0x1b : 0);
            state[3][i] ^= tm ^ tmp;
        }

        // AddRoundKey
        for(i = 0; i < 4; i++) {
            for(j = 0; j < 4; j++) {
                state[j][i] ^= round_key[round * 16 + i * 4 + j];
            }
        }
    }

    // 最后一轮
    // SubBytes
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            state[i][j] = sbox[state[i][j]];
        }
    }

    // ShiftRows
    uint8_t temp;
    temp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = temp;

    temp = state[2][0];
    state[2][0] = state[2][2];
    state[2][2] = temp;
    temp = state[2][1];
    state[2][1] = state[2][3];
    state[2][3] = temp;

    temp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = temp;

    // AddRoundKey
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            state[j][i] ^= round_key[160 + i * 4 + j];
        }
    }

    // 将状态矩阵转回输出数据
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            block[i * 4 + j] = state[j][i];
        }
    }
}

// 初始化 AES 上下文
void aes_init(AES_CTX *ctx, const uint8_t *key, const uint8_t *iv) {
    aes_key_expansion(key, ctx->round_key);
    if(iv) {
        memcpy(ctx->iv, iv, AES_BLOCK_SIZE);
    } else {
        memset(ctx->iv, 0, AES_BLOCK_SIZE);
    }
}

// 修改CFB函数实现（分组处理）
// void aes_encrypt_cfb(AES_CTX *ctx, const uint8_t *plaintext, uint32_t length, uint8_t *ciphertext) {
//     uint8_t buffer[AES_BLOCK_SIZE];
    
//     // 验证数据长度符合分组要求
//     if(length % AES_BLOCK_SIZE != 0) {
//         return; // 或处理错误（SNMPv3要求16字节对齐）
//     }

//     while(length >= AES_BLOCK_SIZE) {
//         // 1. 加密当前IV
//         memcpy(buffer, ctx->iv, AES_BLOCK_SIZE);
//         aes_encrypt_block(ctx->round_key, buffer);
        
//         // 2. 异或完整分组（CFB-128核心）
//         for(int i=0; i<AES_BLOCK_SIZE; i++) {
//             ciphertext[i] = plaintext[i] ^ buffer[i];
//         }
        
//         // 3. 用新密文更新IV（非移位！）
//         memcpy(ctx->iv, ciphertext, AES_BLOCK_SIZE); // 关键修复
        
//         // 4. 移动指针
//         plaintext += AES_BLOCK_SIZE;
//         ciphertext += AES_BLOCK_SIZE;
//         length -= AES_BLOCK_SIZE;
//     }
// }

// 支持任意长度数据的 CFB 加密
void aes_encrypt_cfb(AES_CTX *ctx, const uint8_t *plaintext, 
                     uint32_t length, uint8_t *ciphertext) 
{
    uint8_t block[AES_BLOCK_SIZE];
    
    while (length > 0) {
        // 1. 加密当前 IV 生成密钥流
        memcpy(block, ctx->iv, AES_BLOCK_SIZE);
        aes_encrypt_block(ctx->round_key, block);
        
        // 2. 计算本次处理的字节数
        uint32_t bytes_to_process = (length > AES_BLOCK_SIZE) ? 
                                   AES_BLOCK_SIZE : length;
        
        // 3. 逐字节异或加密
        for (int i = 0; i < bytes_to_process; i++) {
            ciphertext[i] = plaintext[i] ^ block[i];
        }
        
        // 4. 更新 IV (CFB 模式核心)
        // 左移 IV 并添加新密文字节
        memmove(ctx->iv, ctx->iv + bytes_to_process, 
               AES_BLOCK_SIZE - bytes_to_process);
        memcpy(ctx->iv + AES_BLOCK_SIZE - bytes_to_process, 
              ciphertext, bytes_to_process);
        
        // 5. 移动指针
        plaintext += bytes_to_process;
        ciphertext += bytes_to_process;
        length -= bytes_to_process;
    }
}

void aes_encrypt_cfb8(AES_CTX *ctx, const uint8_t *plaintext, uint32_t length, uint8_t *ciphertext) {
    uint8_t iv[AES_BLOCK_SIZE];
    uint8_t block[AES_BLOCK_SIZE];
    
    memcpy(iv, ctx->iv, AES_BLOCK_SIZE);
    
    for(uint32_t i = 0; i < length; i++) {
        // 1. 加密当前 IV
        memcpy(block, iv, AES_BLOCK_SIZE);
        aes_encrypt_block(ctx->round_key, block);
        
        // 2. 加密单个字节 (CFB-8)
        ciphertext[i] = plaintext[i] ^ block[0];
        
        // 3. 更新 IV：左移并添加新密文字节
        memmove(iv, iv + 1, AES_BLOCK_SIZE - 1); // 左移
        iv[AES_BLOCK_SIZE - 1] = ciphertext[i];
    }
    
    // 更新上下文 IV
    memcpy(ctx->iv, iv, AES_BLOCK_SIZE);
}

// 解密函数同理（注意IV更新使用密文）
void aes_decrypt_cfb(AES_CTX *ctx, const uint8_t *ciphertext, uint32_t length, uint8_t *plaintext) {
    uint8_t buffer[AES_BLOCK_SIZE];
    
    while(length >= AES_BLOCK_SIZE) {
        memcpy(buffer, ctx->iv, AES_BLOCK_SIZE);
        aes_encrypt_block(ctx->round_key, buffer);
        
        // 先保存原始密文（用于更新IV）
        uint8_t cipher_block[AES_BLOCK_SIZE];
        memcpy(cipher_block, ciphertext, AES_BLOCK_SIZE);
        
        // 解密当前分组
        for(int i=0; i<AES_BLOCK_SIZE; i++) {
            plaintext[i] = ciphertext[i] ^ buffer[i];
        }
        
        // 用原始密文更新IV
        memcpy(ctx->iv, cipher_block, AES_BLOCK_SIZE); // 关键
        
        plaintext += AES_BLOCK_SIZE;
        ciphertext += AES_BLOCK_SIZE;
        length -= AES_BLOCK_SIZE;
    }
        /* 处理剩余 <16 字节（CFB 流模式） */
    if (length > 0) {
        memcpy(buffer, ctx->iv, AES_BLOCK_SIZE);
        aes_encrypt_block(ctx->round_key, buffer);

        for (uint32_t i = 0; i < length; ++i)
            plaintext[i] = ciphertext[i] ^ buffer[i];

        /* 更新 IV：把实际用到的密文部分拼到 IV 头部 */
        memmove(ctx->iv, ctx->iv + length, AES_BLOCK_SIZE - length);
        memcpy(ctx->iv + AES_BLOCK_SIZE - length, ciphertext, length);
    }
}
