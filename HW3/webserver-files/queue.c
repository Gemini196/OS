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


void enqueue(Queue queue, int fd, struct timeval arrival_time)
{
    Node to_add = (Node)malloc(sizeof(*to_add));
    if(to_add == NULL)
        return;
    to_add->fd = fd;
    to_add->next = NULL;
    to_add->arrival_time = arrival_time;

    // try to lock
    pthread_mutex_lock(queue->queue_lock);

    // Attempted to insert to a full queue!! bad boy
    if (queue->total_request == queue->max_size)
    {
        Node temp_node;
        if(!strcmp(queue->algo, "block"))
            pthread_cond_wait(queue->enqueue_allowed, queue->queue_lock);
        else if(!strcmp(queue->algo, "dt")){ //drop front (end)
            pthread_mutex_unlock(queue->queue_lock);
            Close(fd);
            free(to_add);
            return;
        }
        else if(!strcmp(queue->algo, "dh")){ //drop rear (begin)
            Close(queue->rear->fd);
            if(queue->curr_size == 0) {
                pthread_mutex_unlock(queue->queue_lock);
                free(to_add);
                close(fd);
                return;
            }
            temp_node = queue->rear;
            int fd_to_remove = temp_node->fd;

            // move the front forward
            queue->rear = queue->rear->next;
            queue->curr_size--;
            queue->total_request = queue->total_request - 1;

            // if we reached the end, throw 'back' away
            if(queue->curr_size == 0)
                queue->front = NULL;
            free(temp_node);           // free the first node
            close(fd_to_remove);       // close connection to oldest request
        }
        else if(!strcmp(queue->algo, "random")){//drop random half
            // TODO implement this
        }
    }

    if(queue->curr_size == 0) //if it is the first node
    {
        // make both front and rear points to the new node
        queue->rear = to_add;
        queue->front  = to_add;
        queue->curr_size = 1;       // we have 1 node in queue
    }
    else // we have some nodes in the queue
    {
        queue->front->next = to_add; // add newnode in back->next
        queue->front = to_add;       // make the new node as the rear node
        queue->curr_size++;         // we have 1 more node in queue
    }

    // Update the total request num
    queue->total_request = queue->total_request + 1;

    //release lock and signal to worker threads
    pthread_cond_signal(queue->dequeue_allowed); // Can be done outside but it's better this way =) [we know stuff]
    pthread_mutex_unlock(queue->queue_lock);
}


int dequeue(Queue queue, struct timeval* arrival_time)
{
    // backup rear node
    Node temp_node = queue->rear;
    int fd = 0;

    // try to lock queue
    pthread_mutex_lock(queue->queue_lock);

    // cannot dequeue empty queue
    while(queue->curr_size == 0)
        pthread_cond_wait(queue->dequeue_allowed, queue->queue_lock);
    
    fd = temp_node->fd;
    *arrival_time = temp_node->arrival_time;

    queue->rear = queue->rear->next;
    queue->curr_size--;

    // reached the end
    if(queue->curr_size == 0)
        queue->rear = NULL;
    
    free(temp_node); //free the rear (huehuehue)

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
