#include "device.h"
#include "w5500.h"

CONFIG_MSG ConfigMsg, RecvMsg;

uint8 txsize[MAX_SOCK_NUM] = {16, 0, 0, 0, 0, 0, 0, 0}; // 选择8个Socket每个Socket发送缓存的大小，在w5500.c的void sysinit()有设置过程
uint8 rxsize[MAX_SOCK_NUM] = {16, 0, 0, 0, 0, 0, 0, 0}; // 选择8个Socket每个Socket接收缓存的大小，在w5500.c的void sysinit()有设置过程

extern uint8 MAC[6];

uint8 pub_buf[1460];

// reboot
void reboot(void)
{
  //  pFunction Jump_To_Application;
  //  uint32 JumpAddress;
  //  JumpAddress = *(vu32*) (0x00000004);
  //  Jump_To_Application = (pFunction) JumpAddress;
  //  Jump_To_Application();
}
void set_network(void) // 配置初始化IP信息并打印，初始化8个Socket
{
  uint8 ip[4];
  setSHAR(ConfigMsg.mac);
  setSUBR(ConfigMsg.sub);
  setGAR(ConfigMsg.gw);
  setSIPR(ConfigMsg.lip);

  sysinit(txsize, rxsize); // 初始化8个socket
  setRTR(2000);            // 设置超时时间
  setRCR(3);               // 设置最大重新发送次数

  getSIPR(ip);
  //  printf("IP : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getSUBR(ip);
  //  printf("SN : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getGAR(ip);
  //  printf("GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
}

void set_default(void) // 设置默认MAC、IP、GW、SUB、DNS
{
  uint8 mac[6] = {0x00, 0x08, 0xdc, 0x11, 0x11, 0x11};
  uint8 lip[4] = {192, 168, 1, 112};
  uint8 sub[4] = {255, 255, 255, 0};
  uint8 gw[4] = {192, 168, 1, 1};
  uint8 dns[4] = {114, 114, 114, 114};
  memcpy(ConfigMsg.lip, lip, 4);
  memcpy(ConfigMsg.sub, sub, 4);
  memcpy(ConfigMsg.gw, gw, 4);
  memcpy(ConfigMsg.mac, mac, 6);
  memcpy(ConfigMsg.dns, dns, 4);

  ConfigMsg.dhcp = 0;
  ConfigMsg.debug = 1;
  ConfigMsg.fw_len = 0;

  ConfigMsg.state = NORMAL_STATE;
  ConfigMsg.sw_ver[0] = FW_VER_HIGH;
  ConfigMsg.sw_ver[1] = FW_VER_LOW;
}

void write_config_to_eeprom(void)
{
  //  uint8 data;
  //  uint16 i,j;
  //  uint16 dAddr=0;
  //  for (i = 0, j = 0; i < (uint8)(sizeof(ConfigMsg)-4);i++)
  //  {
  //    data = *(uint8 *)(ConfigMsg.mac+j);
  //    at24c16_write(dAddr, data);
  //    dAddr += 1;
  //    j +=1;
  //  }
}
void get_config(void)
{
  //  uint8 tmp=0;
  //  uint16 i;
  //  for (i =0; i < CONFIG_MSG_LEN; i++)
  //  {
  //    tmp=at24c16_read(i);
  //    *(ConfigMsg.mac+i) = tmp;
  //  }
  //  if((ConfigMsg.mac[0]==0xff)&&(ConfigMsg.mac[1]==0xff)&&(ConfigMsg.mac[2]==0xff)&&(ConfigMsg.mac[3]==0xff)&&(ConfigMsg.mac[4]==0xff)&&(ConfigMsg.mac[5]==0xff))
  //  {
  //    set_default();
  //  }
}
