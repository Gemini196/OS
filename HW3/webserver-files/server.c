#include "segel.h"
#include "request.h"
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

// HW3: Parse the new arguments too
void getargs(int *port, int* threads_num, int* queue_max_size, int argc, char *argv[])
{
    if (argc < 4) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads_num = atoi(argv[2]);
    *queue_max_size = atoi(argv[3]);
}


void* Start_routine(void* thread_id)
{
    int connfd; // dequeue into this
//    struct timeval dispatch_time, curr_time, arrival_time;
//
//    // Initialize stats
//    Stats* stats = malloc(sizeof(*stats));
//    if (stats == NULL) {
//        exit(1); // TODO - lmao XD YEET
//    }
//    stats->handler_thread_stats.handler_thread_id                = *(int*)thread_id;
//    stats->handler_thread_stats.handler_thread_req_count         = 0;
//    stats->handler_thread_stats.handler_thread_static_req_count  = 0;
//    stats->handler_thread_stats.handler_thread_dynamic_req_count = 0;

    while(1)
    {
        connfd = dequeue(to_do_tasks_queue, &arrival_time);    // Taking a task from the 'todo' queue
//        gettimeofday(&curr_time, NULL);                        // Getting current time
//        timersub(&curr_time, &arrival_time, &dispatch_time);
//        //  ^^    (curr)   -  (arrival)  --> dispatch

//        // Stats preparation
//        stats->arrival_time = arrival_time;
//        stats->dispatch_interval = dispatch_time;
//        stats->handler_thread_stats.handler_thread_req_count++;

        requestHandle(connfd, stats);
        queueUpdateRequest(to_do_tasks_queue);
        Close(connfd);
    }

//    free(stats);
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_num, max_requests_size, queue_max_size;
    char* algo;
    struct sockaddr_in clientaddr;
    Queue request_queue;


    getargs(&port, &threads_num, &max_requests_size, argc, argv);
    // queue_max_size = max_requests_size - threads_num;
    pthread_t* thread_pool = (pthread_t*) malloc(sizeof(pthread_t) * threads_num);
    request_queue = queueCreate(max_requests_size, algo);
    if(thread_pool == NULL || request_queue == NULL)
        exit(1);
    for (int i=0; i<threads_num; i++){
        pthread_create(&(thread_pool[i]), NULL, Start_routine, (void*)(&i));
    }

    listenfd = Open_listenfd(port);
    while (1) {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        requestHandle(connfd);

        Close(connfd);
    }

    for (int i=0; i<threads_num; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    queueDestroy(request_queue);
    free(thread_pool);

}


    


 
