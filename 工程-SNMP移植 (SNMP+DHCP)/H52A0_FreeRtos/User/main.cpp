/************************************************
 WKS GD32F470ZIT6核心板
 FreeRTOS移植实验
************************************************/
#include "./SYSTEM/sys/sys.h"

#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/uart/uart.h"
#include "./SYSTEM/can/can.h"
#include "freertos_demo.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/OLED/oled.h"
#include "./BSP/NET/network.h"
#include "./BSP/NET/dhcp.h"
#include "timer.h"

uart0 UART0(UART_0);
uart5 gRS485_3(UART_5);
uart1 gRS485_2(UART_1);
uart2 gRS485_1(UART_2);

uint8_t uart0Sendbuff[4][10] = {0};
uint8_t datalen[4] = {0};
CanPort0 can0(CAN0_PERIPH);
CanPort1 can1(CAN1_PERIPH);
OLED oled;
extern network gnetwork;

struct UartCallbackData
{
    uart1 *uartInstance; // 指向当前UART实例
    uint8_t *buffer;     // 数据缓冲区
    uint32_t bufferSize; // 缓冲区大小
    uint8_t *bufflenth;
};

UartCallbackData callbackData = {
    &gRS485_2, //
    &uart0Sendbuff[2][0],
    10,
    &datalen[2]};

void receiveHandler(void *userData)
{
    if (userData == NULL)
    {
        return; // 无效参数处理
    }

    UartCallbackData *callbackData = static_cast<UartCallbackData *>(userData);

    // 验证UART实例有效性
    if (callbackData->uartInstance == NULL || callbackData->buffer == NULL)
    {
        return;
    }

    // 调用UART实例的getReceivedData获取数据到缓冲区
    uint32_t actualLength = callbackData->uartInstance->getReceivedData(
        callbackData->buffer,
        callbackData->bufferSize);
    *callbackData->bufflenth = actualLength;
}

uint8_t tbuff[100] = {0};
uint8_t tbuff2[100] = {0};

int main(void)
{
    delay_init(240); // 延时初始化
		timer6_int_init(9999,11999);
		gnetwork.setDHCP(DHCP_ENABLE);
		gnetwork.Init();

			//gnet.OsRest();
			
    for (uint8_t i = 0; i < 100; i++)
    {
        tbuff[i] = i + 100;
    }

    //			gRS485_1.init(9600);
    //	    gRS485_2.init(9600);
    //	    gRS485_3.init(9600);
    //	    gRS485_2.setReceiveCallback(receiveHandler, &callbackData);
    //
    //	    can0.init(CAN_BAUD_125K);
    //	    can1.init(CAN_BAUD_250K);
    //	    can1.stop();
    //	    can1.start();
    //    norflash_init();
    //
    //	    norflash_erase_sector(0);
    //		  norflash_write(tbuff, 0, 100);
    //	    norflash_read(tbuff2, 0, 100);

    oled.init();
    oled.show_char(10, 10, 'a', 12, 0);
    oled.refresh_gram();

    freertos_demo(); // 运行FreeRTOS例程

}
