/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

/**
 * @file
 * Provides the required socket operations for the uNabto framework on top of Winsock.
 *
 */

//#include <modules/network/Ublox_gsm/unabto_gsm4click.h>
#include <unabto/unabto_external_environment.h>
#include <platforms/Ublox_gsm_click_4/uart.h>

void nabto_dns_resolve(const char* id)
{
}

nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addr)
{
	char AtCmdBuff[256]; 	
	//char dns_ip[64];
    //uint32_t addr = inet_addr(id);	
	sprintf(AtCmdBuff, "AT+UDNSRN=0,\"%s\"\r\n", id); // AT cammand for DNS
	Send2Gsm(AtCmdBuff);
	int i = 0;
	GetStringIP(dns_ip, '"', &i);
	uint32_t addr = inet_addr(dns_ip);

	
	//uint32_t addr = BS_ipaddr;
	/*
    if (addr == INADDR_NONE)
    {
        // host isn't a dotted IP, so resolve it through DNS
        struct hostent* he = gethostbyname(id);
        if (he == 0)
        {
            return NABTO_DNS_ERROR;
        }
        else
        {
            addr = *((uint32_t*)he->h_addr_list[0]);
        }
    }
	*/
    *v4addr = htonl(addr);

    return NABTO_DNS_OK;
}
