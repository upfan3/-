/************************************************
 * WKS GD32F470ZIT6核心板
 * 定时器中断 驱动代码
 * 版本：V1.0
 ************************************************/

#include "./BSP/TIMER/timer.h"
// #include "./BSP/LED/led.h"

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
void timer6_int_init(uint16_t arr, uint16_t psc)
{
    timer_parameter_struct timer_initpara; /* timer_initpara用于存放定时器的参数 */

    /* 使能RCU相关时钟 */
    rcu_periph_clock_enable(RCU_TIMER6); /* 使能TIMER6的时钟 */

    /* 复位TIMER6 */
    timer_deinit(TIMER6);                    /* 复位TIMER6 */
    timer_struct_para_init(&timer_initpara); /* 初始化timer_initpara为默认值 */

    /* 配置TIMER6 */
    timer_initpara.prescaler = psc;                     /* 设置预分频值 */
    timer_initpara.counterdirection = TIMER_COUNTER_UP; /* 设置向上计数模式 */
    timer_initpara.period = arr;                        /* 设置自动重装载值 */
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;    /* 设置时钟分频因子 */
    timer_init(TIMER6, &timer_initpara);                /* 根据参数初始化定时器 */

    /* 使能定时器及其中断 */
    timer_interrupt_enable(TIMER6, TIMER_INT_UP); /* 使能定时器的更新中断 */
    nvic_irq_enable(TIMER6_IRQn, 1, 3);           /* 配置NVIC设置优先级，抢占优先级1，响应优先级3 */
    timer_enable(TIMER6);                         /* 使能定时器TIMER6 */
}

/**
 * @brief       基本定时器TIMER6中断服务函数
 * @param       无
 * @retval      无
 */
void TIMER6_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER6, TIMER_INT_FLAG_UP) == SET) /* 判断定时器更新中断是否发生 */
    {
        // LED2_TOGGLE();                                                 /* LED2翻转 */
        timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP); /* 清除定时器更新中断标志 */
    }
}
