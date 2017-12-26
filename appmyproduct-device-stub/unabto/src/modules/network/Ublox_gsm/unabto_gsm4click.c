/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

/**
 * @file
 * Provides the required socket operations for the uNabto framework on top of Winsock.
 *
 */

#include <windows.h>
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_util.h>
#include "unabto_gsm4click.h"
#include <unabto/unabto_context.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_common_main.h>

#include <modules/network/select/unabto_network_select_api.h>
#include "platforms/Ublox_gsm_click_4/uart.h"
#include <modules/list/utlist.h>

char AtCmdBuff[256]; //buffer to store AT data


typedef struct socketListElement {
    nabto_socket_t socket;
    struct socketListElement *prev;
    struct socketListElement *next;
} socketListElement;

static struct socketListElement* socketList = 0;

//static bool unabto_gsm_initialize(void);
//static void unabto_gsm_shutdown(void);

bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* sock)
{
	nabto_socket_t sd = INVALID_SOCKET;   
    socketListElement* se;
/*
    if(!unabto_gsm_initialize())
    {
        return false;
    }
*/	

	int j;	
	sprintf(AtCmdBuff, "AT+USOCR=17,%d\r\n", *localPort);
	j=Send2Gsm(AtCmdBuff);  // enable udp socket with local port
	if (j == 0) {
		NABTO_LOG_ERROR(("UDP_socket_creation_failed"));
		return false;
	}
	for (j = 0; j < strlen(SerialBuffer)-1; j++) {		// j < ResponseLength-1 to remove the dupliated last character
		if (SerialBuffer[j] == ':')
			break;
	}
	sd = (SerialBuffer[j+2] - '0');
	
    if (sd == INVALID_SOCKET)
    {		
        return false;
	}     
	

    se = (socketListElement*)malloc(sizeof(socketListElement));
    
    if (!se) {
        NABTO_LOG_ERROR(("Malloc of a single small list element should not fail!"));
        closesocket(sd);
        return false;
    }
	*sock = sd;	
    se->socket = sd;
    DL_APPEND(socketList, se);
    return true;
}

void nabto_close_socket(nabto_socket_t* sock)
{
    if (sock && *sock != INVALID_SOCKET)
    {
		
        socketListElement* se;
        socketListElement* found = 0;
        DL_FOREACH(socketList,se) {
            if (se->socket == *sock) {
                found = se;
                break;
            }
        }
        if (!found) {
            NABTO_LOG_ERROR(("Socket %i Not found in socket list", *sock));
        } else {
            DL_DELETE(socketList, se);
            free(se);
        }
	
		sprintf(AtCmdBuff, "AT+USOCL=%d\r\n", *sock); //close udp socket
		Send2Gsm(AtCmdBuff);
        *sock = INVALID_SOCKET;
    }
}

ssize_t nabto_read(nabto_socket_t sock, uint8_t* buf, size_t len, uint32_t* addr, uint16_t* port)
{ 

	int i = 0;
	long x;
	char temp[512];
	char lenght[10];
	nabto_endpoint ep;
	sprintf(AtCmdBuff, "AT+USORF=%d,512\r\n", sock); //GSM read 
	Send2Gsm(AtCmdBuff);
	len = 0; //init
	
	if (strlen(SerialBuffer) < 30) { 	
		return len; }	
		GetString(temp, '"', &i);		
		x = inet_addr(temp);
		*addr = ntohl(x);		
		GetString(temp, ',', &i);
		//*port = ntohs(atoi(temp));		
		*port = atoi(temp);
		GetString(lenght, ',', &i);
		len = atoi(lenght);
		GetString(temp, '"', &i);	
		
		int fchar, schar, newval;   // convert hex to binary
		for (i = 0; i < len*2; i += 2) {
			//fchar = temp[i] - '0';
			switch (temp[i]) {
			case 'A': fchar = 10; break;
			case 'B': fchar = 11; break;
			case 'C': fchar = 12; break;
			case 'D': fchar = 13; break;
			case 'E': fchar = 14; break;
			case 'F': fchar = 15; break;
			default: fchar = temp[i] - '0';
			}
			switch (temp[i + 1]) {
				case 'A': schar = 10; break;
				case 'B': schar = 11; break;
				case 'C': schar = 12; break;
				case 'D': schar = 13; break;
				case 'E': schar = 14; break;
				case 'F': schar = 15; break;
				default: schar = temp[i + 1] - '0';
			}
			newval = fchar * 16 + schar;
			WRITE_U8(buf, newval); buf += 1;			
		}		
		ep.addr = *addr;
		ep.port = *port;
	return len;
}

	void GetString(char *address, char delimiter, int *startpos)
	{
		int j, startstr = 0, endstr=0; 
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
			sprintf(address, "%.*s", endstr - startstr + 1, SerialBuffer + startstr);
	}
	


