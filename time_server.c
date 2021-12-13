#if define(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
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

// detect if compiler is running on windows ^
// define some macros and abstract difference between
// winsock and berkely sockets

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else 
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

//standard c header files 

#include <stdio.h>
#include <string.h>
#include <time.h>

//initialize winsock if compiling on windows (ewwww)

int main(){
    WSADATA d;
    if (WSAStartup(MAKEWORD(2,2), &d)) {
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }

    //figure out local address that our web server should bind to

    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    //use getaddrinfo() to fill in a struct addrinfo structre with the
    //necessary info. 
    //takes hints as parameter, and zero it out using memst()
    //set ai_family = AF_INET (IPv4) 
    //set ai_socktype to SOCK_STREAM, indicated using TCP (could use 
    //SOCK_DGRAM for UDP if you want
    //ai_flags = AI_PASSIVE, which tells getaddrinfo() that we want
    //to bind a wildcard address (i.e. listen on any available network
    //interface
    //
    /*
     * once hints is set up, we declare a pointer to struct addrinfo
     * structure, which holds return information from getaddrinfo(), which 
     * essentially generates for us a address for bind().
     * To generate that, we need to pass first parameter as NULL and have
     * AI_PASSIVE flag set in hints.ai_flags.
     *
     * second parameter for getaddrinfo() is the port we're listening
     * for connections on. 
     * IMPORTANT: only privileged users on UNIX OS can bind to ports 0-1023.
     */
    getaddrinfo(0, "8080", &hints, &bind_address);

    // now create the socket
   /* define socket_listen as SOCKET type (macroed above as int)
    * call socket() to generate actual socket. 
    *
    * using getaddrinfo() first and then passing parts of bind_address into
    * arguments of socket make it easier to change the protocol
    */
    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);

    // check socket() for success

    if (!ISVALIDSOCKET(socket_listen)){
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
}
