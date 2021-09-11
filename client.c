#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080

//taking integer and pointer to character for arguments
int main(int argc, char const *argv[])
{
    //sock defined as 0 and valread
    int sock = 0, valread;
    //defining structure based off of sockaddr_in
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    //if sock is assigned a value that's negative, output error
    //means the variable didn't receive an ip address or port for 
    //some reason
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    //assigned ip address to struct and assigned the converted PORT to network
    //byte order
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    // inet_pton converts character string of source onto network address
    // structure in the af address family, the copies network address structure
    // to dst. If the integer returned is negative, print error
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address or Address not supported \n");
        return -1;
    }
    // connect() connects socket specified to address specified by addr
    // if integer returned is negative, print error
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Connection Failed \n");
        return -1;
    }
    
    send(sock , hello , strlen(hello) , 0);
    printf("Hello message sent\n");
    valread = read( sock, buffer, 1024);
    printf("%s\n",buffer );
    return 0;

}
