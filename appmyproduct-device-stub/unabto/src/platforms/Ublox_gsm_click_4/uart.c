/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_PERIPHERAL

#include "uart.h"
#include <unabto/util/unabto_queue.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_common_main.h>
#include <modules/network/Ublox_gsm/unabto_gsm4click.h>

#define RECEIVE_QUEUE_BUFFER_SIZE     4096
#define TRANSMIT_QUEUE_BUFFER_SIZE    4096
#define UART_NUMBER_OF_CHANNELS 4

typedef struct
{
  queue_t receiveQueue;
  uint8_t receiveQueueBuffer[RECEIVE_QUEUE_BUFFER_SIZE];
  HANDLE hPort;
  DCB dcb;
  COMMTIMEOUTS timeouts;
} uart_channel;

HANDLE hComm;                           // Handle to the Serial port

BOOL   Status;
DWORD dwEventMask;                      // Event mask to trigger
char  TempChar;                         // Temperory Character
char  SerialBuffer[1024];               // Buffer Containing Rxed Data
DWORD NoBytesRead;                      // Bytes read by ReadFile()
int   ResponseLength = 0;
static bool low_level_read_from_uart(uart_channel* uartChannel);
static uart_channel* look_up_uart_channel(uint8_t channel);
static uart_channel channels[UART_NUMBER_OF_CHANNELS];

void uart_initialize(uint8_t channel, const void* name, uint32_t baudrate, uint8_t databits, uart_parity parity, uart_stopbits stopbits)
{ 
  const char* pipeIdentifier = "\\\\.\\pipe\\";
  int pipeIdentifierLength = strlen(pipeIdentifier);
    
  uart_channel* uartChannel = look_up_uart_channel(channel); 
  queue_init(&uartChannel->receiveQueue, uartChannel->receiveQueueBuffer, sizeof(uartChannel->receiveQueueBuffer)); 

/*
  char nameBuffer[1024];
  if (memcmp(pipeIdentifier, name, pipeIdentifierLength) != 0 && strlen(name) > 4 && ((const char*)name)[0] != '\\') // it's a com port higher than 9
  {
    sprintf_s(nameBuffer, sizeof(nameBuffer), "\\\\.\\%s", name);
    name = nameBuffer;
  }
  */
  uartChannel->hPort = CreateFileA((LPCSTR)name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (uartChannel->hPort == INVALID_HANDLE_VALUE)
  {

    //DWORD error = GetLastError();
    //char* errorString = strerror(error);
    //NABTO_LOG_TRACE(("Error:%l", error));
    NABTO_LOG_FATAL(("Unable to open COM port '%s'!", (const char*)name));
  }
  hComm = uartChannel->hPort;
 
  if (memcmp(pipeIdentifier, name, pipeIdentifierLength) == 0) // it's a pipe
  {
    DWORD mode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
	
    if (!SetNamedPipeHandleState(uartChannel->hPort, &mode, NULL, NULL))
    {
      //DWORD error = GetLastError();
      //char* errorString = strerror(error);
      //NABTO_LOG_TRACE(("Error:%l", error));
      NABTO_LOG_FATAL(("Unable to make pipe non-blocking '%s'!", (const char*)name));

    }
  }
  else // it's a COM port
  {
    // Configure com port
    memset(&uartChannel->dcb, 0, sizeof(DCB));
    uartChannel->dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(uartChannel->hPort, &uartChannel->dcb))
    {
      NABTO_LOG_FATAL(("Unable to get COM port state!"));
    }

    uartChannel->dcb.BaudRate = baudrate;

    if (databits < 5 || 8 < databits)
    {
      NABTO_LOG_FATAL(("Invalid number of databits for UART!"));
    }
    uartChannel->dcb.ByteSize = databits;
	
    switch (parity)
    {
      case UART_PARITY_NONE:
        uartChannel->dcb.Parity = NOPARITY;
        break;

      case UART_PARITY_EVEN:
        uartChannel->dcb.fParity = TRUE;
        uartChannel->dcb.fErrorChar = FALSE;
        uartChannel->dcb.Parity = EVENPARITY;
        break;

      case UART_PARITY_ODD:
        uartChannel->dcb.fParity = TRUE;
        uartChannel->dcb.fErrorChar = FALSE;
        uartChannel->dcb.Parity = ODDPARITY;
        break;

      case UART_PARITY_MARK:
        uartChannel->dcb.fParity = TRUE;
        uartChannel->dcb.fErrorChar = FALSE;
        uartChannel->dcb.Parity = MARKPARITY;
        break;

      case UART_PARITY_SPACE:
        uartChannel->dcb.fParity = TRUE;
        uartChannel->dcb.fErrorChar = FALSE;
        uartChannel->dcb.Parity = SPACEPARITY;
        break;

      default:
        NABTO_LOG_FATAL(("Invalid number of databits for UART!"));
    }

    switch (stopbits)
    {
      case UART_STOPBITS_ONE:
        uartChannel->dcb.StopBits = ONESTOPBIT;
        break;

      case UART_STOPBITS_TWO:
        uartChannel->dcb.StopBits = TWOSTOPBITS;
        break;

      default:
        NABTO_LOG_FATAL(("Invalid number of stopbits for UART!"));
    }

    uartChannel->dcb.fBinary = TRUE;

    if (!SetCommState(uartChannel->hPort, &uartChannel->dcb))
    {
      NABTO_LOG_FATAL(("Unable to set COM port state!"));
    }

    // set event masks so the receive queue can be polled.
    if (!SetCommMask(uartChannel->hPort, EV_RXCHAR | EV_ERR))
    {
      DWORD error = GetLastError();
      NABTO_LOG_FATAL(("Unable to set COM port state (%lu)!", error));
    }

    // set read timeouts so ReadFile performs non-blocking reads.
    // RMW the port's timeouts structure
    if (!GetCommTimeouts(uartChannel->hPort, &uartChannel->timeouts))
    {
      NABTO_LOG_FATAL(("Unable to get COM port timeouts!"));
    }

    uartChannel->timeouts.ReadIntervalTimeout = 50;
    uartChannel->timeouts.ReadTotalTimeoutConstant = 1000;
    uartChannel->timeouts.ReadTotalTimeoutMultiplier = 10;
    uartChannel->timeouts.WriteTotalTimeoutConstant = 50;
    uartChannel->timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(uartChannel->hPort, &uartChannel->timeouts))
    {
      NABTO_LOG_FATAL(("Unable to set COM port timeouts!"));
    }
  }
 

    //char AtCmdBuff[256];
	Send2Gsm("ATE0\r\n");					//stop echo
	DelayS(100);
	Send2Gsm("AT+UDCONF=1,1\r\n");			//enable hex mode
	DelayS(100);
	Send2Gsm("AT+UPSDA=0,3\r\n"); 			//activate GPRS
	DelayS(100);
	Send2Gsm("AT+UPSND=0,0\r\n"); 			//activate GPRS
	DelayS(100);		
}
 
