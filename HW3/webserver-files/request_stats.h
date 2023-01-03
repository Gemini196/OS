#ifndef REQUEST_STATS
#define REQUEST_STATS

#include "thread_stats.h"
#include <stdlib.h>


typedef struct req_stats_t {
    ThreadStats thread_stats;
    struct timeval arrival_time;
    struct timeval dispatch_interval;
} *ReqStats;


#endif // REQUEST_STATS