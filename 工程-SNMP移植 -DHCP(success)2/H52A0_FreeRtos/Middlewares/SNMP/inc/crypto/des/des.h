/*
 * des.h - Tiny C DES implementation header (Public Domain)
 * Suitable for embedding in SNMPv3 or other projects.
 * Source: https://github.com/mmoss/tdes
 */

#ifndef DES_H
#define DES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DES key schedule: generates 16 subkeys for the given 8-byte key.
 * @param key      8-byte input key (only 56 bits used)
 * @param schedule Output: 16 rounds x 6 bytes each = 96 bytes
 */
void des_key_setup(const uint8_t key[8], uint8_t schedule[16][6]);

/**
 * Encrypt a single 8-byte block using the given key schedule.
 * @param in       8-byte input plaintext
 * @param out      8-byte output ciphertext
 * @param schedule Subkeys from des_key_setup
 */
void des_encrypt(const uint8_t in[8], uint8_t out[8], const uint8_t schedule[16][6]);

/**
 * Decrypt a single 8-byte block using the given key schedule.
 * @param in       8-byte input ciphertext
 * @param out      8-byte output plaintext
 * @param schedule Subkeys from des_key_setup
 */
void des_decrypt(const uint8_t in[8], uint8_t out[8], const uint8_t schedule[16][6]);

#ifdef __cplusplus
}
#endif

#endif // DES_H
