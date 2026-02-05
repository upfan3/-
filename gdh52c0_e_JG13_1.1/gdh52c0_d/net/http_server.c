#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "w5500.h"
#include "socket.h"
#include "http_server.h"
#include "utility.h"
//#include "webpage.h"

#include "FreeRTOS.h"
#include "task.h"
//#include "filebase.h"



//收到数据不在0~9或A~F以内，返回0，表示收到数据无效
u8 httpAsciitoHex(u8 *op,u8* pdat)
{ u8 i=2,tmp;
	 
	  if((*pdat<0x30)||(*pdat==0x40)||(*pdat>0x46)) return 0;
	    if((*(pdat+1)<0x30)||(*(pdat+1)==0x40)||(*(pdat+1)>0x46)) return 0;
	     *op=0;
	while(i--)
	{
	    if( *pdat<0x40)
			{
				  tmp=*pdat-0x30;
				
			}
			if(*pdat>0x40)
			{
				tmp=*pdat-0x37;
			}
			   pdat++;
			
			  *op=*op|(tmp<<(i*4));
		}
	  return 1;
}
 


_RX_STR *gprxinfo=NULL;
//_RX_STR *gptxinfo=NULL;
st_http_request *gphttp_request=NULL;	/*定义http请求报文头的结构体指针*/

/*-------------------------------------------------------------------------*/
/*	转化转义字符为ascii charater 						 -----*/
/*   参数url：需要转化网页地址                                     */
/*------------------------------------------------------------------------*/
void unescape_http_url(char * url)
{
  int x, y;
  
  for (x = 0, y = 0; url[y]; ++x, ++y) 
  {
    if ((url[x] = url[y]) == '%') 
	{
      url[x] = c2d(url[y+1])*0x10+c2d(url[y+2]);
      y+=2;
    }
  }
  url[x] = '\0';
}
/*-------------------------------------------------------------------------*/
/*   执行一个答复，如 html, gif, jpeg,etc.				  -----*/
/*   参数buf- 答复数据 type- 答复数据类型 len-  答复数据长度     */
/*------------------------------------------------------------------------*/
void make_http_response_head(unsigned char * buf,char type,uint32 len)
{
  char * head;
  char tmp[10];
  memset(buf,0x00,MAX_URI_SIZE); 
  /* 文件类型*/
  if 	(type == PTYPE_HTML) head = RES_HTMLHEAD_OK;
  else if (type == PTYPE_GIF)	head = RES_GIFHEAD_OK;
  else if (type == PTYPE_TEXT)	head = RES_TEXTHEAD_OK;
  else if (type == PTYPE_JPEG)	head = RES_JPEGHEAD_OK;
  else if (type == PTYPE_FLASH)	head = RES_FLASHHEAD_OK;
  else if (type == PTYPE_MPEG)	head = RES_MPEGHEAD_OK;
  else if (type == PTYPE_PDF)	head = RES_PDFHEAD_OK;
  else if (type == PTYPE_PDF)	head = RES_PDFHEAD_OK;
	else if (type == PTYPE_JSON)	head = RES_JSONHEAD_OK;
	else if (type == PTYPE_JS)	head = RES_JAVASCRIPT_OK;
	else if (type == PTYPE_CSS)	head = RES_CSS_OK;
	else if (type == PTYPE_XML) 	head = RES_XML_OK;
	
  sprintf(tmp,"%ld", len);	
  strcpy((char*)buf, head);
  strcat((char*)buf, tmp);
  strcat((char*)buf, "\r\n\r\n");
  //printf("%s\r\n", buf);
}
 /*-------------------------------------------------------------------------*/
 /*  寻找一个MIME类型文件 			                               -----*/
 /*   参数buf- MIME型文件 type-数据类型	                                   */
 /*------------------------------------------------------------------------*/
