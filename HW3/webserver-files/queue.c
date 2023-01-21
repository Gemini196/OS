#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "segel.h"
#include <unistd.h>
#include "queue.h"



Queue queueCreate(int max_size, char* algo)
{
    Queue queue = malloc(sizeof(*queue));
    if (queue == NULL)
        return NULL;
    
    // init 
    queue->front = NULL;
    queue->rear = NULL;
    queue->max_size=  max_size;
    queue->curr_size = 0;
    queue->algo = algo;
    queue->total_requests = 0;

    // lock & conds
    queue->dequeue_allowed = malloc(sizeof(pthread_cond_t)); //cond1
    if (queue->dequeue_allowed == NULL){
        free(queue);
        return NULL;
    }
    queue->enqueue_allowed = malloc(sizeof(pthread_cond_t)); //cond2
    if (queue->enqueue_allowed == NULL){
        free(queue->dequeue_allowed);
        free(queue);
        return NULL;
    }
    queue->queue_lock = malloc(sizeof(pthread_mutex_t));   //lock
    if (queue->queue_lock == NULL){
        free(queue->dequeue_allowed);
        free(queue->dequeue_allowed);
        free(queue);
        return NULL;
    }
        
    pthread_cond_init(queue->dequeue_allowed, NULL);
    pthread_cond_init(queue->enqueue_allowed, NULL);
    pthread_mutex_init(queue->queue_lock, NULL);

    return queue;
}

void queueDestroy(Queue queue)
{
    if (queue == NULL) { return; }

    ClearQueue(queue);

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

    Node new_node = (Node)malloc(sizeof(*new_node));
    if(new_node == NULL)
        return;
    new_node->fd = fd;
    new_node->previous = NULL;
    new_node->arrival_time = arrival_time;

    // try to lock
    pthread_mutex_lock(queue->queue_lock);
    
    // Attempted insert to a full queue
    if (queue->total_requests == queue->max_size)
    {
        Node temp_node;
        if(!strcmp(queue->algo, "block"))
        {
            while (queue->total_requests == queue->max_size) {
                pthread_cond_wait(queue->enqueue_allowed, queue->queue_lock);
            }
        }

        else if(!strcmp(queue->algo, "dt") || queue->curr_size == 0){ //drop tail
            pthread_mutex_unlock(queue->queue_lock);
            free(new_node);
            close(fd);
            return;
        }
        
        else if(!strcmp(queue->algo, "random"))
            DropRandom(queue);

        else if(!strcmp(queue->algo, "dh")){ //drop front (begin)
            temp_node = queue->front;
            int fd_to_remove = temp_node->fd;

            // move the front forward
            queue->front = queue->front->previous;
            queue->curr_size--;
            queue->total_requests = queue->total_requests - 1;

            if(queue->curr_size == 0)
                queue->rear = NULL;
            free(temp_node);          
            close(fd_to_remove);
        }
    }

    if(queue->curr_size == 0) //if first node
        queue->front = new_node;
    else // if we have some nodes in the queue
         queue->rear->previous = new_node;

    queue->rear = new_node;   
    queue->curr_size++;
    queue->total_requests++;

    //release lock and signal to worker threads
    pthread_cond_signal(queue->dequeue_allowed); // Can be done outside but it's better this way =) [we know stuff]
    pthread_mutex_unlock(queue->queue_lock);
}

// Given a reference (pointer to pointer) to
// the head of a list and a position, deletes
// the node at the given position
/*void deleteNode(Queue queue, Node to_remove){
 
    // If linked list is empty
    if (queue->front == NULL)
        return;
 
    // Store head node
    Node temp = queue->front;
 
    // If head needs to be removed
    if (queue->front->to_drop) {
 
        // Change head
        queue->front = temp->previous;
 
        // Free old head
        free(temp);
        return;
    }
 
    // Find next node of the node to be deleted
    while (temp->previous != to_remove){
        temp = temp->previous;
    }
    
    // Node temp->previous is the node to be deleted
    // Store pointer to the previous of node to be deleted
    Node next = temp->previous->previous;
 
    // Unlink the node from linked list
    free(temp->previous); // Free memory
 
    // Unlink the deleted node from list
    temp->previous = next;
}


void RemoveIndexesFromQueue(Queue queue, int* indexes_to_drop, int indexes_to_drop_count){
    Queue old_queue = queue;

    // Create new list that will be patched into the queue
    Node iterator = old_queue->front;
    int index = 0;

    // mark nodes to delete
    while (indexes_to_drop_count != 0)
    {
        iterator->to_drop = false;
        if (indexes_to_drop[index])
            iterator->to_drop = true;
        iterator = iterator -> previous;
        indexes_to_drop_count--;
    }

    iterator = old_queue->front;
    
    while (iterator != NULL)
    {
        if (iterator->to_drop)
            deleteNode(queue, iterator);
        iterator = iterator->previous;
    }

    iterator = old_queue->front;
    while (iterator->previous != NULL)
    {
        iterator = iterator->previous;
    }
    old_queue->rear = iterator;
}*/

