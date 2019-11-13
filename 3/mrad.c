/*
    COMP3520 Assignment 3 - Memory Requirement Aware Dispatcher (MRAD)

    usage:

        ./mrad <TESTFILE>
        where <TESTFILE> is the name of a job list
*/

/************************************************************************************************************************

    ** Revision history **

    Current version: 2.0
    Date: 8 November 2019

    2.0: Major revision
    1.5: Modified text and job file reader
    1.4: Modified descriptions
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

    3. ** PLEASE TYPE YOUR SID HERE. **

    Copyright of this code and associated material is vested in the original contributors.

    This code is NOT in the Public Domain. Unauthorized posting of this code or derivatives thereof is not permitted.

    ** DO NOT REMOVE THIS NOTICE. **

 ***********************************************************************************************************************/

/* Include files */
#include "mrad.h"

/* NOTE: Substantial changes are needed in order to arrive at the final solution! */

int main (int argc, char *argv[])
{
    /*** Main function variable declarations ***/

    FILE * input_list_stream = NULL;
    PcbPtr job_queue_0 = NULL;
    PcbPtr job_queue_12 = NULL;
    PcbPtr rr_queue = NULL;
    PcbPtr suspend_queue = NULL;
    PcbPtr current_process = NULL;
    PcbPtr process = NULL;
    int dispatcher_timer = 0;

    MabPtr mab = memCreate(MEMORY_SIZE);
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
        if ((val_check = fscanf(input_list_stream, "%d, %d, %d, %d",
          &(process->arrival_time), &(process->priority), &(process->memory_requirement), &(process->remaining_cpu_time))) == 4)
        {
            process->status = PCB_INITIALIZED;
            // assign to job queue based on priority
            if (process->memory_requirement <= MEMORY_SIZE) {
                if (process->priority == 0) {
                    job_queue_0 = enqPcb(job_queue_0, process);
                } else {
                    job_queue_12 = enqPcb(job_queue_12, process);
                }
            } else {
                // if we failed to add, free the pointer
                free(process);
            }
            
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
        else {
            free(process);
            break;
        }
    }

//  2. Whenever there is a currently running process or any queue is not empty:

    while (current_process || job_queue_0 || job_queue_12 || suspend_queue || rr_queue)
    {
//      i. While (a job in the priority 0 dispatch queue has "arrived")
        while (job_queue_0 && job_queue_0->arrival_time <= dispatcher_timer)
        {          
            MabPtr block = memAlloc(job_queue_0->memory_requirement);
            // if we were able to allocate, dequeue the job and enqueue it to the rr queue
            if (block) {
                process = deqPcb(&job_queue_0);
                process->status = PCB_READY;
                process->memory_block = block;
                rr_queue = enqPcb(rr_queue, process);	

            // if we were not able to, find a priority 2 job in the round robin queue we can swap out
            } else {
                int swapped = 0;
                PcbPtr curr = rr_queue;
                PcbPtr prev = NULL;

                while (curr && !swapped) {
                    if (curr->priority == 2) {
                        MabPtr curr_block = curr->memory_block;
                        if (getPossibleMem(curr_block) >= job_queue_0->memory_requirement) {
                            process = deqPcb(&job_queue_0);
                            process->status = PCB_READY;
                            if (!prev) {
                                rr_queue = curr->next;
                            } else {
                                prev->next = curr->next;
                            }

                            memFree(curr_block);
                            curr->memory_block = NULL;
                            curr->next = NULL;
                            suspend_queue = enqPcb(suspend_queue, curr);

                            process->memory_block = memAlloc(process->memory_requirement);
                            rr_queue = enqPcb(rr_queue, process);
                            // we only swapping the first p2 job
                            swapped = 1;
                        }

                    }

                    prev = curr;
                    curr = curr->next;
                }
                // if we couldn't find memory in the rr queue, check the current process
                if (!swapped) {
                    if (current_process && current_process->priority == 2 && 
                        getPossibleMem(current_process->memory_block) >= job_queue_0->memory_requirement) {
                        suspendPcb(current_process);
                        current_process->remaining_cpu_time --;
                        
                        memFree(current_process->memory_block);
                        current_process->memory_block = NULL;
                        suspend_queue = enqPcb(suspend_queue, current_process);
                        current_process = NULL;

                        MabPtr block = memAlloc(job_queue_0->memory_requirement);
                        process = deqPcb(&job_queue_0);
                        process->status = PCB_READY;
                        process->memory_block = block;
                        rr_queue = enqPcb(rr_queue, process);
                        swapped = 1;
                    }
                }

                // here, check if a swap happened
                if (!swapped) {
                    break;
                }
            }

        }

        // while (priority 1|2 job has "arrived" and no priority 0 jobs)
        while (job_queue_12 && job_queue_12->arrival_time <= dispatcher_timer && 
                !(job_queue_0 && job_queue_0->arrival_time <= dispatcher_timer))
        {
//          if the head is priority 2, and there are jobs in the suspend queue waiting, priotise them
            if (job_queue_12->priority == 2 && suspend_queue) {
                break;
            }

            MabPtr block = memAlloc(job_queue_12->memory_requirement);
            // if we could not allocate, break
            if (!block) {
                break;
            }

            // dequeue from dispactch and enqueue to rr
            process = deqPcb(&job_queue_12);
            process->status = PCB_READY;
            process->memory_block = block;
            rr_queue = enqPcb(rr_queue, process);	
        }

        // while (job in suspend queue waiting and no priority 0 jobs have arrived and no priority 1 jobs have arrived)
        if (suspend_queue && 
                !(job_queue_0 && job_queue_0->arrival_time <= dispatcher_timer) && 
                !(job_queue_12 && job_queue_12->priority == 1 && job_queue_12->arrival_time <= dispatcher_timer))
        {
            PcbPtr curr = suspend_queue;
            PcbPtr prev = NULL;
            while (curr) {
                MabPtr block = memAlloc(curr->memory_requirement);
                // if we could allocate memory, assign it the block and enqueue in in the rr queue
                if (block) {
                    curr->status = PCB_READY;
                    curr->memory_block = block;

                    // remove from suspend queue
                    if (!prev) {
                        deqPcb(&suspend_queue);
                    } else {
                        prev->next = curr->next;
                        curr->next = NULL;
                    }

                    rr_queue = enqPcb(rr_queue, curr);
                }
                prev = curr;
                curr = curr->next;
            }
        }

//      ii. If there is a currently running process;
        if (current_process)
        {
//          a. Decrement the process's remaining_cpu_time variable;
            current_process->remaining_cpu_time--;

//          b. If the process's allocated time has expired:
            if (current_process->remaining_cpu_time <= 0)
            {
//              A. Terminate the process;
                terminatePcb(current_process);
                memFree(current_process->memory_block);
//              B. Deallocate the PCB (process control block)'s memory
                free(current_process);
                current_process = NULL;
            }

//          c. Else if the RR queue is not empty
            else if (rr_queue)
            {				
//              A. Suspend the process;
                suspendPcb(current_process);

//              B. Enqueue the suspended process onto the RR queue
                rr_queue = enqPcb(rr_queue, current_process);
                current_process = NULL;
            }
        }

//      iii. If there is no running process and there is a process ready to run:
        if (!current_process && rr_queue)
        {
//          a. Dequeue a job from the RR queue
            current_process = deqPcb(&rr_queue);

//          b. Launch or resume job and set it as currently running process
            startPcb(current_process);
        }
        
//      iv. Let the dispatcher sleep for one second;
        sleep(1);
        
//      v. Increment the dispatcher's timer;
        dispatcher_timer++;

//      vi. Go back to 2.
    }
    
//  3. Terminate the RR dispatcher
    free(mab);
    fclose(input_list_stream);
    exit(EXIT_SUCCESS);
}
