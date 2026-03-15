#ifndef USART_H
#define USART_H

#include "Tqueue.h"


class UsartPort{
	
private:
	
u8 m_Comid ;
u32 m_bouad;
Queue<u8> *m_precvdat;
Queue<u8> *m_psenddat;
Ts_gpio *m_pCtrIo;
u8 m_tail;
u8 m_rendflag;

public:
	
//CanPort(void){};	
UsartPort(void)
{}
UsartPort(u8 Comid,u16 bouad,Queue<u8> *rdatbuf,Queue<u8> *sdatbuf);
u8 SendData(u8 *pdat);
void SendData(u8 *pbuf,u16 len);
void SendDataEx(u8 *pbuf,u16 len);
static void IRQpfn(UsartPort &s);
void EnableRecv(void);
void DisableRecv(void);
void onRecvFrame(PTRFUN1 pFun=NULL);
void CRCcheck(u8 *pdat,u8 len);
void	onRecvModulebus(PTRFUN1 pFun=NULL);
bool  onRecvCommon(PTRFUN1 pFun);
 void SetTail(u8 taii)
 {
	   m_tail=taii;
 }	 
};




#endif

