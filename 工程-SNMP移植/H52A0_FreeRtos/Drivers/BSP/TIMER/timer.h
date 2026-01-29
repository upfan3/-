/************************************************
 * WKS GD32F470ZIT6核心板
 * 定时器中断 驱动代码
 * 版本：V1.0
 ************************************************/

#ifndef _TIMER_H
#define _TIMER_H

#include "./SYSTEM/sys/sys.h"

void timer6_int_init(uint16_t arr, uint16_t psc); /* 基本定时器TIMER6定时中断初始化 */

#endif
