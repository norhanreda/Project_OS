#include <limits.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include "Ready_queue.h"

void swap(struct PCB **p1,struct PCB **p2)
{
	struct PCB* temp = *p1;
	*p1 = *p2;
	*p2 = temp;
}

void Min_Heapify(struct Queue* q, int parent)
{
   int left = 2 * parent + 1;
   int right = 2 * parent + 2;
   int min = 0;

   if(left < q-> size && q->array_of_processes[left]->priority < q->array_of_processes[parent]->priority)
      min = left;
   else
      min = parent;
        
   if(right < q-> size && q->array_of_processes[right]->priority < q->array_of_processes[min]->priority)
      min = right;

   if(min!=parent)
   {
      swap(&q->array_of_processes[min], &q->array_of_processes[parent]);
      Min_Heapify(q,min);
   }   
}

struct PCB* Heap_MIN(struct Queue* q)
{
   return q->array_of_processes[0];
}

struct PCB* Heap_Extract_MIN(struct Queue* q)
{
   if(q->size < 1)
      return 0;

   struct PCB* min = q->array_of_processes[0];
   q->array_of_processes[0] = q->array_of_processes[q->size-1];

   (q->size)--;
   Min_Heapify(q,0);
   return min;
}

void Heap_decrease_key(struct Queue* q, int i, int key)
{
   if(key > q->array_of_processes[i]->priority)
      return;

   q->array_of_processes[i]->priority = key;
   int parent=(i-1)/2;

   while(i > 0 && q->array_of_processes[parent]->priority > q->array_of_processes[i]->priority)
   {
      swap(&q->array_of_processes[i], &q->array_of_processes[parent]);
      i = parent;
   }
}

void Min_Heap_Insert(struct Queue* q, struct PCB* p)
{
   if(q->size == q->capacity)
      return;
        
   q->array_of_processes[q->size] = p;
   int key = p->priority;
   q->array_of_processes[q->size]->priority = INT_MAX;
   q->size++;     
   Heap_decrease_key(q, q->size -1, key);
}


void print(struct Queue *q)
{
  for (int i = 0; i < q->size; i++)
  {
    printf("element %d is %d\n", i, q->array_of_processes[i]->priority);
  }
}