#ifndef WEBSERVER_FILES_QUEUE_H
#define WEBSERVER_FILES_QUEUE_H
#include <stdlib.h>
#include "segel.h"
typedef struct queue_t *Queue;

Queue queueCreate(int max_size, char* algo);
void enqueue(Queue queue, int fd, struct timeval arrival_time);
int dequeue(Queue queue, struct timeval* arrival_time);            // returns the data of the dequeued node
void queueDestroy(Queue queue);
void queueUpdateRequest(Queue queue);


#endif //WEBSERVER_FILES_QUEUE_H
