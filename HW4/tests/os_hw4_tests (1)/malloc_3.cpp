#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <random>
#include <stdbool.h>
#define MAX_DELTA 100000000
#define MIN_BLOCK_SIZE 128
#define MMAP_THRESHOLD 131072

//------------------------------------- STRUCT -----------------------------------------------
struct MallocMetadata {
    size_t cookies;
    size_t size;
    bool is_free;
    bool is_mapped;
    MallocMetadata* next;
    MallocMetadata* prev;
    MallocMetadata* next_free;
    MallocMetadata* prev_free;
};

//-------------------------------- Global variables ------------------------------------------
void* first_block = sbrk(0); // let's assume this works
void* last_block = NULL; 

void* first_free_block = NULL;
void* last_free_block = NULL;

size_t free_blocks = 0;
size_t free_bytes = 0;
size_t allocated_blocks = 0;
size_t allocated_bytes = 0;
size_t metadata_bytes = 0;
size_t global_cookies = rand();
//-----------------------------------Declarations---------------------------------------------
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

//void updateListStats(UpdateStatus status, MallocMetadata* meta);
void listRemoveSpecific(MallocMetadata* to_remove);
void listRemoveSpecificFree(MallocMetadata* to_remove);
void* splitBlock(void* p, size_t new_size);
//void mergeBlocks(MallocMetadata* meta);
void* mergeBlocks(void* p);
void* mergeWithPrevious(void* p);
void* mergeWithNext(void* p);
void freeListInsert(MallocMetadata *to_add);
MallocMetadata* freeListRemove(size_t size);

void* sreallocCaseB(MallocMetadata* meta, size_t size_to_copy, void* oldp, int size);
void* sreallocCaseC(MallocMetadata* meta, size_t diff);
void* sreallocCaseD(MallocMetadata* meta, void* oldp, int size);
void* sreallocCaseE(MallocMetadata* meta, size_t size_to_copy, void* oldp, int size);

void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);

bool isOverflow(MallocMetadata* meta);

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


