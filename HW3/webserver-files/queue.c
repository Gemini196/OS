#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "queue.h"
#include "segel.h"


Queue queueCreate(int max_size, char* algo)
{
    Queue queue = malloc(sizeof(*queue));
    if (queue == NULL)
        return NULL;
    
    
    // Basic initialization 
    queue->front = NULL;
    queue->rear = NULL;
    queue->max_size=  max_size;
    queue->curr_size = 0;
    queue->algo = algo;
    queue->total_request = 0;

    // Complex initialization
    queue->dequeue_allowed = malloc(sizeof(pthread_cond_t));   // need to alloc the condition var
    queue->enqueue_allowed = malloc(sizeof(pthread_cond_t));   // need to alloc the condition var
    queue->queue_lock = malloc(sizeof(pthread_mutex_t));       // need to alooc mutex lock;

    if(queue->dequeue_allowed == NULL || queue->enqueue_allowed == NULL || queue->queue_lock == NULL) // memory leak, but i just want to make sure no segfaults
        return NULL;
        
    pthread_cond_init(queue->dequeue_allowed, NULL);
    pthread_cond_init(queue->enqueue_allowed, NULL);
    pthread_mutex_init(queue->queue_lock, NULL);

    return queue;
}

void queueDestroy(Queue queue)
{
    if (queue == NULL) { return; }
    
    Node temp_node = NULL;
    // Scan nodes and free them
    while (queue->front != NULL)
    {
        temp_node = queue->front;
        queue->front = queue->front->previous;
        free(temp_node);
    }
    pthread_cond_destroy(queue->dequeue_allowed);
    pthread_cond_destroy(queue->enqueue_allowed);
    pthread_mutex_destroy(queue->queue_lock);
    free(queue->dequeue_allowed);
    free(queue->enqueue_allowed);
    free(queue->queue_lock);
    free(queue);
}

void enqueue(Queue queue, int fd)
{
    struct timeval arrival_time; 
    gettimeofday(&arrival_time, NULL);

    // New node
    Node to_add = (Node)malloc(sizeof(*to_add));
    if(to_add == NULL)
        return;

    to_add->fd = fd;
    to_add->previous = NULL;
    to_add->arrival_time = arrival_time;

    // try to lock
    pthread_mutex_lock(queue->queue_lock);
    
    // Attempt insertion into a full queue :: DROP using the relevant algorithm
    if (queue->total_request == queue->max_size)
    {
        Node temp_node;
        if(!strcmp(queue->algo, "block"))
        {
            // Wait to insert for as long as the queue's full
            while (queue->total_request == queue->max_size) {
                pthread_cond_wait(queue->enqueue_allowed, queue->queue_lock);
            }
        }

        else if(!strcmp(queue->algo, "dt") || queue->curr_size == 0){ //drop tail (or if curr_size=0)
            pthread_mutex_unlock(queue->queue_lock);
            free(to_add);
            close(fd);
            return;         // Ignore request and return
        }

        else if(!strcmp(queue->algo, "random")){
            // Drop 30% random elements from the queue 
            DropRandom(queue);
        }
        
        else if(!strcmp(queue->algo, "dh")){ //drop front
            temp_node = queue->front;
            int fd_to_remove = temp_node->fd;

            // move the front
            queue->front = queue->front->previous;
            queue->curr_size--;
            queue->total_request--;

            // if we reached the end, throw 'rear' away
            if(queue->curr_size == 0)
                queue->rear = NULL;
            
            free(temp_node);           // free the first node            
            close(fd_to_remove);       // close connection to oldest request
        }
    }

    // case1: Insert into empty queue
    if(queue->curr_size == 0)
        queue->front = to_add;
    
    // case2: Insert into non empty queue
    else
        queue->rear->previous = to_add; // add newnode in rear->previous

    queue->rear = to_add;        
    queue->curr_size++;
    queue->total_request++;

    //release lock and signal to worker threads
    pthread_cond_signal(queue->dequeue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
}


// Helper function for the DROP_RANDOM overload policy
// Assume we have queue lock and total_request lock
void DropRandom(Queue queue)
{
    // If we reached here, we can drop AT LEAST 1 waiting task   
    int to_drop = ((queue->curr_size) * 3 ) / 10;               // TODO - Piazza @309, do we round up???? 
    to_drop += (((queue->curr_size * 3) % 10 != 0)? 1 : 0);     // Round up - if the remainder is not zero, add 1
    //printf("[DEBUG] dropping %d requests: \n", to_drop);
    if (to_drop > queue->curr_size) {
        to_drop = queue->curr_size;
    }

    int* who_to_drop = calloc(queue->curr_size, sizeof(int));
    if (who_to_drop == NULL) {
        exit(1);
    }

    // The randomization magic is done here 
    for (int i = 0; i < to_drop; i++) // We made sure that to_drop is not larger than queue->curr_size
    {
        int new_num_added = 0;

        // Try to randomize a number until succeeded to randomize a unique one
        while (!new_num_added)
        {
            int place = rand() % queue->curr_size;
            if(who_to_drop[place] == 0) {
                who_to_drop[place] = 1;
                new_num_added      = 1;
            }
        }
    }
    
    // Create new list that will be patched into the queue
    Node new_front = NULL, new_rear = NULL;
    Node iterator = queue->front;
    int new_size = 0;
    for (int i = 0; i < queue->curr_size; i++)
    {
        if(who_to_drop[i] == 1) {
            close(iterator->fd);
            iterator = iterator->previous;
            continue;
        }
        
        Node to_add = malloc(sizeof(*to_add));
        if(to_add == NULL)
            exit(1);
        to_add->fd        = iterator->fd;
        to_add->arrival_time = iterator->arrival_time;
        iterator = iterator->previous;

        if (new_front == NULL) {
            new_front = to_add;
            new_rear  = to_add;
            new_size++;
            continue; 
        }
        
        new_rear->previous = to_add;
        new_rear = new_rear->previous;
        new_size++;
    }
    
    free(who_to_drop);
    
    // Clear old queue
    Node temp_node = NULL;
    while (queue->front != NULL)
    {
        temp_node = queue->front;
        queue->front = queue->front->previous;
        free(temp_node);
    }
    
    // Update old fds
    queue->front          = new_front;
    queue->rear           = new_rear;
    queue->curr_size      = new_size;
    queue->total_request = queue->total_request - to_drop;
}

int dequeue(Queue queue, struct timeval* arrival_time)
{
    // try to lock queue
    pthread_mutex_lock(queue->queue_lock);

    while(queue->curr_size == 0) {
        pthread_cond_wait(queue->dequeue_allowed, queue->queue_lock);
    }

    Node temp_node     = queue->front;
    int fd            = temp_node->fd;
    *arrival_time = temp_node->arrival_time;

    // move the front forward
    queue->front = queue->front->previous;
    queue->curr_size--;

    // if we reached the end, throw 'rear' away
    if(queue->curr_size == 0)
        queue->rear = NULL;
    
    free(temp_node); //free the first node

    //release lock and signal
    pthread_cond_signal(queue->enqueue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
    return fd;
}

void queueUpdateRequest(Queue queue)
{
    pthread_mutex_lock(queue->queue_lock);
    queue->total_request--;
    pthread_cond_signal(queue->enqueue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
}
