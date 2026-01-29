#ifndef _NETWORK_H_
#define _NETWORK_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "w5500.h"

#ifdef __cplusplus
}
#endif

#define Data_BuffSize   1024

typedef enum {
    DHCP_DISABLE = 0,  // 禁用DHCP，使用静态IP
    DHCP_ENABLE  = 1   // 启用DHCP，自动获取IP
} DHCP_MODE;


class network
{

private:
    CONFIG_MSG m_ConfigMsg;

public:
    u8 m_delaytimer;
    u8 m_constatus;

    network();
    void Init();
    void setIP(u8 *pIP);
    void setMac(u8 *pMac);
    void setSub(u8 *pSub);
    void setGw(u8 *pGw);
    void setDns(u8 *pDns);
    void setNetPara(void);
    void setDefault(void);
    void setKeepAlive(SOCKET s);
    void OnTick(void);
    void OsRest(void);
    void ScanState(u8 *ptcp_err);
    void onTick(void);
		void setDHCP(DHCP_MODE mode);
		uint8 getDHCP(void);

};

#endif
