#include "os_threadpool.h"
#include "os_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


pthread_cond_t condQueue;

/* === TASK === */

/* Creates a task that thread must execute */
os_task_t *task_create(void *arg, void (*f)(void *))
{
	os_task_t *task = malloc(sizeof(os_task_t));

	task->argument = arg;
	task->task = f;
	return task;
}

/* Add a new task to threadpool task queue */
void add_task_in_queue(os_threadpool_t *tp, os_task_t *t)
{
	os_task_queue_t *new_node = malloc(sizeof(os_task_queue_t));

	new_node->next = NULL;
	new_node->task = t;

	if (tp->tasks == NULL) {
		tp->tasks = new_node;
	} else {
		os_task_queue_t *p = tp->tasks;

		while (p->next != NULL) {
			p = p->next;
        }
		p->next = new_node;
	}
}

/* Get the head of task queue from threadpool */
os_task_t *get_task(os_threadpool_t *tp)
{
    //If the queue is empty return NULL
    if (tp->tasks == NULL){
        return NULL;
    }
    // Else take the first element out
    os_task_t *target = tp->tasks->task;
    tp->tasks = tp->tasks->next;

    return target;
}

/* === THREAD POOL === */

/* Initialize the new threadpool */
os_threadpool_t *threadpool_create(unsigned int nTasks, unsigned int nThreads)
{
    os_threadpool_t *threadpool = malloc(sizeof(os_threadpool_t));
    threadpool->tasks = NULL;
    threadpool->threads = malloc(sizeof(pthread_t) * nThreads);
    threadpool->num_threads = nThreads;
    threadpool->should_stop = 0;
    pthread_mutex_init(&threadpool->taskLock,NULL);
    return threadpool;
}

/* Loop function for threads */
void *thread_loop_function(void *args)
{
    os_threadpool_t *tp = (os_threadpool_t*)args;
    while (1) {

        pthread_mutex_lock(&(tp->taskLock));
        os_task_t *task = get_task(tp);
        pthread_mutex_unlock(&(tp->taskLock));
        if (task != NULL) {
            task->task(task->argument);
        } 
        else {
            break;
        }
    }
    return NULL;
}

/* Stop the thread pool once a condition is met */
void threadpool_stop(os_threadpool_t *tp, int (*processingIsDone)())
{
    return;
}
