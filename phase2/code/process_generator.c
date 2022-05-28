#include "headers.h"
#include <inttypes.h>
#include <stdio.h>

//////////////////////

struct process
{
	int id;			
	int arrival;
	int runtime;
	int priority;	 
	int arrived;
    int memsize;
};

struct msgbuff
{
    long mtype;
    struct process mtext;
};

int number_of_processes; 


void clearResources(int);
struct process* CreateProcesses(char* inputfile); // this function return an array of processes read from input file

int sem, sem1, sem2, semTemp;
int shmid;
union Semun semun, semun1, semun2, semunTemp;

char *Itoa(int number)
{
        uint8_t ch;
        int numberSize = 0;
        char *numberStr = NULL;
        if (number != 0)
        {
                while ((ch = number % 10))
                {
                        number /= 10;

                        numberSize++;
                        numberStr = (char *)realloc(numberStr, numberSize);
                        numberStr[numberSize - 1] = ch + '0';
                }
        }
        else
        {
                numberSize++;
                numberStr = (char *)realloc(numberStr, numberSize);
                numberStr[numberSize - 1] = '0';
        }

        //null terminate the string
        numberSize++;
        numberStr = (char *)realloc(numberStr, numberSize);
        numberStr[numberSize - 1] = '\0';

        return numberStr;
}

int main(int argc, char * argv[]) 
{
    int current_time = -1;

    /////////////////////////////

    shmid = shmget(65,1000*sizeof(struct process), IPC_CREAT | 0664);
    sem1 = semget(66, 1, 0666 | IPC_CREAT);
    sem2 = semget(67, 1, 0666 | IPC_CREAT);
    semTemp = semget(68, 1, 0666 | IPC_CREAT);
    if (shmid == -1 || sem1==-1 || sem2==-1)
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
    if(semctl(semTemp, 0, SETVAL, semunTemp) == -1)
    {
        perror("Error in semctl \n");
        exit(-1);
    }



    
    struct process *shmaddr = shmat(shmid, (void *)0, 0);
    if (shmaddr == NULL)
    {
        perror("Error in client to attach to memory \n");
        exit(-1);
    }
    
    signal(SIGINT, clearResources);

    /////////////semaphore////////////
    sem = semget(20, 1, 0666 | IPC_CREAT);
    if (sem == -1)
    {
                perror("Error in create sem");
                exit(-1);
    }

    semun.val = 0; /* initial value of the semaphore, Binary semaphore */
    if (semctl(sem, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }

    // TODO Initialization
    // 1. Read the input files.
    struct process* processes = CreateProcesses("processes.txt");  

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int Algorithm, Quantum;
    printf("Choose scheduling algorithm: \n 1- Highest Priority First \n 2- Shortest Remaining Job First \n 3- Round Robin \n");
    if ((Algorithm = fgetc(stdin)) == EOF)
        {
                fprintf(stderr, "error reading chosen algorithm\n");
                exit(-1);
        }
        fgetc(stdin); // take the newline out of the stdin

        Algorithm -= '0';

        if (Algorithm == 3)
        {
                printf("Enter quantum for round robin \n");

                char value[100];
                if (fgets(value, 100, stdin) == NULL)
                {
                        fprintf(stderr, "error reading quantum o\n");
                        exit(-1);
                }

                Quantum = atoi(value);
        }
    
    // 3. Initiate and create the scheduler and clock processes.

    //Run the clock
    if (fork() == 0)
    {
        if (execl("clk.out", "clk.out", NULL) == -1)
        {
            perror("process_generator: couldn't run clk.out\n");
            exit(-1);
        }
    }

    //Run the scheduler
    int schaduler_id = fork();
    if (schaduler_id == 0)
    {
        
        if(execl("scheduler.out", "scheduler.out",Itoa(Algorithm) ,Itoa(number_of_processes) ,Itoa(Quantum) , NULL)  == -1)
        {
            perror("process_generator: couldn't run scheduler.out\n");
            exit(-1);
        }        
        perror("process_generator: scheduler.out runs now \n");
    }
 
    //4. Use this function after creating the clock process to initialize clock
    initClk();

    // To get time use this
    int x = getClk();
    printf("current time ---> is %d\n", x);

    // TODO Generation Main Loop

    // 5. Create a data structure for processes and provide it with its parameters.
       //done 

    // 6. Send the information to the scheduler at the appropriate time.
    while(1)
    {
        
        if(current_time == getClk())
            continue; // this to skip if the current time is still the same

        current_time = getClk();

        printf("current time is %d\n", getClk());
         
        for(int i = 0; i < number_of_processes; i++)
        {
            if(processes[i].arrived == 0) 
            {
                // printf("Process id from process generator: %d\n", processes[i].id);
                if(processes[i].arrival == current_time)
                { 
                    down(semTemp);
                    // printf("At time %d process generator sent process %d\n",current_time, processes[i].id);
                    processes[i].arrived = 1;
                    printf("arrival %d  \t currenttime %d  \t clk  %d \n", processes[i].arrival,current_time,getClk());
                    // if the arrival time is the current time send it to schadular 
                    *shmaddr = processes[i];
                    up(sem1);
                    //inform the scheduler to receive
                    kill(schaduler_id,SIGUSR2);
                }
            } 
        }
        up(sem2);
    }

    // 7. Clear clock resources
    destroyClk(true);
    exit(0);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}

//Read from input file and intialize process paramenters
struct process* CreateProcesses(char* inputFileName)
{
    FILE * inputFile;
    char* line = NULL;
    size_t len = 0;
    int count = 0;
    inputFile = fopen(inputFileName, "r");
    //get number of processes
    while(getline(&line,&len,inputFile) != -1)
    {
        if(line[0] != '#')
        {
            number_of_processes++;
        }
    }
    struct process* processes = malloc ((number_of_processes+1) * sizeof *processes); 
    fclose(inputFile);

    // get processes info
    inputFile = fopen(inputFileName, "r");
 
    if (inputFile == NULL) {
        printf("file can't be opened \n");
    }
    else
    {
        printf("content of this file are \n");
        int count = 0;
        while (!feof(inputFile)) 
        {
                // lines beginning with # and should be ignored.
            if(fgetc(inputFile) =='#')
            {
                while(fgetc(inputFile) !='\n');
            }

            fscanf(inputFile,"%d",&processes[count].id);    
            fscanf(inputFile,"%d",&processes[count].arrival);
            fscanf(inputFile,"%d",&processes[count].runtime);
            fscanf(inputFile,"%d",&processes[count].priority);
            fscanf(inputFile,"%d",&processes[count].memsize);
            printf("%d\n",processes[count].memsize);
            processes[count].arrived = 0;
            count++;
            
        }
            fclose(inputFile);
    }
    return processes;
}


