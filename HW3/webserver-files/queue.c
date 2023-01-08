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
    
    // initialize everything
    queue->front = NULL;
    queue->rear = NULL;
    queue->max_size=  max_size;
    queue->curr_size = 0;
    queue->algo = algo;
    queue->total_requests = 0;

    // malloc for multithreading
    queue->queue_lock = malloc(sizeof(pthread_mutex_t));
    queue->dequeue_allowed = malloc(sizeof(pthread_cond_t));
    queue->enqueue_allowed = malloc(sizeof(pthread_cond_t));


    // make sure mallocs succeeded
    if(queue->queue_lock == NULL || queue->enqueue_allowed == NULL || queue->dequeue_allowed == NULL)
        return NULL;

    // initialize lock and cond variables
    pthread_mutex_init(queue->queue_lock, NULL);
    pthread_cond_init(queue->dequeue_allowed, NULL);
    pthread_cond_init(queue->enqueue_allowed, NULL);
    return queue;
}

void queueDestroy(Queue queue)
{
    if (queue == NULL)
        return;
    
    Node temp_node = NULL;

    // delete all nodes
    while (queue->front != NULL)
    {
        temp_node = queue->front;
        queue->front = queue->front->previous;
        free(temp_node);
    }
    // free all fields
    free(queue->dequeue_allowed);
    free(queue->enqueue_allowed);
    free(queue->queue_lock);

    pthread_cond_destroy(queue->dequeue_allowed);
    pthread_cond_destroy(queue->enqueue_allowed);
    pthread_mutex_destroy(queue->queue_lock);

    free(queue);
}

