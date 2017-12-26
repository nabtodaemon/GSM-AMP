/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_TYPES_H_
#define _UNABTO_PLATFORM_TYPES_H_

#define NABTO_INVALID_SOCKET    -1
#define nabtoMsec2Stamp

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long ssize_t;

typedef int nabto_socket_t;
typedef ssize_t nabto_stamp_t;
typedef ssize_t nabto_stamp_diff_t;

enum
{
  false,
  true
};
typedef uint8_t bool;


#endif
