#include "Screen.h"

Item **passWordToChange=NULL;
extern Item *pPASSWORD;
extern u16 delayPassWord;
extern u16 passWord;
extern u8 ctrl_bee;
u8 password[4]={0,0,0,0};
u8 passwordshow[4]={11,11,11,11};
u8 genterflag=0;
//u8 isx=0;

Screen gScreen(&pDCDMenu, &pMainMenu);
Screen *pScreen=&gScreen;

Screen::Screen(Item **ppItem,Item **ppSItem)
{
	m_FirstItem=ppItem;
	//m_SecondItem=ppSItem;
	m_LayerCount=1;
	m_pkey =NULL;
	m_coursItem=0;
	m_EnterStatus=0;
  m_flash=0;
	m_head=0;
	
	m_CoursLine.st_locate=1;
	m_CoursLine.st_EnterFlag=0;
	m_layer[m_LayerCount].st_ppItem=ppItem;
	m_layer[m_LayerCount].st_ppPreItem=ppSItem;//在主界面按退出键

	m_pfun=NULL;
	 m_SreenChange=0;
   //m_pItem=*ppItem;
}

Screen::~Screen()
{
	;
}


void Screen::SetOled(OLED * _poled)
{
	  m_poled=_poled;
}

void Screen::SetScreenFun(PTRFUN _pfun)
{
	m_pfun=_pfun; // 设置回调函数
	m_pfun(m_FirstItem);// 调用回调函数处理首个项目
	m_pItem=*m_FirstItem;  // 初始化当前项目为首个项目
}

void Screen::SetKey(KeyStats *_pkey)
{
	  m_pkey=_pkey;
}




u8  Screen::printD(u8 * px,u8 *py , u8 *pi, void *pPara,  u8 mode ,u8 enterkey,u8 flash,u8 minbit)
{ u32 tdata;
	
	Item rItem;
	
	
	
	u8 bitCount=1,j,x=*px ,y=*py,i=*pi;
     tdata =*(u16 *)pPara; 
	
	   if(minbit==0x0d)//minbit等于0x0d时输入为单字节数据
		 {
			 //u8 tmpd
			 tdata=*(u8 *)pPara; 
		 }
		 else if(minbit==0x0E)
		 {
			 tdata=*(u32 *)pPara;
		 }
		 
	  
	
///////////////////计算数据长度////////////////////////////
	
	            if((minbit==0)||(minbit>10))//minbit>10或等于0时，按实际数目输出
							{
                 if(tdata>=10)
								 {   bitCount=2;
									 if(tdata>=100)
									 {     bitCount=3;
										 if(tdata>=1000)
										 {   bitCount=4;
											 if(tdata>=10000)
											  {bitCount=5;
													if(tdata>=100000)
													{
														bitCount=6;
														if(tdata>=1000000)
															{
																bitCount=7;
																if(tdata>=10000000)
																{
																	bitCount=8;
																	
																}
															}
													}
													
												}
										 }
									 }
									 
								 }
							 }
							else if(minbit<10) //最大位数小于10
							{
								 bitCount=minbit;
							}
/////////////////////取出各位数据并转为ASCII码///////////////////////////////////

							   u8 data[9];

									 for(j=bitCount;j>0;j--)
									 {
										 data[j-1]=tdata%10+0x30;
										 tdata=tdata/10;
										 i++;
										
										 
									 }
									 data[bitCount]='\0';
									 
////////////////////////更改显示内容并写OLED///////////////////////////////////////						 
									   rItem.st_pContext=(u32)data;
									   rItem.st_CtrlType=enterkey;
									   print(x, y ,mode, flash, &rItem,NULL);//注这为递归调用
									
									 x+=6*bitCount;  
									

//////////////////////更新坐标///////////////////////////////////////////////////////
									 *px=x;
									 *py=y;
									 *pi=i;
              
								 return bitCount;
								 
}





