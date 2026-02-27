#include "sys_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "malloc.h"
#include <stdio.h>
#include <string.h>

// 全局系统监控数据结构体实例
system_monitor_data_t g_system_monitor_data = {0};

// 栈溢出处理钩子函数
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    taskDISABLE_INTERRUPTS();
     
    // printf("Stack overflow detected!\r\n");
    // printf("Task name: %s\r\n", pcTaskName);
    
    if (xTask != NULL) {
        UBaseType_t uxPriority = uxTaskPriorityGet(xTask);
        // printf("Task priority: %u\r\n", (unsigned int)uxPriority);
        
        configSTACK_DEPTH_TYPE uxStackHighWaterMark = uxTaskGetStackHighWaterMark(xTask);
        // printf("Task stack high water mark: %u words\r\n", (unsigned int)uxStackHighWaterMark);
    }
    
    UBaseType_t uxTaskCount = uxTaskGetNumberOfTasks();
    // printf("Total number of tasks: %u\r\n", (unsigned int)uxTaskCount);
    
    size_t xFreeHeapSize = xPortGetFreeHeapSize();
    size_t xMinimumHeapSize = xPortGetMinimumEverFreeHeapSize();
    // printf("Current free heap size: %u bytes\r\n", (unsigned int)xFreeHeapSize);
    // printf("Minimum ever free heap size: %u bytes\r\n", (unsigned int)xMinimumHeapSize);
    
    
    while(1)
    {
        __asm("BKPT #0");
    }
}

void system_monitor_task(void *pvParameters)
{
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    uint32_t ulTotalRunTime;

    my_mem_init(SRAMIN);   // 初始化内部SRAM内存管理
    my_mem_init(SRAMCCM);  // 初始化CCM SRAM内存管理
    my_mem_init(SRAMEX);   // 初始化外部SDRAM内存管理
    
    while(1) {
        // 1. 堆栈水位监控和任务状态查看
        // 获取任务数量
        uxArraySize = uxTaskGetNumberOfTasks();
        g_system_monitor_data.task_count = uxArraySize;
        
        // 分配内存以存储任务状态
        pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
        
        if(pxTaskStatusArray != NULL) {
            // 获取所有任务的状态信息
            uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);
            g_system_monitor_data.total_run_time = ulTotalRunTime;
            
            // 遍历所有任务并存储状态信息
            for(x = 0; x < uxArraySize && x < 32; x++) {
                // 复制任务名称
                strncpy(g_system_monitor_data.tasks[x].name, pxTaskStatusArray[x].pcTaskName, configMAX_TASK_NAME_LEN-1);
                g_system_monitor_data.tasks[x].name[configMAX_TASK_NAME_LEN-1] = '\0';
                
                // 存储任务状态信息
                g_system_monitor_data.tasks[x].state = pxTaskStatusArray[x].eCurrentState;
                g_system_monitor_data.tasks[x].priority = pxTaskStatusArray[x].uxCurrentPriority;
                g_system_monitor_data.tasks[x].stack_high_water_mark = pxTaskStatusArray[x].usStackHighWaterMark;
                g_system_monitor_data.tasks[x].task_number = pxTaskStatusArray[x].xTaskNumber;
            }
            
            // 释放内存
            vPortFree(pxTaskStatusArray);
        }
        
        // 2. 内存使用统计
        // 检查内存管理模块是否已初始化
        g_system_monitor_data.memory.sramin_initialized = mallco_dev.memrdy[0];
        if(mallco_dev.memrdy[0]) {
            g_system_monitor_data.memory.sramin_usage_percent = mallco_dev.perused(0);
        }
        
        g_system_monitor_data.memory.sramccm_initialized = mallco_dev.memrdy[1];
        if(mallco_dev.memrdy[1]) {
            g_system_monitor_data.memory.sramccm_usage_percent = mallco_dev.perused(1);
        }
        
        g_system_monitor_data.memory.sramex_initialized = mallco_dev.memrdy[2];
        if(mallco_dev.memrdy[2]) {
            g_system_monitor_data.memory.sramex_usage_percent = mallco_dev.perused(2);
        }
        
        // 3. FreeRTOS堆内存使用情况
        g_system_monitor_data.freertos_heap.total_heap_size = configTOTAL_HEAP_SIZE;
        g_system_monitor_data.freertos_heap.current_free_heap = xPortGetFreeHeapSize();
        g_system_monitor_data.freertos_heap.minimum_ever_free_heap = xPortGetMinimumEverFreeHeapSize();
        g_system_monitor_data.freertos_heap.usage_percent = 
        (uint16_t)(((configTOTAL_HEAP_SIZE - g_system_monitor_data.freertos_heap.current_free_heap) * 100) / configTOTAL_HEAP_SIZE);
        
        // 每隔5秒执行一次监控
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
