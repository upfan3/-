#ifndef __NETTASK_H
#define __NETTASK_H

#include "FreeRTOS.h"
#include "./core/snmp.h"

// SNMP 消息结构
typedef struct
{
    uint8_t remote_ip[4];
    uint16_t remote_port;
    uint16_t data_length;
    uint8_t snmp_data[SNMPV3_BUF_MAX];
} SNMP_Message_t;

// 全局队列句柄
extern QueueHandle_t xSNMPQueue;

// 函数声明
void SNMP_Tasks_Init(void);
void SNMP_Receive_Task(void *pvParameters);
void SNMP_Process_Task(void *pvParameters);
#endif
