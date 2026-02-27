/*!
    \file    gd32f4xx_it.c
    \brief   interrupt service routines

    \version 2023-06-25, V3.1.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2023, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f4xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/uart/uart.h"

extern "C"
{

    /*!
        \brief    this function handles NMI exception
        \param[in]  none
        \param[out] none
        \retval     none
    */

    void NMI_Handler(void)
    {
    }

    /*!
        \brief    this function handles HardFault exception
        \param[in]  none
        \param[out] none
        \retval     none
    */
    void HardFault_Handler(void)
    {
        /* if Hard Fault exception occurs, go to infinite loop */
        while (1)
        {
        }
    }

    /*!
        \brief    this function handles MemManage exception
        \param[in]  none
        \param[out] none
        \retval     none
    */
    void MemManage_Handler(void)
    {
        /* if Memory Manage exception occurs, go to infinite loop */
        while (1)
        {
        }
    }

    /*!
        \brief    this function handles BusFault exception
        \param[in]  none
        \param[out] none
        \retval     none
    */
    void BusFault_Handler(void)
    {
        /* if Bus Fault exception occurs, go to infinite loop */
        while (1)
        {
        }
    }

    /*!
        \brief    this function handles UsageFault exception
        \param[in]  none
        \param[out] none
        \retval     none
    */
    void UsageFault_Handler(void)
    {
        /* if Usage Fault exception occurs, go to infinite loop */
        while (1)
        {
        }
    }

/*!
    \brief    this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
#if (!SYS_SUPPORT_OS)
    void SVC_Handler(void)
    {
    }
#endif

    /*!
        \brief    this function handles DebugMon exception
        \param[in]  none
        \param[out] none
        \retval     none
    */
    void DebugMon_Handler(void)
    {
    }

/*!
    \brief    this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
#if (!SYS_SUPPORT_OS)
    void PendSV_Handler(void)
    {
    }
#endif

/*!
    \brief    this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
#if (!SYS_SUPPORT_OS)
    void SysTick_Handler(void)
    {
        // HAL_IncTick();
    }
#endif

    // 串口断处理函数

    void USART0_IRQHandler(void)
    {
        if (uart0::s_instances[UART_0])
        {
            uart0::s_instances[UART_0]->handleInterrupt();
        }
    }
    void USART1_IRQHandler(void)
    {
        if (uart1::s_instances[UART_1])
        {
            uart1::s_instances[UART_1]->handleInterrupt();
        }
    }

    void USART2_IRQHandler(void)
    {
        if (uart2::s_instances[UART_2])
        {
            uart2::s_instances[UART_2]->handleInterrupt();
        }
    }

    void UART3_IRQHandler(void)
    {
        if (uart3::s_instances[UART_3])
        {
            uart3::s_instances[UART_3]->handleInterrupt();
        }
    }

    void UART4_IRQHandler(void)
    {
        if (uart4::s_instances[UART_4])
        {
            uart4::s_instances[UART_4]->handleInterrupt();
        }
    }

    void USART5_IRQHandler(void)
    {
        if (uart5::s_instances[UART_5])
        {
            uart5::s_instances[UART_5]->handleInterrupt();
        }
    }
}
