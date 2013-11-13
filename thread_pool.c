#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "thread_pool.h"

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

typedef struct threadpool_task_t{
    void (*function)(void *);
    void *argument;
    struct threadpool_task_t* next;
}threadpool_task_t;


struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads; //array of threads
  threadpool_task_t *queue; //LL of threadpool_tasks 
  int thread_count;
  int task_queue_size_limit;
};

/**
 * @function void *threadpool_work(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
static void *thread_do_work(void *threadpool);


/*
 * Create a threadpool, initialize variables, etc
 *
 */
threadpool_t *threadpool_create(int thread_count, int queue_size)
{ 

printf("Initalizing thread pool \n");
/*create thread pool and initialize variables*/
threadpool_t* thread_pool = (threadpool_t*) malloc (sizeof(threadpool_t));
thread_pool->thread_count=thread_count;
thread_pool->task_queue_size_limit = queue_size;

/*create mutex*/
pthread_mutex_t lock;
pthread_mutex_init(&lock, NULL);
thread_pool->lock=lock;

/*create condition*/
pthread_cond_t notify;
pthread_cond_init (&notify, NULL);
thread_pool->notify=notify;

/*create the threads and add them to pool*/

/*create a default thread attribute*/
pthread_attr_t attr;
pthread_attr_init(&attr);

int i;
pthread_t *threads = (pthread_t*) malloc (sizeof(pthread_t)*thread_count) ;
int pthread_create_error;

for (i=0; i<thread_count; i++)
{
	printf("creating thread %i \n", i);
	pthread_create_error = pthread_create(&threads[i], &attr, thread_do_work, (void*)thread_pool);
	if (pthread_create_error)
	{ printf("ERROR: return code from pthread_create() is %d \n", pthread_create_error);
	exit(-1);
	}		 
} 
thread_pool->threads = threads;

thread_pool->queue=NULL; /*task queue is initally empty*/

return thread_pool;
}


/*
 * Add a task to the threadpool
 *
 */
int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *argument)
{
    int err = 0;
    /* Get the lock */
    pthread_mutex_t* lock = &(pool->lock);
    /*Lock the queue while you update it (is this what thread safe means?).*/
     printf("In critical section. Main thread is adding a task to the queue. Queue is locked from other threads \n"); 
    err = pthread_mutex_lock(lock);
    if (err)
    {printf("Error when locking mutex. pthread_mutex_lock returned: %d \n", err);
return -1;
}
    /*set the lock, now the queue is locked from the other threads*/
    
    /* Add task to queue */

    /*create a new task*/
    threadpool_task_t* new_task = (threadpool_task_t*) malloc (sizeof(threadpool_task_t));
    new_task->function=function;
    new_task->argument=argument;
    new_task->next=NULL;

    /*add task to pool's queue*/
    
    /*If queue is empty initialize*/
     if (pool->queue==NULL) 
     {pool->queue=new_task;} 

   /*update LL*/  
   else
     {
     threadpool_task_t* curr = pool->queue;
     while (curr->next!=NULL)
      {curr = curr->next;}
   
     curr->next = new_task;
     } 

   /*done updating the queue, notify the sleeping threads and unlock mutex*/


    /*get condition*/
    pthread_cond_t* notify = &(pool->notify);
    /* pthread_cond_broadcast and unlock */
    err =  pthread_cond_broadcast(notify);
    if (err)
    {printf("Error when broadcasting cond. pthread_cond_broadcast returned: %d \n", err);
return -1;
}

    err = pthread_mutex_unlock(lock);     
       if (err)
    {printf("Error when unlocking mutex. pthread_mutex_unlock returned: %d \n", err);
return -1;
}

    return err;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int threadpool_destroy(threadpool_t *pool)
{
    int err = 0;

   /* Wake up all worker threads */
   threadpool_add_task(pool, NULL, NULL);
   pthread_cond_broadcast(&(pool->notify));
   /* Join all worker thread */
   int i;
	for (i=0; i<pool->thread_count; i++)
	{pthread_join(pool->threads[i],NULL);}  		

    /* Only if everything went well do we deallocate the pool */
   free ((void*) pool->queue);
   free ((void*) pool->threads);
   free ((void*) pool);  

    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *threadpool)
{ 

    while(1) {

        /*Typecaste threadpool as threadpool_t* */
        threadpool_t* pool = (threadpool_t*) threadpool; 

        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_t *lock = &(pool->lock); 
        pthread_cond_t *notify = &(pool->notify);

        /*Set the lock*/
        int err = pthread_mutex_lock(lock);
        if(err)
        {
	   printf("pthread_mutex_lock error \n");
	   continue;
	}
        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), do some task. */
       err = pthread_cond_wait(notify, lock);
       if (err)
       {printf("pthread_cond_wait error \n");}
        /*block on notify. Atomically(?) release lock- what does this mean? */
        
        /* Grab our task from the queue */
	threadpool_task_t* curr_task = pool->queue;

       if (curr_task==NULL)
         {
            continue; 
         }

	if(curr_task->function == NULL)
        {
         pthread_mutex_unlock(&(pool->lock));
         pthread_exit(NULL);
    	 return(NULL);
        }

       /*delete task from LL*/
        pool->queue=pool->queue->next;   
        
        /*Unlock mutex for others*/
	err = pthread_mutex_unlock(lock);
        if(err)
        {printf("pthread_mutex_unlock error \n");}

        /* Start the task */
         void (*function) (void*);
         void *argument;
         function = curr_task->function;
         argument = curr_task->argument;
         
         free ((void*) curr_task);    
         function(argument);
     
    }

}