bool isOverflow(MallocMetadata* meta)
{
    if (meta == NULL)
        return false;
    
    return meta->cookies != global_cookies;
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

    void* ptr;  

    if(size >= MMAP_THRESHOLD){
        ptr = mmap(NULL, size + _size_meta_data(), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if(!ptr)
            return NULL;

        MallocMetadata* meta = (MallocMetadata*) ptr;
        meta->cookies = global_cookies;
        meta->is_free = false;
        meta->is_mapped = true;
        meta->next = NULL;
        meta->prev = NULL;
        meta->next_free = NULL;
        meta->prev_free = NULL;
    }

    else{
        ptr = freeListRemove(size); // attempt to remove a free block of right size
    }

    if (ptr != NULL)
    {
        MallocMetadata* cast_ptr = (MallocMetadata*)ptr;
        cast_ptr->is_free = false;
        allocated_blocks++;
        free_blocks--;
        allocated_bytes += cast_ptr->size;
        free_bytes -= cast_ptr->size;
        return cast_ptr + _size_meta_data();
    }

    // No free blocks -> sbrk
    ptr = sbrk(size + _size_meta_data());
    if(ptr == (void*)-1)
        return NULL;

    // new metadata block
    MallocMetadata* mdata = (MallocMetadata*)ptr;
    mdata->cookies = global_cookies;
    mdata->size = size;
    mdata->is_free = false;
    mdata->is_mapped = false;
    mdata->next = (MallocMetadata*)last_block;
    mdata->prev = NULL;
    
    // if list not empty
    if (last_block != NULL)
        ((MallocMetadata*)last_block)->next = mdata;

    // update last block
    last_block = (void*) &mdata;

    allocated_blocks++;
    allocated_bytes += size;
    metadata_bytes += _size_meta_data();

    // Arithmetic chaos
    unsigned long long tmp = (unsigned long long)last_block + _size_meta_data();

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

    if (isOverflow(metadata))
        exit(0xdeadbeef);

    if(metadata->is_free)
        return;

    if(metadata->is_mapped){
        p = (void*)((char*)p - _size_meta_data());
        munmap(p, metadata->size + _size_meta_data());
        allocated_bytes -= metadata->size + _size_meta_data();
        allocated_blocks--;
    }
    else{
        metadata->is_free = true;
        mergeBlocks((void*)metadata);
        freeListInsert(metadata);
        free_blocks++;
        free_bytes += metadata->size;
    }
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
    if (isOverflow(old_metadata))
        exit(0xdeadbeef);

    size_t size_to_copy = (old_metadata->size < size) ? old_metadata->size : size; // how many bytes to copy (take minimum)

    // block was allocated using mmap
    if(old_metadata->is_mapped){
        if(old_metadata->size == size)
            return oldp; 

        void* newp = smalloc(size);             // malloc does the rest
        std::memmove(newp, oldp, size_to_copy);
        sfree(oldp);
        return newp;
    }

    // If we're here: block wasn't allocated using mmap

    // Case A: If request fits in current block
    if(size <= old_metadata->size){
        // Check if can split (if so: SPLIT)
        size_t diff = old_metadata->size - size;
        if (MIN_BLOCK_SIZE+sizeof(MallocMetadata) <= diff) {
//            free_blocks++;
//            free_bytes += diff - _size_meta_data();
            splitBlock(old_metadata, size);
        }
        return oldp;
    }

    // CaseB: If the previous is freed AND merging with it SHALL SATISFY ME
    if(old_metadata->prev && old_metadata->prev->is_free)
    {
        // merge with previous block is good enough
        size_t merged_block_size = old_metadata->prev->size + old_metadata->size + sizeof(MallocMetadata);
        if (merged_block_size >= size) {
            return sreallocCaseB(old_metadata, size_to_copy, oldp, size);
        }

        // merge with previous block but we need to enlarge program break
        else if(last_block == old_metadata) { // 
            int delta = size - merged_block_size;
            sbrk(delta);
            old_metadata->size += delta;
            allocated_bytes += delta;
            return sreallocCaseB(old_metadata, size_to_copy, oldp, size);
        }
    }

    // CASE C :: If the block is the wilderness chunk, enlarge it.
    if((MallocMetadata*)last_block == old_metadata) {
        size_t diff = size - old_metadata->size;
        return sreallocCaseC(old_metadata, diff);
    }

    // Try to merge with the adjacent block with the higher address (Case D) OR
    // both adjacent blocks with higher and lower addresses (Case E)
    if(old_metadata->next && old_metadata->next->is_free)
    {
        size_t merged_with_next = old_metadata->next->size + old_metadata->size + _size_meta_data();
        // merge with next block is good enough (case D)
        if( merged_with_next >= size) {
            return sreallocCaseD(old_metadata, oldp, size);
        }
        //CASE E :: merge with next block is NOT enough - if previous is free and has enough space take it as well
        else if (old_metadata->prev && old_metadata->prev->is_free &&
                old_metadata->prev->size + merged_with_next + _size_meta_data() >= size)
        {
            return sreallocCaseE(old_metadata, size_to_copy, oldp, size);
        }
        // CASE F: merge + enlarge wilderness
        // else if old_metadata->next is the wilderness (last) block and it's free
        else if ((void*)old_metadata->next == last_block){
            size_t diff;
            // case1: I can merge both ways, and enlarge wilderness
            if(old_metadata->prev && old_metadata->prev->is_free){
                // call CaseC to enlarge wilderness
                diff = size - old_metadata->size - 2*_size_meta_data() - old_metadata->next->size - old_metadata->prev->size;
                sreallocCaseC(old_metadata->next, diff);
                // actually merge
                return sreallocCaseE(old_metadata, size_to_copy, oldp, size);
            }
            // case2: merge only with next and enlarge
            else{
                // call CaseC to enlarge wilderness
                diff = size - old_metadata->size - _size_meta_data() - old_metadata->next->size;
                sreallocCaseC(old_metadata->next, diff);
                // actually merge
                return sreallocCaseD(old_metadata, oldp, size);
            }
        }

        return NULL;
        // not supposed to arrive here
    }

    else{
        void* newp = smalloc(size);
        if (newp == NULL)
            return NULL;
        std::memmove(newp, oldp, old_metadata->size);
        sfree(oldp);
        return newp;
    }
    // not supposed to arrive here
    return NULL;
}


//----------------------------------------------------------------

void listRemoveSpecificFree(MallocMetadata* to_remove) 
{
    if (to_remove->prev_free) {                            // prev <--> to_remove <--> next
        to_remove->prev_free->next_free = to_remove->next_free;  // prev --> next
    }
    else { // to_remove is list_mem_head
        first_free_block = to_remove->next_free;
    }

    if (to_remove->next_free) {                                // prev <--> to_remove <--> next
        to_remove->next_free->prev_free = to_remove->prev_free;  // prev <-- next
    }
    else {
        last_free_block = to_remove->prev_free;
    } 
}

void listRemoveSpecific(MallocMetadata* to_remove){
    MallocMetadata* temp = (MallocMetadata*)first_block;
    
    //edge case: to_remove is the only one
    if(temp == (MallocMetadata*)first_block && temp == (MallocMetadata*)last_block){
        first_block = NULL;
        last_block = NULL;
    }
    // deal with previous in both lists
    if(to_remove->prev){
        to_remove->prev->next = to_remove->next;
    }
    else{
        first_block = to_remove->next;
        if(to_remove->is_free){
            first_free_block = to_remove->next_free;
        }
    }
    // deal with next in both lists
    if(to_remove->next){
        to_remove->next->prev = to_remove->prev;
    }
    else{
        last_block = to_remove->prev;
        if(to_remove->is_free){
            last_free_block = to_remove->prev_free;
        }
    }
}

void freeListInsert(MallocMetadata *to_add) 
{    
    // If list is empty
    if (_num_free_bytes()==0){
        first_free_block = to_add;
        last_free_block = to_add;
        return;
    }

    // Case 1: Add to list head
    if(to_add->size < ((MallocMetadata *)first_free_block)->size || 
        (to_add->size ==  ((MallocMetadata *)first_free_block)->size && to_add < first_free_block)) 
        {
            to_add->next_free = ((MallocMetadata *)first_free_block);
            to_add->prev_free = NULL;
            ((MallocMetadata *)first_free_block)->prev_free = to_add;
            first_free_block = to_add;
            return;
        }
    
    // Otherwise: find location to insert
    MallocMetadata *temp = ((MallocMetadata *)first_free_block);
    while (temp && temp->next_free && to_add->size > temp->next_free->size) {
        temp = temp->next_free;
    }

    // temp = the first block with size smaller than to_add
    // we'll find the block where we can insert to_add by address size
    while(temp && temp->next_free && temp->next_free->size == to_add->size &&
          temp->next_free < to_add) // address comparison
    {                                       
        temp = temp->next_free;
    }

    // Insert to_add as temp's next
    to_add->next_free = temp->next_free;
    to_add->prev_free = temp;
    if (temp->next_free)
        temp->next_free->prev_free = to_add;
    temp->next_free = to_add;


    // If to_add was added last
    if (temp == first_free_block)
        last_free_block = to_add;
}


// Returns a pointer to a FREE block of size <size>
MallocMetadata* freeListRemove(size_t size)
{
    // assume size > 0 && size < Max
    MallocMetadata* to_remove = ((MallocMetadata *)first_free_block);
    while (to_remove && to_remove->size < size)
        to_remove = to_remove->next;

    // no blocks of right size
    if (to_remove == NULL){
        // challenge 3
        to_remove = (MallocMetadata *)last_block;
        if(!to_remove->is_free)
            return NULL;

        // enlarge wilderness (last block)
        void* p = sbrk(size - to_remove->size);
        if (p == NULL)
            return NULL;
        to_remove->size = size;
    }

    listRemoveSpecificFree(to_remove);
    
    to_remove->next_free = NULL;
    to_remove->prev_free = NULL;
    
    //Challenge 1
    if (to_remove->size - size >= MIN_BLOCK_SIZE + sizeof(MallocMetadata)) {
        MallocMetadata* new_block = (MallocMetadata*)splitBlock(to_remove, size);
        freeListInsert(new_block);
    }
    to_remove->is_mapped = false;
    return to_remove;
}


void* splitBlock(void* p, size_t new_size){
    struct MallocMetadata* old_p = (MallocMetadata*)p;
    size_t old_size = old_p->size;

    // find block2 start address
    unsigned long long block2 = (unsigned long long)p;
    block2 += new_size + _size_meta_data();

    struct MallocMetadata* block2_ptr = (MallocMetadata*)block2;

    struct MallocMetadata* old_next = old_p->next;

    old_p->size = new_size;
    old_p->next = block2_ptr;

    block2_ptr->prev = old_p;
    block2_ptr->next = old_next;

    // check if old block was the last block
    if(block2_ptr->next){
        (block2_ptr->next)->prev = block2_ptr;
    }
    else{
        last_block = block2_ptr;
        last_free_block = block2_ptr;
    }
    // update rest of metadata
    block2_ptr->size = old_size - new_size - _size_meta_data();
    block2_ptr->is_free = true;
    block2_ptr->is_mapped = false;

    metadata_bytes += _size_meta_data();
    free_blocks++;
    allocated_blocks++;
    free_bytes -= _size_meta_data();
    free_bytes += old_size - new_size;

    // In case we can merge
    void* fin_block = mergeBlocks(block2_ptr);
    if (fin_block)
        return fin_block;

    return (void*)block2_ptr;
}

//void mergeBlocks(MallocMetadata* meta);


void* mergeBlocks(void* p){
    void* new_p1 = mergeWithNext(p);
    void* new_p2 = mergeWithPrevious(p);
    if (!new_p1 && !new_p2)
        return NULL;
    else if(new_p2)
        return new_p2;
    return p;
}

void* mergeWithPrevious(void* p){
    MallocMetadata* curr_block = (MallocMetadata*)p;
    MallocMetadata* prev_block = curr_block->prev;

    if (isOverflow(curr_block) || isOverflow(prev_block))
        exit(0xdeadbeef);

    if(!prev_block || !prev_block->is_free) //previous block doesn't exist or isn't free
        return NULL;


    // update metadata of prev
    prev_block->size += curr_block->size + _size_meta_data();
    // the rest happens here
    listRemoveSpecific(curr_block);

    // update stats
    free_blocks--;
    allocated_blocks--;
    allocated_bytes += _size_meta_data();
    free_bytes += _size_meta_data();
    metadata_bytes -= _size_meta_data();
    return (void*)prev_block;
}

void* mergeWithNext(void* p){
    MallocMetadata* curr_block = (MallocMetadata*)p;
    MallocMetadata* next_block = curr_block->next;

    if (isOverflow(curr_block) || isOverflow(next_block))
        exit(0xdeadbeef);

    if(!next_block || !next_block->is_free) //previous block doesn't exist or isn't free
        return NULL;

    // update metadata of curr
    curr_block->size += next_block->size + _size_meta_data();
    // rest of updates happen here
    listRemoveSpecific(next_block);

    // update stats
    free_blocks--;
    allocated_blocks--;
    allocated_bytes += _size_meta_data();
    free_bytes += _size_meta_data();
    metadata_bytes -= _size_meta_data();

    return (void*)curr_block;
}


// Case: reallocation
void* sreallocCaseB(MallocMetadata* meta, size_t size_to_copy, void* oldp, int size)
{
//    MallocMetadata* new_meta = meta->prev;
//    void* new_ptr = (void*)((char*)new_meta + sizeof(MallocMetadata));
//
//    // Update stats
//    free_blocks--;
//    free_bytes -= meta->prev->size;
//    allocated_bytes += _size_meta_data();
//
//    // update previous block's meta (size += <>, pointers etc)
//    meta->prev->size += meta->size + _size_meta_data();
//    meta->prev->is_free = false;
//    listRemoveSpecific(meta);                            // Removes from the memory list
//    listRemoveSpecificFree(meta->prev);                  // Removes previous block from free list

    MallocMetadata* new_meta = meta->prev;
    void* new_ptr = mergeWithPrevious(oldp);

    // copy content of current block to previous block
    std::memmove(new_ptr, oldp, size_to_copy);
    size_t diff = new_meta->size - size;

    // check if can split
    if (MIN_BLOCK_SIZE + _size_meta_data() <= diff) {
//        free_blocks++;
//        free_bytes += diff - _size_meta_data();
        splitBlock(new_meta, size);
    }
    return new_ptr;
}

// CaseC: enlarge wilderness
void* sreallocCaseC(MallocMetadata* meta, size_t diff)
{
    // sbrk
    if (sbrk(diff) == NULL) {
        return NULL;
    }
    
    // Update stats
    allocated_bytes += diff;
    meta->size = meta->size + diff;
    return (void*)((char*)meta + _size_meta_data());
}


// CaseD: merge with next
void* sreallocCaseD(MallocMetadata* meta, void* oldp, int size)
{
//    // Update stats
//    free_blocks--;
//    free_bytes -= meta->next->size;
//    allocated_bytes += _size_meta_data();
//
//    // update previous block's meta (size += <>, pointers etc)
//    meta->size += meta->next->size + _size_meta_data();
//    listRemoveSpecific(meta->next);                               // Removes next block from free list
//    listRemoveSpecificFree(meta->next_free);                      // Removes next from the memory list
    mergeWithNext(oldp);
    // void* new_ptr = (void*)((char*)meta + _size_meta_data());
    size_t diff = meta->size - size;
    if (MIN_BLOCK_SIZE + _size_meta_data() <= diff) {
//        free_blocks++;
//        free_bytes += diff -_size_meta_data();
        splitBlock(meta, size);
    }
    // pointer doesn't change when block is merged with next
    return oldp;
}


void* sreallocCaseE(MallocMetadata* meta, size_t size_to_copy, void* oldp, int size)
{
    oldp = sreallocCaseD(meta, oldp, meta->size + meta->next->size);               // Merge with the block AFTER meta
    return sreallocCaseB(meta, size_to_copy, oldp, size);                              // Merge with the block BEFORE meta
}




