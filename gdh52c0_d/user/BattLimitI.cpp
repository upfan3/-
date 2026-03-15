#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stddef.h>  // 新增：用于NULL判断
#include "globalval.h"


#define TOTAL_MODULE_NUM 30
// -------------------------- 系统配置参数（支持动态修改）--------------------------
typedef struct {
    float Itag;                // 目标充电电流（A）
    uint8_t rect_module_num;   // 整流模块数量
    uint8_t pv_module_num;     // 光伏模块数量
    float rect_total_limit;    // 整流总限流（A）
    float pv_total_limit;      // 光伏总限流（A）
    float current_deadband;    // 电流调节死区（A）
    float max_adjust_step;     // 单次调节步长上限（A）
    float pv_probe_step;       // 光伏试探步长（A）
    uint32_t probe_stable_ms;  // 试探稳定等待时间（ms）
    uint8_t max_probe_times;   // 最大试探次数
    uint32_t control_cycle_ms; // 控制周期（ms）
} SystemConfig;

// 全局配置实例（可通过接口动态修改）
SystemConfig g_sys_cfg = {
    10.0f,//目标充电电流（A）
    2,//整流模块数目
    4,//光伏模块数目
    15.0f,//整流总限流（A）
    20.0f,//光伏总限流（A）
    0.1f, // 电流调节死区（A）
    0.5f, // 单次调节步长上限（A）
    0.2f, // 光伏试探步长（A）
    500,  // 试探稳定等待时间（ms）
    10, //最大试探次数
    100  // 控制周期（ms）
};

// -------------------------- 系统状态定义（扩展故障检测）--------------------------
// 电流采集数据
typedef struct {
    float rect_total;  // 整流总电流（A）
    float pv_total;    // 光伏总电流（A）
    float battery;     // 电池电流（A）：≥0充电，<0放电
    bool valid;        // 采集数据有效性（true=有效）
} CurrentData;

// 控制目标数据
typedef struct {
    float rect_total;  // 整流总目标电流（A）
    float pv_total;    // 光伏总目标电流（A）
    float rect_per;    // 单个整流模块限流（A）
    float pv_per;      // 单个光伏模块限流（A）
    // 光伏试探状态
    float pv_last_probe;   // 上一次试探电流（A）
    uint8_t probe_count;   // 试探次数
    bool is_probing;       // 试探中标志
    uint32_t probe_start_ms;// 试探开始时间戳（ms）
} ControlTarget;

// 系统运行状态（新增：故障、模式标识）
typedef struct {
    CurrentData current;
    ControlTarget control;
    bool is_fault;         // 故障标志
    uint8_t fault_code;    // 故障码（0=无故障）
    bool is_charging;      // 充电模式标志
} SystemState;

// 全局系统状态
SystemState g_sys_state;

// -------------------------- 故障码定义（新增）--------------------------
typedef enum {
    FAULT_NONE = 0,
    FAULT_CURRENT_INVALID,  // 电流采集无效
    FAULT_RECT_OVER_LIMIT,  // 整流电流超限
    FAULT_PV_OVER_LIMIT,    // 光伏电流超限
    FAULT_BATTERY_ABNORMAL  // 电池电流异常（如过充/过放）
} FaultCode;

// -------------------------- 工具函数（扩展功能）--------------------------
/**
 * @brief 数值钳位
 */
static float clamp(float value, float min_val, float max_val) {
    return (value < min_val) ? min_val : (value > max_val) ? max_val : value;
}

/**
 * @brief 滑动平均滤波（新增：电流数据滤波，减少噪声影响）
 */
static float sliding_average_filter(float new_val, float *buf, uint8_t len, uint8_t *idx) {
    if (buf == NULL || len == 0) return new_val;
    
    buf[*idx] = new_val;
    *idx = (*idx + 1) % len;
    
    float avg = 0.0f;
    for (uint8_t i = 0; i < len; i++) {
        avg += buf[i];
    }
    return avg / len;
}

/**
 * @brief 动态修改系统配置（新增：支持上位机/按键配置参数）
 */
bool sys_set_config(const SystemConfig *cfg) {
    if (cfg == NULL) return false;
    
    // 配置参数有效性校验
    if (cfg->rect_module_num == 0 || cfg->pv_module_num == 0 ||
        cfg->rect_total_limit <= 0 || cfg->pv_total_limit <= 0 ||
        cfg->current_deadband < 0 || cfg->max_adjust_step <= 0 ||
        cfg->control_cycle_ms < 10) {  // 控制周期不小于10ms
        return false;
    }
    
    // 原子操作修改配置（避免中断中读取时数据不一致）
    #ifdef __CC_ARM__
        __disable_irq();  // 关闭中断
    #endif
    g_sys_cfg = *cfg;
    #ifdef __CC_ARM__
        __enable_irq();   // 开启中断
    #endif
    return true;
}