void find_http_uri_type(u_char * type, char * buf) 
{
  /* Decide type according to extention*/
  if 	(strstr(buf, ".pl"))				*type = PTYPE_PL;
  else if (strstr(buf, ".html") || strstr(buf,".htm"))	*type = PTYPE_HTML;
  else if (strstr(buf, ".gif"))				*type = PTYPE_GIF;
  else if (strstr(buf, ".text") || strstr(buf,".txt"))	*type = PTYPE_TEXT;
  else if (strstr(buf, ".jpeg") || strstr(buf,".jpg"))	*type = PTYPE_JPEG;
  else if (strstr(buf, ".swf")) 				*type = PTYPE_FLASH;
  else if (strstr(buf, ".mpeg") || strstr(buf,".mpg"))	*type = PTYPE_MPEG;
  else if (strstr(buf, ".pdf")) 				*type = PTYPE_PDF;
  else if (strstr(buf, ".cgi") || strstr(buf,".CGI"))	*type = PTYPE_CGI;
  else if (strstr(buf, ".js") || strstr(buf,".JS"))	*type = PTYPE_TEXT;	
  else if (strstr(buf, ".xml") || strstr(buf,".XML"))	*type = PTYPE_HTML;
  else 							*type = PTYPE_ERR;
}
 /*-------------------------------------------------------------------------*/
 /*   解析每一个http响应			                               -----*/
 /*   参数request： 定义一个指针                                   */
 /*------------------------------------------------------------------------*/ 
void parse_http_request(st_http_request * request,u_char * buf)
{
  char * nexttok;
	char * gflagend;
	u8 strlenth;
	//request=(st_http_request *)buf;
  nexttok = strtok((char*)buf," ");
  if(!nexttok)
  {
    request->METHOD = METHOD_ERR;
    return;
  }
  if(!strcmp(nexttok, "GET") || !strcmp(nexttok,"get"))
  {
    request->METHOD = METHOD_GET;
	  	nexttok = strtok(NULL," ");
   			
  }
  else if (!strcmp(nexttok, "HEAD") || !strcmp(nexttok,"head"))	
  {
    request->METHOD = METHOD_HEAD;
    nexttok = strtok(NULL," ");
  		
  }
  else if (!strcmp(nexttok, "POST") || !strcmp(nexttok,"post"))
  {
   
    
    request->METHOD = METHOD_POST;
 		 nexttok = strtok(NULL," ");
  }
  else
  {
    request->METHOD = METHOD_ERR;
  }	 
  if(!nexttok)
  {
    request->METHOD = METHOD_ERR; 			
    return;
  }
  //strcpy(request->FileNAME,nexttok); 	
    gflagend=strstr(nexttok,"?");
    strlenth=strlen(nexttok);
	   if(gflagend!=NULL)
	   strlenth=strlenth-strlen(gflagend);
	  //memset (request->FileNAME,0,16);
	  memcpy(request->FileNAME,nexttok,strlenth);
   request->FileNAME[strlenth]=0x00;
		  request->pdata=nexttok+strlenth+1;
	
}

////////////////////////////////////////
//分拆post数据//////
//1取出CID1及CID2
//2若数据参数不超过7字节（原始数据为14字）以CMD数组传出,返回来NULL
//3若数据参数超过7字节（原始数据为14字）伸请内存空间返回数据指针
//4若数据参数超过7字节（原始数据为14字），但数据有效也返回NULL
////////////////////////////////////////
unsigned char *get_post_1363(char *p,unsigned char * cmd,unsigned char n)
{unsigned char tmp_buf[3];
	uint16 len;
	unsigned char * ptmpdata=NULL;
	for(u8 i=0 ;i<n;i++)
	{
	    char *p1=strstr(p,"=");
	    char *p2=strstr(p,"&");
	    len=p2-p1-1;
		if(i<2) //cid1,cid2为十进制数处理
		{
		  memset(tmp_buf,0,3);
		  memcpy(tmp_buf,p1+1,len);
		  cmd[i]=ATOI((char *)tmp_buf,10);
		}
		else// data 按十六进制处理
		{ 
			
			  if(len<=14)
				{
					for(u8 j=0;j<len/2;j++)
					{
						 memset(tmp_buf,0,3);
						 memcpy(tmp_buf,p1+1+j*2,2);
						 httpAsciitoHex(&cmd[i+j],&tmp_buf[0]);
						
					}
				}
				else
				{
					 ptmpdata=(unsigned char *)pvPortMalloc(sizeof(unsigned char)*len/2);
					 if(ptmpdata!=NULL)
					 {
						 
						 for(u8 j=0;j<len/2;j++)
							{
								 memset(tmp_buf,0,3);
								 memcpy(tmp_buf,p1+1+j*2,2);
								if( httpAsciitoHex(&ptmpdata[j],&tmp_buf[0])==0){ 
									   vPortFree(ptmpdata);
									return NULL;//若解释出错，直接仍丢
								}
								
							}
					 }
				}
		}
		p=p2+1;
	}
	return  ptmpdata;
}



 /*-------------------------------------------------------------------------*/
 /*   得到响应过程中的下一个参数		                               -----*/
 /*   参数url：需要转化网页地址                                   */
 /*------------------------------------------------------------------------*/  


