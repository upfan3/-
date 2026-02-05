#ifndef  _GPIO_H_
#define  _GPIO_H_

#include "Tqueue.h"

//-------------IO pin name ------------------//
//output
#define Relayer0 0
#define Relayer1 1
#define Relayer2 2
#define Relayer3 3
#define Relayer4 4
#define Relayer5 5
#define OC6      6
#define OC7      7
#define OC8      8
#define OC9      9
#define OC10     10
#define OC11     11
#define WARN_LED    12
#define RUN_LED    13
#define BEE      14

//input
#define READ_DI0  0
#define READ_DI1  1
#define READ_DI2  2
#define READ_DI3  3
#define READ_DI4  4
#define READ_DI5  5 
#define READ_DI6  6
#define READ_DI7  7
#define READ_DI8  8
#define READ_DI9  9
#define READ_DI10 10
#define READ_DI11 11

#define KEY_OK 12
#define KEY_DOWN 13
#define KEY_UP 14
#define KEY_ESC 15



//-------------IO pin status------------//
#define ON        1
#define OFF       0

#define LED_ON    0
#define LED_OFF   1

#define BEE_ON    0
#define BEE_OFF   1

#define DOWN 0
#define UP   1

#define Bit_SET 1
#define Bit_CLR 0

//----------------------Key status--------//

typedef enum {
  KEYNONE =0,
  KEYUP ,
	KEYDOWN,
	KEYENTER,
	KEYESC
} KeyStats;

typedef struct
{
	u8 presskey:4;
	u8 keyConut:4;
}KeyCtrl;



class gpio{
	

private:	
	KeyCtrl keyCtrl[4];	
public:	

KeyStats m_vkey;
u16 m_delay;
	
gpio();
void set(u8 pin,u8 _status);
FlagStatus get(u8 pin);	
void setOnOff(u8 pin);
void scankey(u16 _delay);	
	
};


#endif

