/* 
* File: server.c
* Author: Ashwini Kumar
* Date: 28.04.23
*
* Description: First try at writing server file, refering the udemy course "Socket Programming in linux" for the AF_UNIX
*              For a value sent to the server, the server will keep calculating the summation of the values until the value 0 is sent to the server. 
*               On recieveing 0 to the server, the server sends the total sum computed untill now.
*/

// INCLUDES

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>

/* Macro definitions */
#define SOCKET_NAME_CONST "/tmp/masterSocket" // should be unique
#define MAX_CLIENTS 20 // Set the maximum number of clients the OS should handle.
#define BUFFER_SIZE 128 //SIze of buffer for the stream data

/* Main */

int main (int argc, char *argv[])
{
    struct sockaddr_un name;

    /* Variables */
    int master_soc_fd;
    int ret; //generic valiable to hold return values
    int client_result;
    int client_socket;
    char buffer[BUFFER_SIZE];
    int data;

    /*Destroy old instance of the socket with same name, a precaution line*/
    unlink(SOCKET_NAME_CONST);

    /* Create Master Socket File Descriptor */

    master_soc_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    //error check: note the fd will be -ve in case of an error
    if(master_soc_fd == -1)
    {
        perror("Master socket FD: unable to create");
        exit(EXIT_FAILURE);
    }

    printf("Master socket: created successfully\n");

    /*           Now do the binding of the socket 
    *  
    *     This bind, function call binds(attaches) the SOCKET_NAME to this fd. This means when OS rx an data targeted for SOCKET_NAME, the 
    *     data is redirected by the OS to this server process.
    * */
    printf("Binding: Started\n");

    /*prep for binding*/
    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME_CONST, sizeof(name.sun_path) -1);

    /* Bind now */
    ret = bind(master_soc_fd, 
                (const struct sockaddr_un *)&name,
                sizeof(struct sockaddr_un));

    //error check
    if(ret == -1)
    {
        perror("Bind: Error in binding");
        exit(EXIT_FAILURE);
    }
    printf("Bind: Binding successful\n");

    /*  Listen system call 
    *   Start listning, also set the maximum number of clients teh OS should maintain.
    *   If more than MAX_CLIENTS requiest are rx, the new request will drop.
    **/
   printf("Listen: Started\n");
   ret = listen(master_soc_fd, MAX_CLIENTS);

   //error handel
   if(ret == -1)
   {
        perror("Listen: Error in listen");
        exit(EXIT_FAILURE);
   }
   printf("listen: listen successful\n");

   while(1)
   {
    /* Wait for incomming connection request, block*/
    printf("Waiting for accept system call, \nthis will block the server until a connect szstem call is made\n");

    client_socket = accept(master_soc_fd, NULL, NULL);

    if(client_socket == -1 )
    {
        perror("Accept: Error");
        exit(EXIT_FAILURE);
    }
    printf("Accept: Connection accepted by client\n");

    client_result = 0;

    // Cater to this client process/socket

    while(1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        printf("Wait for another value from the client\n");

        ret = read(client_socket, buffer, BUFFER_SIZE);
        if(ret == -1)
        {
            perror("Read: Error");
            exit(EXIT_FAILURE);
        }

        memcpy(&data, buffer, sizeof(int));
        if(data ==0)break;
        client_result += data;
    }

    /* Send the Result back to client*/

    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "Result = %d", client_result);

    printf("Sending total result to the client\n");
    ret = write(client_socket, buffer, BUFFER_SIZE);

    if(ret == -1)
    {
        perror("Write: error");
        exit(EXIT_FAILURE);
    }

    /* close the client socket*/
    close(client_socket);

   }

   /*close the master socket*/
   close(master_soc_fd);
   printf("Connection Closed ..\n");

   unlink(SOCKET_NAME_CONST);
   exit(EXIT_SUCCESS);


    
}

/* Definitions: struct sockaddr_un
* 
* struct sockaddr_un{
            sa_family_t sun_family; // AF_UNIX, can use other values here too.
            char sun_path[108]; //pathname, this holds the name of the socket
}
*/
