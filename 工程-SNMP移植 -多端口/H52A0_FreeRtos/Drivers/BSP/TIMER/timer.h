#ifndef _TIMER_H
#define _TIMER_H


#include "./SYSTEM/sys/sys.h"
// 包含dhcp.h，获取dhcp_time_handler_wrapper的声明
#include "dhcp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief       基本定时器TIMER6定时中断初始化
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为60M, 所以定时器TIMER6时钟 = 120Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void timer6_int_init(uint16_t arr, uint16_t psc);

/**
 * @brief       基本定时器TIMER6中断服务函数（声明，供中断向量表匹配）
 * @note        中断服务函数无需手动调用，由硬件触发
 * @param       无
 * @retval      无
 */
void TIMER6_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H */
