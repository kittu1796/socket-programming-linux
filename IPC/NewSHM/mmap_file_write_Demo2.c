/* 
* File: mmap_as_malloc_Demo2.c
* Author: Ashwini Kumar
* Date: 30.04.23
*
* Description: An implementation of Linux mmap syscall for sharing memroy with external storage device This implementation is 
*               done reffereing to a course on udemy titled "IPC programming in Linux". 
*               Import txt file from external storage memory and update the contents with singel process.
*/

//Inlcudes
#include<stdio.h>
#include<sys/mman.h> 
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<memory.h>
#include<unistd.h>
#include<sys/stat.h>

//Maros
#define NUM_BYTES 5

//function prototype
static void write_something(int *, int);
static void read_memory(int *, int);

//structures

typedef struct student_
{
    int roll_num;
    int marks;
    char name[128];
    char city[128];
}student_t;

//Global variables

//Main
int main(int argc, char *argv[])
{

    //Variables
    const char *filepath = argv[1];

    /* Check if file path to read is provided as input to the executable*/
    if(argc < 2)
    {
        printf("Enter the file path to read the file\n");
        exit(EXIT_FAILURE);
    }
    printf("Received file path as input argument\n");


    /*Open the file in Read-Write mode*/
    int file_fd =  open(filepath, O_RDWR | O_CREAT);

    if( file_fd < 0)
    {
        perror("Open File: ");
        exit(EXIT_FAILURE);
    }
    printf("SUccessfully opened the file\n");

    /*Get the size of file*/
    struct stat statbuf;
    int err = fstat(file_fd, &statbuf);
    if (err < 0 )
    {
        perror("File size:");
        exit(EXIT_FAILURE);
    }
    printf("SUccessfully got the file size\n");


    int page_size = getpagesize();
    int map_size = ((statbuf.st_size / page_size) + 1) * page_size;
    char *mem_ptr = mmap(NULL,
                        map_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        file_fd,
                        0);

    if(mem_ptr == MAP_FAILED)
    {
        perror("mmap failed:");
        exit(EXIT_FAILURE);
    }
    printf("mmap successful\n");

    close(file_fd); // NO need of this fd, since the process can access direct memory location(Virtual Aaddress).



    //Perform action on memory
    student_t stud = {123, 90, "Ashwini", "Stuttgart"};
    memcpy(mem_ptr, &stud, sizeof(stud));

    err=msync(mem_ptr, sizeof(stud), MS_SYNC);
    if(err != 0)
    {
        perror("msync error:");
        exit(EXIT_FAILURE);
    }
    printf("msync successful\n");

    //Unmap the memory
    err = munmap(mem_ptr, statbuf.st_size);
    if(err != 0)
    {
        perror("unmap error:");
        exit(EXIT_FAILURE);
    }
    printf("unmap successful\n");
    return 0;


}

/**
 * Function name: write_something
 * return: None
 * input: memory pointer and number of int bytes to write.
 * 
 * Description: Write content to respective memory location.
*/

static void write_something(int *ptr, int count)
{
    printf("Starting to perform action on the memory\n");

    
    printf("Wrtieing Complete\n");
}

/**
 * Function name: read_memory
 * return: None
 * input: memory pointer and number of int bytes to read.
 * 
 * Description: read the memory and print content.
*/

static void read_memory(int *ptr, int count)
{
    printf("Reading from memory\n");
    for(int i=0; i<count; i++)
    {
        printf("[%d]", ptr[i]);
    }
    printf("\nReading complete\n");
}