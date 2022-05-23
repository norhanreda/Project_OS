enum State
{
  ready,
  blocked,
  paused,
  finished
};

//process control block 
struct PCB
{
  enum  State state;  
  int id; 
  int process_id;
  int arrival_time;                    // time to arrive to reeady queue
  int run_time;                        // the burst time of the process
  int priority;                        // priority assume low value is high priority
  int starting_time;                   // it is the time to start use the cpu (schaduled)
  int finsihing_time;                  // end its running time
  int remaining_time;                  // time remained to finish 
  int last_run;                  
  int waiting_time;                    //Total waiting time since it start use cpu
  int stopping_time;                   //The time in which the process paused.
   
  struct PCB *next;
  struct PCB *back;   
};

struct Queue
{ 
  unsigned capacity;                //max capacity
  int size;                         //current size
  int front;                        //front index in array
  int rear;                         //rear index in array
  struct PCB** array_of_processes;  //the queue
};

