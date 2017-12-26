#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_logging.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>

HANDLE hComm;                           // Handle to the Serial port
	char   ComPortName[] = "COM3";			// Name of the Serial port(May Change) to be opened,
	BOOL   Status;
	DWORD dwEventMask;                      // Event mask to trigger
	char  TempChar;                         // Temperory Character
	char  SerialBuffer[1024];               // Buffer Containing Rxed Data
	DWORD NoBytesRead;                      // Bytes read by ReadFile()
	int   ResponseLength = 0, SocketNo;
	char  BS_ipaddr[32];	// Base Station IP address

void SerialPortInit()		
	{
		printf("\n\n +==========================================+");
		printf("\n |  Serial Transmission (Win32 API)         |");
		printf("\n +==========================================+\n");
		/*----------------------------------- Opening the Serial Port --------------------------------------------*/

		hComm = CreateFile( ComPortName,                       // Name of the Port to be Opened
							GENERIC_READ | GENERIC_WRITE,      // Read/Write Access
							0,                                 // No Sharing, ports cant be shared
							NULL,                              // No Security
							OPEN_EXISTING,                     // Open existing port only
							0,                                 // Non Overlapped I/O
							NULL);                             // Null for Comm Devices

		if (hComm == INVALID_HANDLE_VALUE)
			printf("\n   Error! - Port %s can't be opened", ComPortName);
		else 
			printf("\n   Port %s Opened\n ", ComPortName);

		
		/*------------------------------- Setting the Parameters for the SerialPort ------------------------------*/

		DCB dcbSerialParams = { 0 };                        // Initializing DCB structure
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		
		Status = GetCommState(hComm, &dcbSerialParams);     //retreives  the current settings

		if (Status == FALSE)
			printf("\n   Error! in GetCommState()");

		dcbSerialParams.BaudRate = CBR_115200;    // Setting BaudRate = 115200
		dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
		dcbSerialParams.StopBits = ONESTOPBIT;    // Setting StopBits = 1
		dcbSerialParams.Parity   = NOPARITY;      // Setting Parity = None 

		Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB 

		if (Status == FALSE)
			{
				printf("\n   Error! in Setting DCB Structure");
			}
		else
			{
				printf("\n   Setting DCB Structure Successfull\n");
				printf("\n       Baudrate = %d", dcbSerialParams.BaudRate);
				printf("\n       ByteSize = %d", dcbSerialParams.ByteSize);
				printf("\n       StopBits = %d", dcbSerialParams.StopBits);
				printf("\n       Parity   = %d", dcbSerialParams.Parity);
			}
		
		/*------------------------------------ Setting Timeouts --------------------------------------------------*/
		
		COMMTIMEOUTS timeouts = { 0 };

		timeouts.ReadIntervalTimeout         = 50;
		timeouts.ReadTotalTimeoutConstant    = 1000;
		timeouts.ReadTotalTimeoutMultiplier  = 10;
		timeouts.WriteTotalTimeoutConstant   = 50;
		timeouts.WriteTotalTimeoutMultiplier = 10;
		
		if (SetCommTimeouts(hComm, &timeouts) == FALSE)
			printf("\n   Error! in Setting Time Outs");
		else
			printf("\n\n   Setting Serial Port Timeouts Successfull");

		/*------------------------------------ Setting Receive Mask ----------------------------------------------*/
			
		Status = SetCommMask(hComm, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception
	
		if (Status == FALSE)
			printf("\n\n    Error! in Setting CommMask");
		else
			printf("\n\n    Setting CommMask successfull");
	}	
	

int Send2Gsm(char *lpBuffer)
	{
		/*----------------------------- Writing a Character to Serial Port----------------------------------------*/
		
		DWORD  dNoOFBytestoWrite;              // No of bytes to write into the port
		DWORD  dNoOfBytesWritten = 0;          // No of bytes written to the port
		int j =0;
		
		memset(SerialBuffer,'\0',sizeof(SerialBuffer));
		dNoOFBytestoWrite = strlen(lpBuffer);   // Calculating the no of bytes to write into the port

		Status = WriteFile(hComm,               // Handle to the Serialport
						   lpBuffer,            // Data to be written to the port 
						   dNoOFBytestoWrite,   // No of bytes to write into the port
						   &dNoOfBytesWritten,  // No of bytes written to the port
						   NULL);
		
		if (Status == TRUE)
			printf("\n\n    %s - Written to %s  %d   %d", lpBuffer, ComPortName, dNoOFBytestoWrite, dNoOfBytesWritten);
		else
			printf("\n\n   Error %d in Writing to Serial Port",GetLastError());

		//sleep(3);
		//DelayS(2000);
		
        /*------------------------------------ Setting WaitComm() Event   ----------------------------------------*/
		
		printf("\n\n    Waiting for Data Reception");

		Status = WaitCommEvent(hComm, &dwEventMask, NULL); //Wait for the character to be received
	
		/*-------------------------- Program will Wait here till a Character is received ------------------------*/				

		if (Status == FALSE)
			{
				printf("\n    Error! in Setting WaitCommEvent()");
			}
		else //If  WaitCommEvent()==True Read the RXed data using ReadFile();
			{
				printf("\n\n    Characters Received");
				ResponseLength = 0;
				do
					{
						Status = ReadFile(hComm, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
						SerialBuffer[ResponseLength] = TempChar;
						ResponseLength++;
				    }
				while (NoBytesRead > 0);

				/*------------Printing the RXed String to Console----------------------*/

				printf("\n\n    ");
				for (j = 0; j < ResponseLength-1; j++)		// j < ResponseLength-1 to remove the dupliated last character
					printf("%c", SerialBuffer[j]);
				
		}
		
		j = strncmp(SerialBuffer+j-4, "OK", 2);
		return j;
	}
	
	int GetSocketNo()
	{
		int j;
		for (j = 0; j < ResponseLength-1; j++) {		// j < ResponseLength-1 to remove the dupliated last character
			if (SerialBuffer[j] == ':')
				break;
		}
		return(SerialBuffer[j+2] - '0');
	}
	
	
	void GetString(char *addr, char delimiter, int *startpos)
	{
		int j, startstr=0, endstr=0;
		for (j = *startpos; j < ResponseLength-1; j++) {		// j < ResponseLength-1 to remove the dupliated last character
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
	
	void DelayS(int secs)
	{
		SleepEx(secs,0);
	}
	
	bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* sock) {
    nabto_socket_t sd;
    char AtCmdBuff[256];
    int k=0;
	SerialPortInit();
	FlushFileBuffers(hComm);
	Send2Gsm("ATE0\r\n");
	DelayS(500);
	Send2Gsm("AT+COPS?\r\n");
	DelayS(500);
	
	
	Send2Gsm("AT+CGATT?\r\n");
	DelayS(500);
	
	Send2Gsm("AT+UPSDA=0,3\r\n"); 			//activate GPRS
	DelayS(500);
	Send2Gsm("AT+UPSDA=0,0\r\n"); 			//activate GPRS
	DelayS(500);
	
	int i=0;
	GetString(BS_ipaddr, '"', &i);
	DelayS(500);
	
	if (Send2Gsm("AT+USOCR=17\r\n") != 0) {			//UDP socket creation
		printf("\nRetrying...\n");
		DelayS(500);
		Send2Gsm("AT+USOCR=17\r\n");
		}

    NABTO_LOG_TRACE(("Open socket: ip=" PRIip ", port=%u", MAKE_IP_PRINTABLE(localAddr), (int)*localPort));
    
    sd = GetSocketNo();
    if (sd == -1) {
        NABTO_LOG_ERROR(("Unable to create socket: (%i) '%s'.", errno, strerror(errno)));
        return false;
    }

    localAddr = BS_ipaddr;
    
    NABTO_LOG_TRACE(("Socket opened: ip=" PRIip ", port=%u", MAKE_IP_PRINTABLE(localAddr), (int)*localPort));
    
    return true;
}

void nabto_close_socket(nabto_socket_t* sock) {
    if (sock && *sock != NABTO_INVALID_SOCKET) {
        close(*sock);
        *sock = NABTO_INVALID_SOCKET;
    }
}

ssize_t nabto_read(hComm, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);{
int j, startstr=0, endstr=0;
		for (j = *startpos; j < ResponseLength-1; j++) {		// j < ResponseLength-1 to remove the dupliated last character
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


ssize_t nabto_write(hComm,               // Handle to the Serialport
						   lpBuffer,            // Data to be written to the port 
						   dNoOFBytestoWrite,   // No of bytes to write into the port
						   &dNoOfBytesWritten,  // No of bytes written to the port
						   NULL);{
	 sprintf(AtCmdBuff, "AT+USOST=%d,\"%s\",5562,5\r\n", SocketNo,BS_ipaddr);
		Send2Gsm(AtCmdBuff);
	
		DelayS(500);
		Send2Gsm("hello");
		DelayS(6000);
		sprintf(AtCmdBuff, "AT+USORF=%d,5\r\n", SocketNo);
		Send2Gsm(AtCmdBuff);
		//Send2Gsm("AT+USORF=1,4");
		DelayS(500);
}

