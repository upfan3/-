/************************************************
 * WKS GD32F470ZIT6核心板
 * SPI 驱动代码
 * 版本：V1.0
 ************************************************/

#ifndef __SPI_H
#define __SPI_H

#include "./SYSTEM/sys/sys.h"

/* SPI总线速度设置 */
#define SPI_SPEED_2 0
#define SPI_SPEED_4 1
#define SPI_SPEED_8 2
#define SPI_SPEED_16 3
#define SPI_SPEED_32 4
#define SPI_SPEED_64 5
#define SPI_SPEED_128 6
#define SPI_SPEED_256 7

void spi4_init(void);                         /* SPI4初始化 */
void spi4_set_speed(uint8_t speed);           /* 设置SPI4速度 */
uint8_t spi4_read_write_byte(uint8_t txdata); /* SPI4读写一个字节 */

#endif
