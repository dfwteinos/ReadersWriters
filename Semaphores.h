#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/*
struct sembuf {
    short sem_num;
    short sem_op;
    short sem_flg;
};
*/


union senum {
    int val;
    struct semid_ds *buff;
    unsigned short *array;
};

enum gender{Reader,Writer};

/*Semaphores Functions*/

int CreateSem(key_t);                   //Create Semaphore

/*

//int ChangeSem(int);                   //Change the value of a Semaphore

//int ControlSem(int);                  //Control the values of a Semaphore

*/

int SemaphoreUp(int);                   //Up

int SemaphoreDown(int);                 //Down

int DeleteSem(int);                     //Delete the Semaphore

void RWFormat(int*,int*,float,int);      //How many Readers and how many Counters depending on Ratio


