/*
 * queue.h
 *
 *  Created on: 19 feb 2021
 *      Author: Tizio
 */

#ifndef QUEUE
#define QUEUE

struct queue
{
    int *v;
    int head,tail,size;
};

typedef struct queue queue;

queue * create_queue();

int is_empty(queue *q);

int is_full(queue *q);

void push(queue *q, int x);

int pop(queue *q);

void stampa(queue *q);

#endif