void enqueue(Queue queue, int fd)
{
    struct timeval arrival_time; 
    gettimeofday(&arrival_time, NULL);

    // New node to add
    Node new_node = (Node)malloc(sizeof(*new_node));

    if(new_node == NULL)
        return;

    new_node->fd = fd;
    new_node->previous = NULL;
    new_node->arrival_time = arrival_time;
    new_node->to_drop = false;

    // acquire lock
    pthread_mutex_lock(queue->queue_lock);
    
    // queue is full, deal with it according to algo
    if (queue->total_requests == queue->max_size)
    {
        Node temp_node;
        if(!strcmp(queue->algo, "dt") || queue->curr_size == 0){ //drop tail (or if curr_size=0)
            pthread_mutex_unlock(queue->queue_lock);
            free(new_node);
            close(fd);
            return;         // Ignore request and return
        }

        
        else if(!strcmp(queue->algo, "dh")){ //drop front
            temp_node = queue->front;
            int fd_to_remove = temp_node->fd;

            // move the front
            queue->front = queue->front->previous;
            queue->curr_size--;
            queue->total_requests--;

            // if we reached the end, throw 'rear' away
            if(queue->curr_size == 0)
                queue->rear = NULL;
            
            free(temp_node);           // free the first node            
            close(fd_to_remove);       // close connection to oldest request
        }
        else if(!strcmp(queue->algo, "random")){
            // Drop 50% random elements from the queue
            DropRandom(queue);
        }
        else{ // if(!strcmp(queue->algo, "block"))
            // block until can be added
            while (queue->total_requests == queue->max_size) {
                pthread_cond_wait(queue->enqueue_allowed, queue->queue_lock);
            }
        }
    }

    // update the queue pointers before adding the new node
    if(queue->curr_size == 0)
        queue->front = new_node;

    else
        queue->rear->previous = new_node;

    // actually add the new node to the end of the queue
    queue->rear = new_node;
    queue->curr_size++;
    queue->total_requests++;

    // release the lock
    pthread_cond_signal(queue->dequeue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
}



void DropRandom2(Queue queue){
    int num_to_drop = (1 + queue->curr_size) / 2;

    if (num_to_drop > queue->curr_size) {
        num_to_drop = queue->curr_size;
    }
    // if the number is bigger than the queue, delete all nodes in it
    if (num_to_drop > queue->curr_size) {
        num_to_drop = queue->curr_size;
        // aquire lock
        pthread_mutex_lock(queue->queue_lock);
        while (queue->curr_size){
            Node temp_node = queue->front;
            queue->front = queue->front->previous;
            queue->curr_size--;
            free(temp_node);
        }
        queue->rear = NULL;
        pthread_cond_signal(queue->enqueue_allowed);
        pthread_mutex_unlock(queue->queue_lock);
    }

    // find which nodes to drop
    int* indices_to_drop = calloc(queue->curr_size, sizeof(int));;
    for (int i=0; i<num_to_drop; i++){
        int new_num_added = 0;
        // make sure the numbers are actually random
        while (!new_num_added){
            int place = rand() % queue->curr_size;
            if(indices_to_drop[place] == 0) {
                indices_to_drop[place] = 1;
                new_num_added = 1;
            }
        }
    }

    // update bool
    int it = 0;
    Node temp_node = queue->front;
    while (temp_node->previous){
        if (indices_to_drop[it]){ // this means the node was picked to be dropped
            temp_node->to_drop = true;
        }
        temp_node = temp_node->previous;
        it++;
    }

    // create new queue

    Node new_front = NULL, new_rear = NULL;
    temp_node = queue->front;
    int new_size = 0;
    while (temp_node->previous){
        if (temp_node->to_drop){ // we need to delete it from the queue
            Node to_free = temp_node;
            close(to_free->fd);
            temp_node = temp_node->previous;
            free(to_free);
        }
        else{ // add it to new queue
            Node new_node = malloc(sizeof(*new_node));
            new_node->fd = temp_node->fd;
            new_node->arrival_time = temp_node->arrival_time;
            temp_node = temp_node->previous;
            if (!new_front){
                new_front = new_node;
                new_rear  = new_node;
            }
            else{
                new_rear->previous = new_node;
                new_rear = new_rear->previous;
            }

            new_size++;
        }
    }

    // delete the old queue
    temp_node = NULL;
    while (queue->front != NULL)
    {
        temp_node = queue->front;
        queue->front = queue->front->previous;
        free(temp_node);
    }
    queue->front = new_front;
    queue->rear = new_rear;
    queue->curr_size = new_size;
    queue->total_requests = queue->total_requests - num_to_drop;
    free(indices_to_drop);

}



void DropRandom(Queue queue){
    // If we reached here, we can drop AT LEAST 1 waiting task
    // rounded up, maybe wrong
    int num_to_drop = (1 + queue->curr_size) / 2;

    //to_drop += (((queue->curr_size * 3) % 10 != 0)? 1 : 0);     // Round up - if the remainder is not zero, add 1
    //printf("[DEBUG] dropping %d requests: \n", to_drop);

    // NADA TO NOA: WHY CAN'T WE JUST REMOVE ALL OF THEM HERE AND RETURN????
    if (num_to_drop > queue->curr_size) {
        num_to_drop = queue->curr_size;
    }

    int* who_to_drop = calloc(queue->curr_size, sizeof(int));
    if (who_to_drop == NULL) {
        exit(1);
    }

    // The randomization magic is done here 
    for (int i = 0; i < num_to_drop; i++) // We made sure that to_drop is not larger than queue->curr_size
    {
        int new_num_added = 0;

        // Try to randomize a number until succeeded to randomize a unique one
        while (!new_num_added)
        {
            int place = rand() % queue->curr_size;
            if(who_to_drop[place] == 0) {
                who_to_drop[place] = 1;
                new_num_added = 1;
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
    queue->total_requests = queue->total_requests - num_to_drop;
}

int dequeue(Queue queue, struct timeval* arrival_time)
{
    // try to lock queue
    pthread_mutex_lock(queue->queue_lock);

    // if queue is empty, wait
    while(queue->curr_size == 0) {
        pthread_cond_wait(queue->dequeue_allowed, queue->queue_lock);
    }

    Node temp_node = queue->front;
    int fd = temp_node->fd;
    *arrival_time = temp_node->arrival_time;

    // update the front
    queue->front = queue->front->previous;
    queue->curr_size--;

    // if we reached the end, throw 'rear' away
    if(queue->curr_size == 0)
        queue->rear = NULL;
    
    free(temp_node); //free the first node

    // release lock and signal
    pthread_cond_signal(queue->enqueue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
    return fd;
}

void queueUpdateRequest(Queue queue)
{
    pthread_mutex_lock(queue->queue_lock);
    queue->total_requests--;
    pthread_cond_signal(queue->enqueue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
}
