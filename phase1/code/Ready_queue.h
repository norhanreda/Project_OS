

#include <limits.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include"PCB.h"

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
