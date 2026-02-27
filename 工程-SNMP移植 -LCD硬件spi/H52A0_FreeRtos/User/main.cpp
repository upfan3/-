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
//#include "./BSP/OLED/oled.h"
#include "./BSP/NET/network.h"
#include "./BSP/NET/dhcp.h"
#include "timer.h"
#include "./BSP/LCD/lcd.h"

uart0 UART0(UART_0);
uart5 gRS485_3(UART_5);
uart1 gRS485_2(UART_1);
uart2 gRS485_1(UART_2);

uint8_t uart0Sendbuff[4][10] = {0};
uint8_t datalen[4] = {0};
CanPort0 can0(CAN0_PERIPH);
CanPort1 can1(CAN1_PERIPH);
//OLED oled;
LCD_ST7567 lcd;
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
//		timer6_int_init(9999,11999);
//		gnetwork.setDHCP(DHCP_ENABLE);
//		gnetwork.Init();
		lcd.init();
		lcd.clear();
		lcd.refresh_gram();
		delay_ms(500);
//    /********************* 示例2：显示单个数字（16x16） *********************/
//    // 参数说明：x(列起始坐标), y(行起始坐标), 字符, 显示模式(0=正常,1=反显)
//    // 屏幕分辨率128x64，16x16数字建议x≤112，y≤48（避免超出屏幕）
//    lcd.show_char(0, 0, '0', 0);  // 坐标(10,10)显示数字0（正常模式）
//    lcd.show_char(20, 10, '1', 0);  // 坐标(20,10)显示数字1（正常模式）
//    lcd.show_char(30, 10, '2', 0);  // 坐标(30,10)显示数字2（正常模式）
//    lcd.show_char(40, 10, '3', 0);  // 坐标(40,10)显示数字3（正常模式）	
//    lcd.show_char(50, 10, '8', 1);  // 坐标(50,10)显示数字8（反显模式）
//    lcd.show_char(60, 25, '9', 0);  // 坐标(60,25)显示数字9（换行显示）
//    lcd.refresh_gram();            // 刷新缓存到屏幕，字符才会显示

   
	lcd.show_full_screen_numbers();
	    while(1)
    {
			
    }
			//gnet.OsRest();
//			
//    for (uint8_t i = 0; i < 100; i++)
//    {
//        tbuff[i] = i + 100;
//    }

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

//    oled.init();
//    oled.show_char(10, 10, 'a', 12, 0);
//    oled.refresh_gram();
		
		

   
//	 freertos_demo(); // 运行FreeRTOS例程
		
		    // 4. 核心显示操作（按需调用驱动提供的接口）


}
