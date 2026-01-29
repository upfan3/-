#include "tcp.h"

//#include "user.h"
//#include "cydt1363.h"

// --- 全局变量 ---
tcp gtcp;
tcp *ptcp=&gtcp;

//CYDT1363 net1363;
u8 netdatbuf[2048];

// --- 全局数据处理函数 (保持不变) ---
u8 *DuleWithNetData(u8 *pdat, u16 *plen)
{
//    u8 *psend;
//    // ... (您的DuleWithNetData函数内容保持不变) ...
//    if(UpdateFlag==2)
//    {
//        psend=UpDateFlash(pdat,plen,2);
//        return psend;
//    }
//    else if((pdat[0]==0x7E)&&(pdat[2]!=0)&&(pdat[3]!=0))
//    {
//        if(net1363.Get1363Data(pdat)==0)
//            return net1363.m_senddata;
//        net1363.DuleWith1363Data();
//        return net1363.m_senddata;
//    }
//    else if((pdat[0]=='M')&&(pdat[1]=='N')&&(pdat[2]=='C')&&(pdat[3]=='-')&&(pdat[4]=='E')&&(pdat[5]=='N'))
//    {
//        if(UpdateFlag==0)
//        {
//            psend=UpDateFlash(pdat,plen,2);
//            return psend;
//        }
//        else
//        {
//            psend[0]='\0';
//            return psend;
//        }
//    }
//    else if((pdat[2]==0)&&(pdat[3]==0))
//    {
//        psend=ModbusDulewithData(pdat,plen);
//        return psend;
//    }
//    else
//    {
//        psend=ModbusDulewithData(pdat,plen);
//        return psend;
//    }
}




// --- 类成员函数实现 ---

tcp::tcp(){};
tcp::~tcp(){};

void tcp::Init(void)
{
	for(int i=0;i<8;i++)
	{
		tcp_err[i]=0;
	}

    m_lock=0;
//    net1363.Init(netdatbuf);
}

/**
 * @brief 维护单个Socket的连接状态（兼容TCP/UDP）
 */
void tcp::maintainConnection(uint8 ch, uint16 port, ProtocolType proto)
{
	    // 参数有效性检查
    if (ch >= MAX_SOCK_NUM || port == 0) {
        return;
    }
	
    uint8 sock_status = getSn_SR(ch);

    if(proto == PROTOCOL_UDP)
    {
        // UDP专属逻辑：强制关闭后重新创建UDP Socket（避免模式冲突）
        if(sock_status != SOCK_UDP) // 只要不是UDP模式，就重新创建
        {
            close(ch); // 先关闭旧Socket
            // 明确创建UDP模式Socket
            socket(ch, Sn_MR_UDP, port, 0x00); 
        }
        // UDP无状态，直接返回，不执行任何TCP逻辑
        return;
    }

    // TCP原有逻辑（不变）
    switch(sock_status)
    {
        case SOCK_CLOSED:
            socket(ch, Sn_MR_TCP, port, 0x00);
            break;
                
        case SOCK_INIT:
            listen(ch);
            tcp_err[ch] = 100;
            break;  
        
        case SOCK_LISTEN:
            if(!(getPHYCFGR()&0x01))
            {
                disconnect(ch);   
                close(ch);
            }
            break;      
        
        case SOCK_ESTABLISHED:
            if(!(getPHYCFGR()&0x01))
            {
                disconnect(ch);   
                close(ch);
                return;
            }
            if(getSn_IR(ch) & Sn_IR_CON)
            {
                setSn_IR(ch, Sn_IR_CON);
            }
            
            if(tcp_err[ch] > 0) tcp_err[ch]--;
            if(tcp_err[ch] == 0)
            {
                disconnect(ch);
                close(ch);
            }
            break;
        
        case SOCK_CLOSE_WAIT:
            disconnect(ch);   
            break;
    }
}

/**
 * @brief 从单个Socket接收数据（兼容TCP/UDP）
 */
