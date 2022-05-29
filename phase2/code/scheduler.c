#include "headers.h"
#include <math.h>
#include "process_generator.h"
#include "priority_queue.h"
#include "signal.h"
#include"buddy.h"
#define NUM 1000

int messageQueueofprocessesid;

struct msgbuff
{
    long mtype;
    struct process proc;
};

/********************************** All variables that we need ***********************************/
int TA = 0, total_time = 0, idle_time = 0, cur_time = 0;
float WTA = 0, STD_WTA = 0, WTA_avg = 0, waiting_avg = 0, CPU_utilization = 0;
int schedulerType;
int num_of_proc, processes_count;
int const_num_of_proc;
float *WTA_arr;
int quantum, cur_quantum = 0;
int shared_memory_id;       // shared memory for remaing time
int *shared_memory_address; // shared memory for remaing time
struct PCB *running_proc = NULL;
struct Queue *queue;
struct Queue *waiting_list;
FILE *outputLogFile;
FILE *outputPerfFile;
FILE *outputMemoryFile;
/************************************************************************************************/

/********************************** All functions declarations **********************************/
void createNewProcess(struct process proc);
void readMessage(int signum);
void startProcess();
void finishProcess(int signum);
void RoundRobin(int q);            // Round Robin
void HighestPriorityFirst();       // Non-preemptive highest priority first algorithm
void ShortestRemainingTimeFirst(); // Preemitive Shortest Remaining Time algorithm
void printMemoryOutputFile();
/************************************************************************************************/

