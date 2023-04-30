/* 
* File: sender.c
* Author: Ashwini Kumar
* Date: 29.04.23
*
* Description: An implementation of Linux MessageQueue sender. This implementation is 
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

//Global variables

//main
int main(int argc, char **argv)
{
    char sending_buffer[MSG_BUFFER_SIZE]; //hold the msg in this buffer
    mqd_t msgQ_fd;
    int ret_mq;


    //This process expects the msgQ id as argument to this process. 
    if(argc <= 1)
    {
        printf("Provide the receiving msgQ name: format </msgQ-name>\n");
        exit(EXIT_SUCCESS);
    }

    //clear the msg buffer
    memset(sending_buffer, 0, MSG_BUFFER_SIZE);

    //get the msg to send on the queue
    printf("Enter the msg to send on queue %s: ", argv[1]);
    scanf("%s", sending_buffer);

    //Open the msg queue, as write only mode, also set the create flag.
    msgQ_fd = mq_open(argv[1], 
                        O_WRONLY | O_CREAT, 
                        0, 
                        0);
    if(msgQ_fd == -1)
    {
        perror("MQ Open: ");
        exit(EXIT_FAILURE);
    }
    printf("Message-queue %s was opened successfully\n", argv[1]);
    
    // Send the message to the queue
    ret_mq =  mq_send(msgQ_fd, 
                            &sending_buffer,
                            MSG_BUFFER_SIZE,
                            0 );

    if(ret_mq == -1)
    {
        perror("Sending failed:");
        exit(EXIT_FAILURE);
    }
    printf("Sending message successfull, sent data: %d\n", ret_mq);

    ret_mq = mq_close(msgQ_fd);
    if(ret_mq == -1)
    {
        perror("Closing error:");
        exit(EXIT_FAILURE);
    }

    printf("Successfully closed the queue, or atleast invoked\n");
    return 0;


}