uint16 tcp::receiveData(uint8 ch, _RX_STR *prxinfo,ProtocolType proto)
{
    if(prxinfo == NULL) return 0;

    // UDP协议接收逻辑：记录远端IP/端口到prxinfo
    if(proto == PROTOCOL_UDP)	
    {
        prxinfo->rx_len = getSn_RX_RSR(ch);
        if(prxinfo->rx_len > 0)
        {
            // 关键：recvfrom把远端IP/端口写入prxinfo
            prxinfo->rx_len = recvfrom(ch, prxinfo->rx_buff, prxinfo->rx_len,
                                     prxinfo->remote_ip, &prxinfo->remote_port);
            prxinfo->rx_buff[prxinfo->rx_len] = 0x00;
            return prxinfo->rx_len;
        }
        return 0;
    }
		else if(proto == PROTOCOL_TCP)
		{
			if(getSn_SR(ch) == SOCK_ESTABLISHED)
			{
					prxinfo->rx_len = getSn_RX_RSR(ch);
					if(prxinfo->rx_len > 0)
					{
							recv(ch, prxinfo->rx_buff, prxinfo->rx_len);
							prxinfo->rx_buff[prxinfo->rx_len] = 0x00;
							tcp_err[ch] = 100;
							return prxinfo->rx_len;
					}
			}
		}

    return 0;
}

/**
 * @brief UDP专用发送函数（指定远端IP和端口）
 */
void tcp::udpSendData(uint8 ch,  u8 *data, uint16 len,  uint8 *remote_ip, uint16 remote_port)
{
    if(data == NULL || len == 0 || remote_ip == NULL) return;
    sendto(ch, data, len, remote_ip, remote_port);
}


// 定义测试响应模式（按需切换）
#define TEST_RESPONSE_MODE_ECHO    1   // 模式1：回显收到的数据（原样返回）
#define TEST_RESPONSE_MODE_FIXED   2   // 模式2：返回固定测试报文
#define CURRENT_TEST_MODE          TEST_RESPONSE_MODE_FIXED  // 当前使用的模式

// 测试响应缓冲区（全局/静态，避免栈溢出，大小适配测试需求）
static u8 g_test_response_buff[512] = {0};

/**
 * @brief Socket4测试数据处理函数
 * @param data 收到的原始数据
 * @param send_len [输出] 要发送的响应数据长度
 * @return 指向响应数据的指针（全局缓冲区）
 */
u8* HandleTestNetData(const u8 *data, u16 *send_len)
{
    // 入参校验（避免空指针/长度异常）
    if (data == NULL || send_len == NULL) {
        memset(g_test_response_buff, 0, sizeof(g_test_response_buff));
        return NULL;
    }

    // 清空响应缓冲区（避免残留数据）
    memset(g_test_response_buff, 0, sizeof(g_test_response_buff));

#if CURRENT_TEST_MODE == TEST_RESPONSE_MODE_ECHO
    // 模式1：回显收到的数据（原样返回）
    // 限制最大长度，避免缓冲区溢出
    uint16 recv_len = *send_len;  // 传入的len是收到的数据长度
    *send_len = (recv_len > sizeof(g_test_response_buff)) ? sizeof(g_test_response_buff) : recv_len;
    memcpy(g_test_response_buff, data, *send_len);
   // printf("[Socket4测试] 收到数据，长度：%d，回显数据...\r\n", *send_len);

#elif CURRENT_TEST_MODE == TEST_RESPONSE_MODE_FIXED
    // 模式2：返回固定测试报文（便于识别响应是否成功）
    const char *fixed_response = "W5500 Socket4 Test OK! Received Data: ";
    uint16 fixed_len = strlen(fixed_response);
    uint16 recv_len = *send_len;

    // 拼接固定报文 + 收到的前16字节数据（避免过长）
    memcpy(g_test_response_buff, fixed_response, fixed_len);
    uint16 copy_len = (recv_len > 16) ? 16 : recv_len;
    memcpy(g_test_response_buff + fixed_len, data, copy_len);
    *send_len = fixed_len + copy_len;
   // printf("[Socket4测试] 返回固定响应，长度：%d\r\n", *send_len);

#endif

    // 返回响应缓冲区指针（全局缓冲区，避免函数结束后指针失效）
    return g_test_response_buff;
}

