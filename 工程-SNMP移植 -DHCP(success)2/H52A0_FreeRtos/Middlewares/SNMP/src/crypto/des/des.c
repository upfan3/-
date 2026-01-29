/*
 * des.c - Tiny C DES implementation (Public Domain)
 * Suitable for embedding in SNMPv3 or other projects.
 * Source: https://github.com/mmoss/tdes
 */

#include "des.h"
#include <string.h>

#define GETBIT(val, pos) (((val) >> (pos)) & 1)
#define SETBIT(val, pos) ((val) << (pos))
#define ROTL28(x, n) (((x) << (n)) | ((x) >> (28 - (n)))) & 0x0FFFFFFF

// Initial Permutation Table
static const uint8_t IP[64] = {
 58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,
 62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,
 57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,
 61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7
};
// Final Permutation Table
static const uint8_t FP[64] = {
 40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,
 38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,
 36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,
 34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25
};
// Expansion Table
static const uint8_t E[48] = {
 32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,
 12,13,14,15,16,17,16,17,18,19,20,21,20,21,22,23,24,25,
 24,25,26,27,28,29,28,29,30,31,32,1
};
// Permutation Table
static const uint8_t P[32] = {
 16,7,20,21,29,12,28,17,
 1,15,23,26,5,18,31,10,
 2,8,24,14,32,27,3,9,
 19,13,30,6,22,11,4,25
};
// PC1 Table
static const uint8_t PC1[56] = {
 57,49,41,33,25,17,9,1,58,50,42,34,26,18,
 10,2,59,51,43,35,27,19,11,3,60,52,44,36,
 63,55,47,39,31,23,15,7,62,54,46,38,30,22,
 14,6,61,53,45,37,29,21,13,5,28,20,12,4
};
// PC2 Table
static const uint8_t PC2[48] = {
 14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,
 26,8,16,7,27,20,13,2,41,52,31,37,47,55,30,40,
 51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32
};
// Key rotation schedule
static const uint8_t key_shifts[16] = {
 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};
// S-boxes
static const uint8_t S[8][4][16] = {
// S1:
{{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
 {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
 {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
 {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}},
// S2:
{{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
 {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
 {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
 {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}},
// S3:
{{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
 {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
 {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
 {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}},
// S4:
{{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
 {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
 {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
 {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}},
// S5:
{{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
 {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
 {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
 {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}},
// S6:
{{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
 {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
 {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
 {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}},
// S7:
{{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
 {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
 {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
 {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}},
// S8:
{{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
 {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
 {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
 {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}}
};

// Utility: permutation
static void permute(const uint8_t *table, int n, const uint8_t *in, uint8_t *out) {
    int i;
    for (i = 0; i < n; ++i) {
        int pos = table[i] - 1;
        int byte = pos / 8;
        int bit = 7 - (pos % 8);
        if (in[byte] & (1 << bit))
            out[i/8] |= (1 << (7 - (i%8)));
        else
            out[i/8] &= ~(1 << (7 - (i%8)));
    }
}

// Utility: left rotate 28 bits
static void left_rotate_28(uint8_t *in, int shifts) {
    uint32_t val = ((in[0] << 20) | (in[1] << 12) | (in[2] << 4) | (in[3] >> 4)) & 0x0FFFFFFF;
    val = ((val << shifts) | (val >> (28-shifts))) & 0x0FFFFFFF;
    in[0] = (val >> 20) & 0xFF;
    in[1] = (val >> 12) & 0xFF;
    in[2] = (val >> 4) & 0xFF;
    in[3] = (in[3] & 0x0F) | ((val & 0x0F) << 4);
}

// DES key schedule
void des_key_setup(const uint8_t key[8], uint8_t schedule[16][6]) {
    uint8_t pc1[7] = {0};
    uint8_t c[4] = {0}, d[4] = {0};
    int i, j;
    // Permuted Choice 1
    permute(PC1, 56, key, pc1);

    memcpy(c, pc1, 4);
    d[0] = (pc1[3]&0x0F)<<4 | (pc1[4]>>4);
    d[1] = (pc1[4]&0x0F)<<4 | (pc1[5]>>4);
    d[2] = (pc1[5]&0x0F)<<4 | (pc1[6]>>4);
    d[3] = (pc1[6]&0x0F)<<4;

    for (i = 0; i < 16; ++i) {
        // Shift
        for (j = 0; j < key_shifts[i]; ++j) {
            left_rotate_28(c, 1);
            left_rotate_28(d, 1);
        }
        // Combine C and D
        uint8_t cd[7] = {0};
        cd[0] = c[0]; cd[1] = c[1]; cd[2] = c[2]; cd[3] = (c[3]&0xF0)|(d[0]>>4);
        cd[4] = (d[0]<<4)|(d[1]>>4);
        cd[5] = (d[1]<<4)|(d[2]>>4);
        cd[6] = (d[2]<<4)|(d[3]>>4);
        // Permuted Choice 2
        memset(schedule[i], 0, 6);
        permute(PC2, 48, cd, schedule[i]);
    }
}

// Feistel function
static void feistel(const uint8_t in[4], const uint8_t subkey[6], uint8_t out[4]) {
    uint8_t expanded[6] = {0};
    uint8_t sbox_in[8], sbox_out[4] = {0};
    int i;

    permute(E, 48, in, expanded);
    for (i = 0; i < 6; ++i)
        expanded[i] ^= subkey[i];

    // S-box substitution
    for (i = 0; i < 8; ++i) {
        int off = i*6;
        int b = ((expanded[off/8] << (off%8)) & 0xFC) >> 2;
        int row = ((b & 0x20) >> 4) | (b & 0x01);
        int col = (b >> 1) & 0x0F;
        uint8_t val = S[i][row][col];
        if (i%2 == 0)
            sbox_out[i/2] = val << 4;
        else
            sbox_out[i/2] |= val;
    }
    permute(P, 32, sbox_out, out);
}

// DES main block
static void des_block(const uint8_t in[8], uint8_t out[8], uint8_t schedule[16][6], int encrypt) {
    uint8_t block[8], temp[4], l[4], r[4], f[4];
    int i;

    // Initial permutation
    memcpy(block, in, 8);
    uint8_t ip[8] = {0};
    permute(IP, 64, block, ip);

    memcpy(l, ip, 4);
    memcpy(r, ip+4, 4);

    for (i = 0; i < 16; ++i) {
        memcpy(temp, r, 4);
        feistel(r, schedule[encrypt ? i : (15-i)], f);
        for (int j = 0; j < 4; ++j)
            r[j] = l[j] ^ f[j];
        memcpy(l, temp, 4);
    }
    // Preoutput: R||L
    uint8_t preout[8];
    memcpy(preout, r, 4);
    memcpy(preout+4, l, 4);

    // Final permutation
    memset(out, 0, 8);
    permute(FP, 64, preout, out);
}

void des_encrypt(const uint8_t in[8], uint8_t out[8], const uint8_t schedule[16][6]) {
    des_block(in, out, (uint8_t (*)[6])schedule, 1);
}
void des_decrypt(const uint8_t in[8], uint8_t out[8], const uint8_t schedule[16][6]) {
    des_block(in, out, (uint8_t (*)[6])schedule, 0);
}