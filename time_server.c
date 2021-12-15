#if defined(_WIN32)
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
#if defined(_WIN32) 
    WSADATA d;
    if (WSAStartup(MAKEWORD(2,2), &d)) {
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }
#endif

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

    // call bind() to associate it with out address form getaddrinfo()
   //if bind() fails, trying using another port. Release address memory
   //with freeaddrinfo()
    printf("Binding socket to local address...\n");
    if (bind(socket_listen, 
                bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
        }
        freeaddrinfo(bind_address);
    //start listing for connections

    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0){
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    //10 is the number of connections allowed to queue up. OS will reject new
    //connections until one is removed from queue
    //
    //accept incoming connections

    printf("Waiting for connection...\n");
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET socket_client = accept(socket_listen,
        (struct sockaddr*) &client_address, &client_len);
    if (!ISVALIDSOCKET(socket_client)){
        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    /*accept() will block program until new connection is made (sleep), and
     * once connection is made, accept() will create a new socket. The
     * original socket will continue to listen for new connections, but the
     * new socket returned by accept() can be used to send and receive data over
     * the newly established connection -- accept() will fill in address info
     * of client that is connected
     *
     * need to declare struct sockaddr_storage to store address of connecting client
     * this struct type is guaranteed to be large enough to hold the largest supported
     * address on the system. Also need to say the size of buffer (client_len)
     *
     * if accept() is successful, the TCP connection has been established
     */

    printf("Client is connected...");
    char address_buffer[100];
    getnameinfo((struct sockaddr*)&client_address,
            client_len, address_buffer, sizeof(address_buffer), 0,0,
            NI_NUMERICHOST);
    printf("%s\n", address_buffer);

    /* getnameinfo() takes client's address and length ( in order to work with
     * IPv4 and IPv6). Then pass output buffer and buffer length. getnameinfo()
     * also outputs service name and its length, but that's not really needed,
     * hence the zeroes. NI_NUMERICHOST flag is passed in order to specify
     * we want the hostname as the IP address.
     *
     * We want the client (browser) to send an http request /
     */

    printf("Reading request...\n");
    char request[1024];
    int bytes_received = recv(socket_client, request, 1024, 0);
    printf("Received %d bytes.\n", bytes_received);

    /* request buffer stores browser http request (1024 bytes allocated)
     * recv() returns number of bytes received. theoretically, recv() could 
     * return 0 or -1 if client terminates connection, and this should be
     * accounted for when writing a real program last param for recv() is for
     * flags, which we are not passing, hence a zero. 
     *
     * request received from client should follow HTTP protocol. In practice,
     * a web server would have to parse this out. 
     *
     * Now! To print browser's request to console 
     */

    printf("%.*s", bytes_received, request);
    //"%.*s" tells print() to output a specific number of characters received
    //mistake would be to print data receieved form recv() directly as C string
    //since there's no guarantee that the data received is null terminated
    //(hence could spit out garbage or cause seg fault)

    printf("Sending response..\n");
    const char *response = 
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytes_sent = send(socket_client, response, strlen(response), 0);
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));

    /* char *response set to a standard HTTP response header. HTTP requires
     * line endings to take form of carriage return followed by new line char. 
     * send the data to the client using the send() function. send() returns
     * the number of bytes -- good idea to check if the number sent matches
     * what was expected. 
     *
     */

    time_t timer;
    time(&timer);
    char *time_msg = ctime(&timer);
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    printf("Sent %d bytes of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

    //now close connection to indicate all data being sent
    //
    
    printf("Closing connection...\n");
    CLOSESOCKET(socket_client);

    //in theory, we could call accept() to take more connections, but for now
    //we can end it here

    printf("closing listening socket...\n");
    CLOSESOCKET(socket_listen);

#if defined(_WIN32)
    WSACleanup();
#endif

    printf("Finished.\n");

    return 0;
}
