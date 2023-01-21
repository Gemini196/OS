#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#define MAX_DELTA 100000000


//------------------------------------- STRUCT -----------------------------------------------
struct MallocMetadata {
    size_t size;
    bool is_free; 
    MallocMetadata* next;
    MallocMetadata* prev;
};
//-------------------------------- Global variables ------------------------------------------
void* first_block = sbrk(0); // lets assume this works
void* last_block = NULL; 
size_t free_blocks;
size_t free_bytes;
size_t allocated_blocks;
size_t allocated_bytes;
size_t metadata_bytes;

//---------------------------------- stats methods -------------------------------------------

/* Returns the number of allocated blocks in the heap that are currently free.*/
size_t _num_free_blocks(){
    return free_blocks;
}

/* Returns the number of bytes in all allocated blocks in the heap that are currently free, 
excluding the bytes used by the meta-data structs.*/
size_t _num_free_bytes(){
    return free_bytes;
}

/* Returns the overall (free and used) number of allocated blocks in the heap.*/
size_t _num_allocated_blocks(){
    return allocated_blocks;
}
 
/* Returns the overall number (free and used) of allocated bytes in the heap, excluding 
the bytes used by the meta-data structs. */
size_t _num_allocated_bytes(){
    return allocated_bytes;
}

/* Returns the overall number of meta-data bytes currently in the heap. */
size_t _num_meta_data_bytes(){
    return metadata_bytes;
}


/* Returns the number of bytes of a single meta-data structure in your system.*/
size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}



//-------------------------------------- memory management methods ---------------------------------------


/*1. void* smalloc(size_t size):
    ● Searches for a free block with at least ‘size’ bytes or allocates (sbrk()) one if none are 
    found.
    ● Return value:
        i. Success – returns pointer to the first byte in the allocated block (excluding the meta-data of 
        course) 
        ii. Failure – 
            a. If size is 0 returns NULL.
            b. If ‘size’ is more than 10^8, return NULL. 
            c. If sbrk fails in allocating the needed space, return NULL. 
*/
void* smalloc(size_t size)
{
    // validate
    if (size == 0 || size > MAX_DELTA)
        return NULL;

    size_t msize =  _size_meta_data();

    // Search list for free block
    MallocMetadata* temp = (MallocMetadata*)first_block;
    while(temp && free_blocks > 0)
    {
        if(temp->is_free) {                     // found a place to allocate
            if(temp->size >= size) {
                temp->is_free = false;
                allocated_blocks++;
                free_blocks--;
                allocated_bytes += temp->size;
                free_bytes -= temp->size;
                return temp + msize;
            }
        }
        temp = temp->next;
    }

    // No free blocks -> sbrk
    void* ptr = sbrk(size + msize); 
    if(ptr == (void*)-1)
        return NULL;
    

    // new metadata block
    struct MallocMetadata mdata = {size, false, NULL, (MallocMetadata*)last_block};
    
    // if list not empty
    if (last_block != NULL)
        *((MallocMetadata*)last_block)->next = mdata;

    // update last block
    last_block = (void*) &mdata;

    allocated_blocks++;
    allocated_bytes += size;
    metadata_bytes += msize;

    // Arithmetic chaos
    unsigned long long tmp = (unsigned long long)last_block + msize;

    return (void*)tmp;
}


/*
2. void* scalloc(size_t num, size_t size):
    ● Searches for a free block of at least ‘num’ elements, each ‘size’ bytes that are all set to 0 
    or allocates if none are found. In other words, find/allocate size * num bytes and set all 
    bytes to 0.
    ● Return value:
        i. Success - returns pointer to the first byte in the allocated block. 
        ii. Failure – 
            a. If size or num is 0 returns NULL. 
            b. If ‘size * num’ is more than 108, return NULL. 
            c. If sbrk fails in allocating the needed space, return NULL. 
*/
void* scalloc(size_t num, size_t size)
{
    void* p = smalloc(size*num);
    if(p == NULL)
        return NULL;
    
    std::memset(p, 0, size*num);
    return p;
}



/*
    3. void sfree(void* p):
    ● Releases the usage of the block that starts with the pointer ‘p’. 
    ● If ‘p’ is NULL or already released, simply returns. 
    ● Presume that all pointers ‘p’ truly points to the beginning of an allocated block.
*/
void sfree(void* p)
{
    if (p == NULL)
        return;
    // arithmetic stuff
    struct MallocMetadata* metadata = (MallocMetadata*) ((char*)(p) - _size_meta_data());
    if(metadata->is_free)
        return;
    metadata->is_free = true;
    free_blocks++;
    free_bytes += metadata->size;
}


/*
4. void* srealloc(void* oldp, size_t size):
● If ‘size’ is smaller than or equal to the current block’s size, reuses the same block. 
Otherwise, finds/allocates ‘size’ bytes for a new space, copies content of oldp into the 
new allocated space and frees the oldp.
● Return value: 
    i. Success – 
        a. Returns pointer to the first byte in the (newly) allocated space. 
        b. If ‘oldp’ is NULL, allocates space for ‘size’ bytes and returns a pointer to it. 
    ii. Failure – 
        a. If size is 0 returns NULL. 
        b. If ‘size’ if more than 108, return NULL. 
        c. If sbrk fails in allocating the needed space, return NULL. 
        d. Do not free ‘oldp’ if srealloc() fails. 
*/
void* srealloc(void* oldp, size_t size)
{
    // validate
    if(size == 0 || size > MAX_DELTA)
        return NULL;
    
    if(oldp == NULL)
        return smalloc(size);

    // go back to metadata to find size
    struct MallocMetadata* old_metadata = (MallocMetadata*) ((char*)oldp - _size_meta_data());

    size_t old_size = old_metadata->size;

    // checking whether new size fits in old size, if so - do nothing
    if(size <= old_size)
        return oldp;
    
    // we need to find space and then copy the content onto it
    void* newp = smalloc(size);
    if (newp == NULL)
        return NULL;

    std::memmove(newp, oldp, old_size);
    sfree(oldp);
    return newp;
}
