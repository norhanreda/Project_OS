

#include <limits.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include"PCB.h"

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



struct Queue* CreateQueue(unsigned capacity)  
{ 
   //  initialize values 

        struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue)); 
        queue->capacity = capacity; 
        queue->front = queue->size = 0; 
        queue->rear = capacity - 1; 
        queue->array_of_processes= malloc(queue->capacity * sizeof(struct PCB*)); // (struct PCB**)
        return queue; 
} 

int isfull(struct Queue * q)
{
     if(q->size==q->capacity)
       return 1; //queue is full
       else
       return 0; //queue is not full
}

int isEmpty(struct Queue * q)

{
if(q->size==0)
 return 1;
 else 
 return 0;
}
void Enqueue(struct Queue* queue, struct PCB* p)  // enqueue a process into queue
{ 
        if (isfull(queue)) 
        return;  // can't insertt is full :(
        // can insert
        queue->rear = (queue->rear + 1) % queue->capacity;
        /*if(queue->rear == queue->capacity)
        {
         queue->rear=0;

        }*/
        queue->array_of_processes[queue->rear] = p; 
        queue->size = queue->size + 1; 
} 

struct PCB* Dequeue(struct Queue* queue) 
{ 
        if (isEmpty(queue)) 
        return NULL; 
       struct PCB* p = queue->array_of_processes[queue->front]; 
        queue->front = (queue->front + 1) % queue->capacity; 
        queue->size = queue->size - 1; 
        return p; 
} 

struct PCB* Front(struct Queue* queue) 
{ 
    if (isEmpty(queue))
     return NULL; 
    return queue->array_of_processes[queue->front]; 
} 

struct PCB* Rear(struct Queue* queue) 
{ 
    if (isEmpty(queue)) 
    return NULL; 
    return queue->array_of_processes[queue->rear]; 
} 