int shmid;
int sem1; 
int sem2; 
int semTemp;
int processSem;
union Semun semun1;
union Semun semun2;
union Semun semunTemp;
union Semun processSemun;
struct process *shm;
/********************buddu instance***************************************/
struct buddy my_buddy;
int main(int argc, char *argv[])
{
    // TODO implement the scheduler :)

    // Create queue for arriving processes
    queue = CreateQueue(NUM);
    //create waiting list for memory allocation
    waiting_list=CreateQueue(NUM);

    //instance of buddy
    initialize_buddy(&my_buddy);
    
    perror("ahhhhhhh------->");

    // Read file arguments
    if (argc < 4)
    {
        perror("few arguments in scheduler \n");
        exit(-1);
    }

    schedulerType = atoi(argv[1]);
    num_of_proc = atoi(argv[2]);
    const_num_of_proc = num_of_proc;
    WTA_arr = (float *)malloc(sizeof(float) * num_of_proc);
    quantum = atoi(argv[3]);

    processes_count = num_of_proc; // To calculate average waiting and average WTA

    printf("%d \n", schedulerType);
    printf("%d \n", num_of_proc);
    printf("%d \n", quantum);

    /**************************** Open file to write through out the scheduler ****************************/
    outputLogFile = fopen("scheduler.log.txt", "w");
    if (outputLogFile == NULL)
    {
        perror("Error opening file");
        exit(-1);
    }
    fputs("# At time x process y state arrival w total z remaining y wait k \n", outputLogFile);
    /******************************************************************************************************/
        /**************************** Open file to write through out the scheduler ****************************/
    outputMemoryFile = fopen("memory.log", "w");
    if (outputLogFile == NULL)
    {
        perror("Error opening file");
        exit(-1);
    }
    fputs("# At time x allocated y bytes for process z from i to j \n", outputMemoryFile);
    /******************************************************************************************************/

    /********************************** shared memory for remaining time **********************************/
    shared_memory_id = shmget(SHKEYRT, 4, 0644 | IPC_CREAT);
    if (shared_memory_id == -1)
    {
        perror("can't create a shared memory for remaing time in schadular \n");
        exit(-1);
    }

    shared_memory_address = (int *)shmat(shared_memory_id, (void *)0, 0);
    if ((long)shared_memory_address == -1)
    {
        perror("error in attach in process and schadular !\n");
        exit(-1);
    }
    /******************************************************************************************************/

    /************************ Signals ************************/
    signal(SIGUSR1, finishProcess);
    signal(SIGUSR2, readMessage);
    /*********************************************************/

    /************************ Semaphores ************************/
    int sem = semget(20, 1, 0666);
    if (sem == -1)
    {
        perror("Error in create sem");
        exit(-1);
    }

    shmid = shmget(95, 1000 * sizeof(struct process), IPC_CREAT | 0664);
    sem1 = semget(96, 1, 0666 | IPC_CREAT);
    sem2 = semget(97, 1, 0666 | IPC_CREAT);
    semTemp = semget(98, 1, 0666 | IPC_CREAT);
    if (shmid == -1 || sem1 == -1 || sem2 == -1)
    {
        perror("Error in create \n");
        exit(-1);
    }

    semun1.val = 0; /* initial value of the semaphore, Binary semaphore */
    semun2.val = 0;
    if (semctl(sem1, 0, SETVAL, semun1) == -1 || semctl(sem2, 0, SETVAL, semun2) == -1)
    {
        perror("Error in semctl \n");
        exit(-1);
    }

    semunTemp.val = 1;
    if (semctl(semTemp, 0, SETVAL, semunTemp) == -1)
    {
        perror("Error in semctl \n");
        exit(-1);
    }

    processSem = semget(69, 1, 0666 | IPC_CREAT);
    if(processSem == -1)
    {
        perror("Error in creating semaphore \n");
        exit(-1);
    }

     processSemun.val = 0;
    if (semctl(processSem, 0, SETVAL, processSemun) == -1)
    {
        perror("Error in semctl \n");
        exit(-1);
    }

    shm = shmat(shmid, (void *)0, 0);
    if (shm == NULL)
    {
        perror("Error in client to attach to memory \n");
        exit(-1);
    }

    /*********************************************************/
    cur_time = -1, total_time = 0, idle_time = 0;
    initClk();
    // Loop until there are no more processes left
    running_proc = NULL;
    while (num_of_proc)
    {
        if (cur_time == getClk())
            continue;

        cur_time = getClk();
        down(sem2);
        if (running_proc != NULL)
            down(processSem);
        printf("current time in scheduler is %d \n", cur_time);
        total_time++;

         //if (running_proc != NULL)

        //     printf("_____________________________________________________ %d \n", cur_time);
        if (!(isEmpty(queue)) || running_proc)
        {
            switch (schedulerType)
            {
            case 1:
                HighestPriorityFirst();
                break;
            case 2:
                ShortestRemainingTimeFirst();
                break;
            case 3:
                RoundRobin(quantum);
                break;
            default:
                break;
            }
        }
        else if (!running_proc)
        {
            idle_time++;
            printf("current time is %d and idle time is %d\n", cur_time, idle_time);
        }
        //if (running_proc != NULL)
        //up(processSem);
    }

    // Calculate CPU utilization
    CPU_utilization = ((total_time - idle_time) / (float)total_time) * 100.0;

    //STD_WTA
    for (int i = 0; i < const_num_of_proc; i++)
        {
               STD_WTA  += pow((double)(WTA_arr[i] - WTA_avg), 2);
        }

        STD_WTA = sqrt(STD_WTA );

    /*************************** Print scheduler.perf file ******************************/
    outputPerfFile = fopen("scheduler.perf.txt", "w");
    if (outputPerfFile == NULL)
    {
        perror("Error opening file");
        exit(-1);
    }
    fprintf(outputPerfFile, "CPU utilization = %.2f%% \n", CPU_utilization);
    fprintf(outputPerfFile, "Avg WTA = %.2f \n", WTA_avg / processes_count);
    fprintf(outputPerfFile, "Avg Waiting = %.2f \n", waiting_avg / processes_count);
    fprintf(outputPerfFile, "Std WTA = %.2f \n", STD_WTA);
    /************************************************************************************/

    // Upon termination release the all resources.
    fclose(outputPerfFile);
    fclose(outputLogFile);
    fclose(outputMemoryFile);
    shmctl(shmid, IPC_RMID, NULL);
    shmctl(shared_memory_id, IPC_RMID, NULL);



    /*********** clear all IPC reasources ***********************/
     /*shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
     semctl(sem1, 0, IPC_RMID, semun1);
     semctl(sem2, 0, IPC_RMID, semun2);*/




    /*************************************************************/
    destroyClk(true);

    return 0;
}

