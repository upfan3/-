#ifndef _SCREEN_H
#define _SCREEN_H

#include "Tqueue.h"
#include "gpio.h"
#include "oled_display.h"

#define WINDOW_MAX_SIZE   4
#define SCREEN_MAX_LAYER  6
#define MAX_ITEM 32

#define COURSMODE_0 0
#define COURSMODE_1 1
#define COURSMODE_2 2
#define COURSMODE_3 3
#define COURSMODE_4 4
#define COURSMODE_5 5

#define SCREEN_ONE_BYTE 0x10
#define SCREEN_TWO_BYTE 0x20
#define SCREEN_FOUR_BYTE 0x30


#define PARA_MODIDI_OPTION 0x03
#define MODIFI_ONE_PARA 3
#define MODIFI_PARA 2
#define NEXT_SCREEN 1
#define NONE_DO     0

/**************************************************************************************
 st_pParalist  st_plink
1    NULL    NULL    只作显示用途，无任何操作
3    NULL   !NULL    跳转一屏   _plink指示下一屏坐标
2   !NULL   !NULL    设置非数字参数 _pMask指示参数在字符串中的位置 参数列表
***************************************************************************************/




#pragma pack(push,1)
typedef struct{

u32 st_pContext;	//项目显示内容
void * st_plink;//项目跳转指针   指向位置可以是下一屏项目指针，可以是数据列表	
u8 * st_pParalist;//参数列表
u8 st_CtrlType ; //所指向屏幕的项目数   0~15  0：不可修改参数  1：指向一屏  2：可修改参数
u8 dec;
}Item;



typedef struct{
//Item * st_rubbishItem;//暂存上一屏项目指针
Item ** st_ppPreItem;//上一屏指针的地址
Item ** st_ppItem;   //当前屏指针的地址
	u8 st_coursItem ; //
	u8 st_LayerHead ; //
	
}LayerData;


#pragma pack(pop)



typedef struct{
	u8	st_locateCount:4;
	u8 	st_locate:4;
	u8  st_keyEnterCount:4;
	u8  st_keyEnter:4;
	u8  st_Paratype;
	u8  st_EnterFlag;
	
} CoursCtrl;

class Screen{
	
private:
	
OLED * m_poled;

public:
   Item *m_pItem;   //屏幕项目指针
   Item **m_FirstItem;
   //Item **m_SecondItem;
   PTRFUN m_pfun;
  s8 m_EnterStatus;//enter键的按键状态
  KeyStats *m_pkey;  //按键值

  u8 m_head;        //屏幕首条项目的位置
  u8 m_coursItem;  //记录每屏光标位置
  
  u8 m_LayerCount ;   //层计数
  u8  m_flash;     //闪动效效生成
  u8 m_SreenChange;//屏莫切换
  //u16 m_MenSize;
   LayerData m_layer[SCREEN_MAX_LAYER];//储存各层显示信息
   CoursCtrl m_CoursLine;
   




 


Screen(Item **ppItem,Item **ppSItem);
 
 ~Screen(void);
void SetOled(OLED * _poled);
void SetScreenFun(PTRFUN _pfun);
void SetKey(KeyStats *_pkey);
//void Init(Item **ppItem,Item **ppSItem);//第一个参数为主界面，第二参数为主界面按退出键（一般为菜单界面），这两个界面互为退出
Screen * HMI(void);
void Set(Item *pItem);
void FreshGram();
void reSet(Item *pItem,u8 ctrl);
void DealWithKey(Item *pItem);
u8 print(u8 x,u8 y ,u8 mode,u8 flash,Item * pItem,CoursCtrl * pCoursline);
u8 printD(u8 * px,u8 *py , u8 *pi, void *pPara,  u8 mode ,u8 enterkey,u8 flash,u8 minbit);
void ClearScreen();
};

//显示界面定义
extern Item * pMainMenu;//主菜单页
extern Item * pDCDMenu;//首页



u8 setMenu(Item **ppItem);
//void IintItemData(void);

#endif 

