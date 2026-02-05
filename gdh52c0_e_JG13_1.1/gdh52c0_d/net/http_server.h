#ifndef  _HTTP_SERVER_H_
#define  _HTTP_SERVER_H_

#include "types.h"
#include "valtype.h"

#define HTTP_SERVER_PORT		80		/**< Http server well-known port number */

/* HTTP Method */
#define		METHOD_ERR			0				/**< Error Method. */
#define		METHOD_GET			1				/**< GET Method.   */
#define		METHOD_HEAD			2				/**< HEAD Method.  */
#define		METHOD_POST			3				/**< POST Method.  */

/* HTTP GET Method */
#define		PTYPE_ERR				0				/**< Error file. */
#define		PTYPE_HTML			1				/**< HTML	file.  */
#define		PTYPE_GIF				2				/**< GIF	file.  */
#define		PTYPE_TEXT			3				/**< TEXT file.  */
#define		PTYPE_JPEG			4				/**< JPEG file.  */
#define		PTYPE_FLASH			5				/**< FLASH file. */
#define		PTYPE_MPEG			6				/**< MPEG file.  */
#define		PTYPE_PDF				7				/**< PDF file.   */
#define 	PTYPE_CGI				8				/**< CGI */
#define 	PTYPE_PL				9 		  /**pl*/
#define   PTYPE_JSON      10      /**json*/
#define   PTYPE_JS        11      /**js*/
#define   PTYPE_CSS        12      /**js*/
#define   PTYPE_XML        13      /**js*/

/* HTML Doc. for ERROR */
#define ERROR_HTML_PAGE "HTTP/1.1 200 OK\r\nCache-Control: max-age=86400\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0"
//static char  ERROR_HTML_PAGE[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 78\r\n\r\n<HTML>\r\n<BODY>\r\nSorry, the page you requested was not found.\r\n</BODY>\r\n</HTML>\r\n\0";

#define ERROR_REQUEST_PAGE "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0"
//static char ERROR_REQUEST_PAGE[] = "HTTP/1.1 400 OK\r\nContent-Type: text/html\r\nContent-Length: 50\r\n\r\n<HTML>\r\n<BODY>\r\nInvalid request.\r\n</BODY>\r\n</HTML>\r\n\0";

#define RETURN_CGI_PAGE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 59\r\n\r\n<html><head><title>iWeb - Configuration</title></head><BODY>CGI command was executed.</BODY></HTML>\0"


/* Response header for HTML*/
#define RES_HTMLHEAD_OK	"HTTP/1.1 200 OK\r\nCache-Control: max-age=86400\r\nContent-Type: text/html\r\nLast-Modified: Wed,01 Sep 2021 02:12:53 GMT\r\nAccept-Ranges: bytes\r\nVary: Accept-Encoding\r\nServer: Microsoft-IIS/7.5\r\nX-Powered-By: ASP.NET\r\nDate: Fri,24 Sep 2021 15:32:51 GMT\r\nContent-Length: "
//static PROGMEM char RES_HTMLHEAD_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
/* Response head for TEXT */
#define RES_TEXTHEAD_OK	"HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\nContent-Length: "

/* Response head for GIF */
#define RES_GIFHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: "

/* Response head for JPEG */
#define RES_JPEGHEAD_OK	"HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: "		

/* Response head for FLASH */
#define RES_FLASHHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/x-shockwave-flash\r\nContent-Length: "
//static PROGMEM char RES_FLASHHEAD_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: application/x-shockwave-flash\r\nContent-Length: ";

/* Response head for MPEG */
#define RES_MPEGHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: video/mpeg\r\nContent-Length: "	

/* Response head for PDF */
#define RES_PDFHEAD_OK "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Length: "

/* Response head for JSON */
#define RES_JSONHEAD_OK "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\nContent-Length: "

/* Response head for JS */ 
#define RES_JAVASCRIPT_OK "HTTP/1.1 200 OK\r\nContent-Type: application/javascript\r\nContent-Length: "

/* Response head for CSS */ 
#define RES_CSS_OK "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\nContent-Length: "

/* Response head for XML */ 
#define RES_XML_OK "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: text/xml\r\nContent-Length: "

//digital I/O out put control result response
#define DOUT_RES_1  "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 1\r\n\r\n1"
#define DOUT_RES_0  "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 1\r\n\r\n0"

#define MAX_URI_SIZE	1460//(TX_RX_MAX_BUF_SIZE/2 - sizeof(char)*2)		

/**
 @brief 	Structure of HTTP REQUEST 
 */
 
typedef char * (*PTRFUN_PCH)(void *,void *,void *); 	 
 
typedef struct _st_http_request
{
  uint8	METHOD;																										/**< request method(METHOD_GET...). */
  uint8	TYPE;	                                                 	/**< request type(PTYPE_HTML...).   */
	char  * pdata;																								
  int8	FileNAME[16];																				          /**< request file name.             */
}st_http_request;


#ifdef __cplusplus
 extern "C" {
#endif

void unescape_http_url(char * url);																/* convert escape character to ascii */

void parse_http_request(st_http_request *, uint8 *);							/* parse request from peer */
	
void find_http_uri_type(uint8 *, char *);													/* find MIME type of a file */

void make_http_response_head(unsigned char *, char, u_long);			/* make response header */

//char get_http_param_value(char* uri);	/* get the user-specific parameter value */
	 
	 
	 
void proc_http(SOCKET s, u_char * buf,PTRFUN_PCH pFun);
u8 http_proc_send(SOCKET s);
void cgi_ipconfig(st_http_request *http_request);
//void trimp(uint8* src, uint8* dst, uint16 len);
uint16 make_msg_response(uint8* buf,int8* msg);

void make_cgi_response(uint16 a,int8* b,int8* c);
void make_pwd_response(int8 isRight,uint16 delay,int8* cgi_response_content, int8 isTimeout);
void vGet_http_data(void);
void vGetIp(u8 *pdsrstr,u8 *pscrstr,u8 Max);	 
	 
	 
	 
	 
#ifdef __cplusplus
}
#endif	 

typedef struct __rx_str
{
	u8 rx_buff[2048];
	u16 rx_len;//接收的长度
	u16 tx_len;//要发送的长度
}_RX_STR;




extern _RX_STR *gprxinfo;
extern  st_http_request *gphttp_request;


#define PASE_BASE        0x8045000//(0x8000000+186*1024)
#define PAGE_DEFAULT     PASE_BASE+*((const u32 *)PASE_BASE+0) 
#define PAGE_CSS         PASE_BASE+*((const u32 *)PASE_BASE+1)
#define PAGE_1363_JS     PASE_BASE+*((const u32 *)PASE_BASE+2)
#define PAGE_WEB_JS      PASE_BASE+*((const u32 *)PASE_BASE+3)
#define XML_BASE         PASE_BASE+*((const u32 *)PASE_BASE+4)
#define LOGO_MNC         PASE_BASE+*((const u32 *)PASE_BASE+5)
#define LOGO_POWER       PASE_BASE+*((const u32 *)PASE_BASE+6)
#define LOGO_LAST        PASE_BASE+*((const u32 *)PASE_BASE+7)	

#define MNCLOGO_LEN     *((const u32 *)PASE_BASE+6)-*((const u32 *)PASE_BASE+5)-1
#define PWRLOGO_LEN     *((const u32 *)PASE_BASE+7)-*((const u32 *)PASE_BASE+6)-1

#define PAGE_EQM          XML_BASE

#endif	/* end of __HTTPD_H__ */ 
