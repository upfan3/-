#ifndef  __DATABASE_H
#define  __DATABASE_H

#include "valtype.h"

/************************************************/

#define MODULE_OUT_VOLTAGE  0
#define MODULE_OUT_CURRENT  1
#define MODULE_STATUS       2  
#define MODULE_IN_VOLTAGE   3
#define MODULE_IN_CURRENT   4
#define MODULE_FREQ         5

/******************数据项结构******************************/

typedef struct _DataItem
{
	u8  st_Name;
	u8  st_MaxLen;
	void  *st_pData;
	_DataItem *st_pNextItem;
	_DataItem *st_pPreItem;
}DataItem;

/************************************************/

#define  OTHER_DATA          3//????


//Index small Type
#define  REC_MODULE        1//整流模块类型
#define  PHO_MODULE       2//光伏模块类型
#define  EXT_MODULE         3//扩展板类型
#define  SMART_MODULE    4//智能表类型
#define  DC_DISTRIBUTION    5//直流瓶类型
#define  AC_DISTRIBUTION     6//交流瓶类型
#define  BATT          7//电池类型
#define  SYS_CONFIG     8//系统配类型
#define  RUN_CTR       9//运行控制类型
#define  LOGIC_CTR        10//运行控制类型






/******************主设备单元结构******************************/

typedef struct
{
	u8 st_Name;
	u8 st_MaxLen;
	DataItem  *st_pItemhead;
	
	//void * st_pDataItem;
	
}EquipmentUnit;

//-------数据类型-------
#define VOLTAGE  0
#define CURRENT  1
#define POWER    2
#define TEMP     3
#define STATUS   4
#define WRANNING 5
#define VOLTAGE_AC  6
#define CURRENT_AC  7
#define MODULETYPE 8
#define PHASEPOSITION 9
#define FREQUENCY  10
#define USERCURRENT1 11
#define USERCURRENT2 12
#define USERCURRENT3 13
#define LIMITCURRENT 14
#define ENERGY1      15
#define ENERGY2      16
#define ENERGY3      17
#define ENERGY4      18
#define WRANNING16   19
#define ONOFF        20
#define LIMIT_CURR_STATUS    21
#define MODULE_RATE_I 22
#define MODULE_EFFICIEN 23


//-------数据类型长度-------
#define ONE_BYTE  1
#define TWO_BYTE  2
#define FOUR_BYTE  4


class DATABASE
{
	
  private:
	 	public:	
	
DATABASE();
~DATABASE();
	
EquipmentUnit m_EquUnit;

public:
void * GetDataAddr(u8 Datatype);
void Init(u8 Eqtype,u8 maxlen);
s8 DeleteData(u8 Datatype);
bool Append(u8 Datatype)	;
};


u16 bswap_16(u16 x) ;
u32 bswap_32(u32 x) ;


//直流配电单元参数

//extern u8 *gpBattCurr;
//extern u8 *gpLoadCurr;
//extern u8 *gpBatttemp;


//#define DCVOLTAGE 0    //系统电压
//#define LOADCURR   2           //负载电流  
//#define BATT_CURR1 4
//#define USER_CURR1 6
//#define USER_CURR2 8
//#define USER_VOLTAGE 10
//#define BATT_SOC   12     //电池容量
//#define BATT_BACK_TIME  14 //电池后备时间
//#define BATT_REMAIN_CAP  16//电池余量
//#define RECT_CURR 18       //整流模块总电流
//#define SOLAR_CURR 20     //整流模块总电流
//#define WIND_CURR 22      //光伏模块总电流
//#define FUEL_CELL_CURR 24 //燃料电池总电流
//#define PV_ENERGY 26      //光伏发电量
//#define WIND_ENERGY 30     //风机发电量
//#define FC_ENERGY 34     //燃料电池发电量
//#define ENV_TEMP 38 //环境温度
//#define HUMIDITY 40 //湿度
//#define WORK_STATUS 42//工作状态
//#define UNIT_ALARM 43//工作告警
//#define BATT_FUSE 45//电池熔丝
//#define LOAD_FUSE 46
//电池熔丝


typedef struct
{
	u8 st_CHState :2;//充电状态
	u8 st_ECOState   :1;//节能状态
	u8 st_PSOState   :1;//削峰状态
	u8 st_NONE        :4;
} stWORK_STATUS;

//对应YDT1363状态起始位
#define  YDT1363_CH_STATE  0x00//充电状态
#define  YDT1363_ECO_MODE  0x20//节能状态
#define  YDT1363_PAOS_STATE 0x30//削峰状态



typedef struct
{
	u16 st_DCVoltAlarm :2;     // 直流电压告警
	u16 st_InsulationAlarm  :1;//绝缘告警
	u16 st_LVBD   :3;    //电池下电告警
	u16 st_EnvTempAlarm        :2;//环境温度告警
	u16 st_HumidityAlarm       :2;//湿度告警
	u16 st_DoorAlarm       :1;//门禁告警
	u16 st_SmokeAlarm       :1;//烟雾告警
	u16 st_FloodAlarm       :1;//水侵告警
	u16 st_NONE :3;
} stUNIT_ALARM;

//对应YDT1363告警起始位
#define  YDT1363_DCV_ALARM  0x00    // 直流电压告警
#define  YDT1363_INSULATION_ALARM  0x20//绝缘告警
#define  YDT1363_LVBD_ALARM 0x30//电池下电告警
#define  YDT1363_ENVTEMP_ALARM 0x60//环境温度告警
#define  YDT1363_HUMIDITY_ALARM 0x80//湿度告警
#define  YDT1363_DOOR_ALARM 0xA0//门禁告警
#define  YDT1363_SMOKE_ALARM 0xB0//烟雾告警
#define  YDT1363_FLOOR_ALARM 0xC0//水侵告警

//告警起始位



#endif
