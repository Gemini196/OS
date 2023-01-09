#ifndef _QUEUE_H
#define _QUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>


// typedef struct node_t *Node;
typedef struct node_t {
    int fd;
    struct timeval arrival_time;
    struct node_t* previous;
    bool to_drop;
} *Node;

typedef struct queue_t {
    Node front;
    Node rear;
    int max_size;
    int curr_size;
    char* algo;
    int total_requests; // all requests in the server
    

    pthread_cond_t* dequeue_allowed;
    pthread_cond_t* enqueue_allowed;
    pthread_mutex_t* queue_lock;
} *Queue;

Queue queueCreate(int max_size, char* algo);
void queueDestroy(Queue queue);
void enqueue(Queue queue, int fd);
int dequeue(Queue queue, struct timeval* arrival_time);
void queueUpdateRequest(Queue queue);
void DropRandom(Queue queue);
void ClearQueue(Queue queue);
void RemoveIndexesFromQueue(Queue* queue, int* indexes_to_drop, int indexes_to_drop_count);
/*void RemoveIndexesFromQueue(Queue queue, int* indexes_to_drop, int indexes_to_drop_count);*/

#endif // _QUEUE_H