// a function that intialize process and and it to ready queue
void createNewProcess(struct process proc)
{
    struct PCB *entry = (struct PCB *)malloc(sizeof(struct PCB));
    entry->id = proc.id;
    entry->arrival_time = proc.arrival;
    entry->run_time = proc.runtime;
    entry->priority = proc.priority;
    entry->state = ready;
    entry->remaining_time = proc.runtime;
    entry->waiting_time = 0;
    entry->memory_size = proc.memsize;
      
    struct pair Pair = allocate(&my_buddy,entry->memory_size);
  
    if(Pair.start==-1 && Pair.end==-1)
    {
        //Enqueue(waiting_list, entry);
        switch (schedulerType)
        {
            case 1: // HPF algorithm
                Min_Heap_Insert(waiting_list, entry);
                break;
            case 2: // SRTF algorithm
                entry->priority = entry->remaining_time;
                Min_Heap_Insert(waiting_list, entry);
                break;
            case 3: // RR algorithm
                Enqueue(waiting_list, entry);
                break;
            default:
                break;
        }

    }
    else
    {
        entry->from_index = Pair.start;
        entry->to_index = Pair.end;
        printf("At time %d allocated %d bytes for process %d from %d to %d \n", cur_time, entry->memory_size,entry->id ,Pair.start, Pair.end);
        fprintf(outputMemoryFile, "At time %d allocated %d bytes for process %d from %d to %d \n", cur_time, entry->memory_size,entry->id ,Pair.start, Pair.end);        
        switch (schedulerType)
        {
            case 1: // HPF algorithm
                Min_Heap_Insert(queue, entry);
                break;
            case 2: // SRTF algorithm
                entry->priority = entry->remaining_time;
                Min_Heap_Insert(queue, entry);
                break;
            case 3: // RR algorithm
                Enqueue(queue, entry);
                break;
            default:
                break;
        }
  }
}

void readMessage(int signum)
{
    down(sem1);
    printf("time after receive is %d id is %d\n", cur_time, shm->id);
    createNewProcess(*shm);
    up(semTemp);
}

void startProcess(struct PCB *pro)
{
    int pid;
    if ((pid = fork()) == 0)
    {
        int rt = execl("process.out", "process.out", NULL);
        if (rt == -1)
        {
            perror("\n\nscheduler can't run  process \n");
            exit(-1);
        }
    }
    else
    {
        pro->process_id = pid;
    }
}

void finishProcess(int signum)
{
    running_proc->finsihing_time = getClk();

    TA = (running_proc->finsihing_time) - (running_proc->arrival_time);
    WTA = ((float)TA) / running_proc->run_time;
    waiting_avg += running_proc->waiting_time;
    WTA_avg += WTA;
    WTA_arr[running_proc->id - 1] = WTA;

    running_proc->remaining_time = *shared_memory_address;

    //deallocate 
    deallocate(&my_buddy, running_proc->from_index, running_proc->to_index);
    printf("At time %d freed %d bytes for process %d from %d to %d \n", cur_time, running_proc->memory_size,running_proc->id ,running_proc->from_index, running_proc->to_index);
    fprintf(outputMemoryFile, "At time %d freed %d bytes for process %d from %d to %d \n", cur_time, running_proc->memory_size,running_proc->id ,running_proc->from_index, running_proc->to_index);

    // enque in ready queue if possible
    //loop on waiting list 
    struct Queue *temp_q;
    temp_q = CreateQueue(NUM);
    struct PCB * temp_process;
    while(!isEmpty(waiting_list))
    {
        temp_process = Dequeue(waiting_list);
        struct pair temp_pair = allocate(&my_buddy,temp_process->memory_size);
        if(temp_pair.start == -1 && temp_pair.end == -1)
        {
            Enqueue(temp_q,temp_process);
            
        }
        else
        {
            Enqueue(queue,temp_process);
            printf("At time %d allocated %d bytes for process %d from %d to %d \n", cur_time, temp_process->memory_size,temp_process->id ,temp_pair.start, temp_pair.end);
            fprintf(outputMemoryFile, "At time %d allocated %d bytes for process %d from %d to %d \n", cur_time, running_proc->memory_size,running_proc->id ,running_proc->from_index, running_proc->to_index);
        }

     }
    waiting_list = temp_q;


    num_of_proc--;

    // print here
    fprintf(outputLogFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", getClk(), running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time, TA, WTA);
    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time, TA, WTA);

    kill(running_proc->process_id, SIGKILL);
    running_proc = NULL;
    signal(SIGUSR1, finishProcess);
       
}


