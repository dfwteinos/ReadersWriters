#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>

#include "Semaphores.h"

int CreateSem(key_t key){

    union senum sem_union;

    int SemaphoreID;
    //sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);

    SemaphoreID=semget(key, 1, 0666 | IPC_CREAT );          //Creating a Semaphore

    if (SemaphoreID<0){
        printf("Could not create the Semaphore");
        return -1;
    }

    sem_union.val=1;                                        //Initializing the First Semaphore of our Group of Semaphore
    if( (semctl(SemaphoreID,0,SETVAL,sem_union))==-1 ){     //Initializing the Semaphore

        printf("Could not Initialize the Semaphore with id:%d\n",SemaphoreID);
        return -1;
    }    

    return SemaphoreID;       //ALL OK
}       

int SemaphoreDown(int SemID){  //Up
    
    struct sembuf Semaphore;
    Semaphore.sem_num=0;

    //!! Here we --; the Semaphore
    Semaphore.sem_op=-1;
    //!! Here we --; the Semaphore

    Semaphore.sem_flg=SEM_UNDO;     //If there is a problem, SEM_UNDO=kanto 0;

    if( (semop(SemID,&Semaphore,1))==-1 ){

        printf("Could not 'DOWN' the Semaphore\n");
        return -1;
    }
    
    return 1;
}       

int SemaphoreUp(int SemID){   //Down

    struct sembuf Semaphore;
    Semaphore.sem_num=0;

    //!!Here we ++; the Semaphore
    Semaphore.sem_op=1;
    //!!Here we +=; the Semaphore

    Semaphore.sem_flg=SEM_UNDO;

    if( (semop(SemID,&Semaphore,1) ) ==-1){

        printf("Could not 'UP' the Semaphore\n");
        return -1;
    }

    return 1;
}

int DeleteSem(int SemID){       //Delete the Semaphore

    union senum sem_union;
    if (SemID<0) return -1;

    if (  (semctl(SemID,0,IPC_RMID,sem_union))==-1 ){

        printf("Failed to Delete the Semaphore\n");
        return -1;
    }

    return 1;       //ALL OK
}

void RWFormat(int* R,int* W,float Ratio,int Iterations){

    *R=(int) (Ratio*Iterations);
    *W=(int)( Iterations-(*R) );
}