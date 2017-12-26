#ifndef _UNABTO_PLATFORM_H_
#define _UNABTO_PLATFORM_H_

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>


#include "unabto_platform_types.h"

#include <platforms/unabto_common_types.h>

#define NABTO_INVALID_SOCKET -1

#define nabtoMsec2Stamp(msec) (msec)

#define NABTO_LOG_BASIC_PRINT(severity, message)

#define NABTO_FATAL_EXIT while(1);

#endif

