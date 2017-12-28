/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_GSM4CLICK_H_
#define _UNABTO_GSM4CLICK_H_

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** The socket type. */
typedef SOCKET nabto_socket_t;

int  Send2Gsm(char *);
void DelayS(int);
void GetString(char *, char, int *);

char AtCmdBuff[256]; //buffer to store AT data
	
HANDLE hComm;                           // Handle to the Serial port
//char   ComPortName[] = "COM3";			// Name of the Serial port(May Change) to be opened,
BOOL   Status;
DWORD dwEventMask;                      // Event mask to trigger
char  TempChar;                         // Temperory Character
char  SerialBuffer[1024];               // Buffer Containing Rxed Data
DWORD NoBytesRead;                      // Bytes read by ReadFile()
//int   ResponseLength = 0, SocketNo;
char  BS_ipaddr[32];					// Base Station IP address


/** Denoting an invalid socket */
#define NABTO_INVALID_SOCKET INVALID_SOCKET

#ifdef __cplusplus
extern "C" {
#endif

uint16_t nabto_read_events(nabto_socket_t* sockets, uint16_t maxSockets, int timeout);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
