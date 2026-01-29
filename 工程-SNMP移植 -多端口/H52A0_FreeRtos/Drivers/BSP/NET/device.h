#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "types.h"
#define DEVICE_ID "W5500"

#define FW_VER_HIGH 1
#define FW_VER_LOW 0

/* Private function prototypes -----------------------------------------------*/

typedef void (*pFunction)(void);
void set_network(void);
// void write_config_to_eeprom(void);
void set_default(void);

// void reboot(void);
void get_config(void);

#endif
