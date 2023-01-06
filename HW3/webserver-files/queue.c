#include "queue.h"

typedef struct node_t {
    int fd;
    struct timeval arrival_time;
    struct node_t* next;
} *Node;


struct queue_t {
    Node rear;
    Node front;
    int max_size;
    int curr_size;
    char* algo;
    int total_request;

    pthread_cond_t* dequeue_allowed;
    pthread_cond_t* enqueue_allowed;
    pthread_mutex_t* queue_lock;
};


Queue queueCreate(int max_size, char* algo){
    if(max_size < 0 || algo == NULL)
        return NULL;

    Queue queue = malloc(sizeof(*queue));
    if (queue == NULL)
        return NULL;
    

    queue->rear = NULL;
    queue->front = NULL;
    queue->max_size = max_size;
    queue->curr_size = 0;
    queue->algo = algo;
    queue->total_request = 0;

    // Complex initialization
    queue->dequeue_allowed = malloc(sizeof(pthread_cond_t));   // need to alloc the condition var
    queue->enqueue_allowed = malloc(sizeof(pthread_cond_t));   // need to alloc the condition var
    queue->queue_lock = malloc(sizeof(pthread_mutex_t));       // need to alloc mutex lock;

    if(queue->dequeue_allowed == NULL || queue->enqueue_allowed == NULL || queue->queue_lock == NULL) // memory leak, but i just want to make sure no segfaults
        return NULL;
    pthread_cond_init(queue->dequeue_allowed, NULL);
    pthread_cond_init(queue->enqueue_allowed, NULL);
    pthread_mutex_init(queue->queue_lock, NULL);

    return queue;
}


void enqueue(Queue queue, int fd)
{
    struct timeval arrival_time;
    gettimeofday(&arrival_time, NULL);

    Node to_add = (Node)malloc(sizeof(*to_add));
    if(to_add == NULL)
        return;
    to_add->fd = fd;
    to_add->next = NULL;
    to_add->arrival_time = arrival_time;

    // try to lock
    pthread_mutex_lock(queue->queue_lock);

    if(queue->curr_size == 0) //if it is the first node
    {
        queue->front = to_add;
        queue->rear  = to_add;
        queue->curr_size = 1;       // we have 1 node in queue
    }

    // Attempted to insert to a full queue!
    else if (queue->total_request == queue->max_size)
    {
        Node temp_node;
        if(!strcmp(queue->algo, "block"))
            pthread_cond_wait(queue->enqueue_allowed, queue->queue_lock);

        else if(!strcmp(queue->algo, "dt")){ //drop tail
            pthread_mutex_unlock(queue->queue_lock);
            Close(fd);
            free(to_add);
            return;
        }

        else if(!strcmp(queue->algo, "dh")){ //drop front (begin)
            
            temp_node = queue->rear;
            int fd_to_remove = queue->front->fd;

            // remove front
            while (temp_node->next != queue->front)
            {
                temp_node = temp_node->next;
            }

            free(temp_node->next);
            
            queue->front = temp_node;
            queue->front->next = NULL;

            // insert new node into rear
            to_add->next = queue->rear;
            queue->rear = to_add;

            Close(fd_to_remove);       // close connection to oldest request
        }

        else if(!strcmp(queue->algo, "random")){//drop random half
            // TODO implement this
        }
    }


    else // ADD NODE TO NOT EMPTY QUEUE
    {
        to_add->next = queue->rear;
        queue->rear = to_add;
        queue->curr_size++; 
    }

    // Update the total request num
    queue->total_request = queue->total_request + 1;

    //release lock and signal to worker threads
    pthread_cond_signal(queue->dequeue_allowed); // Can be done outside but it's better this way =) [we know stuff]
    pthread_mutex_unlock(queue->queue_lock);
}



int dequeue(Queue queue, struct timeval* arrival_time)
{
    // try to lock queue
    pthread_mutex_lock(queue->queue_lock);

    // cannot dequeue empty queue
    while(queue->curr_size == 0)
        pthread_cond_wait(queue->dequeue_allowed, queue->queue_lock);

    int fd = queue->front->fd;
    *arrival_time = queue->front->arrival_time;

    // case: single node queue freed
    if(queue->curr_size == 1)
    {
        free(queue->front);
        queue->rear = NULL;
        queue->front = NULL;
    }

    else
    {
        Node temp_node = queue->rear;

        // remove front
        while (temp_node->next != queue->front)
        {
            temp_node = temp_node->next;
        }

        free(temp_node->next);

        queue->front = temp_node;
        queue->front->next = NULL;
    }

    queue->curr_size--;
    //release lock and signal
    pthread_cond_signal(queue->enqueue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
    return fd;
}

void queueDestroy(Queue queue)
{
    if (queue == NULL)
        return;
    
    Node temp_node = NULL;
    // Scan nodes and free them
    while (queue->rear != NULL)
    {
        temp_node = queue->rear;
        queue->rear = queue->rear->next;
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

void queueUpdateRequest(Queue queue)
{
    pthread_mutex_lock(queue->queue_lock);
    queue->total_request--;
    pthread_cond_signal(queue->enqueue_allowed);
    pthread_mutex_unlock(queue->queue_lock);
}
