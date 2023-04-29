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
#define MAX_CLIENTS 20 // Set the maximum number of clients the OS should handle, i.e 19 since one is master_fd.
#define BUFFER_SIZE 128 //SIze of buffer for the stream data
#define FD_UNINIT_VALUE -1 //since 

/* function prototype */
static void init_monitored_fd_set();
static void update_monitored_fd_set(int new_fd);
static void remove_from_monitored_fd_set(int fd_to_remove);
static void refresh_fd_set(fd_set *fd_set_ptr);
static int get_max_fd();



/*Maintain and observe the all the fd linked to this pocess*/
int monitored_fd_set[MAX_CLIENTS];

/*Maintain the result of eeach client seperately*/
int client_result[MAX_CLIENTS] = {0};

/* Main */

int main (int argc, char *argv[])
{
    struct sockaddr_un name;

    /* Variables */
    int master_soc_fd;
    int ret; //generic valiable to hold return values
    int client_socket;
    char buffer[BUFFER_SIZE];
    int data;
    fd_set os_fd_set;
    int check_fd_itr;
    int curr_socket_fd;

    // init the monitored fd set
    init_monitored_fd_set();
    update_monitored_fd_set(0);

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

   /*Add the master socket FD to the monitored fd data structure*/
   update_monitored_fd_set(master_soc_fd);

   while(1)
   {

    /*Referh the OS's fd_set wrt to monitored fd set*/
    refresh_fd_set(&os_fd_set);
    printf("Refreshed the fd set, next will be blocked by select system call\n");

    /**
     * select system call is to be made. This is also a blocking system call.
     * This server process will be unbloacked once, a connect init call is made to the server process.
    */

    select(get_max_fd()+1, &os_fd_set, NULL , NULL, NULL);

    /* Wait for incomming connection request, block*/
    printf("Waiting for connect system call, \nthis will block the server until a connect szstem call is made or data exchange is to be made with an existing process\n");

    // FD_ISSET, the first parameter is used to check if the fd is present in the Os's fd_set.
    if(FD_ISSET(master_soc_fd, &os_fd_set))
    {
        printf("A new connect request is made to master socket\n");
        client_socket = accept(master_soc_fd, NULL, NULL);

        if(client_socket == -1 )
        {
            perror("Accept: Error");
            exit(EXIT_FAILURE);
        }
        printf("Accept: Connection accepted by client\n");

        /*Udpate the monitored fd_set */
        update_monitored_fd_set(client_socket);



    }
    else
    {
        /*Data request is sent to the os*/
        
        //Figure out which client has sent data request to the socket

        for(check_fd_itr = 0; check_fd_itr<MAX_CLIENTS; check_fd_itr++)
        {
            if(FD_ISSET(monitored_fd_set[check_fd_itr], &os_fd_set))
            {
                curr_socket_fd = monitored_fd_set[check_fd_itr];
                memset(buffer, 0, BUFFER_SIZE);
                printf("Wait for another value from the client: %d\n", curr_socket_fd);

                ret = read(curr_socket_fd, buffer, BUFFER_SIZE);
                if(ret == -1)
                {
                    perror("Read: Error");
                    exit(EXIT_FAILURE);
                }

                memcpy(&data, buffer, sizeof(int));

                if(data == 0)
                {
                    memset(buffer, 0, BUFFER_SIZE);

                    /* Send the Result back to client*/
                    sprintf(buffer, "Result = %d", client_result[check_fd_itr]);

                    printf("Sending total result to the client=%d\n", curr_socket_fd);
                    ret = write(curr_socket_fd, buffer, BUFFER_SIZE);

                    if(ret == -1)
                    {
                        perror("Write: error");
                        exit(EXIT_FAILURE);
                    }

                    /* close the client socket*/
                    close(curr_socket_fd);
                    client_result[check_fd_itr] = 0;
                    remove_from_monitored_fd_set(curr_socket_fd);
                    continue;
                }
                client_result[check_fd_itr] += data; 
            
            }
        }
    }    

   }

   /*close the master socket*/
   close(master_soc_fd);
   remove_from_monitored_fd_set(master_soc_fd);
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

/**
 * Function name: init_monitored_fd_set
 * return: None
 * input: None
 * 
 * Description: Initialize the monitored fd set to -1, for all fd values. for this the FD_UNINIT_VALUE is used(-ve value)
*/
static void init_monitored_fd_set()
{
    int iterator_var=0;

    for(iterator_var=0; iterator_var<MAX_CLIENTS; iterator_var++)
    {
        monitored_fd_set[iterator_var] = FD_UNINIT_VALUE;
    }
}

/**
 * Function name: update_monitored_fd_set
 * return: None
 * input: new_fd_value
 * 
 * Description: ADd the new fd value to the first uninit fd value found in the set.
*/
static void update_monitored_fd_set(int new_fd)
{
    int iterator_var=0;

    for(iterator_var=0; iterator_var<MAX_CLIENTS; iterator_var++)
    {   
        if(monitored_fd_set[iterator_var] == -1 )
        {
            monitored_fd_set[iterator_var] = new_fd;
            break; //only want to update a single element in the set, and don't want to have duplicates.
        }
            
    }
}

/**
 * Function name: remove_from_monitored_fd_set
 * return: None
 * input: fd value to be removed from SET
 * 
 * Description: Assuming a process has sent a close szstem command, the fd value related to that process needs to be removed from the observable set list. 
 *              This is done, by replacing the value of the fd by the uninit fd value macro.
*/
static void remove_from_monitored_fd_set(int fd_to_remove)
{
    int iterator_var =0;
    for(iterator_var=0; iterator_var< MAX_CLIENTS; iterator_var++)
    {
        if(monitored_fd_set[iterator_var]== fd_to_remove)
        {
            monitored_fd_set[iterator_var] = FD_UNINIT_VALUE;
            break; //considering only one instance of a single FD is present
        }
    }
}

/*
 * Function name: refresh_fd_set
 * return: None
 * input: the fd_SET ptr which needs to be refreshed
 * 
 * Description: Remove all the previous fd's  in the fd_set and copies the content of monitored_fd_set into the fd_set.
*/
static void refresh_fd_set(fd_set *fd_set_ptr)
{
    FD_ZERO(fd_set_ptr);
    int iterterator_var = 0;

    for(iterterator_var=0; iterterator_var<MAX_CLIENTS; iterterator_var++)
    {
        if(monitored_fd_set[iterterator_var] != FD_UNINIT_VALUE)
        {
            FD_SET(monitored_fd_set[iterterator_var], fd_set_ptr);
        }
    }

}


/**
 * Function name: get_max_fd
 * return: int
 * input: None
 * 
 * Description: Return the fd, with the maximum vlaue present inside the monitored_fd_set.
*/
static int get_max_fd()
{   
    int iterator_var = 0;
    int max_fd_found = -1; //since fd will only have +ve values

    for(iterator_var=0; iterator_var<MAX_CLIENTS; iterator_var++)
    {
        if(monitored_fd_set[iterator_var] > max_fd_found)
            max_fd_found = monitored_fd_set[iterator_var];
    }

    return max_fd_found;
}

