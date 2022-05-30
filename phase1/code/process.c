#include "headers.h"
#include<signal.h>
/* Modify this file as needed*/

int remainingtime;
int current_time;
int processSem;
union Semun processSemun;

int main(int agrc, char * argv[])
{
    
        
        int shared_memory_id; // shared memory for remaing time
        int* shared_memory_address;
        processSem = semget(69, 1, 0666 | IPC_CREAT);
        if(processSem == -1)
        {
                perror("Error in creating semaphore \n");
                exit(-1);
        }

        shared_memory_id = shmget(SHKEYRT, 4, 0644);
        if (shared_memory_id == -1) {
                perror("can't create a shared memory for remaing time in memory \n");
                exit(-1);
        }

        shared_memory_address= (int *) shmat(shared_memory_id, (void *)0, 0);
        if ((long)shared_memory_address == -1) 
        {
                perror("error in attach in process and schadular !\n");
                exit(-1);
        }


        initClk();
   
        remainingtime = *shared_memory_address;
        current_time = getClk();
    
        //TODO it needs to get the remaining time from somewhere
        //remainingtime = ??;
        while (remainingtime > 0)
        {
        
                if(current_time != getClk())
                {
                        // if(getClk() - current_time > 1)
                        // {
                        //         current_time = getClk();
                        //         continue;
                        // }
        
                current_time = getClk();
                remainingtime--;
                printf("Process: At time %d, remaining time %d\n", current_time, remainingtime);
                *shared_memory_address = remainingtime;
                up(processSem); 
        }             
}  

        //notify the scheduler that this process is finished
        if(remainingtime==0)
        kill(getppid() , SIGUSR1);
    
        if (shmdt(shared_memory_address) == -1)
        {
              printf("can't detach the shared memory in process \n");  
        }


    destroyClk(false);
    exit(0);
}