u8  Screen::print(u8 x,u8 y ,u8 mode,u8 flash,Item * pItem,CoursCtrl* pCoursline)
{

	#define MAX_CHAR_POSX 122
	#define MAX_CHAR_POSY 58 	 
	s16 id;

	    
	u8 qh,bitCount, ParaCount=0,paraCount=0,isfloat=0,Mode=0, *p=(u8 *)pItem->st_pContext,isx=0;
	    
	   void * pPara = pItem->st_plink;
u8 i=0,ParaCount2=0;

	
	
	
	
	

	while(*p!='\0')
	{
	   qh=*p;
	   if(qh>160)
	   {
	    
			 //id=GetCNFontId(p);
			   id=m_poled->GetCNFontId2(p);
			  
			 	if(x>MAX_CHAR_POSX){x=0;y+=16;}
				if(y>MAX_CHAR_POSY){y=x=0;m_poled->Clear();}
				
				
				
				 //////////////////////设置光标及显示模式///////////////////////////////////////	
				
				
				     if(mode== COURSMODE_5)
					 {
						 if(i==pItem->st_CtrlType*2)
						 {
							 Mode=1&flash;
						 }
						 else
						 {
							  Mode= COURSMODE_0;
						 }
					 }
				else if(mode== COURSMODE_4)
				{
					Mode=1&flash;
				} 
				else if(mode== COURSMODE_3)
				{
					Mode=0;
				} 
				else if(mode== COURSMODE_2)
				{
					Mode= COURSMODE_0;
				} 
				else{
					Mode=mode;
				}
				
						 // OLED_ShowFont_12x12(x,y,id,Mode);
				    m_poled->ShowFont2(x,y,id,Mode);
						x+=12;

		p++;				
		p++;
						i++;
						i++;
			 
	   }
	   if(qh<160)
	   {
			 
			 
			  if(x>MAX_CHAR_POSX)
			{
				 x=0;
				 y+=16;
			
				if(y>MAX_CHAR_POSY)
				{
				 y=x=0;
				 //OLED_Clear();
					m_poled->Clear();
				}
			}
			 
			 
			 
			 
			 if((qh=='%')||(qh=='$'))
			 {  
				 ParaCount++;
				 if(qh=='%')
				 {ParaCount2++;}
				    i++;
				 switch (*(p+1))
				 {
					 case 'z':isx|=0x02;
					 case 'o':isx|=0x01;
					 case 'd':
					 {				
                 //////////////////////设置光标及显示模式///////////////////////////////////////					 

					   if(pCoursline!=NULL)
					  {
						        if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount==  pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
								
									  paraCount = printD(&x,&y ,&i, pPara, Mode ,pCoursline->st_keyEnter, flash,0 );

									 
									
									 	 u16 *ptemp;
									    ptemp= (u16 *) pPara;
									    ptemp++;
									   pPara=ptemp;
									 
									 p++;
									 p++;
								 									 

	                if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											
											pCoursline->st_keyEnterCount=paraCount;
									  
												if((isx&0x02)==0x02)
											{
												pCoursline->st_Paratype='z';
												isx=0;
											}
											else if((isx&0x01)==0x01)
											{
												pCoursline->st_Paratype='o';
												pCoursline->st_keyEnterCount=1;
												isx=0;
											}
											else if(isx==0)
											{
											 pCoursline->st_Paratype='d';
												isx=0;
											}
											
										
									 }
								 
								 
					       continue ;
								 }break;
								 }
					 case 'O':isx|=0x02;
					 case 'Z':isx|=0x01;
					 case 'D':
					 {				
                 //////////////////////设置光标及显示模式///////////////////////////////////////					 

					   if(pCoursline!=NULL)
					  {
						        if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount==  pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
								
									  paraCount = printD(&x,&y ,&i, pPara, Mode ,pCoursline->st_keyEnter, flash,0x0d);

									 
									// ((u8 *) pPara)++;
									 u8 *ptemp;
									    ptemp= (u8 *) pPara;
									    ptemp++;
									   pPara=ptemp;
									 
									 p++;
									 p++;
								 									 

	                if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											
											pCoursline->st_keyEnterCount=paraCount;
									 
												if((isx&0x02)==2)
												{
													pCoursline->st_Paratype='O';
													pCoursline->st_keyEnterCount=1;
													isx=0;
												}
												else if((isx&0x01)==1)
												{
													pCoursline->st_Paratype='Z';
													isx=0;
												}
												else if(isx==0)
												{
													pCoursline->st_Paratype='D';
													isx=0;
												}
												
												
									 }
								 
								 
					       continue ;
								 }break;
								 }
					 case 'f':
					 {
						if(pCoursline!=NULL)
					  { 
						  float tfdata;
					  //	u8 negativeflag=0;//负数标志位
						  u16 tdata;
						  if(isfloat==0) 
							{								
						      tfdata =*(float *)pPara ;
								
								    if(tfdata<0)
										{
											tfdata*=-1;//变为正数
											//negativeflag=1;
												//OLED_ShowChar(x,y,'-',12,Mode);//画负号
											m_poled->ShowChar(x,y,'-',12,Mode);//画负号
					                 x+=6;
                            i++;
										}
					        tdata=(u16 )tfdata;
								
								   if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
					 
									 paraCount = printD(&x,&y ,&i,&tdata, Mode ,pCoursline->st_keyEnter ,flash,0);
					          				 

									 if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											
											pCoursline->st_keyEnterCount=paraCount;
									  
											  pCoursline->st_Paratype='f';
										
									 }
								
								if(*(p+2)=='.')
								{
									isfloat=1;
									continue;
								}

							}
					    else//设置小数部分
							{
								   tfdata =*(float *)pPara ; 
                  if(tfdata<0)		
									{
										tfdata*=-1;
									}										
					        // tfdata=*(float *)pPara-tdata;//取出小数部?
									    tfdata=tfdata-tdata;
								        if(mode== COURSMODE_2||mode== COURSMODE_3)
												{
													Mode= COURSMODE_0;
												}
												else
												{
													Mode=mode;
												}
   

											//	OLED_ShowChar(x,y,'.',12,Mode);//画小数点
												m_poled->ShowChar(x,y,'.',12,Mode);//画小数点
								                             
					                 x+=6;
                            i++;
    								bitCount=*(p+3)-0x30;
								    if (bitCount>4)bitCount=4; 
                    for(u8 j=0;j<bitCount;j++)
                   {
										  tfdata=tfdata*10;//计算小数部分显示的数位
                   }
                  tdata=(u16 )tfdata;
									 
									  if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
					 
									 paraCount = printD(&x,&y ,&i,&tdata, Mode,pCoursline->st_keyEnter,flash,bitCount); 
									 p++; 
									 p++;

									  if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											
											pCoursline->st_keyEnterCount=paraCount;
									  
											  pCoursline->st_Paratype='f';
										
									 }
                isfloat=0;									 
							}
							
							 float *ptemp;
							
							//((float *)pPara)++;
					              ptemp=((float *)pPara);
                        ptemp++;
							          pPara=ptemp;
					           p++;
									   p++;
					  

					      continue ;
						}break;	
					}		
           case 'F':
					 {
						if(pCoursline!=NULL)
					  { 
						 
							
							// u16 tdata;
							
							
							
							
							
							s32 s32data;
		
						 
							u16  multiple=1;
							
							
							
								bitCount=*(p+3)-0x30;//获取小数位数
									for(int t=0;t<bitCount;t++)
									{	
					            multiple*=10;
									}
						  if(isfloat==0) 
							{								
						     if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
								     {
											 s32data=*((s16 *)pPara);
										 }
									else if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
									{
										  s32data=*((s32 *)pPara);
									}										
								
								    if(s32data<0)
										{
											s32data*=-1;//变为正数
											//	OLED_ShowChar(x,y,'-',12,Mode);//画负号
												m_poled->ShowChar(x,y,'-',12,Mode);//画负号
					                 x+=6;
                            i++;
										}
										
						
								   s32data=s32data/multiple;
								   if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
					 
									 paraCount = printD(&x,&y ,&i,&s32data, Mode ,pCoursline->st_keyEnter ,flash,0x0E);
					          				 

									 if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											
											pCoursline->st_keyEnterCount=paraCount;
									  
											  pCoursline->st_Paratype='F';
										
									 }
								
//								if(*(p+2)=='.')
//								{
//									isfloat=1;
//									continue;
//								} //9月1日，增加去掉小数点的情况
								if((*(p+2)=='.')&&(bitCount>0))
								{
									isfloat=1;
									continue;
								}
								if(bitCount==0)
								{
									p++;
  								p++;
									 pItem->dec=0;

								}

							}
					    else//设置小数部分
							{
								     if((pItem->st_CtrlType&0x80)!=0x80)
															{
																						 
																
																 pItem->dec=*(p+3)-0x30;
																	
															}
														
								  
                   if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
								     {
											 s32data=*((s16 *)pPara);
										 }
									else if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
									{
										  s32data=*((s32 *)pPara);
									}			
									
                  if(s32data<0)		
									{
										s32data*=-1;
									}										
					        
									 
				
									
								        if(mode== COURSMODE_2||mode== COURSMODE_3)
												{
													Mode= COURSMODE_0;
												}
												else
												{
													Mode=mode;
												}
   

												//OLED_ShowChar(x,y,'.',12,Mode);//画小数点
												m_poled->ShowChar(x,y,'.',12,Mode);//画小数点
								                             
					                 x+=6;
                            i++;
												
												 
													bitCount=*(p+3)-0x30;

									 s32data=s32data%multiple;//取出小数部
												
									  if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
					 
									 paraCount = printD(&x,&y ,&i,&s32data, Mode,pCoursline->st_keyEnter,flash,bitCount); 
  								 p++; 
									 p++;

									  if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											
											pCoursline->st_keyEnterCount=paraCount;
									  
											  pCoursline->st_Paratype='F';
										
									 }
                isfloat=0;									 
							}
							
							 
								
                     if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
								     {
											 
											  s16 * ptemp=(s16 *)pPara;
											  ptemp++;
						            pPara=ptemp;	
											 
											 
										 }
									  else if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
									  {
										    s32 * ptemp=(s32 *)pPara;
											  ptemp++;
						            pPara=ptemp;	
									   }		   






							

					           p++;
									   p++;
					  

					      continue ;
						}break;	
					}						 
		
					 case 'G':
					 {
						   
						if(pCoursline!=NULL)
					  { 
						 
							
							

							
							u32 u32data;
		
						 
							u16  multiple=1;
							
							
							
								bitCount=*(p+3)-0x30;//获取小数位数
									for(int t=0;t<bitCount;t++)
									{	
					            multiple*=10;
									}
						  if(isfloat==0) 
							{								
						     if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
								     {
											 u32data=*((u16 *)pPara);
										 }
									else if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
									{
										  u32data=*((u32 *)pPara);
									}										
								
//								    if(s32data<0)
//										{
//											s32data*=-1;//变为正数
//											//	OLED_ShowChar(x,y,'-',12,Mode);//画负号
//												m_poled->ShowChar(x,y,'-',12,Mode);//画负号
//					                 x+=6;
//                            i++;
//										}
										
						
								   u32data=u32data/multiple;
								   if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
					 
									 paraCount = printD(&x,&y ,&i,&u32data, Mode ,pCoursline->st_keyEnter ,flash,0x0E);
					          				 

									 if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											
											pCoursline->st_keyEnterCount=paraCount;
									  
											  pCoursline->st_Paratype='F';
										
									 }
								
//								if(*(p+2)=='.')
//								{
//									isfloat=1;
//									continue;//9月1日增加没有小数点的情况
//								}
									 if((*(p+2)=='.')&&(bitCount>0))
								{
									isfloat=1;
									continue;
								}
								if(bitCount==0)
								{
									p++;
  								p++;
                  pItem->dec=0;
								}

							}
					    else//设置小数部分
							{
								     if((pItem->st_CtrlType&0x80)!=0x80)
															{
																						 
																
																 pItem->dec=*(p+3)-0x30;
																	
															}
														
								  
                   if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
								     {
											 u32data=*((u16 *)pPara);
										 }
									else if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
									{
										  u32data=*((u32 *)pPara);
									}			
									
//                  if(u32data<0)		
//									{
//										s32data*=-1;
//									}										
					        
									 
				
									
								        if(mode== COURSMODE_2||mode== COURSMODE_3)
												{
													Mode= COURSMODE_0;
												}
												else
												{
													Mode=mode;
												}
   

												//OLED_ShowChar(x,y,'.',12,Mode);//画小数点
												m_poled->ShowChar(x,y,'.',12,Mode);//画小数点
								                             
					                 x+=6;
                            i++;
												
												 
													bitCount=*(p+3)-0x30;

									 u32data=u32data%multiple;//取出小数部
												
									  if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
					 
									 paraCount = printD(&x,&y ,&i,&u32data, Mode,pCoursline->st_keyEnter,flash,bitCount); 
  								 p++; 
									 p++;

									  if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											
											pCoursline->st_keyEnterCount=paraCount;
									  
											  pCoursline->st_Paratype='G';
										
									 }
                isfloat=0;									 
							}
							
							 
								
                     if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
								     {
											 
											  u16 * ptemp=(u16 *)pPara;
											  ptemp++;
						            pPara=ptemp;	
											 
											 
										 }
									  else if((pItem->st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
									  {
										    u32 * ptemp=(u32 *)pPara;
											  ptemp++;
						            pPara=ptemp;	
									   }		   






							

					           p++;
									   p++;
					  

					      continue ;
						}break;	
					 }

					case 'x':isx=1;
       						 
           case 'l':
					 {     
						   if(pCoursline!=NULL)
							 {
						        u8  *pdata, **ppdata ,***pppdata;
					               pppdata=(u8 ***)pPara;
						             ppdata=*pppdata;
								     u8 **tmpp=*pppdata;
								            
								         u8 listLength=0;
								        //////////////////////计算选项数目////////////////////////////////////////
								        while(*(tmpp++)!=NULL)
												{
													listLength++;
												}
												////////////////////////////////////////////////////////////
						             if(pItem->st_pParalist[ParaCount-1]<listLength) //参数值少于选项数n才有效	
						              pdata=ppdata[pItem->st_pParalist[ParaCount-1]]; //pItem->st_pParalist[ParaCount-1]取值为0~n-1
                        else
												{
													pItem->st_pParalist[ParaCount-1]=0;//将参数值更改为选项0
													pdata=ppdata[0];//若参数无效则选择选项0
												}
								         bitCount=strlen((const char *)pdata); 

					              if(mode== COURSMODE_2)
									 { if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_4;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else if(mode== COURSMODE_3)
									 {
										  if(ParaCount== pCoursline->st_locate)
										    {Mode= COURSMODE_5;}
												else
												{Mode= COURSMODE_0;}
									 }
									 else{
										  Mode=mode;
									 }
									  Item rItem;
									 
									  rItem.st_pContext=(u32)pdata;
									 rItem.st_CtrlType=pCoursline->st_keyEnter;
									  print( x,y ,Mode, flash,&rItem,NULL);
									 
					           x+=6*bitCount;
					           i+=bitCount;
					          pppdata++;
					        pPara=pppdata;
					          p++;
					          p++;
					  
					      paraCount=bitCount;

					          if((ParaCount== pCoursline->st_locate)&&(m_coursItem*16==y))
										{
										  
											pCoursline->st_keyEnterCount=1;
											if(isx==0)
											{pCoursline->st_Paratype='l';}
											else
											{
												pCoursline->st_Paratype='x';
												isx=0;
											}
										
									 }
					 
					 continue ; 	
									}break;										
								 }		 					 


					case '%':
						     p++;
						 break;
					 default :
						 i--;
					   ParaCount--;
					   ParaCount2--;
						 break;
						 
				 }
		 
			 }
			 
			 
			 
					 if(mode== COURSMODE_5)
					 {
						 if(i==pItem->st_CtrlType)
						 {
							 Mode=1&flash;
						 }
						 else
						 {
							  Mode= COURSMODE_0;
						 }
					 }
				else if(mode== COURSMODE_4)
				{
					Mode=1&flash;
				} 
				else if(mode== COURSMODE_3)
				{
					Mode= COURSMODE_0;
				} 
				else if(mode== COURSMODE_2)
				{
					Mode= COURSMODE_0;
				} 
				else{
					Mode=mode;
				}
						 

			 // OLED_ShowChar(x,y,*p,12,Mode);
			 m_poled->ShowChar(x,y,*p,12,Mode);

			  x+=6;
			  p++;	
	      i++;
	   }

	}
	        if((pCoursline!=NULL)&&(m_coursItem*16==y))//为光标所在的行
					{
						pCoursline->st_locateCount=ParaCount2;//统计设置参数数量;
					}
	
	return 0;
	
}