char get_http_param_value(char* uri,char ** ppuri)
{
	uint16 len;
	uint8* pos2;
	uint8* name=0; 
	
	uint16 content_len=0;
	int8 tmp_buf[10]={0x00,};
	if(!uri ) return 0;
	/***************/
	mid(uri,"Content-Length: ","\r\n",tmp_buf);
	content_len=ATOI(tmp_buf,10);
	uri = (int8*)strstr(uri,"\r\n\r\n");
	uri+=4;
	//printf("uri=%s\r\n",uri);
	uri[content_len]='&';//增加一个结束标志，以便后面字符提取
	*ppuri=uri;
   len=strlen(uri);
	if(content_len==(len-1))//比较实际收到数据长度与预期数据长度，是否一致
	/***************/	 
	 return 1;
	else
	 return 0;
}



///////////////////////////////////////httputil//////////////////////////////

extern CONFIG_MSG  ConfigMsg;
char tx_buf[MAX_URI_SIZE];

void make_json_data( char * pjsondata,u8 cid1,u8 cid2,u8 *pdat,u8 type)
{//char a[5]={2,3,4,5,6};
  memset(pjsondata,0x00,MAX_URI_SIZE); 

	//sprintf(pjsondata,"{\"V1\":%d, \"I1\":%d,\"I2\":%d,\"W2\":%d,\"W1\":%d}",a[0],a[1],a[2],a[3],a[4]);
	if(type==0)
   sprintf(pjsondata,"{\"cid1\":%d, \"cid2\":%d,\"data\":\"%s\"}",cid1,cid2,pdat);
	else
	  sprintf(pjsondata,"{\"cid1\":%d, \"cid2\":%d,\"err\":\"%d\"}",cid1,cid2,*pdat);	
}



//char *senttmpbuff[10]={0,0,0,0,0,0,0,0,0,0};
u8 *ptxbuff=NULL;
//u8 writebuffCount=0;//写计数
//u8 readbuffCount=0;//读计数
//u8 entyflag=1;//空标志
/*-------------------------------------------------------------------------*/
/*	 发送http						  -----*/
/*------------------------------------------------------------------------*/    
void proc_sent(SOCKET s,const char *sentbuff,uint16 lenth)
{
	
	unsigned long file_len=0;														
	uint16 send_len=0,len;
	send_len=0;
	

//	ptxbuff=(u8 *)pvPortMalloc(sizeof(u8)*1025);
//	 ptxbuff[1024]=0;
	gprxinfo->rx_buff[1024]=0;
	file_len = strlen((const char*)sentbuff);
	if(lenth>0)
		file_len=lenth;
	while(file_len)
	{
		if(file_len>1024)
		{
			if(getSn_SR(s)!=SOCK_ESTABLISHED)
			{
				//vPortFree(ptxbuff);
				return;
			}
			memcpy(gprxinfo->rx_buff,(uint8 *)sentbuff+send_len,1024);
			send(s,gprxinfo->rx_buff, 1024);
			
			send_len+=1024;
			file_len-=1024;
		}
		else
		{
			memcpy(gprxinfo->rx_buff,(uint8 *)sentbuff+send_len,file_len);						
			gprxinfo->rx_buff[file_len]=0;	
			send(s, gprxinfo->rx_buff, file_len);
			send_len+=file_len;
			file_len-=file_len;
		} 
	}



   // vPortFree(ptxbuff);


}


