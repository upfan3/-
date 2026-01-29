//*****************************************************************************
//
//! \file dhcp.h
//! \brief DHCP APIs Header file.
//! \details Processing DHCP protocol as DISCOVER, OFFER, REQUEST, ACK, NACK and DECLINE.
//! \version 1.1.0
//! \date 2013/11/18
//! \par  Revision history
//!       <2013/11/18> 1st Release
//!       <2012/12/20> V1.1.0
//!         1. Move unreferenced DEFINE to dhcp.c
//!       <2012/12/26> V1.1.1
//! \author Eric Jung & MidnightCow
//! \copyright
//!
//! Copyright (c)  2013, WIZnet Co., LTD.
//! All rights reserved.
//! 
//! Redistribution and use in source and binary forms, with or without 
//! modification, are permitted provided that the following conditions 
//! are met: 
//! 
//!     * Redistributions of source code must retain the above copyright 
//! notice, this list of conditions and the following disclaimer. 
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution. 
//!     * Neither the name of the <ORGANIZATION> nor the names of its 
//! contributors may be used to endorse or promote products derived 
//! from this software without specific prior written permission. 
//! 
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
#ifndef _DHCP_H_
#define _DHCP_H_

#include "w5500.h"
#include "stdint.h"
/*
 * @brief 
 * @details If you want to display debug & processing message, Define _DHCP_DEBUG_ 
 * @note    If defined, it depends on <stdio.h>
 */
//#define _DHCP_DEBUG_


/* Retry to processing DHCP */
#define	MAX_DHCP_RETRY          2        ///< Maximum retry count
#define MAX_DHCP_RET    3
#define	DHCP_WAIT_TIME          10       ///< Wait Time 10s


/* UDP port numbers for DHCP */
#define DHCP_SERVER_PORT      	67	      ///< DHCP server port number
#define DHCP_CLIENT_PORT         68	      ///< DHCP client port number


#define MAGIC_COOKIE             0x63825363  ///< Any number. You can modify it to any number
#define DCHP_HOST_NAME           "WIZnet\0"

#define STATE_DHCP_INIT          0        ///< Initialize
#define STATE_DHCP_DISCOVER      1        ///< send DISCOVER and wait OFFER
#define STATE_DHCP_REQUEST       2        ///< send REQUEST and wait ACK or NACK
#define STATE_DHCP_LEASED        3        ///< Received ACK and IP leased
#define STATE_DHCP_REREQUEST     4        ///< send REQUEST for maintaining leased IP
#define STATE_DHCP_RELEASE       5        ///< No use
#define STATE_DHCP_STOP          6        ///< Stop processing DHCP

#define DHCP_FLAGSBROADCAST      0x8000   ///< The broadcast value of flags in RIP_MSG 
#define DHCP_FLAGSUNICAST        0x0000   ///< The unicast   value of flags in RIP_MSG

/* DHCP message OP code */
#define DHCP_BOOTREQUEST         1        ///< Request Message used in op of RIP_MSG
#define DHCP_BOOTREPLY           2        ///< Reply Message used in op of RIP_MSG

/* DHCP message type */
#define DHCP_DISCOVER            1        ///< DISCOVER message in OPT of RIP_MSG
#define DHCP_OFFER               2        ///< OFFER message in OPT of RIP_MSG
#define DHCP_REQUEST             3        ///< REQUEST message in OPT of RIP_MSG
#define DHCP_DECLINE             4        ///< DECLINE message in OPT of RIP_MSG
#define DHCP_ACK                 5        ///< ACK message in OPT of RIP_MSG
#define DHCP_NAK                 6        ///< NACK message in OPT of RIP_MSG
#define DHCP_RELEASE             7        ///< RELEASE message in OPT of RIP_MSG. No use
#define DHCP_INFORM              8        ///< INFORM message in OPT of RIP_MSG. No use

#define DHCP_HTYPE10MB           1        ///< Used in type of RIP_MSG
#define DHCP_HTYPE100MB          2        ///< Used in type of RIP_MSG

#define DHCP_HLENETHERNET        6        ///< Used in hlen of RIP_MSG
#define DHCP_HOPS                0        ///< Used in hops of RIP_MSG
#define DHCP_SECS                0        ///< Used in secs of RIP_MSG

#define INFINITE_LEASETIME       0xffffffff	///< Infinite lease time

#define OPT_SIZE                 312               /// Max OPT size of RIP_MSG
#define RIP_MSG_SIZE             (236+OPT_SIZE)    /// Max size of RIP_MSG

