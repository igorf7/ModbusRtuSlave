#include "scheduler.h"

// Task queue
static QueueItem_t TaskQueue[TASK_QUEUE_SIZE];
// Queue write pointer
static QueueItem_t* writeQPtr = NULL;
// Queue read pointer
static QueueItem_t* readQPtr  = NULL;

// Pointer to background task
static void (*BackgroundTask)(void);

/*!
 \brief Initializes the task queue
 \param Background task callback
 */
void initTaskQueue(void (*callback)(void))
{
	uint32_t i;
    
    BackgroundTask = callback;
	
    for(i = 0; i < TASK_QUEUE_SIZE; i++) {
		TaskQueue[i].funcPtr = NULL;
		TaskQueue[i].funcParam = NULL;
		TaskQueue[i].nextItem = &TaskQueue[i+1];
	}
	TaskQueue[TASK_QUEUE_SIZE-1].nextItem = &TaskQueue[0]; // loop the queue
	writeQPtr = readQPtr  = &TaskQueue[0]; // initialize pointers
}

/*!
 \brief Adds a task to the queue for processing
 \param task to processing
 \param parameter of the task
 */
void putEvent(void(*func)(void*), void* param)
{
__disable_irq();
	writeQPtr->funcPtr = func;
	writeQPtr->funcParam = param;
	writeQPtr = writeQPtr->nextItem;
	if (writeQPtr == readQPtr) {
		readQPtr = readQPtr->nextItem; // queue congestion control
	}
__enable_irq();
}

/*!
 \brief Manages the launch of tasks
 */
void runTaskSheduler(void)
{
    if (writeQPtr == readQPtr) { // checks the task queue is empty or not
        BackgroundTask();  // run backgroud task
    }
    else {
        readQPtr->funcPtr(readQPtr->funcParam); // run active task from queue
        readQPtr = readQPtr->nextItem; // moves the pointer to the next task
    }
}
//eof
