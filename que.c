// Kevin Carlisle
// 82682616
// CS143a 
// Homework 5


#include <stdio.h>
#include <stdlib.h>
#include "que.h"
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

static ELE _que[QUE_MAX];
static int _front = 0, _rear = 0;
extern int producers_working;
static int matches = 0;

// Time Stuff
int              r_v;
struct timespec  t_s;
struct timeval   t_p;
#define MAX_WAIT_TIME 5

// Initialize Mutex & Cond
static pthread_mutex_t match_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t misc_mutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  can_consume = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  can_produce = PTHREAD_COND_INITIALIZER;

void add_match()
{
    //Note: you will need to lock this update because it is a race condition
    pthread_mutex_lock(&match_mutex);
    matches++;
    pthread_mutex_unlock(&match_mutex);
}

void report_matches(char *pattern)
{
    printf("Found %d total matches of '%s'\n", matches, pattern);
}

int que_init()
{
    // Initialize Mutex & Cond
    pthread_mutex_init(&match_mutex, NULL);  
    pthread_mutex_init(&misc_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init (&can_consume, NULL);
    pthread_cond_init (&can_produce, NULL);

    // Set Time
    r_v = gettimeofday(&t_p, NULL);
    t_s.tv_sec  = t_p.tv_sec;
    t_s.tv_nsec = t_p.tv_usec * 1000;
    t_s.tv_sec += MAX_WAIT_TIME;
}

void que_error(char *msg)
{
    fprintf(stderr, "***** Error: %s\n", msg);
    // exit(-1);
}

int que_is_full()
{
    int temp;
    // likely not needed
    pthread_mutex_lock(&misc_mutex);
    temp =  (_rear + 1) % QUE_MAX == _front; /* this is why one slot is unused */
    pthread_mutex_unlock(&misc_mutex);

    return temp;
}

int que_is_empty()
{
    int temp;
    // likely not needed
    pthread_mutex_lock(&misc_mutex);
    temp = (_front == _rear);
    pthread_mutex_unlock(&misc_mutex);

    return temp;
}

void que_enq(ELE v)
{
    pthread_mutex_lock(&queue_mutex);
    while( que_is_full() )
    {
        pthread_cond_wait(&can_produce, &queue_mutex);
    }

    if ( que_is_full() )
    {
        que_error("enq on full queue");
    }
    
    _que[_rear++] = v;
    
    if ( _rear >= QUE_MAX )
    {
        _rear = 0;
    }
    pthread_cond_signal(&can_consume);
    pthread_mutex_unlock(&queue_mutex);
}

ELE que_deq()
{
    
    pthread_mutex_lock(&queue_mutex);
    while( producers_working == 1 && que_is_empty()) 
    {
        pthread_cond_timedwait(&can_consume, &queue_mutex, &t_s);   
    }
    if ( producers_working == 0 && que_is_empty())
    {
        pthread_mutex_unlock(&queue_mutex);
        pthread_exit(NULL);
    }
    
    if ( que_is_empty() )
    {
        //printf("deq on empty: %d\n", producers_working);
        que_error("deq on empty queue");
    }
    
    ELE ret = _que[_front++];
    
    if ( _front >= QUE_MAX )
    {
        _front = 0;
    }
    pthread_cond_signal(&can_produce);
    pthread_mutex_unlock(&queue_mutex);
    return ret;
}


/*

int main()
{
    for ( int i=0; i<QUE_MAX-1; ++i )
    {
        Buffer b;
        sprintf(&b.string, "%d", i);
        que_enq(b);
    }
    while ( !que_is_empty() )
        printf("%s ", que_deq().string);
    putchar('\n');
}
*/
