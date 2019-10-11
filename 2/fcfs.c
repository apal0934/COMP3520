/*
    COMP3520 Exercise 3 - FCFS Dispatcher

    usage:

        ./fcfs <TESTFILE>
        where <TESTFILE> is the name of a job list
*/

/************************************************************************************************************************

    ** Revision history **

    Current version: 1.3 bis
    Date: 7 September 2019

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
#include "fcfs.h"

int main(int argc, char *argv[])
{
    /*** Main function variable declarations ***/

    FILE *input_list_stream = NULL;
    PcbPtr fcfs_queue = NULL;
    PcbPtr current_process = NULL;
    PcbPtr process = NULL;
    int timer = 0;

    //  1. Populate the FCFS queue

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
            fcfs_queue = enqPcb(fcfs_queue, process);
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

    //  2. Whenever there is a running process or the FCFS queue is not empty:

    while (current_process || fcfs_queue)
    {
        //      i. If there is a currently running process;
        if (current_process)
        {
            //          a. Decrement the process's remaining_cpu_time variable;
            current_process->remaining_cpu_time--;
            //          b. If the process's allocated time has expired:
            if (current_process->remaining_cpu_time <= 0)
            {
                //              A. Terminate the process;
                terminatePcb(current_process);

                //              B. Deallocate the PCB (process control block)'s memory
                free(current_process);
                current_process = NULL;
            }
        }

        //      ii. If there is no running process and there is a process ready to run:
        if (!current_process && fcfs_queue && fcfs_queue->arrival_time <= timer)
        {
            //          Dequeue the process at the head of the queue, set it as currently running and start it
            PcbPtr current_process = deqPcb(&fcfs_queue);
            startPcb(current_process);
        }

        //      iii. Let the dispatcher sleep for one second;
        sleep(1);

        //      iv. Increment the dispatcher's timer;
        timer++;

        //      v. Go back to 4.
    }

    //  3. Terminate the FCFS dispatcher
    exit(EXIT_SUCCESS);
}
