#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include "SharedMemory.h"
#include <unistd.h>

int CreateShm(key_t key,int size){

    return shmget(key,size, IPC_CREAT | 0666 );
    // >0 OK
    // <0 PROBLEM
}

int DeleteShm(int Shmid){

    return shmctl(Shmid, IPC_RMID,0);
    // 0 OK
    // -1 PROBLEM

}

Entry * AttachShm(int Shmid){

    return shmat(Shmid, (void *)0,0);

    //-1 PROBLEM
}

int DetachShm(Entry* ShmPtr ){

    return shmdt(ShmPtr);
    //0 OK
    // -1 PROBLEM
}

