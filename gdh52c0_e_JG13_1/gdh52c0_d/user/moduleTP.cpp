#include "globalval.h"

#define SERIALCODE_SIZE 16
#define PASSWD_SIZE 8

/*解密口令函数*/
void Fun_TeaDecrypt(uint16_t* v, uint16_t* k)
{
	uint16_t v0 = v[0], v1 = v[1], i;  /* set up */
	uint16_t delta = 0x9e37;            /* a key schedule constant */
	uint16_t sum = delta << 6;
	uint16_t k0 = k[0], k1 = k[1], k2 = k[2], k3 = k[3];   /* cache key */
	for (i = 0; i < 64; i++) {                         /* basic cycle start */
		v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
		v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
		sum -= delta;
	}                                              /* end cycle */
	v[0] = v0; v[1] = v1;
}

/*36进制字符转换成10进制数*/
uint8_t Fun_Conver(uint8_t str)
{
	if (str >= 0x30 && str <= 0x39)
	{
		return str - 0x30;
	}
	else if (str >= 0x41 && str <= 0x5A)
	{
		return str - 0x41 + 10;
	}
	else if (str >= 0x61 && str <= 0x7A)
	{
		return str - 0x61 + 10;
	}
	else		//非法字符换为 0 
	{
		return 0;
	}
}


/*16进制字符转换成10进制数*/
uint8_t Fun_HexConver(uint8_t str)
{
	if (str >= 0x30 && str <= 0x39)
	{
		return str - 0x30;
	}
	else if (str >= 0x41 && str <= 0x46)
	{
		return str - 0x41 + 10;
	}
	else if (str >= 0x61 && str <= 0x66)
	{
		return str - 0x61 + 10;
	}
	else
	{
		return 0;
	}
}

/*10进制数转换成16进制字符*/
uint8_t Fun_CoverHex(uint8_t num)
{
	//if (num >= 0 && num <= 9)
	if(num <= 9)
	{
		return (48 + num);
	}
	else if(num >= 10 && num <= 15)
	{
		return (num - 10 + 65);
	}
	else
	{
		return 0;
	}
}






/*解绑函数
传入8字节16进制字符串解绑口令，监控模块序列号地址
返回第一次解密之后的口令地址*/
uint8_t *Fun_UbindModule(uint8_t *UnbindPasswd, uint8_t *SerialCode)
{
	uint16_t arr_ciphertext[2] = {0};		//分开存储32位密文
	uint16_t arr_key[4]={0,0,0,0};	//分开存储64位密钥
	int8_t i,j = SERIALCODE_SIZE - 1, k = 0;
	uint32_t valid_key_tonum = 0;

	for (i = 0; i < PASSWD_SIZE; i++)	//生成32位密文
	{
		if (i < 4)
		{
			//arr_ciphertext[0] |= Fun_HexConver(UnbindPasswd[i]) << (12 - (i * 4));	//生成高16位
			arr_ciphertext[0] |= UnbindPasswd[i] << (12 - (i * 4));	//生成高16位
		}
		else
		{
			//arr_ciphertext[1] |= Fun_HexConver(UnbindPasswd[i]) << (12 - ((i - 4) * 4));	//生成低16位
			arr_ciphertext[1] |= UnbindPasswd[i] << (12 - ((i - 4) * 4));	//生成低16位
		}
	}

	for (; j >= 0; j--)		//找到流水码最后一个有效字符
	{
		if ((SerialCode[j] >= '0' && SerialCode[j] <= '9') || (SerialCode[j] >= 'a' && SerialCode[j] <= 'z') || (SerialCode[j] >= 'A' && SerialCode[j] <= 'Z'))
		{
			break;
		}
	}

	for (; j >= 0 && k < 4; j--)	//生成64位密钥
	{
		valid_key_tonum = valid_key_tonum * 36 + Fun_Conver(SerialCode[j]);
		if (valid_key_tonum > 0xffff)
		{
			arr_key[k] = valid_key_tonum & 0xffff;
			valid_key_tonum >>= 16;
			k++;
		}
	}

	Fun_TeaDecrypt(arr_ciphertext, arr_key);	//监控模块用自身流水码进行解密
	//Semi_Decrypt_Passwd = (((uint32_t)(arr_ciphertext[0])) << 16) | (uint32_t)arr_ciphertext[1]; //合并为32位数
	
	UnbindPasswd[0] = (arr_ciphertext[0] >> 12);
	UnbindPasswd[1] = (arr_ciphertext[0] >> 8) & 0x000f;
	UnbindPasswd[2] = (arr_ciphertext[0] >> 4) & 0x000f;
	UnbindPasswd[3] = arr_ciphertext[0] & 0x000f;
	
	UnbindPasswd[4] = (arr_ciphertext[1] >> 12);
	UnbindPasswd[5] = (arr_ciphertext[1] >> 8) & 0x000f;
	UnbindPasswd[6] = (arr_ciphertext[1] >> 4) & 0x000f;
	UnbindPasswd[7] = arr_ciphertext[1] & 0x000f;
	
	for(u8 i=0;i<8;i++)
	{
		 UnbindPasswd[i+8]=Fun_CoverHex(UnbindPasswd[i]);
	}

	return &UnbindPasswd[8];
}