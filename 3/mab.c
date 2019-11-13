/* MAB management functions for RR dispatcher */

/* Include Files */
#include "mab.h"

enum memAllocAlg MabAlgorithm = FIRST_FIT;

static MabPtr next_mab = NULL;    // for NEXT_FIT algorithm

/*******************************************************
 * MabPtr memCreate (int size);
 *    - create the simulated memory address space
 * 
 * returns address of MAB or NULL
 *******************************************************/
MabPtr memCreate(int size)
{
    // if larger than possbile, fail
    if (!memChkMax(size)) {
        return NULL;
    }

    // Allocate memory, set values to initial values
    MabPtr m = malloc(sizeof(Mab));
    m->size = size;
    m->allocated = 0;
    m->offset = 0;
    // cyclic list
    m->prev = m;
    m->next = m;
    next_mab = m;

    return m;
}

/*******************************************************
 * MabPtr memChk (MabPtr arena, int size);
 *    - check for memory available (any algorithm)
 *
 * returns address of "First Fit" block or NULL
 *******************************************************/
MabPtr memChk(int size)
{
    // cycle around the list until back to the start
    // if a block has enough size, return it
    // if back to the start, none was found, fail
    MabPtr curr = next_mab;
    do  {
        if (size <= curr->size && !curr->allocated) {
            return curr;
        }
        curr = curr->next;
    } while (curr != next_mab);
    return NULL;
}
      
/*******************************************************
 * int memChkMax (int size);
 *    - check for memory available (any algorithm)
 *
 * returns TRUE/FALSE
 *******************************************************/
int memChkMax(int size)
{
    return size <= MEMORY_SIZE;
}      

/*******************************************************
 * MabPtr memAlloc (MabPtr arena, int size);
 *    - allocate a memory block
 *
 * returns address of block or NULL if failure
 *******************************************************/
MabPtr memAlloc(int size)
{
    MabPtr freeBlock;
    // if we couldn't find memory, fail
    if (!(freeBlock = memChk(size))) {
        return NULL;
    }

    // if the block is the same size, we don't need to split 
    if (freeBlock->size == size) {
        freeBlock->allocated = 1;
        next_mab = freeBlock->next;
        return freeBlock;
    } else {
        // need to split, check if split worked
        MabPtr split = memSplit(freeBlock, size);
        if (!split) {
            return NULL;
        }
        split->allocated = 1;
        next_mab = split->next;
        return split;
    }
}

/*******************************************************
 * MabPtr memFree (MabPtr mab);
 *    - de-allocate a memory block
 *
 * returns address of block or merged block
 *******************************************************/
MabPtr memFree(MabPtr m)
{
    MabPtr left = m;
    m->allocated = 0;
    // traverse to the left most block that is free
    while (!left->prev->allocated && !isFirstBlock(left)) {
        left = left->prev;
    }

    // traverse right, merging all free blocks as we go until no more free blocks
    while (!left->next->allocated && !isFirstBlock(left->next)) {
        if (next_mab == left->next) {
            next_mab = left->next->next;
        }
        memMerge(left);
    }

    return left;
}
      
/*******************************************************
 * MabPtr memMerge(Mabptr m);
 *    - merge m with m->next
 *
 * returns m
 *******************************************************/
MabPtr memMerge(MabPtr m)
{
    // following the procedure given in the specs
    // if it's the first block, no need to merge
    if (m->next == m || m->prev == m) {
        return m;
    }
    
    MabPtr next = m->next;
    m->size += next->size;
    m->next = next->next;

    m->next->prev = m;

    free(next);
    return m;
}

/*******************************************************
 * MabPtr memSplit(MabPtr m, int size);
 *    - split m into two with first mab having size
 *
 * returns m or NULL if unable to supply size bytes
 *******************************************************/
MabPtr memSplit(MabPtr old, int size)
{
    // following the procedure given in the specs
    if (size >= old->size) {
        return NULL;
    }

    MabPtr new = malloc(sizeof(Mab));
    new->size = old->size - size;
    new->offset = old->offset + size;
    new->allocated = 0;
    new->next = old->next;
    new->prev = old;
    new->next->prev = new;

    old->size = size;
    old->next = new;

    return old; 
}

/**************************************************************
 * int getPossibleMem(MabPtr m);
 *    - calculate how much memory would be freed on a memFree()
 * 
 * returns int of total size of all blocks that would be merged
***************************************************************/
int getPossibleMem(MabPtr m) {
    MabPtr left = m;
    int allocated = m->allocated;
    m->allocated = 0;

    // traverse to the leftmost allocated block
    while (!left->prev->allocated && !isFirstBlock(left)) {
        left = left->prev;
    }

    int possibleMem = 0;
    // traverse right until first allocated block
    // sum the size of the blocks encountered
    while (!left->next->allocated) {
        possibleMem += left->size;
        left = left->next;
        // no need to go beyond the end
        if (isFirstBlock(left->next)) {
            possibleMem += left->size;
            break;
        }
    }

    m->allocated = allocated;
    return possibleMem;
}


/**************************************************************
 * int isFirstBlock(MabPtr m);
 *    - Uses offset value to find out if first block
 * 
 * returns bool
***************************************************************/
int isFirstBlock(MabPtr m) {
    if (m) {
        return m->offset == 0;
    }
    return 0;
}

/*******************************************************
 * void memPrint(MabPtr arena);
 *    - print contents of memory arena
 * no return
 *******************************************************/
void memPrint(MabPtr arena)
{
    MabPtr curr = arena;
    printf("Offset: %d\n", curr->offset);
    printf("Size: %d\n", curr->size);
    printf("Allocation: %d\n", curr->allocated);
    fflush(stdout);
    curr = curr->next;
    for (; !isFirstBlock(curr); curr = curr->next) {
        printf("Offset: %d\n", curr->offset);
        printf("Size: %d\n", curr->size);
        printf("Allocation: %d\n", curr->allocated);
        fflush(stdout);
    }
}