u8 http_proc_send(SOCKET s)
{
	
//	static  u8 isbusy=0;
//	static  uint16 send_len=0,len;
//	static  unsigned long file_len=0;
//	
//	static  char *sentbuff_=NULL;
//		
//	
//	
//	if(entyflag==1) return entyflag;
//	/*定义http请求报文头的结构体指针*/
//	
//	ptxbuff=(u8 *)pvPortMalloc(sizeof(u8)*1025);
//	
//	
//	if(isbusy==0)
//	{
//	   send_len=0;
//	 
//		   ptxbuff[1024]=0;
//		  sentbuff_=senttmpbuff[readbuffCount++];
//	   file_len = strlen((const char*)sentbuff_);
//		 if(readbuffCount==10) readbuffCount=0;
//		 isbusy=1;
//	}
//	else if(isbusy==1)
//	{		
//		 if(file_len>1024)
//		{
//			if(getSn_SR(s)!=SOCK_ESTABLISHED)
//			{
//				 vPortFree(ptxbuff);
//				return 1;
//			}
//			memcpy(ptxbuff,(uint8 *)sentbuff_+send_len,1024);
//			send(s, ptxbuff, 1024);
//			
//			send_len+=1024;
//			file_len-=1024;
//		}
//		else
//		{
//			memcpy(ptxbuff,(uint8 *)sentbuff_+send_len,file_len);						
//			ptxbuff[file_len]=0;	
//			send(s, ptxbuff, file_len);
//			send_len+=file_len;
//			file_len-=file_len;
//			isbusy=0;
//			if(readbuffCount==writebuffCount) entyflag=1;
//			
//			
//		} 
//		
//	}
//	
//	
//	
//	 vPortFree(ptxbuff);
//	return  entyflag;
	
	
}






/*-------------------------------------------------------------------------*/
/*	 接收http请求报文并发送http响应							  -----*/
/*   参数s: http服务器socket  buf：解析报文内容                           */
/*------------------------------------------------------------------------*/    

