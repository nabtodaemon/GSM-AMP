/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_H_
#define _UNABTO_PLATFORM_H_



#include "unabto_platform_types.h"
#include <platforms/unabto_common_types.h>

#include <modules/log/unabto_log_header.h>

#include <stdio.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NABTO_LOG_BASIC_PRINT
	/** Print debugging info.
	* @param loglevel  logging level
	* @param cmsg      the message
	*/
#define NABTO_LOG_BASIC_PRINT(loglevel, cmsg) do {    \
    unabto_log_header(__FILE__, __LINE__);             \
    printf cmsg;                                      \
    printf("\n");                                     \
    fflush(stdout);                                   \
} while(0)
#endif

//#define NABTO_LOG_BASIC_PRINT(Severity,msg) Ql_DebugTrace msg

//extern APP_CONFIG AppConfig; // expose the TCP/IP configuration structure so the application can access it.


#ifdef __cplusplus
} //extern "C"
#endif

#endif
