//unistd provides access to POSIX operating system API
//made up mostly of system call wrapper functions such
//as fork, pipe, I/O primitaves like read, write, close,
//at etc.
#include <unistd.h>
#include <stdio.h>
//include Internet Protocol family of functions and structures to use to 
//create sockets and etc
#include <sys/socket.h>

//macros such as EXIT_SUCCESS and EXIT_FAILURE
//also the NULL pointer value is defined in this library
#include <stdlib.h>

//netinet/in.h includes definitions for internet protocol functions
// in_addri, sin_port, sin_addr, AF_INET
#include <netinet/in.h>

#include <string.h>
//defining our own macro here
#define PORT 8080

//main function takes and integer value and a character constant pointer array for command line arguments
int main(int argc, char const *argv[])
{
    //from the netinet header file, we are declaring variables that will hold
    //integers for the socket, server address, and a value
    int server_fd, new_socket, valread;

    //calling the structure sockaddr_in and naming the variable 'address' to reference the 
    //members of that structure
    struct sockaddr_in address;
    
    int opt=1;
    
    //storing the size of address in addrlen
    int addrlen = sizeof(address);
   
    //storing a 1024 bytes in a buffer array and assigning NULL to first possition of array
    char buffer[1024] = {0};
   
    //storing string in address of char array
    char *hello = "Hello from server";

    //creating socket file descriptor, AF_INET = IPv4 protocol, SOCK_STREAM can be tcp/udp
    //if the socket function returns a 0 to the server_fd, then print an error and quit
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //forcefully attaching socket to port 8080
    //if setsockopt() returns True, then print error and quit
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //from the structure sockaddr_in, we're now defining members from that structure previously
    //defined address variable
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    //htons() translates an short int from host byte order to network byte order
    address.sin_port = htons( PORT );

    //forcefully attaching socket to port 8080
    //if bind()returns value less than 0, print bind error and quit
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failure");
        exit(EXIT_FAILURE);
    }
    //if listen returns a -1, print error. listen() takes the passive socket, an a int value
    //for the backlog (number of times the pending connects to socket can grow). Would
    //normally return 0 if successful
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //accept connection on socket if return value is not -1, if -1 returned, print error
    //according to man7.org description: "...it extracts the first connection request on the 
    //queue of pending conenctions for the listening socket (server_fd) and creates a new 
    //connected socket, and returns a new file description (new_socket) referring to that socket
    //**The newly created socket is not in the listening state, and the original socket is 
    //unaffected by this call
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    //read up to the count (1024) bytes from the file descriptor (new_socket)
    //if successful, read returns the number of bytes, and file position is advanced by this number
    valread = read( new_socket , buffer, 1024);
    printf("%s\n", buffer);
    //once in a connected state, use the new socket and use the message found in buffer with its current length
    //set flag to 0
    send(new_socket , hello , strlen(hello) , 0);
    printf("Hello message sent\n");
    return 0;
}
