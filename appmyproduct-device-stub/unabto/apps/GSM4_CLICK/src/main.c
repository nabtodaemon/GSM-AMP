#include <unabto/unabto_common_main.h>
#include <unabto/unabto_app.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void SerialPortInit();
int  Send2Gsm(char *);
void DelayS(int);
void GetString(char *, char, int *);
int  GetSocketNo();

	
HANDLE hComm;                           // Handle to the Serial port
char   ComPortName[] = "COM3";			// Name of the Serial port(May Change) to be opened,
BOOL   Status;
DWORD dwEventMask;                      // Event mask to trigger
char  TempChar;                         // Temperory Character
char  SerialBuffer[1024];               // Buffer Containing Rxed Data
DWORD NoBytesRead;                      // Bytes read by ReadFile()
int   ResponseLength = 0, SocketNo;
char  BS_ipaddr[32];					// Base Station IP address
int main() {
	
	
		char AtCmdBuff[256];
        int k=0;
		SerialPortInit();
		FlushFileBuffers(hComm);
	

		Send2Gsm("ATE0\r\n");
		DelayS(500);
		Send2Gsm("AT+COPS?\r\n");
		DelayS(500);
	
		//Send2Gsm("AT&V\r\n");
		//DelayS(500);
		Send2Gsm("AT+CGATT?\r\n");
		DelayS(500);
		//Send2Gsm("AT+UPSDA=0,3\r\n"); 			//activate GPRS
		//DelayS(500);
		//Send2Gsm("AT+UPSD=0,1,\"airtelgprs.pr\"\r\n"); 		//check assigned IP addr
		//DelayS(500);
		Send2Gsm("AT+UPSDA=0,3\r\n"); 			//activate GPRS
		DelayS(500);
		Send2Gsm("AT+UPSDA=0,0\r\n"); 			//activate GPRS
		DelayS(500);
		//Send2Gsm("AT+UDNSRN=0,\"google.com\"\r\n");
		//DelayS(500);
		//Send2Gsm("AT+UDNSRN=0,\"daemon.co.in\"\r\n");
		//DelayS(500);
	
		
		int i=0;
		GetString(BS_ipaddr, '"', &i);
		DelayS(500);
		
		
						
		if (Send2Gsm("AT+USOCR=17\r\n") != 0) {			//UDP socket creation
			printf("\nRetrying...\n");
			DelayS(500);
			Send2Gsm("AT+USOCR=17\r\n");
		}
		DelayS(500);
		SocketNo = GetSocketNo();
		Send2Gsm("AT+USOCR=17,12000\r\n");  	//specify local port to be used for sending data
		DelayS(500);
			
		
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
	
	
    nabto_main_setup* nms = unabto_init_context();
    nms->id = "myid.example.net";
    unabto_init();
    while(true) {
        unabto_tick();
    }
}

application_event_result application_event(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer) {
    return AER_REQ_INV_QUERY_ID;
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