void Screen::reSet(Item *pItem,u8 ctrl)
{
	m_pItem=pItem;		
	*m_pkey =KEYNONE;
	m_head=0;
	m_coursItem=0;
   m_CoursLine.st_locate=1;
	 m_EnterStatus=0;
	
   m_flash=0;
	
	  if(ctrl==1)
		{//m_pfun(ppItem)
			if(m_pfun(m_layer[m_LayerCount].st_ppPreItem)==0)     
   				     return;
			m_pItem=*m_layer[m_LayerCount].st_ppPreItem;
			
					if(m_LayerCount>0)
					{
							m_LayerCount--;
							m_EnterStatus=0;
							m_coursItem=m_layer[m_LayerCount].st_coursItem;
							m_head=m_layer[m_LayerCount].st_LayerHead;	

							m_layer[m_LayerCount].st_ppItem=m_layer[m_LayerCount+1].st_ppPreItem;
					}
		}
		else
		{
			if(pItem->dec==1)//将新界面设置为选择翻页
	     m_EnterStatus=3;	
		}
	
	 m_SreenChange=1;
}




extern u8 gscreensleep;
void Screen::DealWithKey(Item *pItem)
{
		 switch(*m_pkey)
	 {
		 
		  case KEYUP: 
			{    gscreensleep=180;
				  
			       if(m_EnterStatus==0)
						 {  
							 m_coursItem--;					 
							// if(m_coursItem<0)
							  if(m_coursItem==0xFF)
							 {
								 m_coursItem=0;
								 if(m_head>0)//表示滚屏存在
								 {
									  
									  m_head--;//更改滚屏首指针
								 }
							 }
							 
							 
							 
							 
						 }

						
						  if(m_EnterStatus==3)
						{
							

							    switch(m_CoursLine.st_Paratype)
									{
										case 'd':
										{
											   u16 * pdat= (u16*)pItem[m_coursItem].st_plink;
													u16 dat=1;

													for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
													{
																	dat*=10;
													}
                           
													pdat[m_CoursLine.st_locate-1]+=dat;

										     
											break;
									   }
										 case 'o':
										{
											 u16 * pdat=(u16 *)pItem[m_coursItem].st_plink;
											      pdat[m_CoursLine.st_locate-1]++;
											break;
										}	
										case 'O':
										{
											 u8 * pdat=(u8 *)pItem[m_coursItem].st_plink;
											      pdat[m_CoursLine.st_locate-1]++;
											break;
										}
										case 'D':
										{
											   u8 * pdat=(u8 *)pItem[m_coursItem].st_plink;
													u8 dat=1;

													for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
													{
																	dat*=10;
													}
                          if((u16)pdat[m_CoursLine.st_locate-1]+dat<=255)
													   pdat[m_CoursLine.st_locate-1]+=dat;

										     
											break;
									   }
										case 'f':
										{
											 float * pdat= (float*)pItem[m_coursItem].st_plink;
											if(m_CoursLine.st_locate%2!=0)
												{    //整数部分
												 u16 dat=1;

																 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																		pdat[(m_CoursLine.st_locate-1)/2]+=dat;
													}
											    else{//小数部分
														float dat=1;
															for(u8 i=0;i<m_CoursLine.st_keyEnter+1;i++)
														{
															   dat/=10;
														}
														  pdat[(m_CoursLine.st_locate-1)/2]+=dat;
														
													}
											
											
											break;
										}
										case 'F':
										{
                         s16 * s16pdat;
									       s32 * pdat;//= (s16*)pItem[m_coursItem].st_plink;
											
														if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
													 {
														 s16pdat=(s16*)pItem[m_coursItem].st_plink;
													 }
														else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
														{
																 pdat=(s32*)pItem[m_coursItem].st_plink;
														}		
											
																
											
											
											if(m_CoursLine.st_locate%2!=0)
												{    //整数部分
												 u32 dat=1;

																 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																
																	
															  	u8 tmp=0;

																		tmp=pItem[m_coursItem].dec;
																
																u16 times=1;
																for(u8 t=0;t<tmp;t++)
																	{
																		times*=10;//扩大小数位数的倍数
																	}
																dat=dat*times;
																	
																		//pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																	
														if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
														 {
															 s16pdat[(m_CoursLine.st_locate-1)/2]+=dat;
														 }
														else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
														 {
																 pdat[(m_CoursLine.st_locate-1)/2]+=dat;
														 }				
																	
																	
													}
											    else{//小数部分
														u16 dat=1;

														 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																		//pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
																 {
																	 s16pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																 }
																else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
																 {
																		 pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																 }				
														
													}
											
											
											break;
										}	
										case 'G':
										{
                         u16 * u16pdat;
									       u32 * pdat;//= (s16*)pItem[m_coursItem].st_plink;
											
														if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
													 {
														 u16pdat=(u16*)pItem[m_coursItem].st_plink;
													 }
														else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
														{
																 pdat=(u32*)pItem[m_coursItem].st_plink;
														}		
											
																
											
											
											if(m_CoursLine.st_locate%2!=0)
												{    //整数部分
												 u32 dat=1;

																 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																
																	
															  	u8 tmp=0;

																		tmp=pItem[m_coursItem].dec;
																
																u16 times=1;
																for(u8 t=0;t<tmp;t++)
																	{
																		times*=10;//扩大小数位数的倍数
																	}
																dat=dat*times;
																	
																		//pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																	
														if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
														 {
															 u16pdat[(m_CoursLine.st_locate-1)/2]+=dat;
														 }
														else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
														 {
																 pdat[(m_CoursLine.st_locate-1)/2]+=dat;
														 }				
																	
																	
													}
											    else{//小数部分
														u16 dat=1;

														 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																		//pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
																 {
																	 u16pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																 }
																else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
																 {
																		 pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																 }				
														
													}
											
											
											break;
										}	
										case 'x':
										case 'l':
										{
											     u8 tmp ;
										//tmp = pItem[m_coursItem].st_Courslocat-1;
										
										    tmp =m_CoursLine.st_locate-1;
										       pItem[m_coursItem].st_pParalist[tmp]++;
										
										 u8***  pppdata;
										   pppdata=(u8***)pItem[m_coursItem].st_plink;//取得字符串列表指针，列表如 {{aa,bb,NULL},{cc,dd,NULL}}
						             
						           u8 ** ppdata;
										      ppdata=*(pppdata+tmp);//取得字符串列表项目指针，项目如{aa,bb}
										
						            u8 *  pdata;
										     pdata=ppdata[pItem[m_coursItem].st_pParalist[tmp]];//取得项目内容，如{aa}或{bb}
										
										if(pdata==NULL)//若项目内容为空表示计数已达最大值
										      pItem[m_coursItem].st_pParalist[tmp]=0;
											
											break;
										
									}

										
										
										default : break;
									} 
						 }
			
			
				  *m_pkey=KEYNONE;
		        break;
				
		 

				}
			case KEYDOWN: 
			{    gscreensleep=180;
				  
			    if(m_EnterStatus==0)
						 {  
							 
							
							 m_coursItem++;
							 
							 if(m_pItem[m_coursItem].st_pContext==NULL)
							 {
								 m_coursItem--;
								 
							 }
								 
							 
							 
							 if(m_coursItem==WINDOW_MAX_SIZE)//己达窗口最取大显示条目数，需进行滚屏设置
							 {
								  m_coursItem=WINDOW_MAX_SIZE-1;
								 m_head++;
								 if(m_pItem[m_head+WINDOW_MAX_SIZE-1].st_pContext==NULL)
								 { 
									 m_head--; //回减headItem，保持headItem不变
									 
								 }
							 }
							 
							 							 
						 }
				
							
						  if(m_EnterStatus==3)
						{
							    
							    switch(m_CoursLine.st_Paratype)
									{
										case 'd':
										 {
											    u16 * pdat= (u16*)pItem[m_coursItem].st_plink;
													u16 dat=1;


													 
											 	for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
													{
																	dat*=10;
													}
												 if((s16)pdat[m_CoursLine.st_locate-1]>0)
													pdat[m_CoursLine.st_locate-1]-=dat;
											 break;
									  }
										   
                     case 'o':
										{
											 u16 * pdat=(u16 *)pItem[m_coursItem].st_plink;
											      pdat[m_CoursLine.st_locate-1]--;
											break;
										}											
										case 'O':
										{
											 u8 * pdat=(u8 *)pItem[m_coursItem].st_plink;
											      pdat[m_CoursLine.st_locate-1]--;
											break;
										}	
										case 'D':
										{
											   u8 * pdat=(u8 *)pItem[m_coursItem].st_plink;
													u8 dat=1;
											


													for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
													{
																	dat*=10;
													}
                              // if((s8)pdat[m_CoursLine.st_locate-1]>0)
													pdat[m_CoursLine.st_locate-1]-=dat;
										     
											break;
									   }
										
										case 'f':
											 {
											 float * pdat= (float*)pItem[m_coursItem].st_plink;
											if(m_CoursLine.st_locate%2!=0)
												{    //整数部分
												 u16 dat=1;

														 	for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
													{
																	dat*=10;
													}
													
													       pdat[(m_CoursLine.st_locate-1)/2]-=dat;
														
													}
											    else{//小数部分
														float dat=1;
														
															for(u8 i=0;i<m_CoursLine.st_keyEnter+1;i++)
														{
															   dat/=10;
														}
														  pdat[(m_CoursLine.st_locate-1)/2]-=dat;
														
														
														
													}
											
											
											break;
										}
										case 'F':
										{
                         s16 * s16pdat;
									       s32 * pdat;//= (s16*)pItem[m_coursItem].st_plink;
											
													  if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
													 {
														 s16pdat=(s16*)pItem[m_coursItem].st_plink;
													 }
														else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
														{
																 pdat=(s32*)pItem[m_coursItem].st_plink;
														}										

											
																
											
											
											if(m_CoursLine.st_locate%2!=0)
												{    //整数部分
												 u32 dat=1;

																 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																
																	
															  	u8 tmp=0;

																		tmp=pItem[m_coursItem].dec;
																
																u16 times=1;
																for(u8 t=0;t<tmp;t++)
																	{
																		times*=10;//扩大小数位数的倍数
																	}
																dat=dat*times;
																	
																		//pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																	
														if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
														 {
															 s16pdat[(m_CoursLine.st_locate-1)/2]-=dat;
														 }
														else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
														 {
																 pdat[(m_CoursLine.st_locate-1)/2]-=dat;
														 }				
																	
																	
													}
											    else{//小数部分
														u16 dat=1;
																						
														 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																		//pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
																 {
																	 s16pdat[(m_CoursLine.st_locate-1)/2]-=dat;
																 }
																else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
																 {
																		 pdat[(m_CoursLine.st_locate-1)/2]-=dat;
																 }				
														
													}
											
											
											break;
										}	
										case 'G':
										{
                         u16 * u16pdat;
									       u32 * pdat;//= (s16*)pItem[m_coursItem].st_plink;
											
													  if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
													 {
														 u16pdat=(u16*)pItem[m_coursItem].st_plink;
													 }
														else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
														{
																 pdat=(u32*)pItem[m_coursItem].st_plink;
														}										

											
																
											
											
											if(m_CoursLine.st_locate%2!=0)
												{    //整数部分
												 u32 dat=1;

																 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																
																	
															  	u8 tmp=0;

																		tmp=pItem[m_coursItem].dec;
																
																u16 times=1;
																for(u8 t=0;t<tmp;t++)
																	{
																		times*=10;//扩大小数位数的倍数
																	}
																dat=dat*times;
																	
																		//pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																	
														if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
														 {
															 u16pdat[(m_CoursLine.st_locate-1)/2]-=dat;
														 }
														else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
														 {
																 pdat[(m_CoursLine.st_locate-1)/2]-=dat;
														 }				
																	
																	
													}
											    else{//小数部分
														u16 dat=1;
																						
														 for(u8 i=0;i<m_CoursLine.st_keyEnterCount-m_CoursLine.st_keyEnter-1 ;i++)
																{
																				dat*=10;
																}
																		//pdat[(m_CoursLine.st_locate-1)/2]+=dat;
																if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE)
																 {
																	 u16pdat[(m_CoursLine.st_locate-1)/2]-=dat;
																 }
																else if((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE)
																 {
																		 pdat[(m_CoursLine.st_locate-1)/2]-=dat;
																 }				
														
													}
											
											
											break;
										}	
										case 'x':
										case 'l':
										{      u8 tmp;
										        // tmp= pItem[m_coursItem].st_Courslocat-1;
											       tmp =m_CoursLine.st_locate-1;


											if(pItem[m_coursItem].st_pParalist[tmp]>0)
											{
										            pItem[m_coursItem].st_pParalist[tmp]--;
											}
											else
											{
												    u8***  pppdata;
										       pppdata=(u8***)pItem[m_coursItem].st_plink;//取得字符串列表指针，列表如 {{aa,bb,NULL},{cc,dd,NULL}}
						             
						              u8 ** ppdata;
										      ppdata=*(pppdata+tmp);//取得字符串列表项目指针，项目如{aa,bb}
										
						              u8 *  pdata;
										       
												
												s8 k=-1;
												while(pdata!=NULL)
												{
													   k++;
													pdata=ppdata[k];//取得项目内容，如{aa}或{bb}
												     
												}
												 pItem[m_coursItem].st_pParalist[tmp]=k-1;
											}
										
										
											break;
										}
										    default : break;
									}
								}
			 
				  *m_pkey=KEYNONE;
		        break;
							}		 
			case KEYENTER: 
			{    gscreensleep=180;                                                        

                 	if(m_EnterStatus==-1)
									{
										  m_EnterStatus++;
									}
               	  else if(m_EnterStatus==0)//0表示参数不可改，1表示跳到下一屏，2表一组参数可改，3表示单个参数可改
									{   
									    
										if((pItem[m_coursItem].st_CtrlType&PARA_MODIDI_OPTION)==NEXT_SCREEN)//触发跳到下一屏
										{
											if((pItem[m_coursItem].st_plink==&passWord)&&(delayPassWord==0))
											{
												//密码界面
												  	Item ** ppItem=	& pPASSWORD;
												     		passWordToChange=(Item **)pItem[m_coursItem].st_pParalist;
												//	if( setMenu(ppItem)!=0)
												    if((m_pfun(ppItem))!=0)
														{																			
																	Item * tmpItem=	* ppItem;			 						
																		if(m_LayerCount<SCREEN_MAX_LAYER)	
																		{								
																				 m_layer[m_LayerCount].st_coursItem=m_coursItem;//保存当前层的光标所在行
																					 m_layer[m_LayerCount].st_LayerHead=m_head;	//保存当前层的首条项目的位置	 
																						m_LayerCount++;

																						 m_layer[m_LayerCount].st_ppPreItem=m_layer[m_LayerCount-1].st_ppItem;//将保存上一屏指针地址，用以恢复 
																						 m_layer[m_LayerCount].st_ppItem=ppItem;//保存本次更新屏的指针地址
																							 reSet(tmpItem,0);//切换到下一层指针
																							 
																		}
														}	
												
											}
											else
											{//非密码界面

													Item ** ppItem= (Item **)pItem[m_coursItem].st_pParalist;
													
											
													
														//if( setMenu(ppItem)!=0)
												    if((m_pfun(ppItem))!=0)
														{																			
																	Item * tmpItem=	* ppItem;			 						
																		if(m_LayerCount<SCREEN_MAX_LAYER)	
																		{								
																				 m_layer[m_LayerCount].st_coursItem=m_coursItem;//保存当前层的光标所在行
																					 m_layer[m_LayerCount].st_LayerHead=m_head;	//保存当前层的首条项目的位置	 
																						m_LayerCount++;

																						 m_layer[m_LayerCount].st_ppPreItem=m_layer[m_LayerCount-1].st_ppItem;//将保存上一屏指针地址，用以恢复 
																						 m_layer[m_LayerCount].st_ppItem=ppItem;//保存本次更新屏的指针地址
																							 reSet(tmpItem,0);//切换到下一层指针
																							 
																		}
														}		
											}												
											
										}
									 else if(m_layer[m_LayerCount].st_ppItem==m_FirstItem)//进入菜单界面
									 {
										 
									        reSet(NULL,1);//切换到上一层指针
									        genterflag=0;	//返回时将enterflag清零
									 }
									 else if((pItem[m_coursItem].st_CtrlType&PARA_MODIDI_OPTION)==MODIFI_PARA)//进入数字参数编辑状态
									 {
											m_EnterStatus=3;
									 }	
									 else
									 {
										 if((pItem[m_coursItem].st_CtrlType&PARA_MODIDI_OPTION)!=NONE_DO)
									   m_EnterStatus++;
									 }	
										
									}
									else if(m_EnterStatus==3)//数字参数编辑状态
									{
										
										
										
										m_CoursLine.st_keyEnter++;
										if(m_CoursLine.st_Paratype=='l')
										{
											
											    if(m_CoursLine.st_keyEnter==m_CoursLine.st_keyEnterCount)
												 {
													    m_CoursLine.st_locate++;
													    if(m_CoursLine.st_locate>m_CoursLine.st_locateCount)
															{
																        m_EnterStatus=0;
                                         genterflag=1;	
																        m_CoursLine.st_locate=1;
															}
												 }
											  
;	
										}
										else
										{	
												 if(m_CoursLine.st_keyEnter==m_CoursLine.st_keyEnterCount)
												 {
																	 m_CoursLine.st_keyEnter=0;
													 
													 
													        if(m_CoursLine.st_Paratype=='x')//码密数据处理
																 {
																	  if(m_CoursLine.st_locate>0)
																		{
																		 password[m_CoursLine.st_locate-1]=passwordshow[m_CoursLine.st_locate-1];
																			passwordshow[m_CoursLine.st_locate-1]=11;//将显示值改为"*"
																		}
																 }
													 
													 
																	 m_CoursLine.st_locate++;
																 if(m_CoursLine.st_locate>m_CoursLine.st_locateCount)
																 {
																	 m_CoursLine.st_locate=1;
																	 ///////参数确定，将缓存数据写具体位置///////////////////////
																			 if(pItem[m_coursItem].st_pParalist!=NULL)
																		{
																			if((m_CoursLine.st_Paratype=='D')||(m_CoursLine.st_Paratype=='O'))//处于理单字节数据
																				{
																					u8 * plinkPara=(u8 *)pItem[m_coursItem].st_plink;	
																					u8 * pParalistPara=(u8 *)pItem[m_coursItem].st_pParalist;		
																					
																					for(u8 i=0;i<m_CoursLine.st_locateCount;i++)
																						 pParalistPara[i]=plinkPara[i];
																			
																				}
																			else if (m_CoursLine.st_Paratype=='d')//处理双字节数据
																						 {
																								u16 * plinkPara=(u16 *)pItem[m_coursItem].st_plink;	
																								u16 * pParalistPara=(u16 *)pItem[m_coursItem].st_pParalist;		
																								
																								for(u8 i=0;i<m_CoursLine.st_locateCount;i++)
																									 pParalistPara[i]=plinkPara[i];
																								
																						 }
																			else if( //处理双字节数据
																								 ((m_CoursLine.st_Paratype=='F')&&((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE))||
																			           ((m_CoursLine.st_Paratype=='G')&&((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_TWO_BYTE))
																							)
																							{
																								u16 * plinkPara=(u16 *)pItem[m_coursItem].st_plink;	
																								u16 * pParalistPara=(u16 *)pItem[m_coursItem].st_pParalist;		
																								
																								for(u8 i=0;i<m_CoursLine.st_locateCount/2;i++)//浮点数要看成单个数据处理
																									 pParalistPara[i]=plinkPara[i];
																							}	
																			else if(//处理四字节数据
																									(m_CoursLine.st_Paratype=='f')||
																								 ((m_CoursLine.st_Paratype=='F')&&((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE))||
																			           ((m_CoursLine.st_Paratype=='G')&&((pItem[m_coursItem].st_CtrlType&SCREEN_FOUR_BYTE)==SCREEN_FOUR_BYTE))
																			        
																						 )
																						{
																								u32 * plinkPara=(u32 *)pItem[m_coursItem].st_plink;	
																								u32 * pParalistPara=(u32 *)pItem[m_coursItem].st_pParalist;		
																								
																								for(u8 i=0;i<m_CoursLine.st_locateCount/2;i++)//浮点数要看成单个数据处理
																									 pParalistPara[i]=plinkPara[i];
																							
																						}
																			}
																	 
																		 genterflag=1;	
																		m_EnterStatus=0;
																 }
											 
										             	
												 }
										
 
									 }
									}
									
									
					








          *m_pkey=KEYNONE;			 
					
						 break;



					
				}
			case KEYESC:
			{	   gscreensleep=180;    
								 
								
				        if(m_EnterStatus==0)
								{
								   if(m_layer[m_LayerCount].st_ppItem==m_FirstItem)
									 { ctrl_bee&=0xFD;}//蜂鸣器静音
									 else
									 {m_EnterStatus--;	}
								}
								else  if(m_EnterStatus==3)
								{
									  
									   m_CoursLine.st_keyEnter--;
									   if(m_CoursLine.st_keyEnter>0x0E)
										 {
											 
											 
											    m_CoursLine.st_keyEnter=0;
											   if(m_CoursLine.st_locate>=1)
												 {
													 m_CoursLine.st_locate--;
													 if(m_CoursLine.st_locate==0)
													 {
														  m_EnterStatus=0;
														  m_CoursLine.st_keyEnter=0;
									            m_CoursLine.st_locate=1;
														 
													 }
												 }
										 }
								}
								
								 if((m_EnterStatus==-1)&&(m_LayerCount>0))
								{			
									 
									 
									  reSet(NULL,1);//切换到上一层指针
									 genterflag=0;	//返回时将enterflag清零
                              
											
										
							  }
								
								if((m_EnterStatus==-1)&&(m_LayerCount==0))//在菜单界面按退出时的情况
								{
									//进入到菜单界面
									
									   m_layer[m_LayerCount].st_coursItem=m_coursItem;//保存当前层的光标所在行
										 m_layer[m_LayerCount].st_LayerHead=m_head;	//保存当前层的首条项目的位置	 
										 m_LayerCount++;
									   m_layer[m_LayerCount].st_ppItem=m_FirstItem;;
										 genterflag=0;	//返回时将enterflag清零
									       // if( setMenu(m_FirstItem)!=0)
									      if((m_pfun(m_FirstItem))!=0)
												{				
 													   
												 			 Item * tmpItem=	*m_FirstItem;
													     reSet(tmpItem,0);//进入到菜单界面
												}													
									
									  
					
									
								}
								
							
	
								
							
											 
				 *m_pkey=KEYNONE;
		        break;
					}
			
			default :break;
		   
	 }
}
	


Screen * Screen::HMI(void)
{ 
	Screen *pScreen=this;
	u8 ItemlienCtr=0,coursMode;
	Item *pItem=&m_pItem[m_head];

 
////////////////////////清屏/////////////////////////////////////////////
{	
	//clrScreen();
	 m_poled->clrScreen();
	
}
////////////////////////按键处理/////////////////////////////////////////////	

	DealWithKey(pItem); 

//////////////////光标闪烁控制//////////////////////////////			 
		  
{	 

	 if(m_EnterStatus==3)
	 {
	    m_flash^=0x01;
	 }
	 else
	 {
		 m_flash=1;
	 
	 }

	 
 }

 
 


   if(m_SreenChange==1)
	 {
		 pItem=&m_pItem[m_head];
		 m_SreenChange=0;
	 }
	 
	Item *   disp_pItem=pItem;
	for(u8 i=0;i<WINDOW_MAX_SIZE;i++)
	 {
		      if(disp_pItem->st_pContext==NULL) 
						break;
////////////////光标显示模式///////////////////////////////////////////		 
		 
		   if(m_coursItem==i)
				{
					if(m_EnterStatus==-1)
					{
						
						coursMode=0;
					}
					if(m_EnterStatus==0)
					{
						//OLED_Fill(0,ItemlienCtr,127,ItemlienCtr+12,1);//整行取反
						m_poled->Fill(0,ItemlienCtr,127,ItemlienCtr+12,1);//整行取反
						coursMode=1;
					}
					if(m_EnterStatus==1)
					{
						coursMode=2;
					}
						if((m_EnterStatus==2)||(m_EnterStatus==3))
					{
						 if((m_CoursLine.st_Paratype=='l')||(m_CoursLine.st_Paratype=='x')||(m_CoursLine.st_Paratype=='O')||(m_CoursLine.st_Paratype=='o'))
						 {  coursMode=2;}
						 else
						 {    coursMode=3;}
					}
					

					
				
				}
				else{
					coursMode=0;
				}
////////////////更新显示内容///////////////////////////////////////////		 
		   			
				 print(0,ItemlienCtr ,coursMode, m_flash,disp_pItem,&m_CoursLine);
				

				         if(m_CoursLine.st_keyEnterCount==m_CoursLine.st_keyEnter)//参数位数不能等于参数位位置号（由于数据位减操作导致他们相等）
									{
													 if(m_CoursLine.st_keyEnterCount>0)
													  m_CoursLine.st_keyEnter--;
									 }
									

		                                  
				ItemlienCtr=ItemlienCtr+16;
		    disp_pItem++;
				
					
		}
	
	
	    return pScreen;
	
}


void Screen::FreshGram()
{
	 pScreen->m_poled->Refresh_Gram();
}

void Screen::ClearScreen()
{
	 pScreen->m_poled->clrScreen();
}


