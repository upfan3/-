/*************************************************************************
 *WKS GD32F470ZIT6核心板
 *FreeRTOS移植实验
 *************************************************************************/
#include "freertos_demo.h"
#include "./SYSTEM/uart/uart.h"
#include "./SYSTEM/can/can.h"
// #include "./BSP/LED/led.h"
// #include "./BSP/LCD/lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sys_monitor.h"
extern "C"
{
#include "malloc.h"
}

/*******************************************FreeRTOS配置***********************************************/

// START_TASK 任务 配置
// 包括: 任务句柄 任务优先级 堆栈大小 创建任务
#define START_TASK_PRIO 1            /* 任务优先级 */
#define START_STK_SIZE 128           /* 任务堆栈大小 */
TaskHandle_t StartTask_Handler;      /* 任务句柄 */
void start_task(void *pvParameters); /* 任务函数 */

// TASK1 任务 配置
// 包括: 任务句柄 任务优先级 堆栈大小 创建任务
#define TASK1_PRIO 2            /* 任务优先级 */
#define TASK1_STK_SIZE 128      /* 任务堆栈大小 */
TaskHandle_t Task1Task_Handler; /* 任务句柄 */
void task1(void *pvParameters); /* 任务函数 */

// TASK2 任务 配置
// 包括: 任务句柄 任务优先级 堆栈大小 创建任务
#define TASK2_PRIO 3            /* 任务优先级 */
#define TASK2_STK_SIZE 128      /* 任务堆栈大小 */
TaskHandle_t Task2Task_Handler; /* 任务句柄 */
void task2(void *pvParameters); /* 任务函数 */

// TASK3
void Net_Task(void * pvParameters);

// 系统监控任务
#define SYS_MONITOR_TASK_PRIO 2
#define SYS_MONITOR_STK_SIZE  128
TaskHandle_t SysMonitorTask_Handler;
// void system_monitor_task(void *pvParameters);

/******************************************************************************************************/

////LCD刷屏时使用的颜色
// uint16_t lcd_discolor[11] = {WHITE, BLACK, BLUE, RED,MAGENTA, GREEN, CYAN, YELLOW,BROWN, BRRED, GRAY};

/*
函数名：freertos_demo
功能：FreeRTOS例程入口函数
参数：无
返回值：无
备注：无
*/

extern uart5 gRS485_3;
extern uart1 gRS485_2;
extern uart2 gRS485_1;

extern uint8_t uart0Sendbuff[4][10];
extern uint8_t datalen[4];

extern CanPort0 can0;
extern CanPort1 can1;
uint8_t cantest[] = {1, 2, 3, 4, 5, 6, 7, 8};
void freertos_demo(void)
{
    xTaskCreate((TaskFunction_t)start_task,          /* 任务函数 */
                (const char *)"start_task",          /* 任务名称 */
                (uint16_t)START_STK_SIZE,            /* 任务堆栈大小 */
                (void *)NULL,                        /* 传入给任务函数的参数 */
                (UBaseType_t)START_TASK_PRIO,        /* 任务优先级 */
                (TaskHandle_t *)&StartTask_Handler); /* 任务句柄 */
    vTaskStartScheduler();
}

/*
函数名：start_task
功能：启动任务
参数：pvParameters  传入参数(未用到)
返回值：无
备注：无
*/

void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); /* 进入临界区 */
    /* 创建任务1 */
    xTaskCreate((TaskFunction_t)task1,
                (const char *)"task1",
                (uint16_t)TASK1_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)TASK1_PRIO,
                (TaskHandle_t *)&Task1Task_Handler);
    /* 创建任务2 */
    xTaskCreate((TaskFunction_t)task2,
                (const char *)"task2",
                (uint16_t)TASK2_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)TASK2_PRIO,
                (TaskHandle_t *)&Task2Task_Handler);

    // 创建网络任务
    xTaskCreate((TaskFunction_t)Net_Task,
                (const char *)"Net",
                768,
                (void *)NULL,
                (UBaseType_t)(tskIDLE_PRIORITY + 4),
                NULL);
    /* 创建系统监控任务 */
    xTaskCreate((TaskFunction_t)system_monitor_task,
                (const char *)"sys_monitor",
                (uint16_t)SYS_MONITOR_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)SYS_MONITOR_TASK_PRIO,
                (TaskHandle_t *)&SysMonitorTask_Handler);

    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}

/*
函数名：task1
功能：任务1
参数：pvParameters  传入参数(未用到)
返回值：无
备注：无
*/
void task1(void *pvParameters)
{
    uint32_t task1_num = 0;

    while (1)
    {

        if (datalen[1] > 0)
            gRS485_1.send(&uart0Sendbuff[1][0], 10);
        if (datalen[2] > 0)
            gRS485_2.send(&uart0Sendbuff[2][0], 10);
        if (datalen[3] > 0)
            gRS485_3.send(&uart0Sendbuff[3][0], 10);

        can0.send_message(0x12345678, cantest);
        for (uint8_t i = 0; i < 20; i++)
            can1.send_message(0x12345670 + i, cantest);
        can1.monitor_status();
        vTaskDelay(1000); /* 延时1000ticks */
    }
}

/*
函数名：task2
功能：任务2
参数：pvParameters  传入参数(未用到)
返回值：无
备注：无
*/
void task2(void *pvParameters)
{
    float float_num = 0.0;

    while (1)
    {

        vTaskDelay(1000); /* 延时1000ticks */
    }
}
