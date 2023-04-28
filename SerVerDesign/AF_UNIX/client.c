/* 
* File: client.c
* Author: Ashwini Kumar
* Date: 28.04.23
*
* Description: First try at writing client file.
*/

// INCLUDES

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>

/* Macro definitions */
#define SOCKET_NAME_CONST "/client/af-unix/socket" // should be unique
#define BUFFER_SIZE 128 //SIze of buffer for the stream data

int main(int argc, char *argv[])
{
    struct sockaddr_un name;
    int this_socket;
    int ret;
    int curr_number;
    char buffer[BUFFER_SIZE];

    memset(&name, 0, sizeof(struct sockaddr_un));

    
    /*Inlink socket of same name*/
    unlink(SOCKET_NAME_CONST);

    /* Create client socket */
    this_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(this_socket == -1)
    {
        perror("Socket: Unable to create socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket creation succesful\n");

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME_CONST, sizeof(name.sun_path) -1);

    ret = connect(this_socket, 
            (const struct sockaddr *) &name,
            sizeof(struct sockaddr_un));
    if (ret == -1){
        fprintf(stderr, "The server is down\n");
        exit(EXIT_FAILURE);
    }

    do{
        printf("Enter Number to send to server: \n");
        scanf("%d", &curr_number);

        ret = write(this_socket, &curr_number, sizeof(int));
        if(ret ==-1){
            perror("Write: Error");
            exit(EXIT_FAILURE);
        }

        printf("No of bytes sent = %d, data sent = %d\n", ret, curr_number);

    }while(curr_number);

    memset(buffer, 0, BUFFER_SIZE);
    ret = read(this_socket, buffer, BUFFER_SIZE);
    if(ret == -1){
        perror("Read: Error");
        exit(EXIT_FAILURE);
    }

    printf("Received from Server: %s", buffer);

    close(this_socket);
    exit(EXIT_SUCCESS);
    
}