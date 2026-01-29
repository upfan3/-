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
};

#endif
