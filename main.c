#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/wait.h>
#include <math.h>

#include "SharedMemory.h"
#include "Semaphores.h"
#define ITER 10
#define KEYNUM 1234
#define MULT 1500

int main(int argc, char *argv[]) {

//Entries,Peers and Ratio of Readers and Writers

    int Entries,Peers;
    float RatioRW;

    if(argc<4){    

        printf("We have Input Error,not enough arguments\n");
        exit(EXIT_FAILURE);
    }

    else if (argc>4){

        printf("We have Input Error,too many arguments\n");
        exit(EXIT_FAILURE);
    }

    else{

        Entries=atoi(argv[1]);        
        Peers=atoi(argv[2]);
        RatioRW=atof(argv[3]);
    }
    if( RatioRW<0.0 || RatioRW>1.0 ){
        printf("We have Input Error,R-W Ratio must be in space={0.0<RatioRW<1.0}\n");
        exit(EXIT_FAILURE);

    }

    FILE *fp=fopen("Results.txt","w+");

    //Our Variables//

    key_t key;
    int ShmID;                                  //SharedMemory ID
    Entry* ShmPTR;                              //SharedMemory Pointer
    int Sem_Reader,Sem_Writer;                  //Semaphores ID
    pid_t pid;                                  //Process ID
    int Gender;                                 //If the child is either a Reader or either a Writer
    int iter;                                   //For Iterations
    int Readers,Writers;                        //For Iterations
    int ReadersCounter,WritersCounter;          //For each Iteration
    

    //Number of Readers and Writers in each iteration
    RWFormat(&Readers,&Writers,RatioRW,ITER);   //Readers=#R , Writers=#W  
    //Number of Readers and Writers in each iteration
    

    int Probability;                            //Probability   
    int SharedReaders,SharedWriters;            //For semaphore sychronization
    int status=0;                               //For the status of each child process
    int EntryNum=0;                             //The number of entry we gonna change
    int i,j;                                    //Counter for our loops
    double T;                                   //The Taf value in our time
    CPTR CPPointer;                             //Child Process Pointer
    double TimeCounter;                         //For our M.O
    int PID=-1;                                 //For each child process
    clock_t start,end;                          //For clock measurements

    //Creating a key for the Shared Memory

    if( ( key=ftok("main.c",'R') )==-1 ){
        perror("IPC Error: ftok");
        exit(EXIT_FAILURE);
    }

    if( (ShmID=CreateShm(key, Entries*sizeof(Entry)))<0  ){
        perror("SharedMem Error: Could not Allocate Space\n");
        exit(EXIT_FAILURE);
    }
        
    //Attaching Shared Memory 
    
    ShmPTR=AttachShm(ShmID);

    if( (ShmPTR==(Entry*) (-1 ) ) ){
        perror("SharedMem Error: Could not Attach Memory\n");
        exit(EXIT_FAILURE);
    }


    for (int e=0; e<Entries; e++){

        //Writer Semaphore

        if ( ( (ShmPTR[e].WrtID)=CreateSem( (key_t)KEYNUM+e+1 ))<0  ){
            
            perror("Samephore Error: Problem in Creating a Semaphore\n");
            exit(EXIT_FAILURE);
        }

        //Mutex Semaphore

        if ( ( (ShmPTR[e].MutexID)=CreateSem( (key_t)KEYNUM-e ))<0  ){
            
            perror("Samephore Error: Problem in Creating a Semaphore\n");
            exit(EXIT_FAILURE);
        }
    
        ShmPTR[e].SharedReaders=0;          //Initializing Shared Reader as 0! Very important part
        ShmPTR[e].Reader_C=0;
        ShmPTR[e].Writer_C=0;

    }

    //Creating the n Peers of Coordinator

    for(int i=0; i<Peers; i++){

        pid=fork();
        if (pid==0){            //If it's a child Process don't fork()
            break;
        }
    
        else if (pid==-1){
            perror(" fork() failed ");
            exit(EXIT_FAILURE);
        }
    }

    if (pid==0){                                //If it's a child Process

        PID++;
        ReadersCounter=Readers;
        WritersCounter=Writers;
        Probability=0;
        TimeCounter=0;

        for(iter=0; iter<ITER; iter++){         //For a spesific number of Iterations
            
            srand( (unsigned int) getpid()+iter);
            Gender= (rand()%11);
            EntryNum=rand()%Entries;            //Number of Entry we goind to visit in our shared memory
            
            if (    Gender>( (int)(RatioRW*ITER)-Probability ) ) {

                Gender=Writer;
            }

            else {

                Gender=Reader;
            }
            
            ///////////////////////////////////////////////////////////////////

            if( (Gender==Reader && ReadersCounter>0) || WritersCounter==0 ) {                  //Reading Process

                ReadersCounter--;
                printf("We have a reader!\n");

                start=clock();
                
                if ( ( SemaphoreDown(ShmPTR[EntryNum].MutexID ) )==-1  ){                        //Mutex.Down--;
                    perror("Could not 'DOWN' the Semaphore\n");
                    exit(EXIT_FAILURE);
                }

                ShmPTR[EntryNum].SharedReaders=ShmPTR[EntryNum].SharedReaders+1;
                if ( ShmPTR[EntryNum].SharedReaders==1 ){

                    if ( ( SemaphoreDown(ShmPTR[EntryNum].WrtID ) )==-1  ){                      //Wrt.Down--;
                        perror("Could not 'DOWN' the Semaphore\n");
                        exit(EXIT_FAILURE);
                    }
                }

                if ( ( SemaphoreUp(ShmPTR[EntryNum].MutexID ) )==-1  ){                          //Mutex.Up--;
                    perror("Could not 'Up' the Semaphore\n");
                    exit(EXIT_FAILURE);
                }

                end=clock();

                //// CRITICAL SECTION ////

                ShmPTR[EntryNum].Reader_C++;
                printf("Inside at CS Reader,with Process id:%d,and Entry:%d\n",getpid(),EntryNum);
                T=MULT*( -log( (double)rand()/(double)RAND_MAX )/clock() );
                sleep(T);

                //// END OF CRITICAL SECTION ////

                end=clock();

                if ( ( SemaphoreDown(ShmPTR[EntryNum].MutexID ) )==-1  ){                        //Mutex.Down--;
                    perror("Could not 'DOWN' the Semaphore\n");
                    exit(EXIT_FAILURE);
                }

                ShmPTR[EntryNum].SharedReaders=ShmPTR[EntryNum].SharedReaders-1;
                if ( ShmPTR[EntryNum].SharedReaders==0 ){

                    if ( ( SemaphoreUp(ShmPTR[EntryNum].WrtID ) )==-1  ){                       //Wrt.Up--;
                        perror("Could not 'Up' the Semaphore\n");
                        exit(EXIT_FAILURE);
                    }
               
                }

                if ( ( SemaphoreUp(ShmPTR[EntryNum].MutexID ) )==-1  ){                          //Mutex.Up--;
                    perror("Could not 'Up' the Semaphore\n");
                    exit(EXIT_FAILURE);
                }
            }   

            else if ( (Gender==Writer && WritersCounter>0) || ReadersCounter==0 ) {             //Writer Process

                WritersCounter--;
                printf("We have a writer!\n");

                start=clock();
            
                if ( ( SemaphoreDown(ShmPTR[EntryNum].WrtID ) )==-1  ) {                        //Wrt.Down--;
                    perror("Could not 'DOWN' the Semaphore\n");
                    exit(EXIT_FAILURE);
               
                }

                end=clock();

                //////////////////////////////////CRITICAL SECTION
                ShmPTR[EntryNum].Writer_C++;
                printf("Inside at CS Writer,with Process id:%d, and Entry:%d\n",getpid(),EntryNum);
                T=MULT*( -log( (double)rand()/(double)RAND_MAX )/clock() );
                sleep(T);
                //////////////////////////////////CRITICAL SECTION

                if ( ( SemaphoreUp(ShmPTR[EntryNum].WrtID ) )==-1  ){                       //Wrt.Up--;
                    perror("Could not 'Up' the Semaphore\n");
                    exit(EXIT_FAILURE);
                }
          
            }
       
            TimeCounter+= ( (double) (end - start)) / CLOCKS_PER_SEC;
            Probability=WritersCounter-ReadersCounter;
        }

        fprintf(fp,"Process id: %d:\n",getpid());
        fprintf(fp,"Average Time: %lf\n",TimeCounter/Entries);
        fprintf(fp,"Readers: %d\n",Readers);
        fprintf(fp,"Writers: %d\n\n",Writers);
        exit(0);

    
    }

   for(j=0; j<Peers; j++){

        pid=wait(&status);
        printf("Child process with id:%d, exited normally\n",pid);
        printf("Child status is%d\n\n",status);

    }

    for (int k=0; k<Entries; k++){

        if ( (DeleteSem(ShmPTR[k].WrtID ) ) <0  ){
            
            perror("Samephore Error: Problem in Deleting a Semaphore\n");
            printf("At thesis:%d\n",k);
            printf("Writer Semaphore\n");
            exit(EXIT_FAILURE);
        }
    
        if ( (DeleteSem(ShmPTR[k].MutexID ) ) <0  ){
            
            perror("Samephore Error: Problem in Deleting a Semaphore\n");
            printf("At thesis:%d\n",k);
            printf("Mutex Semaphore\n");
            exit(EXIT_FAILURE);
        }

    }

    for(j=0; j<Entries; j++){

        fprintf(fp,"For entry number: %d\n",j);
        fprintf(fp,"Total Reads: %d\n",ShmPTR[j].Reader_C);
        fprintf(fp,"Total Writes:%d\n\n",ShmPTR[j].Writer_C);
        
    }

    //SEGM FAULT:printf("Total Reads:%d\n",ShmPTR[2].Reader_C);

    if ( ( DetachShm(ShmPTR))==-1 ){

        perror("Dettaching the shared memory failed!\n");
        exit(EXIT_FAILURE);
    }

    if ( DeleteShm(ShmID)==-1 ){

        printf("Deleting the Shared Memory Failed!\n");
        exit(EXIT_FAILURE);
    }


    printf("See Results.txt file for the output!\n");
    fclose(fp);
    return 0;
}
