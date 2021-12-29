/* will be multiplexing with select() over polling and fork()
 * C protope for select():
 * int select(int nfds, fd_set *readfds, fd_set *writefds, 
 *  fd_set *exceptfds, struct timeval *timeout);
 *  Before calling select(), need to add sockets to fd_set, e.g.:
 *
 *  fd_set our sockets;
 *  FD_ZERO(&our_sockets();
 *  FD_SET(socket_listen, &our_sockets);
 *  FD_SET(socket_a, &our_sockets);
 *  FD_SET(socket_b, &our_sockets);
 *
 *  **important to zero our fd_set using FD_ZERO() before using
 *  have to set socket descriptors one at a time, and can be removed from
 *  fd_set using FD_CLR(), and can check for presence of socket in set using
 *  FD_ISSET().
 * 
 * select() also requires we pass a number that's larger than the largest
 * socket descriptor we are going to monitor //parameter ignored on Windows
 *e.g.:
 * SOCKET max_socket;
 * max_socket = socket_listen;
 * if (socket_a > max_socket) max_socket = socket_a;
 * if (socket_b > max_socket) max_socket = socket_b;
 *
 * when calling select(), it will modify fd_set of sockets to indicate which
 * sockets are ready -- need to copy an fd_set then call select():
 *
 * fd_set copy;
 * copy = our_sockets;
 *
 * select(max_socket+1, &copy, 0,0,0);
 *
 * this call blocks until at least one of the sockets are ready to read from
 * copy is modified so that it contains only sockets that are ready to be read
 * from. 
*/


#include "standard_headers.h"

#if defined(_WIN32)
#include <conio.h>
#endif

int main(int argc, char *argv[]){

#if defined(_WIN32)
        WSADATA d;
        if (WSAStartup(MAKEWORD(2,2), &d)){
            fprintf(stderr, "Failed to initialize.\n");
            return 1;
        }
#endif

       if (argc < 3){
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
       } 
       //first arg is program's name -- actual values stored in
       //argv[]

       printf("Configuring remote address...\n");
       struct addrinfo hints;
       memset(&hints, 0, sizeof(hints));
       hints.ai_socktype = SOCK_STREAM;
       struct addrinfo *peer_address;
       if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)){
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
        }
        //configured above for obtaining remote address
        // "we set hints.ai_socktype = SOCK_STREAM to tell getaddrinfo()
        // that we want a TCP connection". could also set SOCK_DGRAM to 
        // indicate a UDP connection

        /* for call to getaddrinfo(), pass hostname and port as first 
         * two arguments -- passed directly from command line. Remote
         * address is in the peer_address variable.
         */

       printf("Remote address is ");
       char address_buffer[100];
       char service_buffer[100];
       getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
               address_buffer, sizeof(address_buffer),
               service_buffer, sizeof(service_buffer),
               NI_NUMERICHOST);
       printf("%s %s\n", address_buffer, service_buffer);

       printf("Creating socket...\n");
       SOCKET socket_peer;
       socket_peer = socket(peer_address->ai_family,
               peer_address->ai_socktype, peer_address->ai_protocol);
       if (!ISVALIDSOCKET(socket_peer)){
           fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
           return 1;
       }

       printf("Connecting...\n");
       if (connect(socket_peer,
                   peer_address->ai_addr, peer_address->ai_addrlen)){
           fprintf(stderr, "connet() failed. (%d)\n", GETSOCKETERRNO());
           return 1;
        }
       freeaddrinfo(peer_address);

       //connect takes 3 args: socket, remote address, and remote address
       //length. Returns 0 on success. Connect() associates with a remote
       //address and initiates the TCP connection. 

        // so far, if all goes well, a TCP connection is made to remote
        // server
        //

       printf("Connected.\n");
       printf("To send data, enter text followed by enter.\n");

       //program now will loop checking both terminal and socket for new data

       while(1){
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
#if !defined(_WIN32)
        FD_SET(0, &reads);
#endif 
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket_peer+1, &reads, 0, 0, &timeout) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
       }
       /* fd_set reads will store socket set
        * set it to zero and then add our only socket, socket_peer
        *
        * non-Windows systems use select() to monitor for terminal input
        * 0 is descriptor for stdin
        * Windows select() only works for sockets, so cannot use it to monitor
        * for console input, hence timeout set to call for 100 ms, if no 
        * activity after 100ms, select() returns and we check for input 
        * manually
        */

        if (FD_ISSET(socket_peer, &reads)){
            char read[4096];
            int bytes_received = recv(socket_peer, read, 4096, 0);
            if (bytes_received < 1){
                printf("Connection closed by peer.\n");
                break;
            }
            printf("Received (%d bytes): %.*s",
                    bytes_received, bytes_received, read);
    }
            /* data from recv() is not null terminated, so need to use the 
             * weird %.*s format specifier which prints a string of a 
             * specified length. If recv() returns less than 1, then the 
             * connection has ended and we break out of loop to shut it down
             *
             */
            //check for terminal input
#if defined(_WIN32)
            if(kbhit()){
#else
            if(FD_ISSET(0, &reads)){
#endif
            char read[4096];
            if (!fgets(read, 4096, stdin)) break;
            //fgets() includes newline character from input, thus 
            //the sent input always ends with a newline
            printf("Sending: %s", read);
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            printf("Sent %d bytes.\n", bytes_sent);
            }

            /*if socket has closed, send() returns -1, which is ignored
             * since a closed socket causes select() to return immediately
             *
             * e.g. now could send a text file with command such as 
             * `cat my_file.txt | tcp_client 192.168.1.2 8080`
             *
             * "Doing proper piped and console input on Windows is possible
             * ... but it's very complicated." -- go figure
             */
            }

            printf("Closing socket...\n");
            CLOSESOCKET(socket_peer);

#if defined(_WIN32)
            WSACleanup();
#endif

            printf("Finished.\n");
            return 0;
}











