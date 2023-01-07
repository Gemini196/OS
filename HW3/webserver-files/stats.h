#ifndef _STATS_H
#define _STATS_H

#include <stdlib.h>

typedef struct thread_stats_t {
    int id;
    int req_count;
    int static_req_count;
    int dynamic_req_count;
} ThreadStats;

typedef struct request_stats_t {
    ThreadStats thread_stats;
    struct timeval arrival_time;
    struct timeval dispatch_interval;
} *RequestStats;



#endif // _STATS_H