void RemoveIndexesFromQueue(Queue* queue, int* indexes_to_drop, int indexes_to_drop_count)
{
    Queue old_queue = *queue;

    // Create new list that will be patched into the queue
    Node new_front = NULL, new_rear = NULL;
    Node iterator = old_queue->front;
    int new_size = 0;

    for (int i = 0; i < old_queue->curr_size; i++)
    {
        if(indexes_to_drop[i] == 1) {
            close(iterator->fd);
            iterator = iterator->previous;
            continue;
        }
        
        Node new_node = malloc(sizeof(*new_node));
        if(new_node == NULL)
            exit(1);
        new_node->fd        = iterator->fd;
        new_node->arrival_time = iterator->arrival_time;
        iterator = iterator->previous;

        if (new_front == NULL) {
            new_front = new_node;
            new_rear  = new_node;
            new_size++;
            continue; 
        }
        
        new_rear->previous = new_node;
        new_rear = new_rear->previous;
        new_size++;
    }

    // Clear old queue
    ClearQueue(old_queue);
    
    old_queue->front = new_front;
    old_queue->rear = new_rear;
    old_queue->curr_size = new_size;
    old_queue->total_requests -= indexes_to_drop_count;
}

void DropRandom(Queue queue)
{
    // If we reached here, we can drop AT LEAST 1 waiting task
    int old_size = queue-> curr_size;

    // Calc how many items to drop
    int to_drop = old_size / 2;
    to_drop += ((old_size * 5 % 10 != 0)? 1 : 0);

    // case: drop all
    if (to_drop > old_size)
        to_drop = old_size;

    int* indexes_to_drop = calloc(old_size, sizeof(int));
    if (indexes_to_drop == NULL)
        exit(1);

    // (We've made sure that to_drop is no larger than current queue size!!)
    for (int i = 0; i < to_drop; i++)
    {
        int new_num_added = 0;
        while (!new_num_added)        // make sure you've generated unique number each iteration!
        {
            int index = rand() % old_size;
            if(indexes_to_drop[index] == 0) {
                indexes_to_drop[index] = 1;
                new_num_added = 1;
            }
        }
    }

    RemoveIndexesFromQueue(&queue, indexes_to_drop, to_drop);
    free(indexes_to_drop);

    /*
    Node new_rear = queue->front;
    while (new_rear->previous != NULL)
    {
        new_rear = new_rear->previous;
    }

    queue->rear = new_rear;
    queue->curr_size -= to_drop;
    queue->total_requests -= to_drop;
    */
}



int dequeue(Queue queue, struct timeval* arrival_time)
{
    // try to lock
    pthread_mutex_lock(queue->queue_lock);

    while(queue->curr_size == 0) {
        pthread_cond_wait(queue->dequeue_allowed, queue->queue_lock);
    }

    Node temp_node = queue->front;
    int fd = temp_node->fd;
    *arrival_time = temp_node->arrival_time;

    // move the front forward
    queue->front = queue->front->previous;
    queue->curr_size--;

    if(queue->curr_size == 0)
        queue->rear = NULL;
    
    free(temp_node);

    pthread_cond_signal(queue->enqueue_allowed);
    pthread_mutex_unlock(queue->queue_lock);

    return fd;
}

void ClearQueue(Queue queue)
{
    Node temp_node = NULL;

    while (queue->front != NULL){
        temp_node = queue->front;
        queue->front = queue->front->previous;
        free(temp_node);
    }
}

void queueUpdateRequest(Queue queue)
{
    pthread_mutex_lock(queue->queue_lock);
    queue->total_requests--;
    pthread_cond_signal(queue->enqueue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
}