static u8 g_1363_test_buff[512] = {0};

/**
 * @brief SOCK_1363自定义协议测试处理函数
 * @param data 收到的原始数据
 * @param send_len [输出] 响应数据长度
 * @return 响应数据指针
 */
u8* Handle1363TestData(const u8 *data, u16 *send_len)
{
    // 入参校验
    if (data == NULL || send_len == NULL) {
        memset(g_1363_test_buff, 0, sizeof(g_1363_test_buff));
        return NULL;
    }
    memset(g_1363_test_buff, 0, sizeof(g_1363_test_buff));

    uint16 recv_len = *send_len;
    // 自定义协议规则：帧头为0x7E则返回解析成功，否则返回错误提示
    if (recv_len > 0 && data[0] == 0x30) {
        // 拼接响应：协议解析成功 + 帧长度 + 前8字节数据
        const char *success_msg = "socket 1 OK! Frame Len: ";
        uint16 msg_len = strlen(success_msg);
        // 写入成功提示
        memcpy(g_1363_test_buff, success_msg, msg_len);
        // 写入帧长度（转字符串）
        char len_str[8] = {0};
        sprintf(len_str, "%d", recv_len);
        uint16 len_str_len = strlen(len_str);
        memcpy(g_1363_test_buff + msg_len, len_str, len_str_len);
        // 写入前8字节数据（十六进制格式）
        char hex_str[32] = {0};
        uint16 hex_idx = 0;
        for (int i=0; i<16 && i<recv_len; i++) {
            hex_idx += sprintf(hex_str + hex_idx, " %02X", data[i]);
        }
        memcpy(g_1363_test_buff + msg_len + len_str_len, hex_str, hex_idx);
        // 设置响应长度
        *send_len = msg_len + len_str_len + hex_idx;
    } else {
        // 协议帧头错误，返回提示
        const char *error_msg = "1363 Protocol Error! Header not 0x7E";
        *send_len = strlen(error_msg);
        memcpy(g_1363_test_buff, error_msg, *send_len);
    }

    return g_1363_test_buff;
}


static u8 g_test_buff[512] = {0};
u8* HandleTestData(const u8 *data, u16 *send_len)
{
    // 入参校验
    if (data == NULL || send_len == NULL) {
        memset(g_test_buff, 0, sizeof(g_test_buff));
        return NULL;
    }
    memset(g_test_buff, 0, sizeof(g_test_buff));

    uint16 recv_len = *send_len;
    // 自定义协议规则：帧头为0x7E则返回解析成功，否则返回错误提示
    if (recv_len > 0 && data[0] == 0x30) {
        // 拼接响应：协议解析成功 + 帧长度 + 前8字节数据
        const char *success_msg = "socket 2 OK! Frame Len: ";
        uint16 msg_len = strlen(success_msg);
        // 写入成功提示
        memcpy(g_test_buff, success_msg, msg_len);
        // 写入帧长度（转字符串）
        char len_str[8] = {0};
        sprintf(len_str, "%d", recv_len);
        uint16 len_str_len = strlen(len_str);
        memcpy(g_test_buff + msg_len, len_str, len_str_len);
        // 写入前8字节数据（十六进制格式）
        char hex_str[32] = {0};
        uint16 hex_idx = 0;
        for (int i=0; i<16 && i<recv_len; i++) {
            hex_idx += sprintf(hex_str + hex_idx, " %02X", data[i]);
        }
        memcpy(g_test_buff + msg_len + len_str_len, hex_str, hex_idx);
        // 设置响应长度
        *send_len = msg_len + len_str_len + hex_idx;
    } else {
        // 协议帧头错误，返回提示
        const char *error_msg = "1363 Protocol Error! Header not 0x7E";
        *send_len = strlen(error_msg);
        memcpy(g_test_buff, error_msg, *send_len);
    }

    return g_test_buff;
}




/**
 * @brief 处理单个Socket接收到的数据，并发送响应（兼容TCP/UDP）
 * 核心修复：通过prxinfo获取UDP远端地址，不再依赖类成员m_rxinfo
 */
