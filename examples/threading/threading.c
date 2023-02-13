#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
//    printf("\nbreak1\n");  
    sleep(thread_func_args->wait_to_obtain_ms/1000);
    
//    printf("\nbreak2\n");
    pthread_mutex_lock (thread_func_args->mutex);
    
//    printf("\nbreak3\n");
    sleep(thread_func_args->wait_to_release_ms/1000);

//    printf("\nbreak4\n");
    pthread_mutex_unlock (thread_func_args->mutex);
    
//    printf("\nbreak5\n");	
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
//    printf("\nbreak mutex:\n");
    struct thread_data *thread_a = malloc(sizeof *thread_a);
    
    thread_a->mutex = mutex;
    thread_a->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_a->wait_to_release_ms = wait_to_release_ms;

    pthread_t thread_new;

//    printf("\nbreak_C\n");
    int rc_create = pthread_create(&thread_new,NULL,threadfunc, (void *) thread_a);
    if (rc_create == 0){
//	    printf("\nSuccess\n");
	    *thread = thread_new;
	    thread_a->thread = thread;
	    thread_a->thread_complete_success = true;
	    return thread_a->thread_complete_success;
    }else
	    return false;

    
}
