#include "segel.h"
#include "request.h"
#include "queue.h"
#include "request_stats.h"

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

// HW3: Parse the new arguments too
void getargs(int *port, int* threads_num, int* queue_max_size, int argc, char *argv[], char **algo)
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads_num = atoi(argv[2]);
    *queue_max_size = atoi(argv[3]);
    *algo = (char*)argv[4];
}


void* Start_routine(void* thread_id)
{
    if (thread_id == NULL)
        pthread_exit((void*)1);
    int id = *((int*)thread_id);

    int connfd; // dequeue into this
    struct timeval dispatch_time, curr_time, arrival_time;

    ReqStats request_stats = malloc(sizeof(request_stats));
    
    if (request_stats == NULL)
        pthread_exit((void*)1);
    
    request_stats->thread_stats.id =  id;
    request_stats->thread_stats.dynamic_req_count = 0;
    request_stats->thread_stats.static_req_count = 0;
    request_stats->thread_stats.req_count = 0;

    while(1)
    {
        // get task from queue
        connfd = dequeue(request_queue, &arrival_time); // you can get arrival time only if the queue ain't empty
        gettimeofday(&curr_time, NULL);                        
        timersub(&curr_time, &arrival_time, &dispatch_time); // curr - arrival = dispatch

        // Update statistics
        request_stats->arrival_time = arrival_time;
        request_stats->dispatch_interval = dispatch_time;
        request_stats->thread_stats.req_count++;
        requestHandle(connfd, request_stats);
        queueUpdateRequest(request_queue);
        Close(connfd);
    }

    free(request_stats);
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_num, max_requests_size;
    char* algo = NULL;
    struct sockaddr_in clientaddr;

    // Parse command arguments
    getargs(&port, &threads_num, &max_requests_size, argc, argv, &algo);

    // Alloc thread_pool
    pthread_t* thread_pool = (pthread_t*) malloc(sizeof(pthread_t) * threads_num);
    if(thread_pool == NULL || algo == NULL)
        exit(1);
    
    // Create Queue
    request_queue = queueCreate(max_requests_size, algo);
    if(request_queue == NULL){
        free(thread_pool);
        exit(1);
    }

    // Create threads (in threadpool)
    for (int i=0; i<threads_num; i++){
        int* thread_id = (int*) malloc(sizeof(int)); // THIS MUST BE ALLOCATED!!
        *thread_id = i;
        pthread_create(&(thread_pool[i]), NULL, Start_routine, (void*)thread_id);
    }

    // Open server port to listen
    listenfd = Open_listenfd(port);

    // Server is open for requests
    while (1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        // Insert new request node into queue
        enqueue(request_queue, connfd);
    }

    for (int i=0; i<threads_num; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    queueDestroy(request_queue);
    free(thread_pool);
}