void RoundRobin(int qu)
{
    // The process consumes a quantum now
    if (running_proc )
    {
        cur_quantum++;
        //(*shared_memory_address)--;
        printf("quantum now %d, %d, %d\n", cur_time, cur_quantum, quantum);
        running_proc->remaining_time = *shared_memory_address;
        
        if (cur_quantum == qu)
        {
            // insert back to ready queue
            Enqueue(queue, running_proc);
            //kill(running_proc->process_id, SIGSTOP);
            running_proc->last_run = cur_time;
            running_proc->state = blocked;

            // print process is stopped
            fprintf(outputLogFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
            printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);

            // send signal stop to process
             kill(running_proc->process_id, SIGSTOP);
        }
        else // return to run again
        {
            // printf(" HEllo from else ,\n", cur_time);
            return;
        }
    }
    // else
    //     printf(" HEllo from else  %d,\n", cur_time);


    printf("welcome in RR \n");
    cur_quantum = 0;
    /*if(const_num_of_proc==num_of_proc)
    cur_quantum = 1;
    else cur_quantum=0;*/
    struct PCB *temp = running_proc;
    // int flag=
   
    running_proc = Dequeue(queue);

  printf("at time %d , value in queue is:%d \n",cur_time,running_proc->id);
    // the process runs for the first time
    if (running_proc->state == ready)
    {

        // cur_quantum++;
        // First calculate waiting time
        *shared_memory_address = running_proc->remaining_time;
        running_proc->waiting_time = cur_time - running_proc->arrival_time;

        // Print state and info to file
        fprintf(outputLogFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
        printf("At time %d process %d started arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
        startProcess(running_proc);
        // if (running_proc->id == 1)
        // {
        //     goto label;
        //     cur_quantum++;
        //     // running_proc->remaining_time= running_proc->remaining_time+2;
        // }
    }
    // the process runs from a blocing state
    else if (running_proc->state == blocked)
    {

   // // add signal continue
         //kill(running_proc->process_id, SIGCONT);
        running_proc->state = ready;
        running_proc->waiting_time += cur_time - running_proc->last_run;

        // Print state and info to file
        fprintf(outputLogFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
        printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);

        // // add signal continue
         kill(running_proc->process_id, SIGCONT);
    }
}

void HighestPriorityFirst() // Non-preemptive HPF
{
    if (running_proc)
    {
        running_proc->remaining_time = *shared_memory_address;
        return;
    }

    printf("HPF started: \n");
    running_proc = Heap_Extract_MIN(queue);
    printf("%d \n", running_proc->priority);

    if (running_proc->state == ready)
    {
        // First calculate waiting time
        *shared_memory_address = running_proc->remaining_time;
        running_proc->waiting_time = getClk() - running_proc->arrival_time;

        // Print state and info to file
        fprintf(outputLogFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
        printf("At time %d process %d started arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);

        // Start the process
        startProcess(running_proc);
    }
}

// preemitive shortest remaining time algortihm
void ShortestRemainingTimeFirst()
{

    if (running_proc)
    {
        running_proc->remaining_time = *shared_memory_address;
        struct PCB *next_proc = Heap_MIN(queue);
        

        // if the process next has short time then it should context switch to it
        if (running_proc->remaining_time > next_proc->remaining_time)
        {
            Min_Heap_Insert(queue, running_proc);

            // Print state and info to file
            fprintf(outputLogFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
            printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);

            running_proc->state = blocked;
            running_proc->last_run = getClk();

            kill(running_proc->process_id, SIGSTOP);
        }
        else
        {
            return;
        }
    }

    if (running_proc == NULL || running_proc->state == blocked)
    {
        running_proc = Heap_Extract_MIN(queue);

        if (running_proc->state == ready)
        {
            // Calculate waiting time
            *shared_memory_address = running_proc->remaining_time;
            running_proc->waiting_time = getClk() - running_proc->arrival_time;

            // Print state and info to file
            fprintf(outputLogFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
            printf("At time %d process %d started arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
            startProcess(running_proc);
        }
        else if (running_proc->state == blocked)
        {

            running_proc->state = ready;
            running_proc->waiting_time += getClk() - running_proc->last_run;

            // Print state and info to file
            fprintf(outputLogFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);
            printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", cur_time, running_proc->id, running_proc->arrival_time, running_proc->run_time, running_proc->remaining_time, running_proc->waiting_time);

            kill(running_proc->process_id, SIGCONT);
        }
    }
}
