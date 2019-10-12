/*
    COMP3520 Exercise 4 - RR Dispatcher

    usage:

        ./rr <TESTFILE>
        where <TESTFILE> is the name of a job list
*/

/************************************************************************************************************************

    ** Revision history **

    Current version: 1.3 bis
    Date: 13 September 2019

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
#include "rr.h"

int main(int argc, char *argv[])
{
    /*** Main function variable declarations ***/

    FILE *input_list_stream = NULL;
    PcbPtr job_queue = NULL;
    PcbPtr rr_queue = NULL;
    PcbPtr current_process = NULL;
    PcbPtr process = NULL;
    int dispatcher_timer = 0;
    int quantum_timer = 0;
    int time_quantum = 0;

    //  1. Populate the job queue

    if (argc <= 0)
    {
        fprintf(stderr, "FATAL: Bad arguments array\n");
        exit(EXIT_FAILURE);
    }
    else if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <TESTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!(input_list_stream = fopen(argv[1], "r")))
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        int val_check;
        process = createnullPcb();
        if ((val_check = fscanf(input_list_stream, "%d, %d",
                                &(process->arrival_time), &(process->remaining_cpu_time))) == 2)
        {
            process->status = PCB_INITIALIZED;
            job_queue = enqPcb(job_queue, process);
        }
        else if (val_check >= 0)
        {
            free(process);
            fprintf(stderr, "ERROR: Invalid input file \"%s\"\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        else if (ferror(input_list_stream))
        {
            free(process);
            fprintf(stderr, "ERROR: Could not read input file \"%s\"\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        else
        {
            free(process);
            break;
        }
    }

    //  2. Ask the user to specify a time quantum

CHECKPOINT:
    printf("Please enter a positive integer for the time quantum: ");
    scanf("%d", &time_quantum);
    if (time_quantum <= 0)
    {
        goto CHECKPOINT;
    }

    //  3. Whenever there is a currently running process or either queue is not empty:

    while (current_process || (rr_queue || job_queue))
    {
        //      i. While (a job in the job dispatch queue has "arrived")
        while (job_queue && job_queue->arrival_time <= dispatcher_timer)
        {
            PcbPtr job = deqPcb(&job_queue);
            rr_queue = enqPcb(rr_queue, job);
        }

        //      ii. If there is a currently running process;
        if (current_process)
        {
            //          a. Decrement the process's remaining_cpu_time variable;
            current_process->remaining_cpu_time--;

            //          b. Increment the quantum timer;
            quantum_timer++;

            //          c. If the process's allocated time has expired:
            if (current_process->remaining_cpu_time <= 0)
            {
                //              A. Terminate the process;
                terminatePcb(current_process);

                //              B. Deallocate the PCB (process control block)'s memory
                free(current_process);
                current_process = NULL;
            }

            //          d. Else if the RR queue is not empty and the process has run for at least time_quantum without interruption
            else if (rr_queue && quantum_timer >= time_quantum)
            {
                //              A. Suspend the process;
                suspendPcb(current_process);

                //              B. Enqueue the suspended process onto the RR queue
                rr_queue = enqPcb(rr_queue, current_process);
                current_process = NULL;
            }
        }
        //      iii. If there is no running process and there is a process ready to run:
        if (!current_process && rr_queue) // REPLACE THIS LINE WITH YOUR CODE (ONE LINE ONLY)
        {
            //          a. Reset quantum timer to zero;
            quantum_timer = 0;

            //          b. Dequeue a job from the RR queue
            current_process = deqPcb(&rr_queue);

            //          c. Launch or resume job and set it as currently running process
            if (current_process->status == PCB_SUSPENDED)
            {
                kill(current_process->pid, SIGCONT);
            }
            else
            {
                startPcb(current_process);
            }
        }

        //      iv. Let the dispatcher sleep for one second;
        sleep(1);

        //      v. Increment the dispatcher's timer;
        dispatcher_timer++;

        //      vi. Go back to 3.
    }
    //  4. Terminate the RR dispatcher
    exit(EXIT_SUCCESS);
}
