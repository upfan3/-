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

u8 connetstatus=0;
extern tcp *ptcp;


// 声明W5500网络接口
extern "C"
{
    extern const network_interface_t w5500_network_interface;
}
QueueHandle_t xSNMPQueue = NULL;

extern dhcp gdhcp;

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

// 发送报警Trap函数
void send_alarm_trap(int alarm_no,
                     int alarm_status,
                     int alarm_severity,
                     const char *alarm_description,
                     const char *alarm_type)
{

    // 构造发送Trap的OID
    _oid trap_oid = {11, {ALARM_TRAP_OID, 2}};

    // 使用系统时间构造时间字符串
    char alarm_time_str[20] = "2025-08-15 12:00:00";
    // sprintf(alarm_time_str, "%04d-%02d-%02d %02d:%02d:%02d",
    //         gTimer.year, gTimer.mon, gTimer.day,
    //         gTimer.hour, gTimer.min, gTimer.sec);

    // 构造6个变量绑定
    _trap_varbinding var_bindings[6];

    // 1. alarmTrapNo - 报警编号
    _oid alarm_no_oid = {12, {ALARM_TRAP_OID, 1, alarm_no}};
    memcpy(&var_bindings[0].oid, &alarm_no_oid, sizeof(_oid));
    var_bindings[0].data = &alarm_no;
    var_bindings[0].data_length = sizeof(alarm_no);
    var_bindings[0].type = ASN_INT;

    // 2. alarmTime - 报警时间
    _oid alarm_time_oid = {12, {ALARM_TRAP_OID, 2, alarm_no}};
    memcpy(&var_bindings[1].oid, &alarm_time_oid, sizeof(_oid));
    var_bindings[1].data = (void *)alarm_time_str;
    var_bindings[1].data_length = strlen(alarm_time_str);
    var_bindings[1].type = ASN_OCTET_STRING;
	
    // 3. alarmStatusChange - 报警状态
    _oid alarm_status_oid = {12, {ALARM_TRAP_OID, 3, alarm_no}};
    memcpy(&var_bindings[2].oid, &alarm_status_oid, sizeof(_oid));
    var_bindings[2].data = &alarm_status;
    var_bindings[2].data_length = sizeof(alarm_status);
    var_bindings[2].type = ASN_INT;

    // 4. alarmSeverity - 报警严重性
    _oid alarm_severity_oid = {12, {ALARM_TRAP_OID, 4, alarm_no}};
    memcpy(&var_bindings[3].oid, &alarm_severity_oid, sizeof(_oid));
    var_bindings[3].data = &alarm_severity;
    var_bindings[3].data_length = sizeof(alarm_severity);
    var_bindings[3].type = ASN_INT;

    // 5. alarmDescription - 报警描述
    _oid alarm_description_oid = {12, {ALARM_TRAP_OID, 5, alarm_no}};
    memcpy(&var_bindings[4].oid, &alarm_description_oid, sizeof(_oid));
    var_bindings[4].data = (void *)alarm_description;
    var_bindings[4].data_length = strlen(alarm_description);
    var_bindings[4].type = ASN_OCTET_STRING;

    // 6. alarmType - 报警类型
    _oid alarm_type_oid = {12, {ALARM_TRAP_OID, 6, alarm_no}};
    memcpy(&var_bindings[5].oid, &alarm_type_oid, sizeof(_oid));
    var_bindings[5].data = (void *)alarm_type;
    var_bindings[5].data_length = strlen(alarm_type);
    var_bindings[5].type = ASN_OCTET_STRING;

    // 发送Trap
    trap_send(var_bindings, 6, &trap_oid);
}

void Net_Task(void *pvParameters)
{
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
