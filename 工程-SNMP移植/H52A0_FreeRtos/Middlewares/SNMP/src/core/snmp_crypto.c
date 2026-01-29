// SNMP加密处理实现
// 包含HMAC计算、PDU加解密、密钥本地化等安全功能
#include <string.h>
#include <stdlib.h>
#include "./core/snmp_crypto.h"
#include "./asn1/asn.1.h"
#include "./crypto/md5/md5.h"
#include "./crypto/sha1/snmp_sha1.h"
#include "./crypto/sha1/sha256.h"
#include "./crypto/hmac/hmac_md5.h"
#include "./crypto/des/snmp_des.h"
#include "./crypto/aes/snmp_aes.h"

// 静态变量声明
static _auth_priv_str ap;

// 计算HMAC用于v3消息认证
void calculate_hmac(int auth_type, unsigned char *auth_key,
                    unsigned char *data, unsigned int data_len,
                    unsigned char *to)
{
    if (auth_type == AUTH_MD5)
    {
        uint8_t digest[HMAC_MD5_OUTPUT_SIZE];
        hmac_md5(auth_key, 16,
                 data, data_len,
                 digest);
        memcpy(to, digest, 12);
    }
    else if (auth_type == AUTH_SHA)
    {
#ifdef SHA_1
        uint8_t digest[SHA1_DIGEST_SIZE];
        hmac_sha1(auth_key, 20,
                  data, data_len,
                  digest);
        memcpy(to, digest, 12);
#else
        uint8_t digest[SHA256_DIGEST_SIZE];
        hmac_sha256(auth_key, 32,
                    data, data_len,
                    digest);
        memcpy(to, digest, 12);
#endif
    }
    else
    {
        memset(to, 0, 12);
    }
}

// 解密PDU
void decrypt_pdu(int priv_type, unsigned char *priv_key, unsigned char *data, unsigned int data_len, unsigned char *salt, unsigned int engine_boots, unsigned int engine_time)
{
    int i;

    if (priv_type == PRIV_DES)
    {
        uint8_t key[8], iv[8];

        memcpy(key, priv_key, 8);

        for (i = 0; i < 8; i++)
        {
            iv[i] = priv_key[8 + i] ^ salt[i];
        }

        DES_CTX des_ctx;
        des_init(&des_ctx, key, iv);

        des_decrypt_cbc(&des_ctx, data, data_len, data);
    }
    else if (priv_type == PRIV_AES)
    {
        AES_CTX aes_ctx;

        ap.priv.iv[0] = ((char *)&engine_boots)[3];
        ap.priv.iv[1] = ((char *)&engine_boots)[2];
        ap.priv.iv[2] = ((char *)&engine_boots)[1];
        ap.priv.iv[3] = ((char *)&engine_boots)[0];
        ap.priv.iv[4] = ((char *)&engine_time)[3];
        ap.priv.iv[5] = ((char *)&engine_time)[2];
        ap.priv.iv[6] = ((char *)&engine_time)[1];
        ap.priv.iv[7] = ((char *)&engine_time)[0];
        memcpy(ap.priv.iv + 8, salt, 8);

        memcpy(ap.priv.key, priv_key, 16);

        aes_init(&aes_ctx, ap.priv.key, ap.priv.iv);
        aes_decrypt_cfb(&aes_ctx, data, data_len, data);
    }
}

// 加密PDU
void encrypt_pdu(int priv_type, unsigned char *priv_key, unsigned char *data, unsigned int data_len, unsigned char *salt, unsigned int engine_boots, unsigned int engine_time)
{
    int i;

    if (priv_type == PRIV_DES)
    {
        if (salt)
        {
            ((uint32_t *)salt)[0] = engine_boots;
            ((uint32_t *)salt)[1] = rand();
        }
        else
        {
            return;
        }

        uint8_t key[8], iv[8];
        memcpy(key, priv_key, 8);
        for (i = 0; i < 8; i++)
        {
            iv[i] = priv_key[8 + i] ^ salt[i];
        }

        DES_CTX des_ctx;
        des_init(&des_ctx, key, iv);

        des_encrypt_cbc(&des_ctx, data, data_len, data);
    }
    else if (priv_type == PRIV_AES)
    {
        if (salt && sizeof(unsigned int) * 2 <= data_len)
        {
            ((unsigned int *)salt)[0] = rand();
            ((unsigned int *)salt)[1] = rand();
        }
        else
        {
            return;
        }

        ap.priv.iv[0] = ((char *)&engine_boots)[3];
        ap.priv.iv[1] = ((char *)&engine_boots)[2];
        ap.priv.iv[2] = ((char *)&engine_boots)[1];
        ap.priv.iv[3] = ((char *)&engine_boots)[0];
        ap.priv.iv[4] = ((char *)&engine_time)[3];
        ap.priv.iv[5] = ((char *)&engine_time)[2];
        ap.priv.iv[6] = ((char *)&engine_time)[1];
        ap.priv.iv[7] = ((char *)&engine_time)[0];
        memcpy(ap.priv.iv + 8, salt, 8);

        memcpy(ap.priv.key, priv_key, 16);

        aes_init(&ap.priv.t.aes, ap.priv.key, ap.priv.iv);
        aes_encrypt_cfb(&ap.priv.t.aes, data, data_len, data);
    }
}

