/************************************************
 * 使用SysTick的普通计数模式对延迟进行管理
 * 包括delay_us,delay_ms
 ************************************************/

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"

static uint16_t g_fac_us = 0; /* us延时倍乘数 */

#if SYS_SUPPORT_OS
#include "FreeRTOS.h"
#include "task.h"

extern void xPortSysTickHandler(void);

/**
 * @brief       systick中断服务函数,使用OS时用到
 * @param       ticks: 延时的节拍数
 * @retval      无
 */
void SysTick_Handler(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}

#endif

/**
 * @brief       初始化延迟函数
 * @param       sysclk: 系统时钟频率, 即CPU频率(HCLK)
 * @retval      无
 */
void delay_init(uint16_t sysclk)
{
#if SYS_SUPPORT_OS /* 如果需要支持OS. */
    uint32_t reload;
#endif
    SysTick->CTRL = 0;                        /* 清Systick状态，以便下一步重设，如果这里开了中断会关闭其中断 */
    SysTick->CTRL &= ~(1 << 2);               /* SYSTICK使用内核时钟源8分频,因systick的计数器最大值只有2^24 */
    g_fac_us = sysclk / 8;                    /* 不论是否使用OS,g_fac_us都需要使用,作为1us的基础时基 */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* 开启SYSTICK */
    SysTick->LOAD = 0XFFFFFF;                 /* 注意systick计数器24位，所以这里设置最大重装载值 */
#if SYS_SUPPORT_OS                            /* 如果需要支持OS. */
    reload = sysclk / 8;                      /* 每秒钟的计数次数 单位为M */
    reload *= 1000000 / configTICK_RATE_HZ;   /* 根据delay_ostickspersec设定溢出时间 */

    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; /* 开启SYSTICK中断 */
    SysTick->LOAD = reload;                    /* 每1/delay_ostickspersec秒中断一次 */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;  // 开启SYSTICK
#else
#endif
}

/**
 * @brief       延时nus
 * @note        无论是否使用OS, 都是用时钟摘取法来做us延时
 * @param       nus: 要延时的us数.
 * @note        nus取值范围: 0 ~ (2^32 / g_fac_us) (g_fac_us一般等于系统主频/8, 自行套入计算)
 * @retval      无
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload;
    reload = SysTick->LOAD; /* LOAD的值 */
    ticks = nus * g_fac_us; /* 需要的节拍数 */

    told = SysTick->VAL; /* 刚进入时的计数器值 */
    while (1)
    {
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow; /* 这里注意一下SYSTICK是一个递减的计数器就可以了. */
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break; /* 时间超过/等于要延迟的时间,则退出. */
            }
        }
    }
}

/**
 * @brief       延时nms
 * @param       nms: 要延时的ms数 (0< nms <= (2^32 / g_fac_us / 1000))(g_fac_us一般等于系统主频/8, 自行套入计算)
 * @retval      无
 */
void delay_ms(uint16_t nms)
{
    uint32_t i;

    for (i = 0; i < nms; i++)
    {
        delay_us(1000);
    }
}
