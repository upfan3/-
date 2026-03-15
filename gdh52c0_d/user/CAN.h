#ifndef CAN_H
#define CAN_H

#include "Tqueue.h"




 //CID1 def
#define UPDATE			0x01///<升级程序
#define DEVICE_INFO		0x02///<系统信息
#define REMOTE_CONTROL	0x04///<远程请求
#define DEVICE_PARA		0x07///<设备参数
#define BASIC_FRAME		0x08///<基本数据
#define TOTAL			0x0B///<统计帧
#define DEBUG			0x1A///<调试帧

// DTC

#define NONE   0x00
#define ACK    0x01
#define NACK   0x02
#define OVER   0x03
#define CANCEL 0x04


// CID2 def
#define SERIAL_NUMBER	0x10///<序列号
#define MODULE_TYPE     0x20///<模块型号
#define MODULE_VERSION  0x40///<模块版本
#define TIME_SYNC		0x10///<时间同步
#define RATED_PARA		0x70///<额定参数
#define SYSWORK_PARA	0x80///<工作参数
#define GROUP_NUM		0x40///<组号
#define GROUP_PARA		0xB0///<组参数
//#define BASIC_DATA      0x100///<基本数据帧

#define	USE_CAN_RCV_BUF_SIZE	0x20				//驱动程序使用的接收环形缓冲区的大小
#define TRUE 1
#define FALSE 0

#define ADR_MASK 0x3F
#define CID2_MASK 0xFFF
#define DTC_MASK 0x3F
#define  CID1_MASK 0x1F





typedef struct
{
	
	u32 st_ADR:6;
	u32 st_CID2:12;
	u32 st_DTC:6;
	u32 st_CID1:5;
	u32 st_rev:3;
	
}PCid;


typedef struct
{
	u8 ide; //扩展帧、标准帧辨别位，1/0 = 扩展帧/标准帧
	u8 rtr;//远程帧、数据帧辨别位，1/0 =  远程帧/数据帧
	u8 dlc; //数据字节
	u32 id;//发送帧信息ID
	u8 data[8];
}CanBuf,*PCanBuf;


class CanPort{
	
private:
	
u8 m_canId ;
u32 m_bouad;
Queue<CanBuf> *m_precvdat;
Queue<CanBuf> *m_precvdat1;
Queue<CanBuf> *m_psenddat;
u8 _rendflag;

u8 SendCanMessage(PCanBuf data);
void WriteToRecvBuf(can_receive_message_struct* RxMessage,u8 fifo);


public:
	

void SetCanFilterAndFIFO1buf(Queue<CanBuf> *_precvdat1);
CanPort(u8 canId,u16 bouad,Queue<CanBuf> *rdatbuf,Queue<CanBuf> *sdatbuf);
u8 SendData(PCanBuf);

static void sIRQpfn(CanPort &s);
static void rIRQpfn(CanPort &s,u8 fifo);

/*
数据接收处理
prbuf 数据接收指针
_fifo 0取_fifo0 1取fifo1
 返回 false 无有效数据，true 有新数据
*/
bool onRecvFrame(u8 _fifo,CanBuf *prbuf);

};



#endif
