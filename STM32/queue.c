#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#define DIM 20 //Assumo massimo 20 prenotazioni


queue* create_queue()
{
    queue *q;

    q = (queue *) malloc(sizeof(queue));

    q->v = (int *) malloc(sizeof(int)*DIM);

    q->head = -1;
    q->tail = -1;
    q->size = 0;
    return q;
}

int is_empty(queue *q)
{
    if(q->size == 0)
        return 1;
    return 0;
}

int is_full(queue *q)
{
    if(q->size == DIM)
        return 1;
    return 0;
}

void push(queue *q, int x)
{
    if(is_empty(q))
    {
        q->head = q->tail = 0;
        q->v[q->tail] = x;
        q->size++;
    }
    else if(is_full(q))
        exit(1);
    else
    {
        q->tail = (q->tail+1) % DIM;
        q->v[q->tail] = x;
        q->size++;
    }
}

int pop(queue *q)
{
    if(is_empty(q))
        exit(1);
    else
    {
        int x = q->v[q->head];
        q->head = (q->head + 1) % (DIM);
        q->size--;
        return x;
    }
}

