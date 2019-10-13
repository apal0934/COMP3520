/* PCB include header file for Very Simple Experimental Dispatcher (VSED) */

#ifndef VSED_PCB
#define VSED_PCB

/* Include files */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* process management definitions *****************************/

#define MAXARGS 3

#define DEFAULT_PROCESS "./process"

#define N_QUEUES 3
#define HIGH_PRIORITY 0
#define LOW_PRIORITY (N_QUEUES - 1)
#define N_FB_QUEUES (LOW_PRIORITY - HIGH_PRIORITY + 1)

#define PCB_UNINITIALIZED 0
#define PCB_INITIALIZED 1
#define PCB_READY 2
#define PCB_RUNNING 3
#define PCB_SUSPENDED 4
#define PCB_TERMINATED 5

struct pcb
{
    pid_t pid;
    char *args[MAXARGS];
    int arrivaltime;
    int priority;
    int remainingcputime;
    int status;
    int elapsedcputime;
    struct pcb *next;
};

typedef struct pcb Pcb;
typedef Pcb *PcbPtr;

/* process management prototypes *****************************/

PcbPtr startPcb(PcbPtr);
PcbPtr suspendPcb(PcbPtr);
PcbPtr terminatePcb(PcbPtr);
PcbPtr printPcb(PcbPtr);
void printPcbHdr();
PcbPtr createnullPcb();
PcbPtr enqPcb(PcbPtr, PcbPtr);
PcbPtr deqPcb(PcbPtr *);

#endif