int Send2Gsm(char *lpBuffer)   //UART write
{
	/*----------------------------- Writing a Character to Serial Port----------------------------------------*/

	DWORD  dNoOFBytestoWrite;              // No of bytes to write into the port
	DWORD  dNoOfBytesWritten = 0;          // No of bytes written to the port
	int j = 0;

	memset(SerialBuffer, '\0', sizeof(SerialBuffer));
	dNoOFBytestoWrite = strlen(lpBuffer);   // Calculating the no of bytes to write into the port

	Status = WriteFile(hComm,               // Handle to the Serialport
		lpBuffer,            // Data to be written to the port 
		dNoOFBytestoWrite,   // No of bytes to write into the port
		&dNoOfBytesWritten,  // No of bytes written to the port
		NULL);

	if (Status == TRUE)
		NABTO_LOG_TRACE(("%s", lpBuffer));
	else
		NABTO_LOG_TRACE(("Error %d in Writing to Serial Port", GetLastError()));

	DelayS(1000);

	j=ReadGsm();
	return j;
}

int ReadGsm() // UART read
{
	int j = 0;
	//memset(SerialBuffer, 0, sizeof(SerialBuffer));
        /*------------------------------------ Setting WaitComm() Event   ----------------------------------------*/
		
	    NABTO_LOG_TRACE(("Waiting for Data Reception"));

		Status = WaitCommEvent(hComm, &dwEventMask, NULL); //Wait for the character to be received
	
		/*-------------------------- Program will Wait here till a Character is received ------------------------*/				

		if (Status == FALSE)
			{
			NABTO_LOG_TRACE(("Error! in Setting WaitCommEvent()"));
			}
		else //If  WaitCommEvent()==True Read the RXed data using ReadFile();
			{	
				ResponseLength = 0;
				do
					{
						Status = ReadFile(hComm, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
						SerialBuffer[ResponseLength] = TempChar;
						ResponseLength++;
				    }
				while (NoBytesRead > 0);
					
					/*------------Printing the RXed String to Console----------------------*/

				
			    //for (j = 0; j < ResponseLength-1; j++)		// j < ResponseLength-1 to remove the dupliated last character
				NABTO_LOG_TRACE(("%s", SerialBuffer));
				
		}
		
		j = strncmp(SerialBuffer+j-4, "OK", 2);
		return j;
	}
	
	void DelayS(int secs) //Time Delay for GSM
	{
		SleepEx(secs,0);
	}
 
	


void GetStringIP(char *addr, char delimiter, int *startpos) //UART data parse
	{
		int j, startstr=0, endstr=0;
		for (j = *startpos; j < strlen(SerialBuffer)-1; j++) {		// j < ResponseLength-1 to remove the dupliated last character
			if (SerialBuffer[j] == delimiter) {
				if (!startstr)
					startstr = j+1;
				else {
					endstr = j-1;
					break;
				}
			}
		}
		*startpos = j;
		if (endstr > startstr)
			sprintf(addr, "%.*s", endstr - startstr + 1, SerialBuffer + startstr);	
	}





void uart_shutdown(uint8_t channel)
{
  uart_channel* uartChannel = look_up_uart_channel(channel);

  if (uartChannel->hPort != INVALID_HANDLE_VALUE)
  {
    CloseHandle(uartChannel->hPort);
    uartChannel->hPort = INVALID_HANDLE_VALUE;
  }
}

uint16_t uart_can_read(uint8_t channel)
{
  uart_channel* uartChannel = look_up_uart_channel(channel);

  while (low_level_read_from_uart(uartChannel));

  return queue_count(&uartChannel->receiveQueue);
}

uint16_t uart_can_write(uint8_t channel)
{
  look_up_uart_channel(channel); // just used to validate channel number

  return TRANSMIT_QUEUE_BUFFER_SIZE; // assuming we can always write a lot of data to it due to OS buffering
}

uint8_t uart_read(uint8_t channel)
{

  uint8_t value;

  uart_channel* uartChannel = look_up_uart_channel(channel);

  // block until data is received
  while (queue_is_empty(&uartChannel->receiveQueue))
  {
    if (low_level_read_from_uart(uartChannel) == false)
    {
      Sleep(1);
    }
  }

  queue_dequeue(&uartChannel->receiveQueue, &value);

  strcpy(GsmResponseStr, uartChannel->receiveQueueBuffer);

  //printf("RESPONSE: %s", GsmResponseStr);

  return value;
}

void uart_read_buffer(uint8_t channel, void* buffer, uint16_t length)
{
	//ReadFile(hComm, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);

  uint8_t* b = (uint8_t*)buffer;
  uint16_t originalLength = length;

  while (length--)
  {
    *b++ = uart_read(channel);
  }

  NABTO_LOG_TRACE(("Read %u bytes from UART.", (int)originalLength));
}

void uart_write(uint8_t channel, uint8_t value)
{
	
  uart_write_buffer(channel, &value, 1);

}

void uart_write_buffer(uint8_t channel, const void* buffer, uint16_t length)
{
	
  DWORD bytesWritten;
  uart_channel* uartChannel = look_up_uart_channel(channel);
  int i = 0;

  WriteFile(uartChannel->hPort, buffer, length, &bytesWritten, NULL);
  
  if (length != bytesWritten)
  {
    NABTO_LOG_WARN(("Unable to write all bytes to the UART (only %u out of %u was written)!", (int)bytesWritten, (int)length));
  }

  NABTO_LOG_TRACE(("Wrote %u bytes to UART.", (int)bytesWritten));
}

void uart_flush_receiver(uint8_t channel)
{
  uart_channel* uartChannel = look_up_uart_channel(channel);

  while (low_level_read_from_uart(uartChannel));

  queue_reset(&uartChannel->receiveQueue);
}

void uart_flush_transmitter(uint8_t channel)
{
  look_up_uart_channel(channel); // just used to validate channel number

  // can't do anything here
}

// move as much data as possible from the UART OS buffer to the UART driver receive buffer.
static bool low_level_read_from_uart(uart_channel* uartChannel)
{
  uint8_t buffer[1024];
  DWORD count;
  uint16_t length;

  count = queue_free(&uartChannel->receiveQueue);

  if (count == 0) // room for more bytes in the driver receive queue?
  {
    return false; // no
  }

  if (count > sizeof(buffer)) // limit count
  {
    count = sizeof(buffer);
  }

  // try to read as many bytes as there is room for in the driver receive queue
  if (ReadFile(uartChannel->hPort, buffer, count, &count, 0) == false)
  {
    return false; // read failed for some reason
  }

  // no bytes received?
  if (count == 0)
  {
    return false; // no bytes received
  }

  // no bytes received?
  if (count > 0xffff)
  {
    NABTO_LOG_FATAL(("Received an exorbitant amount of data from the UART!"));
    return false; // something is totally wrong
  }

  length = (uint16_t)count;

  NABTO_LOG_TRACE(("Queued %u bytes in UART driver receive buffer.", length));

  queue_enqueue_array(&uartChannel->receiveQueue, buffer, length);

  return true;
}

static uart_channel* look_up_uart_channel(uint8_t channel)
{
  if (channel >= UART_NUMBER_OF_CHANNELS)
  {
    NABTO_LOG_FATAL(("Invalid UART channel specified!"));
    return NULL;
  }

  return &channels[channel];
}