/**
 * @brief 获取当前系统状态（新增：上位机查询接口）
 */
void sys_get_state(SystemState *state) {
    if (state == NULL) return;
    
    #ifdef __CC_ARM__
        __disable_irq();
    #endif
    *state = g_sys_state;
    #ifdef __CC_ARM__
        __enable_irq();
    #endif
}

// -------------------------- 硬件抽象层（增强鲁棒性）--------------------------
/**
 * @brief 采集整流模块总电流（增强：添加有效性判断）
 */
static float hal_collect_rect_total(void) {
    // 实际项目：ADC采集+滤波+有效性校验（如范围判断）
    float adc_value = 0.0f;  // 模拟ADC读数
	
    float current = *((s16*)&gpSysData[RECT_CURR]) * 0.1f;  // 模拟电流转换（需替换为真实校准系数）
    
    // 有效性判断：电流不能为负，且不超过1.2倍总限流（异常值过滤）
    if (current < 0.0f || current > g_sys_cfg.rect_total_limit * 1.2f) {
        g_sys_state.current.valid = false;
        return 0.0f;
    }
    g_sys_state.current.valid = true;
    return current;
}

/**
 * @brief 采集光伏模块总电流
 */
static float hal_collect_pv_total(void) {
    float adc_value = 0.0f;
    float current =  *((s16*)&gpSysData[SOLAR_CURR]) * 0.1f;
    
    if (current < 0.0f || current > g_sys_cfg.pv_total_limit * 1.2f) {
        g_sys_state.current.valid = false;
        return 0.0f;
    }
    g_sys_state.current.valid = true;
    return current;
}

/**
 * @brief 采集电池电流
 */
static float hal_collect_battery(void) {
    float adc_value = 0.0f;
    float current = *((s16 *)&gpSysData[TOTAL_BATTI]) * 0.1f;  // 双向电流（支持正负）
    
    // 异常判断：超过电池允许的充放电电流（示例阈值，需根据硬件修改）
    const float max_charge = 200.0f;  // 最大充电电流
    const float max_discharge = -200.0f;  // 最大放电电流
    if (current > max_charge || current < max_discharge) {
        g_sys_state.current.valid = false;
        return 0.0f;
    }
    g_sys_state.current.valid = true;
    return current;
}

/**
 * @brief 下发光伏模块限流（增强：参数校验）
 */
static void hal_set_pv_module(uint8_t idx, float current) {

      u8 *pv8dat=((u8 *)pModuledata->GetDataAddr(MODULETYPE));
	    if(pv8dat[idx]==DCDC){
	    SetLimitI(pcan1,idx, current);
	    }
}

/**
 * @brief 下发整流模块限流（增强：参数校验）
 */
static void hal_set_rect_module(uint8_t idx, float current) {

	    u8 *pv8dat=((u8 *)pModuledata->GetDataAddr(MODULETYPE));
	    if(pv8dat[idx]==ACDC){
	    SetLimitI(pcan1,idx, current);
			}
}

/**
 * @brief 获取系统时间戳（ms）（真实实现示例：STM32 HAL库）
 */
static uint32_t hal_get_tick_ms(void) {

	       uint32_t tick = xTaskGetTickCount();
	        return tick;
}

/**
 * @brief 故障上报（新增：硬件故障处理接口）
 */
static void hal_report_fault(FaultCode code) {
    g_sys_state.is_fault = (code != FAULT_NONE);
    g_sys_state.fault_code = code;
    
    // 实际项目：触发故障指示灯、上报上位机等
    // printf("故障报警：码=%d\n", code);
}


// -------------------------- 控制逻辑（优化核心算法）--------------------------
/**
 * @brief 光伏最大出力试探（优化：修复试探逻辑漏洞，增加边界处理）
 */
