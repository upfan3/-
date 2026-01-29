#ifndef __SYS_H
#define __SYS_H

#include "gd32f4xx.h"

/**
 * SYS_SUPPORT_OS用于定义系统文件夹是否支持OS
 * 0,不支持OS
 * 1,支持OS
 */
#define SYS_SUPPORT_OS 1

/**
 * 位带操作,实现51类似的GPIO控制功能
 * 具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).M4同M3类似,只是寄存器地址变了.
 */
/* IO口操作宏定义 */
#define BITBAND(addr, bitnum) ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr) *((volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))
/* IO口地址映射 */
#define GPIOA_OCTL_Addr (GPIOA + 0x14) /* 0x40020014 */
#define GPIOB_OCTL_Addr (GPIOB + 0x14) /* 0x40020414 */
#define GPIOC_OCTL_Addr (GPIOC + 0x14) /* 0x40020814 */
#define GPIOD_OCTL_Addr (GPIOD + 0x14) /* 0x40020C14 */
#define GPIOE_OCTL_Addr (GPIOE + 0x14) /* 0x40021014 */
#define GPIOF_OCTL_Addr (GPIOF + 0x14) /* 0x40021414 */
#define GPIOG_OCTL_Addr (GPIOG + 0x14) /* 0x40021814 */
#define GPIOH_OCTL_Addr (GPIOH + 0x14) /* 0x40021C14 */
#define GPIOI_OCTL_Addr (GPIOI + 0x14) /* 0x40022014 */
#define GPIOJ_OCTL_ADDr (GPIOJ + 0x14) /* 0x40022414 */
#define GPIOK_OCTL_ADDr (GPIOK + 0x14) /* 0x40022814 */

#define GPIOA_ISTAT_Addr (GPIOA + 0x10) /* 0x40020010 */
#define GPIOB_ISTAT_Addr (GPIOB + 0x10) /* 0x40020410 */
#define GPIOC_ISTAT_Addr (GPIOC + 0x10) /* 0x40020810 */
#define GPIOD_ISTAT_Addr (GPIOD + 0x10) /* 0x40020C10 */
#define GPIOE_ISTAT_Addr (GPIOE + 0x10) /* 0x40021010 */
#define GPIOF_ISTAT_Addr (GPIOF + 0x10) /* 0x40021410 */
#define GPIOG_ISTAT_Addr (GPIOG + 0x10) /* 0x40021810 */
#define GPIOH_ISTAT_Addr (GPIOH + 0x10) /* 0x40021C10 */
#define GPIOI_ISTAT_Addr (GPIOI + 0x10) /* 0x40022010 */
#define GPIOJ_ISTAT_Addr (GPIOJ + 0x10) /* 0x40022410 */
#define GPIOK_ISTAT_Addr (GPIOK + 0x10) /* 0x40022810 */

/**
 * IO口操作,只对单一的IO口!
 * 确保n的值小于16!
 */
#define PAout(n) BIT_ADDR(GPIOA_OCTL_Addr, n) /* 输出 */
#define PAin(n) BIT_ADDR(GPIOA_ISTAT_Addr, n) /* 输入 */

#define PBout(n) BIT_ADDR(GPIOB_OCTL_Addr, n) /* 输出 */
#define PBin(n) BIT_ADDR(GPIOB_ISTAT_Addr, n) /* 输入 */

#define PCout(n) BIT_ADDR(GPIOC_OCTL_Addr, n) /* 输出 */
#define PCin(n) BIT_ADDR(GPIOC_ISTAT_Addr, n) /* 输入 */

#define PDout(n) BIT_ADDR(GPIOD_OCTL_Addr, n) /* 输出 */
#define PDin(n) BIT_ADDR(GPIOD_ISTAT_Addr, n) /* 输入 */

#define PEout(n) BIT_ADDR(GPIOE_OCTL_Addr, n) /* 输出 */
#define PEin(n) BIT_ADDR(GPIOE_ISTAT_Addr, n) /* 输入 */

#define PFout(n) BIT_ADDR(GPIOF_OCTL_Addr, n) /* 输出 */
#define PFin(n) BIT_ADDR(GPIOF_ISTAT_Addr, n) /* 输入 */

#define PGout(n) BIT_ADDR(GPIOG_OCTL_Addr, n) /* 输出 */
#define PGin(n) BIT_ADDR(GPIOG_ISTAT_Addr, n) /* 输入 */

#define PHout(n) BIT_ADDR(GPIOH_OCTL_Addr, n) /* 输出 */
#define PHin(n) BIT_ADDR(GPIOH_ISTAT_Addr, n) /* 输入 */

#define PIout(n) BIT_ADDR(GPIOI_OCTL_Addr, n) /* 输出 */
#define PIin(n) BIT_ADDR(GPIOI_ISTAT_Addr, n) /* 输入 */

#define PJout(n) BIT_ADDR(GPIOJ_OCTL_Addr, n) /* 输出 */
#define PJin(n) BIT_ADDR(GPIOJ_ISTAT_Addr, n) /* 输入 */

#define PKout(n) BIT_ADDR(GPIOK_OCTL_Addr, n) /* 输出 */
#define PKin(n) BIT_ADDR(GPIOK_ISTAT_Addr, n) /* 输入 */

/* 普通函数 */
void sys_soft_reset(void); /* 系统软复位 */

/* 以下为汇编函数 */
void sys_wfi_set(void);          /* 执行WFI指令 */
void sys_intx_disable(void);     /* 关闭所有中断 */
void sys_intx_enable(void);      /* 开启所有中断 */
void sys_msr_msp(uint32_t addr); /* 设置栈顶地址 */

#endif
