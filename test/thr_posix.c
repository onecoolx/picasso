

#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include "picasso.h"


extern void thread_func1(void* data);
extern void thread_func2(void* data);


void* linux_thread_1(void *p1)
{

    thread_func1(p1);
    return NULL;
}

void* linux_thread_2(void *p1)
{

    thread_func2(p1);
    return NULL;
}

static pthread_t th1, th2;

void start_worker_threads(ps_canvas* cs)
{
    pthread_create(&th1, NULL, linux_thread_1, cs);
    pthread_create(&th2, NULL, linux_thread_2, cs);
}

void stop_worker_threads(void)
{
    pthread_cancel(th1);
    pthread_cancel(th2);
}

void loop_wait(int ms)
{
    usleep(ms*1000);
}