enum dhcp_mode
{
    NETINFO_STATIC = 0,    ///< Static IP configuration by manually.
    NETINFO_DHCP           ///< Dynamic IP configuration from a DHCP server
};

/**
 * @ingroup DATA_TYPE
 *  Network Information for WIZCHIP
 */
struct wiz_NetInfo_t
{
    uint8_t mac[6];  ///< Source Mac Address
    uint8_t ip[4];   ///< Source IP Address
    uint8_t sn[4];   ///< Subnet Mask 
    uint8_t gw[4];   ///< Gateway IP Address
    uint8_t dns[4];  ///< DNS server IP Address
    dhcp_mode dhcp;  ///< 0 - Static, 1 - DHCP
};

/* 
 * @brief return value of DHCP_run()
 */
enum
{
    DHCP_FAILED = 0,  ///< Processing Fail
    DHCP_RUNNING,     ///< Processing DHCP protocol
    DHCP_IP_ASSIGN,   ///< First Occupy IP from DHCP server      (if cbfunc == null, act as default default_ip_assign)
    DHCP_IP_CHANGED,  ///< Change IP address by new ip from DHCP (if cbfunc == null, act as default default_ip_update)
    DHCP_IP_LEASED,   ///< Stand by 
    DHCP_STOPPED      ///< Stop processing DHCP protocol
};

/* 
 * @brief DHCP option and value (cf. RFC1533)
 */
enum
{
   padOption               = 0,
   subnetMask              = 1,
   timerOffset             = 2,
   routersOnSubnet         = 3,
   timeServer              = 4,
   nameServer              = 5,
   dns                     = 6,
   logServer               = 7,
   cookieServer            = 8,
   lprServer               = 9,
   impressServer           = 10,
   resourceLocationServer	= 11,
   hostName                = 12,
   bootFileSize            = 13,
   meritDumpFile           = 14,
   domainName              = 15,
   swapServer              = 16,
   rootPath                = 17,
   extentionsPath          = 18,
   IPforwarding            = 19,
   nonLocalSourceRouting   = 20,
   policyFilter            = 21,
   maxDgramReasmSize       = 22,
   defaultIPTTL            = 23,
   pathMTUagingTimeout     = 24,
   pathMTUplateauTable     = 25,
   ifMTU                   = 26,
   allSubnetsLocal         = 27,
   broadcastAddr           = 28,
   performMaskDiscovery    = 29,
   maskSupplier            = 30,
   performRouterDiscovery  = 31,
   routerSolicitationAddr  = 32,
   staticRoute             = 33,
   trailerEncapsulation    = 34,
   arpCacheTimeout         = 35,
   ethernetEncapsulation   = 36,
   tcpDefaultTTL           = 37,
   tcpKeepaliveInterval    = 38,
   tcpKeepaliveGarbage     = 39,
   nisDomainName           = 40,
   nisServers              = 41,
   ntpServers              = 42,
   vendorSpecificInfo      = 43,
   netBIOSnameServer       = 44,
   netBIOSdgramDistServer	= 45,
   netBIOSnodeType         = 46,
   netBIOSscope            = 47,
   xFontServer             = 48,
   xDisplayManager         = 49,
   dhcpRequestedIPaddr     = 50,
   dhcpIPaddrLeaseTime     = 51,
   dhcpOptionOverload      = 52,
   dhcpMessageType         = 53,
   dhcpServerIdentifier    = 54,
   dhcpParamRequest        = 55,
   dhcpMsg                 = 56,
   dhcpMaxMsgSize          = 57,
   dhcpT1value             = 58,
   dhcpT2value             = 59,
   dhcpClassIdentifier     = 60,
   dhcpClientIdentifier    = 61,
   endOption               = 255
};

/*
 * @brief DHCP message format
 */ 
typedef struct {
	uint8_t  op;            ///< DHCP_BOOTREQUEST or DHCP_BOOTREPLY
	uint8_t  htype;         ///< DHCP_HTYPE10MB or DHCP_HTYPE100MB
	uint8_t  hlen;          ///< DHCP_HLENETHERNET
	uint8_t  hops;          ///< DHCP_HOPS
	uint32_t xid;           ///< DHCP_XID  This increases one every DHCP transaction.
	uint16_t secs;          ///< DHCP_SECS
	uint16_t flags;         ///< DHCP_FLAGSBROADCAST or DHCP_FLAGSUNICAST
	uint8_t  ciaddr[4];     ///< Request IP to DHCP server
	uint8_t  yiaddr[4];     ///< Offered IP from DHCP server
	uint8_t  siaddr[4];     ///< No use 
	uint8_t  giaddr[4];     ///< No use
	uint8_t  chaddr[16];    ///< DHCP client 6bytes MAC address. Others are filled to zero
	uint8_t  sname[64];     ///< No use
	uint8_t  file[128];     ///< No use
	uint8_t  OPT[OPT_SIZE]; ///< Option
} RIP_MSG;

