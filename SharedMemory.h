#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

struct Entries {                        //Shared Memory will storage a struct as an Entry

    int Reader_C;
    int Writer_C;
    int WrtID;
    int MutexID;
    int SharedReaders;
    //double Time;
};

struct ChildProcess{

    int ProcessID;
    int SumReads;
    int SumWrites;
    double Time;
};

typedef struct ChildProcess CP;         //For our final statistics
typedef struct ChildProcess* CPTR;      //Pointer for the array of structs

typedef struct Entries Entry;

/*Shared Mem Functions*/

int CreateShm(key_t,int);               //Create the Shared Memory

Entry * AttachShm(int);                 //Attach the Shared Memory to an Address Space

int DetachShm(Entry*);                  //Detach the Shared Memory to an Address Space

int ControlShm(int);                    //Do some changes in the Shared Memory

int DeleteShm(int);                     //Delete the Shared Memory