void tcp::processAndSendData(uint8 ch, const u8 *data, uint16 len, _RX_STR *prxinfo,ProtocolType proto)
{

    if(prxinfo == NULL) return;

    u16 send_len = len;  
    u8 *psend = NULL;

    // 根据socket编号选择处理函数（原有逻辑不变）
    switch(ch)
    {
        case SOCK_UPDATE:
//            psend = HandleFlashUpdateData((u8*)data, &send_len);
            break;
            
        case SOCK_1363:
            psend = Handle1363TestData((u8*)data, &send_len);
            send_len = 0;
            break;
            
        case SOCK_MODBUS:
           // psend = HandleModbusData((u8*)data, &send_len);
							psend = HandleTestData((u8*)data, &send_len);				
            break;
				
        case 4:  // Socket4（你也可以定义宏 SOCK_TEST=4，更易维护）
            psend = HandleTestNetData((u8*)data, &send_len);  // 调用测试处理函数
            break;
            
        default:
            //psend = DuleWithNetData((u8*)data, &send_len);
            break;
    }

    if(psend == NULL) return;

    // 自动计算长度（原有逻辑不变）
    if(send_len == 0) 
    {
        send_len = strlen((const char *)psend);
        if(send_len == 0) return;
    }

    // 区分TCP/UDP发送逻辑
    if(proto == PROTOCOL_UDP)
    {
        udpSendData(ch, psend, send_len, prxinfo->remote_ip, prxinfo->remote_port);
    }
    else
    {
        // TCP：原有发送逻辑（加状态检查）
        if(send_len > 0 && getSn_SR(ch) == SOCK_ESTABLISHED)
        {
            send(ch, psend, send_len);
        }
    }
}

/**
 * @brief 主Tick函数（兼容TCP/UDP）
 * 修复：调用processAndSendData时传入prxinfo，传递UDP远端地址
 */
void tcp::OnTick(uint16 ch, uint16 port, _RX_STR *prxinfo, ProtocolType proto)
{
    if(prxinfo == NULL) return;

    // 1. 维护连接状态
    maintainConnection(ch, port, proto);

    // 2. 接收数据（UDP会记录远端地址到prxinfo）
    uint16 data_len = receiveData(ch, prxinfo,proto);

    // 3. 处理并发送响应（传入prxinfo，保证UDP地址准确）
    if(data_len > 0)
    {
        processAndSendData(ch, prxinfo->rx_buff, data_len, prxinfo,proto);
    }
}

u8 tcp::checkConnet()
{
    if(!(getPHYCFGR()&0x01))
    {
        return 0; // 断网
    }
    else 
    {
        return 1; // 连接上
    }
}


/**
 * 接口1：处理固件更新相关数据
 * @param pdat 输入数据指针
 * @param plen 数据长度指针
 * @return 处理后的发送数据指针
 */
u8 *HandleFlashUpdateData(u8 *pdat, u16 *plen)
{
    u8 *psend =NULL; 
//  if(UpdateFlag==2)
//	{
//		  psend=UpDateFlash(pdat,plen,2);
			return psend;
//	}

}

/**
 * 接口2：处理1363协议网络数据
 * @param pdat 输入数据指针
 * @param plen 数据长度指针
 * @return 处理后的发送数据指针
 */
u8 *Handle1363NetData(u8 *pdat)
{
			if((pdat[0]==0x7E)&&(pdat[2]!=0)&&(pdat[3]!=0))
			{
				//    if(net1363.Get1363Data(pdat) == 0)
				//    {
				//        return net1363.m_senddata;
				//    }
				//    net1363.DuleWith1363Data();
				//    return net1363.m_senddata;
			}

}


/**
 * 接口4：处理Modbus协议数据
 * @param pdat 输入数据指针
 * @param plen 数据长度指针
 * @return 处理后的发送数据指针
 */
u8 *HandleModbusData(u8 *pdat, u16 *plen)
{
    u8 *psend =NULL; 
	if((pdat[2]==0)&&(pdat[3]==0))
	{
			//psend = ModbusDulewithData(pdat, plen);
	}

    return psend;
}