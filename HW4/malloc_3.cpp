#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#define MAX_DELTA 100000000
#define MIN_BLOCK_SIZE 128

//------------------------------------- STRUCT -----------------------------------------------
struct MallocMetadata {
    size_t size;
    bool is_free; 
    MallocMetadata* next;
    MallocMetadata* prev;
    MallocMetadata* next_free;
    MallocMetadata* prev_free;
};

//-------------------------------- Global variables ------------------------------------------
void* first_block = sbrk(0); // lets assume this works
void* last_block = NULL; 

void* first_free_block = NULL;
void* last_free_block = NULL;

size_t free_blocks;
size_t free_bytes;
size_t allocated_blocks;
size_t allocated_bytes;
size_t metadata_bytes;
//-----------------------------------Declarations---------------------------------------------
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

//void updateListStats(UpdateStatus status, MallocMetadata* meta);
//void listRemoveSpecific(MallocMetadata* to_remove);
void listRemoveSpecificFree(MallocMetadata* to_remove);
void* splitBlock(MallocMetadata* meta, int size);
//void mergeBlock(MallocMetadata* meta);
void freeListInsert(MallocMetadata *to_add);
MallocMetadata* freeListRemove(size_t size);
//void* sreallocCaseB(MallocMetadata* meta, size_t size_to_copy, void* oldp, int size);
//void* sreallocCaseC(MallocMetadata* meta, size_t size);
//void* sreallocCaseD(MallocMetadata* meta, void* oldp, int size);
//void* sreallocCaseE(MallocMetadata* meta, size_t size_to_copy, void* oldp, int size);

void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);

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

    MallocMetadata* ptr = freeListRemove(size); // attempt to remove a free block of right size
    
    if (ptr != NULL)
    {
        ptr->is_free = false;
        allocated_blocks++;
        free_blocks--;
        allocated_bytes += ptr->size;
        free_bytes -= ptr->size;
        return ptr + msize;
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
    //bool splitted = _FALSE;

    while (to_remove && to_remove->size < size)
        to_remove = to_remove->next;
    
    
    // no blocks of right size
    if (to_remove == NULL)
        return NULL;


    listRemoveSpecificFree(to_remove);
    
    to_remove->next = NULL;
    to_remove->prev = NULL;

    
    //Challenge 1
    if (to_remove->size - size >= MIN_BLOCK_SIZE + sizeof(MallocMetadata)) {
        to_remove = splitBlock(to_remove, size);
    }
    // if(splitted) {
    //     updateListStats(REMOVE_SPLIT, temp);
    // }
    // else {
    //     updateListStats(REMOVE, temp);
    // }
    return to_remove;
}


void* splitBlock(void* p, size_t new_size){
    struct MallocMetadata* old_p = (MallocMetadata*)p;
    size_t old_size = old_p->size;
    size_t msize = _size_meta_data();

    // find block2 start address
    unsigned long long block2 = (unsigned long long)p;
    block2 += new_size + msize;

    struct MallocMetadata* block2_ptr = (MallocMetadata*)block2;

    struct MallocMetadata* old_next = old_p->next;
    struct MallocMetadata* old_next_free = old_p->next_free;

    
    old_p->size = new_size;
    old_p->next = block2_ptr;

    block2_ptr->prev = old_p;
    block2_ptr->next = old_next;
    if(block2_ptr->next){
        (block2_ptr->next)->prev = block2_ptr;
    }
    block2_ptr->size = old_size - new_size - _size_meta_data();

    //add new block to histogram
    block2_ptr->is_free = true;
    addToHistogram(block2_ptr);

    num_meta_data_bytes += _size_meta_data();
    num_free_blocks++;
    num_allocated_blocks++;
    num_allocated_bytes -= _size_meta_data();
    num_free_bytes -= _size_meta_data();
    num_free_bytes += old_size - new_size;

    return (void*)block2_ptr;
//    unsigned long long tmp3 = (unsigned long long)new_p;
//    tmp3 += sizeof(MallocMetadata);
//    return (void*)tmp3;
}