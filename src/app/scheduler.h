#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "common.h"

#define TASK_QUEUE_SIZE 16

/* Structure of the task queue item */
typedef struct {
    void (*funcPtr)(void*); // task function pointer
    void* funcParam;        // task function parameter
    void* nextItem;         // pointer to the next item in the queue
} QueueItem_t;

/*** TASK SCHEDULER API ***/
//void initTaskQueue(void);
void initTaskQueue(void (*f)(void));
void putEvent(void(*func)(void*), void* param);
void runTaskSheduler(void);
#endif // __SCHEDULER_H
//eof
