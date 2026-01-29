/************************************************
 * 使用SysTick的普通计数模式对延迟进行管理
 * 包括delay_us,delay_ms
 ************************************************/

#ifndef __DELAY_H
#define __DELAY_H

#include "./SYSTEM/sys/sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void delay_init(uint16_t sysclk); /* 初始化延迟函数 */
    void delay_ms(uint16_t nms);      /* 延时nms */
    void delay_us(uint32_t nus);      /* 延时nus */

#ifdef __cplusplus
}
#endif

#endif
