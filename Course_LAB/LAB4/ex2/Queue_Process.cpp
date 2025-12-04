#include "Queue_Process.h"

/* queue overflow only reported by S */
int enqueue_sample(SampleQueue_t *q, int data)
{
    if (q->count == Q_SIZE)
    {
        return 1; // overflow
    }

    q->buffer[q->tail] = data;
    q->tail = (q->tail + 1) % Q_SIZE;
    q->count++;
    return 0;
}

int dequeue_all(SampleQueue_t *q, int *out)
{
    int n = q->count;

    for (int i = 0; i < n; i++)
    {
        out[i] = q->buffer[q->head];
        q->head = (q->head + 1) % Q_SIZE;
    }

    q->count = 0;
    return n;
}


int queue_empty(SampleQueue_t *q)
{
    return (q->count == 0);
}


void queue_reset(SampleQueue_t *q)
{
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}