#ifndef YDT1363_H
#define YDT1363_H
#include "globalval.h"



typedef struct
{
u8 st_type;  //
u8 len;		
void * pdata;  //数据指针


}ytd1363data;





/////////////////////////////
//st_type:1,bit7: 1 表示输入数值扩大100，用于浮点数数据 ，bit 2~0:表示字节类型：1单字节,2双字节，4四字节。
//        2,当bit6=1,进行递归扩展，bit 5~0:表示扩展项目数。
//pdata;  1，为协议数据指针，可为单字节,双字节，四字节。
//        2，当进行递归调用时，为ytd1363data 指针类型，为扩展数据指针
//        3，当为NULL表示协议中的自定义项数，此时该项填入len，相当于协议中的P
//len:    某一项的个数，递归调用时为扩展个数，相当于协议中 M或N
////////////////////////////


#define LEN_ONEBYTE     0x00        
#define LEN_CIRCLE_ONEBETY  0x40  //对数据字段指针内容为逻辑1时，输出指定的数据  (st_type&0x70>>4) 其它输出为0
                                  //在st_len&3F>1,st_type&0x80==TYPE_CIRCLE_ONEBETY  st_type&0x07==TYPE_THREEBIT时生效
#define LEN_TWOBYTE  0x80
#define LEN_FOURBYTE  0xC0

#define TYPE_ONEBYTE 1
#define TYPE_TWOBYTE 2
#define TYPE_FOURBYTE 3      //4字节大端模式
#define TYPE_FOURBYTE_L 4    //4字节小端模式
#define TYPE_ONEBIT 5
#define TYPE_TWOBIT 6
#define TYPE_THREEBIT 7
#define TYPE_EXTERN 0x08
#define TYPE_CIRCLE_ONEBETY  0x80  //在单一数指针位置的数据里里循环取值，从数据低位开始取，
                                   //如11001101 ，k=0,时取第一位(1)或取第1(1)，2(0)位k=1时，取第二位(0)或取3(1)，4(1)位，如类推
																	 //在st_len&3F>1且st_type&0x07！=LEN_TWOBYTE或TYPE_TWOBYTE时生效
																	 
#define TYPE_FLOAT 0x80           //对输入的指针位置数据乘100，用以浮点数输入 st_type&0x07==LEN_TWOBYTE或TYPE_TWOBYTE时生效

#define  END_SET_YDT1363  SetYDT1363(NULL,0,NULL,0);

 u8 HextoAscii(u8 *op,u8* pdat);
 u8 AsciitoHex(u8 *op,u8* pdat);
 u16 AsciitoHex16(u8* pdat);
 u32 AsciitoHex32(u8* pdat);


u8 * get1363data(u8 *cmd,u8 *predat,u8 *port);

u8 * protocol_TieTa_1363(u8 Ver, u8 COMMAND_GROUP, u8 ADDR_Temp,u8 CID1_Temp,u8 CID2_Temp,u8 * Info_Data );

extern u8 isTieta;
extern u8 bitAlarmTypeList[12];
#endif

