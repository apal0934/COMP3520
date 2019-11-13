/* MAB include header file for RR dispatcher */

#ifndef RR2_MAB
#define RR2_MAB

/* Include files */
#include <stdlib.h>
#include <stdio.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

enum memAllocAlg { FIRST_FIT, NEXT_FIT, BEST_FIT, WORST_FIT};

extern enum memAllocAlg MabAlgorithm;

/* memory management *******************************/

#define MEMORY_SIZE       512

struct mab {
    int id;
    int offset;
    int size;
    int allocated;
    struct mab * next;
    struct mab * prev;
};

typedef struct mab Mab;
typedef Mab * MabPtr; 

/* memory management function prototypes ********/

MabPtr memCreate(int);
MabPtr memChk(int);
int    memChkMax(int);  
MabPtr memAlloc(int);
MabPtr memFree(MabPtr);
MabPtr memMerge(MabPtr);   
MabPtr memSplit(MabPtr, int);
int getPossibleMem(MabPtr);
int isFirstBlock(MabPtr);
void   memPrint(MabPtr);
#endif
