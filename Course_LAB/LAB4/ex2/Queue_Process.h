#ifndef QUEUE_PROCESS_H
#define QUEUE_PROCESS_H

#ifndef Q_SIZE
    #define Q_SIZE 5
#endif

typedef struct
{
    int buffer[Q_SIZE]; // 存放 ADC 样本
    int head;           // 出队位置
    int tail;           // 入队位置
    int count;          // 队列中的元素数量
} SampleQueue_t;

int enqueue_sample(SampleQueue_t *q, int data);
int dequeue_all(SampleQueue_t *q, int *out);
int queue_empty(SampleQueue_t *q);
void queue_reset(SampleQueue_t *q);

#endif /* QUEUE_PROCESS_H */