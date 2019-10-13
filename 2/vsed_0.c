/*
    COMP3520 Assignment 2 - Very Simple Experimental Dispatcher (VSED)

    usage:

        ./vsed <TESTFILE>
        where <TESTFILE> is the name of a job list
*/

/************************************************************************************************************************

    ** Revision history **

    Current version: 1.3 bis
    Date: 18 September 2019

    1.3: Minor reformatting 
    1.2: Modified instructions and variable names for teaching purposes
    1.1: Added instructions to allow additional functions
    1.0: Original version

    Contributors:
    1. COMP3520 teaching staff
       Centre for Distributed and High Performance Computing
       School of Computer Science
       The University of Sydney
       NSW 2006
       Australia

    2. Dr Ian G Graham

    Copyright of this code and associated material is vested in the original contributors.

    This code is NOT in the Public Domain. Unauthorized posting of this code or derivatives thereof is not permitted.

    ** DO NOT REMOVE THIS NOTICE. **

 ***********************************************************************************************************************/

/* Include files */
#include "vsed_0.h"

/******************************************************
 
   internal functions
   
 ******************************************************/

int CheckQueues(PcbPtr *);

/******************************************************/

int main(int argc, char *argv[])
{
    FILE *inputliststream;
    FILE *outputstream;
    PcbPtr inputqueue = NULL;     // input queue buffer
    PcbPtr fbqueue[N_FB_QUEUES];  // feedback queues
    PcbPtr currentprocess = NULL; // current process
    PcbPtr process = NULL;        // working pcb pointer
    int timer = 0;                // dispatcher timer
    int quantum = QUANTUM;        // current time-slice quantum
    int i;                        // working index
    int RQ0, RQ1, RQ2;            // quanta for each priority queue
    float avg_turntime = 0;
    float avg_waittime = 0;
    int num_process = 0;

    //  Write preliminary information
    outputstream = fopen(argv[5], "w");
    fprintf(outputstream, "%s %s %s %s\n\nturn,wait\n", argv[1], argv[2], argv[3], argv[4]);
    fclose(outputstream);

    //  0. Parse command line

    if (argc <= 0)
    {
        fprintf(stderr, "FATAL: Bad arguments array\n");
        exit(EXIT_FAILURE);
    }
    else if (argc != 6)
    {
        fprintf(stderr, "Usage: %s <TESTFILE> <RQ0> <RQ1> <RQ2> <OUTPUTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //  Set input parameters to respective variables

    RQ0 = atoi(argv[2]);
    RQ1 = atoi(argv[3]);
    RQ2 = atoi(argv[4]);

    //  1. Initialize dispatcher queues;

    for (i = 0; i < N_FB_QUEUES; fbqueue[i++] = NULL)
        ;

    //  2. Fill dispatcher queue from dispatch list file;

    if (!(inputliststream = fopen(argv[1], "r")))
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while (!feof(inputliststream))
    { // put processes into input_queue
        process = createnullPcb();
        if (fscanf(inputliststream, "%d, %d, %d",
                   &(process->arrivaltime), &(process->priority),
                   &(process->remainingcputime)) != 3)
        {
            free(process);
            continue;
        }
        process->status = PCB_INITIALIZED;
        inputqueue = enqPcb(inputqueue, process);
        num_process++;
    }

    //  3. Start dispatcher timer;
    //     (already set to zero above)

    //  4. While there's anything in any of the queues or there is a currently running process:

    while (inputqueue || (CheckQueues(fbqueue) >= 0) || currentprocess)
    {

        //      i. Unload any pending processes from the input queue:
        //         While (head-of-input-queue.arrival-time <= dispatcher timer)
        //         dequeue process from input queue and and enqueue on appropriate
        //         priority feedback queue (assigning it the appropriate priority);

        while (inputqueue && inputqueue->arrivaltime <= timer)
        {
            process = deqPcb(&inputqueue); // dequeue process
            process->status = PCB_READY;   // set pcb ready
            fbqueue[process->priority] = enqPcb(fbqueue[process->priority], process);
            // & put on queue
        }

        //     ii. If a process is currently running;

        if (currentprocess)
        {

            //          a. Decrement process remainingcputime by the amount governed by the feedback queue in which it is;
            switch (currentprocess->priority)
            {
            case 0:
                quantum = RQ0;
            case 1:
                quantum = RQ1;
            case 2:
                quantum = RQ2;
            }
            currentprocess->remainingcputime -= quantum;
            //currentprocess->elapsedcputime += quantum;
            //          b. If times up:

            if (currentprocess->remainingcputime <= 0)
            {

                //             A. Send SIGINT to the process to terminate it;

                terminatePcb(currentprocess);
                //  Calculate turnaround and wait time
                int turntime = timer - currentprocess->arrivaltime;
                int waittime = turntime - currentprocess->elapsedcputime;

                //  Append results to output file
                outputstream = fopen(argv[5], "a");
                fprintf(outputstream, "%d,%d\n", turntime, waittime);
                fclose(outputstream);

                //  Contribute to average summation
                avg_turntime += turntime;
                avg_waittime += waittime;

                //             B. Free up process strPrintucture memory

                free(currentprocess);
                currentprocess = NULL;

                //         c. else if other processes are waiting in feedback queues:
            }
            else if ((i = CheckQueues(fbqueue)) <= currentprocess->priority && i != -1)
            {

                //             A. Send SIGTSTP to suspend it;

                suspendPcb(currentprocess);

                //             B. Reduce the priority of the process (if possible) and enqueue it on
                //                the appropriate feedback queue;;

                if (++(currentprocess->priority) >= N_FB_QUEUES)
                    currentprocess->priority = N_FB_QUEUES - 1;
                fbqueue[currentprocess->priority] =
                    enqPcb(fbqueue[currentprocess->priority], currentprocess);
                currentprocess = NULL;
            }
        }

        //    iii. If no process currently running && feedback queues are not empty:

        if (!currentprocess && (i = CheckQueues(fbqueue)) >= 0)
        {

            //         a. Dequeue process from RR queue

            currentprocess = deqPcb(&fbqueue[i]);

            //         b. If already started but suspended, restart it (send SIGCONT to it)
            //              else start it (fork & exec)
            //         c. Set it as currently running process;

            startPcb(currentprocess);
        }

        //      iv. sleep for quantum;

        if (currentprocess && currentprocess->remainingcputime < quantum)
        {
            quantum = currentprocess->remainingcputime;
            currentprocess->elapsedcputime += quantum;
        }
        else if (!(currentprocess))
        {
            quantum = 1;
        }
        else
        {
            currentprocess->elapsedcputime += quantum;
        }

        sleep(quantum);

        //       v. Increment dispatcher timer;

        timer += quantum;
        //printf("%d\n", timer);
        //      vi. Go back to 4.
    }

    //    5. Exit
    avg_turntime = (float)(avg_turntime / num_process);
    avg_waittime = avg_waittime / num_process;
    printf("============\nAverage turnaround time: %f\nAverage wait time: %f\n", avg_turntime, avg_waittime);
    exit(0);
}

/*******************************************************************

int CheckQueues(PcbPtr * queues)

  check array of dispatcher queues

  return priority of highest non-empty queue
          -1 if all queues are empty
*******************************************************************/
int CheckQueues(PcbPtr *queues)
{
    int n;

    for (n = 0; n < N_FB_QUEUES; n++)
        if (queues[n])
            return n;
    return -1;
}
