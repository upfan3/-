#include "BSP/NET/w5500.h"
#include <stdio.h>
#include <string.h>
#include "BSP/NET/socket.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "NetTask.h"
#include "dhcp.h"
#include "tcp.h"
extern "C"
{
#include "./core/snmp.h"
#include "./core/snmp_decode.h"
#include "./core/snmp_trap_simple.h"
#include "./network/network_interface.h"
}
// #include "rtc.h" // 使用RTC头文件以访问gTimer
#define ALARM_TRAP_OID 1, 3, 6, 1, 4, 1, 61739, 10, 1, 1

TaskHandle_t xSNMPReceiveHandle;
TaskHandle_t xSNMPProcessHandle;
tcp gnet;
// ===================== 新增：测试程序相关定义（标志位版） =====================
// 定义测试用的Socket编号和端口（与原有业务端口区分开，避免冲突）
#define TEST_TCP_CH       4       // TCP测试Socket通道（避开SOCK_UPDATE/SOCK_MODBUS/SOCK_SNMP_V3）
#define TEST_TCP_PORT     8082    // TCP测试端口（避开8080/8081/162）
#define TEST_UDP_CH       3       // UDP测试Socket通道
#define TEST_UDP_PORT     8083    // UDP测试端口
#define TEST_BUFF_SIZE    2048    // 测试数据缓冲区大小

// 测试状态枚举
typedef enum {
    TEST_STATUS_IDLE = 0,        // 初始状态
    TEST_STATUS_INIT_OK,         // 测试环境初始化成功
    TEST_STATUS_INIT_ERR,        // 测试环境初始化失败
    TEST_STATUS_TCP_CONN_OK,     // TCP连接维护成功
    TEST_STATUS_TCP_CONN_ERR,    // TCP连接维护失败
    TEST_STATUS_TCP_RECV_OK,     // TCP数据接收模拟成功
    TEST_STATUS_TCP_RECV_ERR,    // TCP数据接收模拟失败
    TEST_STATUS_TCP_SEND_OK,     // TCP数据发送成功
    TEST_STATUS_TCP_SEND_ERR,    // TCP数据发送失败
    TEST_STATUS_UDP_CONN_OK,     // UDP连接初始化成功
    TEST_STATUS_UDP_CONN_ERR,    // UDP连接初始化失败
    TEST_STATUS_UDP_RECV_OK,     // UDP数据接收模拟成功
    TEST_STATUS_UDP_RECV_ERR,    // UDP数据接收模拟失败
    TEST_STATUS_UDP_SEND_OK,     // UDP数据发送成功
    TEST_STATUS_UDP_SEND_ERR,    // UDP数据发送失败
    TEST_STATUS_FINISHED         // 测试全部完成
} Test_Status_E;

// 全局测试标志位
Test_Status_E g_test_status = TEST_STATUS_IDLE;
u8 g_test_result = 0;            // 1=测试通过，0=测试失败
u8 g_test_has_run = 0;           // 1=测试已执行，0=测试未执行（避免重复测试）

// 测试用全局变量
_RX_STR g_test_rx_info;
u8 g_test_rx_buff[TEST_BUFF_SIZE] = {0};

u8 connetstatus=0;
extern tcp *ptcp;


// 声明W5500网络接口
extern "C"
{
    extern const network_interface_t w5500_network_interface;
}
QueueHandle_t xSNMPQueue = NULL;

extern dhcp gdhcp;

void Test_Init(void)
{
    // 清空接收信息结构体（包括其自带的 rx_buff 数组），无需赋值绑定
    memset(&g_test_rx_info, 0, sizeof(_RX_STR));

    // 测试环境初始化成功（tcp实例已由全局gnet初始化，无需重复初始化）
    g_test_status = TEST_STATUS_INIT_OK;
}