ssize_t nabto_write(nabto_socket_t sock, const uint8_t* buf, size_t len, uint32_t addr, uint16_t port)
{  
   char address[64];
   nabto_endpoint ep;
   int i = 0;
   sprintf(address, PRIip, MAKE_IP_PRINTABLE(addr));
   sprintf(AtCmdBuff, "AT+USOST=%d,\"%s\",%d,%d\r\n", sock, address, port, len);  //GSM write 
   Send2Gsm(AtCmdBuff);
   DelayS(500);
   for (i = 0; i < len; i++) {
	   uart_write(1, buf[i]);
   }
   ReadGsm();
   ep.addr = addr;
   ep.port = port;
   return true;
}

#if 0 //not used

bool unabto_winsock_initialize(void)
{
    static bool initialized = false;

    if (!initialized)
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
        wVersionRequested = MAKEWORD(2, 2);

        err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0)
        {
            /* Tell the user that we could not find a usable */
            /* Winsock DLL.                                  */
        
            NABTO_LOG_ERROR(("WSAStartup failed with error: %d\n", err));
            return false;
        }

        /* Confirm that the WinSock DLL supports 2.2.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.2 in addition to 2.2, it will still return */
        /* 2.2 in wVersion since that is the version we      */
        /* requested.                                        */

        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            NABTO_LOG_ERROR(("Could not find a usable version of Winsock.dll\n"));
            WSACleanup();
            return false;
        }

        atexit(uart_shutdown);

        initialized = true;
    }

    return initialized;
}

void unabto_winsock_shutdown(void)
{
    WSACleanup();
}
#endif
/**
 * Return a list of sockets which can be read from.
 */
static uint16_t nabto_read_events(nabto_socket_t* sockets, uint16_t maxSockets, int timeout) {
    fd_set read_fds;
    unsigned int max_fd = 0;

    struct timeval timeout_val;
    int nfds;
    int n = 0;
                        
    nabto_main_context* nmc = unabto_get_main_context();

    timeout_val.tv_sec = timeout/1000;
    timeout_val.tv_usec = (timeout*1000)%1000000;

    FD_ZERO(&read_fds);
    max_fd = 0;

    // Rearm the selectors for each call

#if NABTO_ENABLE_LOCAL_ACCESS
    if (nmc->socketLocal != NABTO_INVALID_SOCKET) {
        FD_SET(nmc->socketLocal, &read_fds);
        max_fd = MAX(max_fd, nmc->socketLocal);
    }
#endif
#if NABTO_ENABLE_REMOTE_ACCESS
    if (nmc->socketGSP != NABTO_INVALID_SOCKET) {
        FD_SET(nmc->socketGSP, &read_fds);
        max_fd = MAX(max_fd, nmc->socketGSP);
    }
#endif
    
    nfds = select(max_fd+1, &read_fds, NULL, NULL, &timeout_val);
    if (nfds < 0) {
        NABTO_LOG_ERROR(("Select failed %i", WSAGetLastError()));
    }
    if (nfds > 0) {
#if NABTO_ENABLE_LOCAL_ACCESS
        if (nmc->socketLocal != NABTO_INVALID_SOCKET && FD_ISSET(nmc->socketLocal, &read_fds)) {
            if (n < maxSockets) {
                sockets[n++] = nmc->socketLocal;
            }
        }
#endif
#if NABTO_ENABLE_REMOTE_ACCESS
        if (nmc->socketGSP != NABTO_INVALID_SOCKET && FD_ISSET(nmc->socketGSP, &read_fds)) {
            if (n < maxSockets) {
                sockets[n++] = nmc->socketGSP;
            }
        }
#endif
    }
    return n;
}



void unabto_network_select_add_to_read_fd_set(fd_set* readFds, int* maxReadFd) {
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        FD_SET(se->socket, readFds);
        if (se->socket != INVALID_SOCKET && ((int)se->socket) > *maxReadFd)
           *maxReadFd = se->socket;
    }
}

void unabto_network_select_read_sockets(fd_set* readFds) {
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        if (FD_ISSET(se->socket, readFds)) {
            unabto_read_socket(se->socket);
        }
    }
}
/*
bool nabto_get_local_ip(uint32_t* ip) {
    struct sockaddr_in si_me, si_other;
    int s;
    
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        NABTO_LOG_ERROR(("Cannot create socket"));
        return false;
    }
    
    memset(&si_me, 0, sizeof(si_me));
    memset(&si_other, 0, sizeof(si_me));
    //bind to local port 4567
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(4567);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //"connect" google's DNS server at 8.8.8.8 , port 4567
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(4567);
    si_other.sin_addr.s_addr = htonl(0x08080808);
    if(connect(s,(struct sockaddr*)&si_other,sizeof si_other) == -1) {
        NABTO_LOG_ERROR(("Cannot connect to host"));
        return false;
    }

    {
        struct sockaddr_in my_addr;
        int len = sizeof my_addr;
        if(getsockname(s,(struct sockaddr*)&my_addr,&len) == -1) {
            NABTO_LOG_ERROR(("getsockname failed"));
            return false;
        }
        *ip = ntohl(my_addr.sin_addr.s_addr);
    }
    closesocket(s);
    return true;
}
*/