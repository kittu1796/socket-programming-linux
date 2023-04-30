/* 
* File: sender.c
* Author: Ashwini Kumar
* Date: 29.04.23
*
* Description: An implementation of Linux MessageQueue receiver. This implementation is 
*               done reffereing to a course on udemy titled "IPC programming in Linux"
*/

//Includes
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<mqueue.h>
#include<errno.h>

//Macro definitions
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE+10)
#define MAX_MESSAGES 10
#define QUEUE_PERMISSIONS   0660

//main
int main(int argc, char **argv)
{
    //Vairables
    struct mq_attr attr;
    mqd_t msgQ_fd;
    fd_set os_fd_set;
    char read_buffer[MSG_BUFFER_SIZE];
    int ret_rx;

    //Check if the msg queue name was provided as cmdline argument
    if(argc <= 1)
    {
        printf("Provide the receiving msgQ name: format </msgQ-name>\n");
        exit(EXIT_SUCCESS);
    }

    /* Set the attributes */
    attr.mq_flags = 0;
    attr.mq_curmsgs = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;

    // Open the message queue
    msgQ_fd = mq_open(argv[1],
                     O_RDONLY | O_CREAT,
                     QUEUE_PERMISSIONS,
                     &attr);
    if(msgQ_fd == -1)
    {
        perror("MQ open failed:");
        exit(EXIT_FAILURE);
    }
    printf("Successfully opend the message quesue : %s\n", argv[1]);

    while(1){
        //Flush all the elements of the fd_set DS
        FD_ZERO(&os_fd_set);

        //Add this msgQ fd to the set
        FD_SET(msgQ_fd, &os_fd_set);

        printf(" Receiver process blocked by selec syscall\n");

        select(msgQ_fd+1, &os_fd_set, NULL, NULL, NULL);

        if(FD_ISSET(msgQ_fd, &os_fd_set))
        {
            printf("Message received from sender on queue %s.\n", argv[1]);

            memset(read_buffer, 0, MSG_BUFFER_SIZE);

            ret_rx = mq_receive(msgQ_fd, 
                                read_buffer,
                                MSG_BUFFER_SIZE,
                                NULL);
            if(ret_rx == -1)
            {
                perror("Receiver error: ");
                exit(EXIT_FAILURE);
            }

            printf("Msg received from sender = %s\n", read_buffer);
        }
    }  

}