static void pv_probe_logic(float load_current, float charge_need) {
    SystemConfig *cfg = &g_sys_cfg;
    ControlTarget *ctrl = &g_sys_state.control;
    CurrentData *curr = &g_sys_state.current;
    
    // 试探前提：无故障、采集有效、未达最大试探次数
    bool can_probe = !g_sys_state.is_fault && curr->valid;
    can_probe &= (ctrl->probe_count < cfg->max_probe_times);
    can_probe &= (ctrl->pv_total < cfg->pv_total_limit);
    
    // 充电模式：需增流才试探；放电模式：始终试探最大出力
    if (g_sys_state.is_charging) {
        can_probe &= (charge_need > cfg->current_deadband);
    }
    
    if (can_probe && !ctrl->is_probing) {
        // 启动试探：步长钳位，避免超总限流
        float new_pv_target = ctrl->pv_total + cfg->pv_probe_step;
        new_pv_target = clamp(new_pv_target, 0.0f, cfg->pv_total_limit);
        
        // 更新目标并下发
        ctrl->pv_total = new_pv_target;
        ctrl->pv_per = ctrl->pv_total / cfg->pv_module_num;
        for (uint8_t i = 0; i < TOTAL_MODULE_NUM; i++) {
            hal_set_pv_module(i, ctrl->pv_per);
        }
        
        // 记录试探状态
        ctrl->is_probing = true;
        ctrl->probe_start_ms = hal_get_tick_ms();
        ctrl->pv_last_probe = curr->pv_total;
    } else if (ctrl->is_probing) {
        // 等待试探稳定
        if (hal_get_tick_ms() - ctrl->probe_start_ms >= cfg->probe_stable_ms) {
            float new_pv_current = curr->pv_total;
            // 判断是否达最大出力：电流变化≤死区，或已超总限流
            bool reach_max = (fabs(new_pv_current - ctrl->pv_last_probe) <= cfg->current_deadband) ||
                            (ctrl->pv_total >= cfg->pv_total_limit);
            
            if (reach_max) {
                ctrl->pv_total = new_pv_current;  // 锁定实际最大出力
                ctrl->probe_count = cfg->max_probe_times;  // 停止试探
            } else {
                ctrl->pv_last_probe = new_pv_current;
                ctrl->probe_count++;
            }
            
            ctrl->is_probing = false;  // 结束本次试探
        }
    }
}

/**
 * @brief 电流分配控制（优化：修复充放电逻辑切换漏洞，增强步长控制）
 */
static void current_allocation_logic(void) {
    SystemConfig *cfg = &g_sys_cfg;
    ControlTarget *ctrl = &g_sys_state.control;
    CurrentData *curr = &g_sys_state.current;
    
    // 计算负载电流（功率平衡：负载=整流+光伏-电池）
    float load_current = curr->rect_total + curr->pv_total - curr->battery;
    load_current = clamp(load_current, 0.0f, INFINITY);  // 负载电流非负
    
    if (g_sys_state.is_charging) {
        // 充电模式：目标I3=Itag，优先光伏，整流补充
        float charge_actual = curr->battery;
        float charge_need = cfg->Itag - charge_actual;
        
        // 光伏试探（动态逼近最大出力）
        if (!ctrl->is_probing) {
            pv_probe_logic(load_current, charge_need);
        }
        
        // 仅在非试探状态下调节整流电流
        if (!ctrl->is_probing) {
            if (charge_need > cfg->current_deadband) {
                // 需增流：光伏已达最大，整流补充剩余（步长约束）
                float pv_available = ctrl->pv_total;
                float rect_needed = cfg->Itag - pv_available;
                float rect_step = clamp(rect_needed, 0.0f, cfg->max_adjust_step);
                ctrl->rect_total = clamp(curr->rect_total + rect_step, 0.0f, cfg->rect_total_limit);
            } else if (charge_need < -cfg->current_deadband) {
                // 需减流：先减整流（市电优先），再减光伏
                float excess = fabs(charge_need);
                float rect_reduce = clamp(excess, 0.0f, cfg->max_adjust_step);
                rect_reduce = clamp(rect_reduce, 0.0f, curr->rect_total);  // 不能减到负
                
                ctrl->rect_total = curr->rect_total - rect_reduce;
                float excess_remaining = excess - rect_reduce;
                
                // 剩余部分减光伏
                if (excess_remaining > cfg->current_deadband) {
                    float pv_reduce = clamp(excess_remaining, 0.0f, cfg->max_adjust_step);
                    pv_reduce = clamp(pv_reduce, 0.0f, ctrl->pv_total);
                    ctrl->pv_total -= pv_reduce;
                    
                    // 重置试探状态，允许重新试探最大出力
                    ctrl->probe_count = 0;
                    ctrl->pv_last_probe = curr->pv_total;
                }
            }
            // 死区内：维持当前目标
        }
    } else {
        // 放电模式：光伏最大化出力，整流补充负载缺口（不向电池充电）
        if (!ctrl->is_probing) {
            pv_probe_logic(load_current, 0.0f);  // 充电需求传0，仅试探最大出力
        }
        
        // 整流补充负载缺口（不超过总限流）
        float rect_needed = load_current - ctrl->pv_total;
        ctrl->rect_total = clamp(rect_needed, 0.0f, cfg->rect_total_limit);
    }
    
    // 最终安全钳位：确保不超限流
    ctrl->rect_total = clamp(ctrl->rect_total, 0.0f, cfg->rect_total_limit);
    ctrl->pv_total = clamp(ctrl->pv_total, 0.0f, cfg->pv_total_limit);
    
    // 计算单模块限流
    ctrl->rect_per = ctrl->rect_total / cfg->rect_module_num;
    ctrl->pv_per = ctrl->pv_total / cfg->pv_module_num;
    
    // 下发指令（非试探状态下）
    if (!ctrl->is_probing && !g_sys_state.is_fault) {
        for (uint8_t i = 0; i < TOTAL_MODULE_NUM; i++) {
            hal_set_rect_module(i, ctrl->rect_per);
        }
        for (uint8_t i = 0; i < TOTAL_MODULE_NUM; i++) {
            hal_set_pv_module(i, ctrl->pv_per);
        }
    }
}

