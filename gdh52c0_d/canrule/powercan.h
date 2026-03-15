#ifndef  __POWERCAN_H
#define  __POWERCAN_H



#include "globalval.h"

//#define test_energy		

//CID1

#define UPDATA     0x01         //升级
#define DEVICE_MESSAGE  0x02        //设备信息
#define REMOTE_CTRL  0x04        //遥控
#define DEVICE_PARA  0x07      //设备参数
#define BASIC_DATA    0x08   //基本数据
#define STATISTICS_DATA 0x0B // 统计参数
#define DEBUG_DATA  0x1A    //调试数据

//DTC

#define MAIN_CTRL 0x01//主控
#define ACM       0x03//交流配电单元
#define DCM       0x05//直流瓶电单元
#define MRM      0x11//整流模块
#define WR      0x12//风机模块
#define SC      0x18//光伏模块
#define JGSW    0x2A//京硅智能开关
#define BATTERY_DOWN 0x04//电池下电板


typedef struct
{
	u8 st_times:4;
	u8 st_success:4;
	
}DcmParaSucc;


class PowrCan{
	
	
	


	public:	

#ifdef test_energy	//跑电能测试，以0.3度每分钟跑		
u8  CanGetEnergy();	
#endif
void DuleWiteData(CanBuf *prbuf);
u8 CanPowerOnTick(CanBuf *prbuf,void *pScreen);
void UpDataUserEnergy(void);
void UpDataACEnergy(void);
void AccUserEnergy(void);
void OnMinTick(void);
	
	
};


s8 CompareTime(DateTimeF33 *pTime1,DateTimeF33 *pTime2);


extern u8 gAcmOnLineCount;
extern u8 gAcOnLineFlag;




extern u8 powerCanSendEnable;



#endif 