class dhcp
{
public:
	

    uint8_t DHCP_SOCKET;                      // Socket number for DHCP
    uint8_t DHCP_SIP[4];                      // DHCP Server IP address

    // Network information from DHCP Server
    uint8_t OLD_allocated_ip[4];        // Previous IP address
    uint8_t DHCP_allocated_ip[4];       // IP address from DHCP
    uint8_t DHCP_allocated_gw[4];       // Gateway address from DHCP
    uint8_t DHCP_allocated_sn[4];       // Subnet mask from DHCP
    uint8_t DHCP_allocated_dns[4];      // DNS address from DHCP

    int8_t dhcp_state;      // DHCP state
    int8_t dhcp_retry_count;

    uint32_t dhcp_lease_time;
    volatile uint32_t dhcp_tick_1s;       // unit 1 second
    uint32_t dhcp_tick_next;

    uint32_t DHCP_XID;                        // Any number
    RIP_MSG* pDHCPMSG;                        // Buffer pointer for DHCP processing
    uint8_t HOST_NAME[sizeof(DCHP_HOST_NAME)];
    uint8_t DHCP_CHADDR[6];                   // DHCP Client MAC address

    /*
    * @brief DHCP client initialization (outside of the main loop)
    * @param s   - socket number
    * @param buf - buffer for processing DHCP message
    */
    void DHCP_init(uint8_t s, uint8_t* buf);

    /*
    * @brief DHCP 1s Tick Timer handler
    * @note SHOULD BE registered to your system 1s Tick timer handler 
    */
    void DHCP_time_handler(void);

    /* 
    * @brief Register call back function 
    * @param ip_assign   - callback func when IP is assigned from DHCP server first
    * @param ip_update   - callback func when IP is changed
    * @param ip_conflict - callback func when the assigned IP is conflict with others.
    */
    void reg_dhcp_cbfunc(void(*ip_assign)(void), void(*ip_update)(void), void(*ip_conflict)(void));

    /*
    * @brief DHCP client in the main loop
    * @return    The value is as the follow \n
    *            DHCP_FAILED     \n
    *            DHCP_RUNNING    \n
    *            DHCP_IP_ASSIGN  \n
    *            DHCP_IP_CHANGED \n
    * 	      DHCP_IP_LEASED  \n
    *            DHCP_STOPPED    \n
    *
    * @note This function is always called by your main task.
    */ 
    uint8_t DHCP_run(void);

    /*
    * @brief Stop DHCP processing
    * @note If you want to restart, call DHCP_init() and DHCP_run()
    */ 
    void DHCP_stop(void);

    /* Get Network information assigned from DHCP server */
    /*
    * @brief Get IP address
    * @param ip  - IP address to be returned
    */
    void getIPfromDHCP(uint8_t* ip);
    /*
    * @brief Get Gateway address
    * @param ip  - Gateway address to be returned
    */
    void getGWfromDHCP(uint8_t* ip);
    /*
    * @brief Get Subnet mask value
    * @param ip  - Subnet mask to be returned
    */
    void getSNfromDHCP(uint8_t* ip);
    /*
    * @brief Get DNS address
    * @param ip  - DNS address to be returned
    */
    void getDNSfromDHCP(uint8_t* ip);

    /*
    * @brief Get the leased time by DHCP server
    * @return unit 1s
    */
    uint32_t getDHCPLeasetime(void);

    void my_ip_assign(void);

    void my_ip_conflict(void);

    void wizchip_setnetinfo(wiz_NetInfo_t* pnetinfo);

    void reset_DHCP_timeout(void);

    void makeDHCPMSG(void);

    void send_DHCP_DISCOVER(void);

    void send_DHCP_REQUEST(void);

    void send_DHCP_DECLINE(void);
		
		void Dhcp_Run(void);

    int8_t parseDHCPMSG(void);

    uint8_t check_DHCP_timeout(void);

    int8_t check_DHCP_leasedIP(void);

	


};

void default_ip_assign(void);
void default_ip_update(void);
void default_ip_conflict(void);

void my_ip_assign_wrapper(void);
void my_ip_conflict_wrapper(void);

extern wiz_NetInfo_t gWIZNETINFO;
extern uint16_t ipPort;

#endif	/* _DHCP_H_ */