// TCP收发测试函数
void Test_TCP_Transceive(void)
{
//    if (g_test_status != TEST_STATUS_INIT_OK) return;

    // 1. 维护TCP连接（核心接口调用）
    gnet.maintainConnection(TEST_TCP_CH, TEST_TCP_PORT, PROTOCOL_TCP);
    
    // 检查网络连接状态
    if (gnet.checkConnet() != 1) {
        g_test_status = TEST_STATUS_TCP_CONN_ERR;
        return;
    }
    g_test_status = TEST_STATUS_TCP_CONN_OK;

    // 2. 模拟单次Tick处理（接收+处理+发送）
    gnet.OnTick(TEST_TCP_CH, TEST_TCP_PORT, &g_test_rx_info, PROTOCOL_TCP);

    // 3. 手动模拟接收到TCP数据（Modbus格式，符合原有HandleModbusData条件）
    u8 test_tcp_data[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
    u16 test_tcp_len = sizeof(test_tcp_data);
    
    // 验证数据合法性
    if (test_tcp_data == NULL || test_tcp_len == 0) {
        g_test_status = TEST_STATUS_TCP_RECV_ERR;
        return;
    }
    g_test_status = TEST_STATUS_TCP_RECV_OK;

    // 4. 处理并发送响应
    gnet.processAndSendData(TEST_TCP_CH, test_tcp_data, test_tcp_len, &g_test_rx_info, PROTOCOL_TCP);
    
    // 5. 检查Socket状态，标记发送结果
    if (getSn_SR(TEST_TCP_CH) == SOCK_ESTABLISHED) {
        g_test_status = TEST_STATUS_TCP_SEND_OK;
    } else {
        g_test_status = TEST_STATUS_TCP_SEND_ERR;
    }
}

// UDP收发测试函数
void Test_UDP_Transceive(void)
{
    if (g_test_status != TEST_STATUS_TCP_SEND_OK) return; // 依赖TCP测试完成

    // 1. 维护UDP连接（核心接口调用）
    gnet.maintainConnection(TEST_UDP_CH, TEST_UDP_PORT, PROTOCOL_UDP);
    
    // 检查网络连接状态
    if (gnet.checkConnet() != 1) {
        g_test_status = TEST_STATUS_UDP_CONN_ERR;
        return;
    }
    g_test_status = TEST_STATUS_UDP_CONN_OK;

    // 2. 模拟单次Tick处理（接收+处理+发送）
    gnet.OnTick(TEST_UDP_CH, TEST_UDP_PORT, &g_test_rx_info, PROTOCOL_UDP);

    // 3. 手动模拟接收到UDP数据（1363协议格式）
    u8 test_udp_data[] = {0x7E, 0x01, 0x02, 0x03, 0x04, 0x05, 0x7E};
    u16 test_udp_len = sizeof(test_udp_data);
    
    // 设置远端IP和端口（模拟数据来源）
    u8 remote_ip[4] = {192, 168, 12, 115}; // 与SNMP Trap目标IP一致，方便测试
    uint16 remote_port = 5000;
    memcpy(g_test_rx_info.remote_ip, remote_ip, 4);
    g_test_rx_info.remote_port = remote_port;

    // 验证数据合法性
    if (test_udp_data == NULL || test_udp_len == 0 || remote_ip == NULL) {
        g_test_status = TEST_STATUS_UDP_RECV_ERR;
        return;
    }
    g_test_status = TEST_STATUS_UDP_RECV_OK;

    // 4. 处理并发送响应（原路返回）
    gnet.processAndSendData(TEST_UDP_CH, test_udp_data, test_udp_len, &g_test_rx_info, PROTOCOL_UDP);
    g_test_status = TEST_STATUS_UDP_SEND_OK;

    // 5. 标记测试全部完成，统计最终结果
    g_test_result = (g_test_status == TEST_STATUS_UDP_SEND_OK) ? 1 : 0;
    g_test_status = TEST_STATUS_FINISHED;
}

// 整合测试入口函数（执行TCP+UDP测试）
void Test_Start(void)
{
    //if (g_test_has_run == 1) return; // 避免重复执行测试

//    // 1. 初始化测试环境
//    Test_Init();
//    if (g_test_status != TEST_STATUS_INIT_OK) {
//        g_test_has_run = 1;
//        return;
//    }

    // 2. 执行TCP测试
    Test_TCP_Transceive();

    // 3. 执行UDP测试
    Test_UDP_Transceive();

    // 4. 标记测试已执行
    g_test_has_run = 1;
}

_snmp_params snmp_parameters = {
    1,                                                                        // enabled
    0,                                                                        // OLD1
    1,                                                                        // use_v3
    "public",                                                                 // read_community
    "private",                                                                // write_community
    {0x80, 0x00, 0x1F, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, // engine_id
    // {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    12, // engine_id_len
    0,  // boots
    1,  // traps_enabled
    // {192, 168, 12, 112}, // trap_ip
    0, // current_user_index
    {  // trap_targets - 添加目标列表
     {
         // 目标1
         {192, 168, 12, 115}, // target_ip
         162,                 // target_port
         0,                   // disabled
         0                    // user_index - 使用用户0 (initial)
     },
     {
         // 目标2
         {192, 168, 12, 115}, // target_ip
         162,                 // target_port
         0,                   // enabled
         1                    // user_index - 使用用户1 (admin)
     },
     {
         // 目标3
         {192, 168, 12, 115}, // target_ip
         162,                 // target_port
         1,                   // disabled
         2                    // user_index
     },
     {
         // 目标4
         {192, 168, 12, 115}, // target_ip
         162,                 // target_port
         1,                   // disabled
         3                    // user_index
     },
     {
         // 目标5
         {192, 168, 12, 115}, // target_ip
         162,                 // target_port
         1,                   // disabled
         4                    // user_index
     }},
    {            // users
     {           // 用户1
      "initial", // username
                 // "guestpass", // auth_pass
                 // "guestprivpass", // priv_pass
      "123",
      "123",
      // AUTH_NONE,   // auth_type
      AUTH_MD5,
      // PRIV_NONE,   // priv_type
      PRIV_AES,
      {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
       0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20}, // auth_key
      {0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19,
       0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11}, // priv_key
      USER_LEVEL_READ_WRITE},
     {              // 用户2
      "admin",      // username
      "maplesyrup", // auth_pass
      "maplesyrup", // priv_pass
      AUTH_SHA,     // auth_type
      PRIV_AES,     // priv_type
      {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
       0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}, // auth_key
      {0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
       0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01}, // priv_key
      USER_LEVEL_READ_WRITE},
     {        // 用户3
      "jake", // username
              // "guestpass", // auth_pass
              // "guestprivpass", // priv_pass
      "123",
      "123",
      // AUTH_NONE,   // auth_type
      AUTH_MD5,
      // PRIV_NONE,   // priv_type
      PRIV_AES,
      {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
       0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20}, // auth_key
      {0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19,
       0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11}, // priv_key
      USER_LEVEL_NONE},
     {          // 用户4
      "carmen", // username
                // "guestpass", // auth_pass
                // "guestprivpass", // priv_pass
      "123",
      "123",
      // AUTH_NONE,   // auth_type
      AUTH_SHA,
      // PRIV_NONE,   // priv_type
      PRIV_AES,
      {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
       0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20}, // auth_key
      {0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19,
       0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11}, // priv_key
      USER_LEVEL_READ_WRITE},
     {        // 用户5
      "lily", // username
              // "guestpass", // auth_pass
              // "guestprivpass", // priv_pass
      "123",
      "123",
      AUTH_NONE, // auth_type
      PRIV_NONE, // priv_type
      {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
       0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20}, // auth_key
      {0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19,
       0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11}, // priv_key
      USER_LEVEL_READ_WRITE},
     {        // 用户6
      "jack", // username
              // "guestpass", // auth_pass
              // "guestprivpass", // priv_pass
      "123",
      "123",
      AUTH_SHA, // auth_type
      PRIV_DES, // priv_type
      {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
       0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20}, // auth_key
      {0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19,
       0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11}, // priv_key
      USER_LEVEL_READ_WRITE}}};

// 全局缓冲区：仅SNMP_Process_Task使用，命名加前缀标识归属
_trap_varbinding g_snmp_trap_bindings[6] = {0};  // 存储6个变量绑定
char g_snmp_alarm_desc_buf[64] = {0};            // 报警描述缓冲区
char g_snmp_alarm_type_buf[32] = {0};            // 报警类型缓冲区
char g_snmp_alarm_time_buf[20] = "2025-08-15 12:00:00"; // 时间缓冲区

// ===================== 第二步：改造send_alarm_trap函数 =====================
void send_alarm_trap(int alarm_no,
                     int alarm_status,
                     int alarm_severity,
                     const char *alarm_description,
                     const char *alarm_type)
{
    // 1. 构造Trap主OID（栈上小变量，消耗可忽略）
    _oid trap_oid = {11, {ALARM_TRAP_OID, 2}};

    // 2. 清空全局缓冲区，消除上一次调用的残留数据（关键）
    memset(g_snmp_trap_bindings, 0, sizeof(g_snmp_trap_bindings));
    memset(g_snmp_alarm_desc_buf, 0, sizeof(g_snmp_alarm_desc_buf));
    memset(g_snmp_alarm_type_buf, 0, sizeof(g_snmp_alarm_type_buf));

    // 3. 填充第1个绑定：报警编号
    _oid alarm_no_oid = {12, {ALARM_TRAP_OID, 1, alarm_no}};
    memcpy(&g_snmp_trap_bindings[0].oid, &alarm_no_oid, sizeof(_oid));
    g_snmp_trap_bindings[0].data = &alarm_no;
    g_snmp_trap_bindings[0].data_length = sizeof(alarm_no);
    g_snmp_trap_bindings[0].type = ASN_INT;

    // 4. 填充第2个绑定：报警时间（复用全局时间缓冲区）
    _oid alarm_time_oid = {12, {ALARM_TRAP_OID, 2, alarm_no}};
    memcpy(&g_snmp_trap_bindings[1].oid, &alarm_time_oid, sizeof(_oid));
    g_snmp_trap_bindings[1].data = g_snmp_alarm_time_buf;
    g_snmp_trap_bindings[1].data_length = strlen(g_snmp_alarm_time_buf);
    g_snmp_trap_bindings[1].type = ASN_OCTET_STRING;

    // 5. 填充第3个绑定：报警状态
    _oid alarm_status_oid = {12, {ALARM_TRAP_OID, 3, alarm_no}};
    memcpy(&g_snmp_trap_bindings[2].oid, &alarm_status_oid, sizeof(_oid));
    g_snmp_trap_bindings[2].data = &alarm_status;
    g_snmp_trap_bindings[2].data_length = sizeof(alarm_status);
    g_snmp_trap_bindings[2].type = ASN_INT;

    // 6. 填充第4个绑定：报警严重性
    _oid alarm_severity_oid = {12, {ALARM_TRAP_OID, 4, alarm_no}};
    memcpy(&g_snmp_trap_bindings[3].oid, &alarm_severity_oid, sizeof(_oid));
    g_snmp_trap_bindings[3].data = &alarm_severity;
    g_snmp_trap_bindings[3].data_length = sizeof(alarm_severity);
    g_snmp_trap_bindings[3].type = ASN_INT;

    // 7. 填充第5个绑定：报警描述（拷贝到全局缓冲区，避免指针失效）
    _oid alarm_description_oid = {12, {ALARM_TRAP_OID, 5, alarm_no}};
    strncpy(g_snmp_alarm_desc_buf, alarm_description, sizeof(g_snmp_alarm_desc_buf) - 1);
    memcpy(&g_snmp_trap_bindings[4].oid, &alarm_description_oid, sizeof(_oid));
    g_snmp_trap_bindings[4].data = g_snmp_alarm_desc_buf;
    g_snmp_trap_bindings[4].data_length = strlen(g_snmp_alarm_desc_buf);
    g_snmp_trap_bindings[4].type = ASN_OCTET_STRING;

    // 8. 填充第6个绑定：报警类型（同理，拷贝到全局缓冲区）
    _oid alarm_type_oid = {12, {ALARM_TRAP_OID, 6, alarm_no}};
    strncpy(g_snmp_alarm_type_buf, alarm_type, sizeof(g_snmp_alarm_type_buf) - 1);
    memcpy(&g_snmp_trap_bindings[5].oid, &alarm_type_oid, sizeof(_oid));
    g_snmp_trap_bindings[5].data = g_snmp_alarm_type_buf;
    g_snmp_trap_bindings[5].data_length = strlen(g_snmp_alarm_type_buf);
    g_snmp_trap_bindings[5].type = ASN_OCTET_STRING;

    // 9. 发送Trap（传入全局缓冲区地址）
    trap_send(g_snmp_trap_bindings, 6, &trap_oid);
}


_RX_STR rx_info[4];
void Net_Task(void *pvParameters)
{
    Test_Init();
    snmp_init(&snmp_parameters); // 初始化SNMP

    // 初始化SNMP Trap系统
    trap_init();
    trap_send_warmStart(&snmp_parameters); // 发送warmStart Trap给所有启用的目标

    // 创建SNMP消息队列
    xSNMPQueue = xQueueCreate(1, sizeof(SNMP_Message_t));

    if (xSNMPQueue == NULL)
    {
        // 队列创建失败处理
        // 可以根据需要添加错误处理代码
        return;
    }
    // 创建接收任务
    BaseType_t ret;
    int error = 0;
    ret = xTaskCreate(SNMP_Receive_Task, "SNMP_Rx", 256, NULL, 3, &xSNMPReceiveHandle);
    if (ret != pdPASS)
    {
        // printf("SNMP_Receive_Task create failed!\n");
        error++;
    }
    ret = xTaskCreate(SNMP_Process_Task, "SNMP_Proc", 512, NULL, 4, &xSNMPProcessHandle);
    if (ret != pdPASS)
    {
        // printf("SNMP_Process_Task create failed!\n");
        error++;
    }

    //#ifdef TEST_TRAP
    int trap_test_counter = 0;
    int trap_type = 0; // 仅用于测试不同类型的Trap

    while(1)
    {
			
//			gnet.OnTick(1,8082,&rx_info[0]);
			gnet.OnTick(2,8083,&rx_info[1]);
			gnet.OnTick(4,8081,&rx_info[2],PROTOCOL_UDP);
							 gdhcp.Dhcp_Run();
			
    // 每100次循环发送一个Trap测试
    trap_test_counter++;
    if(trap_test_counter >= 100) {
        trap_test_counter = 0;

        // 仅用于测试不同类型的Trap
        switch(trap_type) {
            case 0:
                // 发送默认Trap，包含sysUpTime和trapOID
                trap_send_simple(NULL);
                break;
            case 1:
                {
                    // 发送自定义Trap (通用)
                    static _oid temp_trap_oid = {10, {1, 3, 6, 1, 6, 3, 1, 1, 5, 1}};
                    trap_send_simple(&temp_trap_oid);
                }
                break;
            case 2:
                {
                    // 发送自定义Trap (接口状态)
                    static _oid if_trap_oid = {10, {1, 3, 6, 1, 6, 3, 1, 1, 5, 2}};
                    trap_send_simple(&if_trap_oid);
                }
                break;
            case 3: 
                // 发送自定义层叠Trap，包含自定义的多个变量
                send_alarm_trap(1,                   // alarm_no
                               1,                   // alarm_status (Active)
                               2,                   // alarm_severity (Major)
                               "Temperature Alarm", // alarm_description
                               "Major");            // alarm_type
                break;
            default:
                // 发送默认Trap
                trap_send_simple(NULL);
                break;
        }

        trap_type = (trap_type + 1) % 4;
    }
		


        vTaskDelay(50/ portTICK_RATE_MS); // 适当延时
    }
    //#endif
    vTaskDelete(NULL);
}

const network_interface_t *netif = &w5500_network_interface;
// SNMP 接收任务（生产者）
void SNMP_Receive_Task(void *pvParameters)
{
    SNMP_Message_t xMessage;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 20ms检查周期
		u8 commflag = 0;
    while (1)
    {
        // 使用统一网络状态检查函数
        if (netif->check_network_status(SOCK_SNMP_V3, SNMP_PORT) != 0)
        {
						commflag = 1;
            // 网络状态异常，函数内已处理重新初始化
            //continue;
        }

        // 直接将数据写入xMessage结构体，避免中间拷贝
        uint16_t len = netif->receive(SOCK_SNMP_V3, xMessage.snmp_data, SNMPV3_BUF_MAX, xMessage.remote_ip, &xMessage.remote_port);

        if (len > 0)
        {
            xMessage.data_length = len;

            // 更新接收统计
            // snmp_stats.messages_received++;

            // 发送到处理队列（非阻塞方式，带超时）
            if (xQueueSend(xSNMPQueue, &xMessage, pdMS_TO_TICKS(5)) != pdPASS)
            {
                // 队列满，增加丢弃计数器
                // snmp_stats.queue_drops++;
            }
        }
        // 使用vTaskDelayUntil保持精确的周期
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// SNMP 处理任务（消费者）
void SNMP_Process_Task(void *pvParameters)
{
    SNMP_Message_t xMessage;
    uint16_t out_len;

    while (1)
    {
        // 等待SNMP消息（阻塞方式）
        if (xQueueReceive(xSNMPQueue, &xMessage, portMAX_DELAY) == pdPASS)
        {
            // 添加异常处理
            if (xMessage.data_length > SNMPV3_BUF_MAX)
            {
                // snmp_stats.processing_errors++;
                continue; // 跳过异常数据
            }

            // 处理SNMP消息
            out_len = snmp_process(xMessage.snmp_data, xMessage.data_length, xMessage.snmp_data);

            if (out_len > 0)
            {
                // 发送SNMP响应
                if (netif->send(SOCK_SNMP_V3, xMessage.snmp_data, out_len,
                                xMessage.remote_ip, xMessage.remote_port) != 0)
                {
                    // 发送失败处理
                    // snmp_stats.processing_errors++;
                }
            }

            // 更新处理统计
            // snmp_stats.messages_processed++;
        }
    }
}
