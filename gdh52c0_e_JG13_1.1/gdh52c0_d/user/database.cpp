#include "DATABASE.h"




DATABASE::DATABASE()
{
}
DATABASE::~DATABASE()
{
}



////////////////////////////////////////////////////
//删除某一数据类型的所有数据，成功则返回所删除的数据类型
//无数据删除则返回-1
////////////////////////////////////////////////////

s8 DATABASE::DeleteData(u8 Datatype)
{
	 if(m_EquUnit.st_pItemhead!=NULL)
		{
			DataItem *pItem=m_EquUnit.st_pItemhead;
			   while(pItem!=NULL)
				 {
					   if(pItem->st_Name==Datatype)
						 {
							 
							   if((pItem->st_pPreItem!=NULL)&&(pItem->st_pNextItem!=NULL))
								 {
									   pItem->st_pPreItem->st_pNextItem=pItem->st_pNextItem;//删除中间
									   pItem->st_pNextItem->st_pPreItem=pItem->st_pPreItem;
								 }else if((pItem->st_pPreItem!=NULL)&&(pItem->st_pNextItem==NULL))
								 {
									 pItem->st_pPreItem->st_pNextItem=NULL;//删除尾
								 }else if((pItem->st_pPreItem==NULL)&&(pItem->st_pNextItem!=NULL))
							   {
									 pItem->st_pNextItem->st_pPreItem=NULL;//删除头
									 m_EquUnit.st_pItemhead=pItem->st_pNextItem;
								 }else if((pItem->st_pPreItem==NULL)&&(pItem->st_pNextItem==NULL))
								 {
									  m_EquUnit.st_pItemhead=NULL;  //只有头数据的情况
								 }
							 
								   vPortFree(pItem->st_pData);
								   vPortFree(pItem);
								 
								
								 
							  return Datatype;
							 
						 }
						pItem=pItem->st_pNextItem; 
				 }
			
			    
	  }
		 return -1;
}


void DATABASE::Init(u8 Eqtype,u8 maxlen)
{
	 m_EquUnit.st_Name=Eqtype;
	 m_EquUnit.st_MaxLen=maxlen;
	
}


bool  DATABASE::Append(u8 Datatype)
{
	
	  DataItem *pPer=NULL;  
	  u8 Typelen;
	

	
if(
	  (Datatype==VOLTAGE)||(Datatype==CURRENT)||
    (Datatype==USERCURRENT1)||(Datatype==USERCURRENT2)||
	  (Datatype==USERCURRENT3)||(Datatype==LIMITCURRENT)||
     (Datatype==VOLTAGE_AC)||(Datatype==CURRENT_AC)||
             (Datatype==FREQUENCY)||(Datatype==TEMP)||
 
    (Datatype==POWER)||(Datatype==STATUS)||(Datatype==WRANNING16)
	 
  )
	{
		Typelen=TWO_BYTE;
	}
	else if(  (Datatype==POWER)||(Datatype==STATUS)||  
		      (Datatype==ENERGY1)||(Datatype==ENERGY2)||
	        (Datatype==ENERGY3)||(Datatype==ENERGY4)
	
		     )
	{
		Typelen=FOUR_BYTE;
	}
	else if((Datatype==WRANNING)||
	       (Datatype==MODULETYPE)||
	       (Datatype==PHASEPOSITION)||
	       (Datatype==ONOFF)||
	       (Datatype==LIMIT_CURR_STATUS)|| 
	       (Datatype==MODULE_RATE_I)||
	       (Datatype==MODULE_EFFICIEN)
	
	
	)
	{
		Typelen=ONE_BYTE;
	}
		
	
	
	
	
	 if(m_EquUnit.st_pItemhead!=NULL)
		{
			DataItem *pItem=m_EquUnit.st_pItemhead;
			   while(pItem->st_pNextItem!=NULL)
				 {
					 
						pItem=pItem->st_pNextItem; 
				 }
				 
				    pItem->st_pNextItem=(DataItem *)pvPortMalloc (sizeof(DataItem));
		
				if( pItem->st_pNextItem!=NULL)
				{
					 pItem->st_pNextItem->st_Name=Datatype;
					 pItem->st_pNextItem->st_MaxLen=m_EquUnit.st_MaxLen*Typelen;		
					 pItem->st_pNextItem->st_pData=(u8 *)pvPortMalloc ( pItem->st_pNextItem->st_MaxLen*sizeof(u8));
					 if(pItem->st_pNextItem->st_pData==NULL)
					 {
						  vPortFree( pItem->st_pNextItem);
						 pItem->st_pNextItem=NULL;
						 return false;
					 }						 
					 
					 pItem->st_pNextItem->st_pNextItem=NULL;
					 pItem->st_pNextItem->st_pPreItem=pItem;
				}
				else{
					    return false;
				}
			    
	  }
		else{//第一次新建
			  
			    
		      m_EquUnit.st_pItemhead=(DataItem *)pvPortMalloc (sizeof(DataItem));
		
				if(m_EquUnit.st_pItemhead!=NULL)
				{
					m_EquUnit.st_pItemhead->st_Name=Datatype;
					m_EquUnit.st_pItemhead->st_MaxLen=m_EquUnit.st_MaxLen*Typelen;		
					m_EquUnit.st_pItemhead->st_pData=(u8 *)pvPortMalloc ((m_EquUnit.st_pItemhead->st_MaxLen)*sizeof(u8));
					 if(m_EquUnit.st_pItemhead->st_pData==NULL)
					 {
						  vPortFree( m_EquUnit.st_pItemhead);
						  m_EquUnit.st_pItemhead=NULL;
						 return false;
					 }			
					m_EquUnit.st_pItemhead->st_pNextItem=NULL;
					m_EquUnit.st_pItemhead->st_pPreItem=pPer;
				}
				else{
					return false;
				}
		}
		return true;
}

void *DATABASE::GetDataAddr(u8 Datatype)
{
	 
			DataItem *pItem=m_EquUnit.st_pItemhead;
			   while(pItem!=NULL)
				 {
					   if(pItem->st_Name==Datatype)
						 {
							 
								 
							  return pItem->st_pData;
							 
						 }
						pItem=pItem->st_pNextItem; 
				 }
			
			return NULL;    
	
}


//无符号整型16位  
u16 bswap_16(u16 x)  
{  
    return (((u16)(x) & 0x00ff) << 8) | \
           (((u16)(x) & 0xff00) >> 8) ;  
}  
 
//无符号整型32位
u32 bswap_32(u32 x)  
{  
    return (((u32)(x) & 0xff000000) >> 24) | \
           (((u32)(x) & 0x00ff0000) >> 8) | \
           (((u32)(x) & 0x0000ff00) << 8) | \
           (((u32)(x) & 0x000000ff) << 24) ;  
} 