void proc_http(SOCKET s, uint8 * buf,PTRFUN_PCH pFun)
{
	int8* context; 											
	  int8 req_name[10]={0x00,};		/*定义一个http响应报文的指针*/
    int8 req_name1[10]={0x00,};															
	
	unsigned long file_len=0;															

	uint8* http_response;            /*定义一个http响应报文的指针*/

	st_http_request *phttp_request=gphttp_request;
	http_response=(uint8*)tx_buf;	

	parse_http_request(phttp_request, buf);    							/*解析http请求报文头*/
	  
	switch (phttp_request->METHOD)		
    {
		case METHOD_ERR :																			/*请求报文头错误*/
			memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
			//send(s, (uint8 *)http_response, strlen((int8 const*)http_response));
			break;
		
		case METHOD_HEAD:			/*HEAD请求方式*/
			
		case METHOD_GET:			/*GET请求方式*/
		    
			//context = http_request->FileNAME;
		
			if(strcmp(phttp_request->FileNAME,"/index.htm")==0 || strcmp(phttp_request->FileNAME,"/")==0 || (strcmp(phttp_request->FileNAME,"/index.html")==0))
			{  //testconut[0]=1;
			
				file_len = strlen((const char *)PAGE_DEFAULT);		
				//file_len=PAGE_CSS-PAGE_DEFAULT;
				make_http_response_head((uint8*)http_response, PTYPE_HTML,file_len);			
				send(s,http_response,strlen((char const*)http_response));
				proc_sent(s,(const char *)PAGE_DEFAULT,0);
				
				
				
			}
			/*
			else if(strcmp(phttp_request->FileNAME,"/Logo.png")==0)
			{ //testconut[1]=1;
		
				
						file_len = 0x13ee;			
						make_http_response_head((uint8*)http_response, PTYPE_JPEG,file_len);			
						send(s,http_response,strlen((char const*)http_response));
						proc_sent(s,(const char *)PAGE_LOGO,file_len);
			}*/
			else if(strcmp(phttp_request->FileNAME,"/mnc.png")==0)
			{ //testconut[1]=1;
		
				
						file_len = MNCLOGO_LEN;			
						make_http_response_head((uint8*)http_response, PTYPE_JPEG,file_len);			
						send(s,http_response,strlen((char const*)http_response));
						proc_sent(s,(const char *)LOGO_MNC,file_len);
			}
			else if(strcmp(phttp_request->FileNAME,"/power.png")==0)
			{ //testconut[1]=1;
		
				
						file_len =PWRLOGO_LEN;			
						make_http_response_head((uint8*)http_response, PTYPE_JPEG,file_len);			
						send(s,http_response,strlen((char const*)http_response));
						proc_sent(s,(const char *)LOGO_POWER,file_len);
			}
			
			else if(strcmp(phttp_request->FileNAME,"/Default.css")==0)
			{ //testconut[2]=1;
				  file_len = strlen((const char *)PAGE_CSS);	
				//file_len=PTYPE_JS-PAGE_CSS;
				make_http_response_head((uint8*)http_response, PTYPE_CSS,file_len);			
				send(s,http_response,strlen((char const*)http_response));
				proc_sent(s,(const char *)PAGE_CSS,0);
			}
			else if(strcmp(phttp_request->FileNAME,"/YDT1363.js")==0)
			{ //testconut[3]=1;
				file_len = strlen((const char *)PAGE_1363_JS);	
          
        //  file_len=PAGE_WEB_JS-PTYPE_JS;				
				make_http_response_head((uint8*)http_response, PTYPE_JS,file_len);			
				send(s,http_response,strlen((char const*)http_response));
				proc_sent(s,(const char *)PAGE_1363_JS,0);
				

	
			}
			else if(strcmp(phttp_request->FileNAME,"/TABLE.js")==0)
			{ //testconut[4]=1;
				  file_len = strlen((const char *)PAGE_WEB_JS);	
			//	file_len=PAGE_EQM-PAGE_WEB_JS;
				make_http_response_head((uint8*)http_response, PTYPE_JS,file_len);			
				send(s,http_response,strlen((char const*)http_response));
				proc_sent(s,(const char *)PAGE_WEB_JS,0);
				

	
			}
				else if(strcmp(phttp_request->FileNAME,"/H52C0.XML")==0)
			{// testconut[5]=1;
		
				
        file_len = strlen((const char *)PAGE_EQM);			
				make_http_response_head((uint8*)http_response, PTYPE_XML,file_len);			
				send(s,http_response,strlen((char const*)http_response));
				proc_sent(s,(const char *)PAGE_EQM,0);
			}


			break;
			
		case METHOD_POST:		/*POST请求*/
			
		    if(pFun==NULL)	
         return;		
		  
			if(strcmp(phttp_request->FileNAME,"/Default.htm")==0)	
			{ unsigned char cmd[10];
				u8 ttemp=get_http_param_value((char *)phttp_request->pdata,&phttp_request->pdata);
				if(ttemp==0)
				{
					return;
				}
				unsigned char *pdat=get_post_1363(phttp_request->pdata,cmd,3);
				     u8 port=3;
				     u8 * pdatainfo= (u8*)pFun(cmd,pdat,&port);
				
				    if(pdat!=NULL)
						 {
							  vPortFree(pdat);//释放由get_post_1363产生的内存空间
						 }
		
				    if(pdatainfo!=NULL)
						{
							 make_json_data( (char *)gprxinfo->rx_buff,cmd[0],cmd[1],pdatainfo,0);//返回数据
							   vPortFree(pdatainfo);	//释放 pdatainfo的内存，	该内存由pFun(cmd)对应的函数伸请。
                   pdatainfo	= NULL;	
							
						}
						else
						{
							if(cmd[1]==0x45)
							{ u8 tmp=0;
								 make_json_data( (char *)gprxinfo->rx_buff,cmd[0],cmd[1],&tmp,1);//返回 rtn==0
							}
							
							else if((cmd[1]==0xD4)||(cmd[1]==0xD0))
							{
								 u8 tmp=7;
								 make_json_data( (char *)gprxinfo->rx_buff,cmd[0],cmd[1],&tmp,1);//返回 rtn==7
							}
							else if(cmd[1]==0xD2)
							{
								if(cmd[2]==6)//当cmd[2]为6（无效数据时才有返回）
								{
							  	u8 tmp=6;
								  make_json_data( (char *)gprxinfo->rx_buff,cmd[0],cmd[1],&tmp,1);//返回 rtn==6
									
								}
								else if(cmd[2]==0xFF)
								{
									 return ;
								}
							}
							else
							{
								return ;
							}
							
							
							
						}
				
				 
				
				
				
				
				  file_len=strlen((const char *)gprxinfo->rx_buff);
					make_http_response_head((uint8*)http_response, PTYPE_JSON,file_len);
				 file_len=strlen((const char *)http_response);
					sprintf((char *)http_response+file_len,"%s",gprxinfo->rx_buff);
				  send(s,http_response,strlen((char const*)http_response));
				
			}
				
		   
			break;
			
		default :
			break;
	}

 






}



