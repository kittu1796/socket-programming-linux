/* 
* File: mmap_as_malloc_Demo1.c
* Author: Ashwini Kumar
* Date: 30.04.23
*
* Description: An implementation of Linux mmap syscall for dynamic memory allocation. This implementation is 
*               done reffereing to a course on udemy titled "IPC programming in Linux"
*/

//Inlcudes
#include<stdio.h>
#include<sys/mman.h> 
#include<stdlib.h>

//Maros
#define NUM_BYTES 5

//function prototype
static void write_something(int *, int);
static void read_memory(int *, int);

//Global variables

//Main
int main(int argc, char **argv)
{
    int *mem_ptr = mmap(NULL,
                        NUM_BYTES*sizeof(int),
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        0,
                        0);

    if(mem_ptr == MAP_FAILED)
    {
        perror("mmap failed:");
        exit(EXIT_FAILURE);
    }
    printf("mmap successful\n");


    //Perform action on memory
    write_something(mem_ptr, NUM_BYTES);
    read_memory(mem_ptr, NUM_BYTES);

    //Unmap the memory
    int err = munmap(mem_ptr, NUM_BYTES*sizeof(int));
    if(err == -1)
    {
        perror("unmap error:");
        exit(EXIT_FAILURE);
    }
    printf("unmap successful\n");
    return 0;


}

static void write_something(int *ptr, int count)
{
    printf("Starting to perform action on the memory\n");

    for(int i=0; i<count;i++)
    {
        ptr[i] = i*10;
    }
    printf("Wrtieing Complete\n");
}

static void read_memory(int *ptr, int count)
{
    printf("Reading from memory\n");
    for(int i=0; i<count; i++)
    {
        printf("[%d]", ptr[i]);
    }
    printf("\nReading complete\n");
}