// 密钥本地化
void localize_key(unsigned int auth_type, unsigned char *key, unsigned char *lkey, unsigned char *engine_id, unsigned int engine_id_len)
{
    if (auth_type == AUTH_MD5)
    {
        memcpy(ap.auth.password_buf, key, 16);
        memcpy(ap.auth.password_buf + 16, engine_id, engine_id_len);
        memcpy(ap.auth.password_buf + 16 + engine_id_len, key, 16);

        md5_init(&ap.auth.t.md5);
        md5_update(&ap.auth.t.md5, ap.auth.password_buf, 32 + engine_id_len);
        md5_final(lkey, &ap.auth.t.md5);
    }
#ifdef SHA_1
    else if (auth_type == AUTH_SHA)
    {
        memcpy(ap.auth.password_buf, key, 20);
        memcpy(ap.auth.password_buf + 20, engine_id, engine_id_len);
        memcpy(ap.auth.password_buf + 20 + engine_id_len, key, 20);

        sha1_init(&ap.auth.t.sha);
        sha1_update(&ap.auth.t.sha, ap.auth.password_buf, 40 + engine_id_len);
        sha1_final(lkey, &ap.auth.t.sha);
    }
#else
    else if (auth_type == AUTH_SHA)
    {
        memcpy(ap.auth.password_buf, key, 32);
        memcpy(ap.auth.password_buf + 32, engine_id, engine_id_len);
        memcpy(ap.auth.password_buf + 32 + engine_id_len, key, 32);

        sha256_init(&ap.auth.t.sha256);
        sha256_update(&ap.auth.t.sha256, ap.auth.password_buf, 64 + engine_id_len);
        sha256_final(&ap.auth.t.sha256, lkey);
    }
#endif
}

// 密码转密钥
void password_to_key(char *password, unsigned char *key, unsigned int auth_type)
{
    unsigned char *cp;
    unsigned int password_index = 0;
    unsigned int count = 0;
    unsigned int i;
    unsigned int password_len;

    if (auth_type == AUTH_MD5)
    {
        md5_init(&ap.auth.t.md5);
    }
    else if (auth_type == AUTH_SHA)
    {
#ifdef SHA_1
        sha1_init(&ap.auth.t.sha);
#else
        sha256_init(&ap.auth.t.sha256);
#endif
    }
    else
    {
        return;
    }

    password_len = strlen((const char *)password);

    while (count < 1048576)
    {
        cp = ap.auth.password_buf;
        for (i = 0; i < 64; i++)
        {
            *cp++ = password[password_index++ % password_len];
        }
        if (auth_type == AUTH_MD5)
            md5_update(&ap.auth.t.md5, ap.auth.password_buf, 64);
#ifdef SHA_1
        else if (auth_type == AUTH_SHA)
            sha1_update(&ap.auth.t.sha, ap.auth.password_buf, 64);
#else
        else if (auth_type == AUTH_SHA)
            sha256_update(&ap.auth.t.sha256, ap.auth.password_buf, 64);
#endif
        count += 64;
    }
    if (auth_type == AUTH_MD5)
        md5_final(key, &ap.auth.t.md5);
#ifdef SHA_1
    else if (auth_type == AUTH_SHA)
        sha1_final(key, &ap.auth.t.sha);
#else
    else if (auth_type == AUTH_SHA)
        sha256_final(&ap.auth.t.sha256, key); // 注意参数顺序
#endif
}

// 初始化密钥
void snmp_init_keys(void)
{
    int i;

    for (i = 0; i < MAX_SNMPV3_USERS; i++)
    {
        if ((strlen((const char *)snmp_params->users[i].username) > 0) && (strlen((const char *)snmp_params->users[i].auth_key) > 0))
        {
            password_to_key(snmp_params->users[i].auth_pass, snmp_params->users[i].auth_key, snmp_params->users[i].auth_type);

            if ((snmp_params->users[i].priv_type != PRIV_NONE) && (strlen((const char *)snmp_params->users[i].priv_key) > 0))
            {
                password_to_key(snmp_params->users[i].priv_pass, snmp_params->users[i].priv_key, snmp_params->users[i].auth_type);
            }
        }
    }
}
