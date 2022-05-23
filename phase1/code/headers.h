#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define SHKEYRT 301 //remaing time key

///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================


// enum State{ready,blocked,paused,finished};

// //process control block 
// struct PCB {
//   enum  State state;  
//    int id; 
//    int process_id;
//    int arrival_time;                     //time to arrive to reeady queue
//    int run_time;                         // the burst time of the process
//    int priority;                         //priority assume low value is high priority
//    int starting_time;                   // it is the time to start use the cpu (schaduled)
//    int finsihing_time;                  // end its running time
//    int remaining_time;                   // time remained to finish 
//    int last_run;                  
//    int waiting_time;                      //Total waiting time since it start use cpu
//    int stopping_time;                 //The time in which the process paused.
   
//    struct PCB *next;
//    struct PCB *back;   
// };



//  struct Queue
// { 
//        unsigned capacity; //max capacity
//         int size; //current size
//         int front; //front index in array
//         int rear; //rear index in array
       
//       struct PCB** array_of_processes;  //the queue

// };



/* arg for semctl system calls. */
union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};

void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("Error in down()");
       // exit(-1);
    }
}

void up(int sem)
{
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        //exit(-1);
    }
}

int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
