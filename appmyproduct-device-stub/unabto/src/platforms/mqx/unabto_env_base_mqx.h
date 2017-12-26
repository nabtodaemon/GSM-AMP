/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_MQX_H_
#define _UNABTO_ENV_BASE_MQX_H_

/**
 * @file
 * The Basic environment for the Nabto Micro Device Server (MQX device), Interface.
 *
 * This file holds definitions, declarations and prototypes to be supplied by the host.
 */

#include <stdint.h>
#include <string.h>
#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>

typedef unsigned char bool;

#define true 1
#define false 0

typedef int ssize_t;

typedef uint32_t nabto_socket_t;
        /**
         * Defined as in netinet/in.h
         */
#define INADDR_NONE 0xffffffffu
#define NABTO_INVALID_SOCKET RTCS_SOCKET_ERROR

/************ The Time Handling basics (PC Device), Interface *************/

/** The timestamp definition. @return */
typedef uint32_t nabto_stamp_t;
typedef int32_t  nabto_stamp_diff_t;

/**
 * Convert milleseconds to nabto_stamp_t difference (duration)
 * @param msec  number of milliseconds
 * @return      the stamp difference
 */
#define nabtoMsec2Stamp(msec)   (msec)

#endif
