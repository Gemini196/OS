#ifndef THREAD_STATS
#define THREAD_STATS

typedef struct thread_stats_t {
    int id;
    int req_count;
    int static_req_count;
    int dynamic_req_count;
} ThreadStats;

#endif // THREAD_STATS