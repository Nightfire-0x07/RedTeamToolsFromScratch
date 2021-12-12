# if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
//pragma tells Microsoft Visual C compiler to link your program against the
//Winsock library ws3_32.lib
#pragma comment(lib, "ws2_32.lib")

#else 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif

#include <stdio.h> 

int main() {

#if defined(_WIN32)
    WSDATA d;
    if (WSAStartup(MAKEWORD(2,2), &d)){
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }
#endif

    printf("Ready to use Socket API\n");

#if defined(_WIN32)
    WSACleanup();
#endif 
    return 0;
}