/**
 * @brief 故障检测（新增：电流超限、采集异常检测）
 */
static void fault_detection(void) {
    SystemConfig *cfg = &g_sys_cfg;
    CurrentData *curr = &g_sys_state.current;
    
    if (!curr->valid) {
        hal_report_fault(FAULT_CURRENT_INVALID);
        return;
    }
    
    // 整流电流超限检测（1.1倍总限流为告警阈值）
    if (curr->rect_total > cfg->rect_total_limit * 1.1f) {
        hal_report_fault(FAULT_RECT_OVER_LIMIT);
        return;
    }
    
    // 光伏电流超限检测
    if (curr->pv_total > cfg->pv_total_limit * 1.1f) {
        hal_report_fault(FAULT_PV_OVER_LIMIT);
        return;
    }
    
    // 电池电流异常检测（充电模式下放电，或放电模式下充电）
    if (g_sys_state.is_charging && curr->battery < -cfg->current_deadband) {
        hal_report_fault(FAULT_BATTERY_ABNORMAL);
        return;
    }
    
    // 无故障
    hal_report_fault(FAULT_NONE);
}

// -------------------------- 初始化与主控制（优化流程）--------------------------
/**
 * @brief 系统初始化
 */
void sys_control_init(void) {
    SystemConfig *cfg = &g_sys_cfg;
    ControlTarget *ctrl = &g_sys_state.control;
    CurrentData *curr = &g_sys_state.current;
    
    // 初始化状态变量
    curr->rect_total = 0.0f;
    curr->pv_total = 0.0f;
    curr->battery = 0.0f;
    curr->valid = false;
    
    ctrl->rect_total = 0.0f;
    ctrl->pv_total = 0.0f;
    ctrl->rect_per = 0.0f;
    ctrl->pv_per = 0.0f;
    ctrl->pv_last_probe = 0.0f;
    ctrl->probe_count = 0;
    ctrl->is_probing = false;
    ctrl->probe_start_ms = 0;
    
    g_sys_state.is_fault = false;
    g_sys_state.fault_code = FAULT_NONE;
    g_sys_state.is_charging = false;  // 初始为放电模式
    
    // 初始化硬件（实际项目：ADC、CAN、定时器等）
    // hal_hardware_init();
    
    // 初始下发0限流，避免模块误输出
    for (uint8_t i = 0; i < TOTAL_MODULE_NUM; i++) {
        hal_set_rect_module(i, 0.0f);
    }
    for (uint8_t i = 0; i < TOTAL_MODULE_NUM; i++) {
        hal_set_pv_module(i, 0.0f);
    }
}

/**
 * @brief 主控制循环（定时器中断触发）
 */
void sys_control_loop(void) {
    SystemConfig *cfg = &g_sys_cfg;
    CurrentData *curr = &g_sys_state.current;
    
    // 1. 采集电流数据（带滤波）
    static float rect_filter_buf[5] = {0};  // 5点滑动平均
    static float pv_filter_buf[5] = {0};
    static float bat_filter_buf[5] = {0};
    static uint8_t filter_idx = 0;
    
    float raw_rect = hal_collect_rect_total();
    float raw_pv = hal_collect_pv_total();
    float raw_bat = hal_collect_battery();
    
    // 滤波处理
    curr->rect_total = sliding_average_filter(raw_rect, rect_filter_buf, 5, &filter_idx);
    curr->pv_total = sliding_average_filter(raw_pv, pv_filter_buf, 5, &filter_idx);
    curr->battery = sliding_average_filter(raw_bat, bat_filter_buf, 5, &filter_idx);
    
    // 2. 更新工作模式（充电/放电）
    g_sys_state.is_charging = (curr->battery >= -cfg->current_deadband);  // 允许微小负电流（测量误差）
    
    // 3. 故障检测
    fault_detection();
    if (g_sys_state.is_fault) {
        // 故障处理：关闭所有模块输出
        for (uint8_t i = 0; i < TOTAL_MODULE_NUM; i++) {
            hal_set_rect_module(i, 0.0f);
        }
        for (uint8_t i = 0; i < TOTAL_MODULE_NUM; i++) {
            hal_set_pv_module(i, 0.0f);
        }
        return;
    }
    
    // 4. 电流分配控制
    current_allocation_logic();
}

