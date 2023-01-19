#include <unistd.h>
#define MAX_DELTA 100000000
/*
    void* smalloc(size_t size) 
        ● Tries to allocate ‘size’ bytes.
        ● Return value:
            i. Success –a pointer to the first allocated byte within the allocated block. 
            ii. Failure – 
                a. If ‘size’ is 0 returns NULL. 
                b. If ‘size’ is more than 10^8, return NULL. 
                c. If sbrk fails, return NULL.
*/
void* smalloc(size_t size)
{
    // validate
    if (size == 0 || size > MAX_DELTA)
        return NULL;
    
    void* ptr = sbrk(size);
    if (ptr == NULL || *(int*)ptr == -1)
        return NULL;

    // success
    return ptr;
}

/*
    Discussion: Before proceeding, try discussing the current implementation with your partner.
    What’s wrong with it? (Many things)
    What’s missing? (Free)
    Are we handling fragmentation? (no handling, but there's no free, so is there actually fragmentation?..)
    What would you do differently? (implement free and then think of all the problems this creates)
*/
