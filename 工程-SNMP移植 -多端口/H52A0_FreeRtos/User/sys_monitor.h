#ifndef _SYS_MONITOR_H_
#define _SYS_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"

// 系统监控数据结构体定义
typedef struct {
    // 任务状态信息
    UBaseType_t task_count;                           // 任务总数
    struct {
        char name[configMAX_TASK_NAME_LEN];          // 任务名称
        eTaskState state;                            // 任务状态
        UBaseType_t priority;                        // 当前优先级
        configSTACK_DEPTH_TYPE stack_high_water_mark; // 栈历史最低水位
        UBaseType_t task_number;                     // 任务编号
    } tasks[32];                                     // 最多支持32个任务
    uint32_t total_run_time;                         // 总运行时间
    
    // 内存使用信息
    struct {
        uint8_t sramin_initialized;                  // 内部SRAM初始化状态
        uint16_t sramin_usage_percent;               // 内部SRAM使用百分比
        uint8_t sramccm_initialized;                 // CCM SRAM初始化状态
        uint16_t sramccm_usage_percent;              // CCM SRAM使用百分比
        uint8_t sramex_initialized;                  // 外部SDRAM初始化状态
        uint16_t sramex_usage_percent;               // 外部SDRAM使用百分比
    } memory;
    
    // FreeRTOS堆内存信息
    struct {
        size_t total_heap_size;                      // 总堆大小
        size_t current_free_heap;                    // 当前空闲堆大小
        size_t minimum_ever_free_heap;               // 历史最小空闲堆大小
        uint16_t usage_percent;                      // 使用百分比
    } freertos_heap;
} system_monitor_data_t;

// 全局系统监控数据结构体声明
extern system_monitor_data_t g_system_monitor_data;

// 系统监控任务函数声明
void system_monitor_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // _SYS_MONITOR_H_
