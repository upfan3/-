#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H


#include "Tqueue.h"


class OLED{


	
private:


u8  GRAM[128][8];
void DrawPoint(u8 x,u8 y,u8 t);



 

public:
OLED();	
s16 GetCNFontId2(u8 *pid);
s16 GetCNFontId(u8 *pid);
u16 m_delayms;
void Init(void);
void initSPI(void);
void ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
void ShowFont(u8 x,u8 y,u16 fno,u8 mode);
void ShowFont2(u8 x,u8 y,u16 fno,u8 mode);
void Show_all(u8 x,u8 y,u8 *p,u8 size,u8 mode);
void Show_all2(u8 x,u8 y,u8 *p,u8 size,u8 mode);
void Clear(void);
void Refresh_Gram(void);
void Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot); 
void clrScreen(void);	
void delayms(u8 tick);
};


#endif


