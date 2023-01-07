#include "segel.h"
#include "request.h"
#include <pthread.h>
#include "queue.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

Queue request_queue;

void getargs(int *port, int *threads_num, int *queue_max_size, int argc, char *argv[], char** algo)
{   
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <port> <threads_num> <queue_size> <sched algorithem>\n", argv[0]);
        exit(1);
    }
    *port            = atoi(argv[1]);
    *threads_num     = atoi(argv[2]);
    *queue_max_size  = atoi(argv[3]);
    *algo = argv[4];
}

void* Start_routine(void* thread_id)
{
    if (thread_id == NULL)
        pthread_exit((void*)1);

    int connfd;
    struct timeval dispatch_time, curr_time, arrival_time;
    
    RequestStats stats = (RequestStats)malloc(sizeof(struct request_stats_t));
    if (stats == NULL)
        pthread_exit((void*)1);
    
    stats->thread_stats.id = *(int*)thread_id;
    stats->thread_stats.req_count = 0;
    stats->thread_stats.static_req_count = 0;
    stats->thread_stats.dynamic_req_count = 0;

    while(1)    
    {
        connfd = dequeue(request_queue, &arrival_time);
        gettimeofday(&curr_time, NULL);
        timersub(&curr_time, &arrival_time, &dispatch_time);  //(curr)   -  (arrival)  --> dispatch

        // UpdateStats
        stats->arrival_time = arrival_time;
        stats->dispatch_interval = dispatch_time;
        stats->thread_stats.req_count++;
        
        requestHandle(connfd, stats);                         // handle request
        queueUpdateRequest(request_queue);               // total_requests--
        Close(connfd);
    }

    free(stats);
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_num, queue_max_size;
    char* algo;
    struct sockaddr_in clientaddr;

    // Parse args
    getargs(&port, &threads_num, &queue_max_size, argc, argv, &algo);
    if(algo == NULL)
        exit(1);

    // Create thread pool
    pthread_t* thread_pool = (pthread_t*) malloc(sizeof(pthread_t) * threads_num);
    if(thread_pool == NULL)
        exit(1);

    // Create queue
    request_queue = queueCreate(queue_max_size, algo);
    
    if(request_queue == NULL)
    {
        free(thread_pool);
        exit(1);
    }
    
    // Practically asserting that the allocations succeeded
    if(thread_pool == NULL || request_queue == NULL) // || doing_tasks_list == NULL)
        exit(1);
    
    // Initialize deadpool (threadpool)
    for(int i = 0 ; i < threads_num ; i++) {
        int* thread_id = (int*) malloc(sizeof(int));   // MUST BE ALLOCATED
        *thread_id = i;
        pthread_create(&(thread_pool[i]), NULL, Start_routine, (void*)thread_id);
    }    

    listenfd = Open_listenfd(port);

    while (1) 
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        enqueue(request_queue, connfd);
    }
    
    for (int i = 0; i < threads_num; i++) {
        pthread_join(thread_pool[i], NULL);
    }
    queueDestroy(request_queue);
    free(thread_pool);
}

