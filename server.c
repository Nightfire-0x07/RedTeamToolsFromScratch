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

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt =1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    //creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //forcefully attaching scoket to port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //forcefully attaching socket to port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failure");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    valread = read( new_socket , buffer, 1024);
    printf("%s\n", buffer);
    send(new_socket , hello , strlen(hello) , 0);
    printf("Hello message sent\n");
    return 0;
}
