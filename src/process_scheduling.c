#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define UNUSED(x) (void)(x)

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
    // decrement the burst time of the pcb
    --process_control_block->remaining_burst_time;
}

int cmpfuncArrival(const void *a, const void *b) // compare function for shortest job first
{
    if (((ProcessControlBlock_t *)a)->arrival < ((ProcessControlBlock_t *)b)->arrival)
    {
        return -1;
    }
    else if (((ProcessControlBlock_t *)a)->arrival == ((ProcessControlBlock_t *)b)->arrival)
    {
        return 0;
    }

    return 1;
}

int cmpfuncRemainingTime(const void *a, const void *b)
{
    return (((ProcessControlBlock_t *)a)->remaining_burst_time - ((ProcessControlBlock_t *)b)->remaining_burst_time); // compare the remaining burst time
}

int cmpfuncShortest(const void *a, const void *b) // compare function for shortest job first
{
    if (((ProcessControlBlock_t *)a)->remaining_burst_time < ((ProcessControlBlock_t *)b)->remaining_burst_time)
    {
        return -1;
    }
    else if (((ProcessControlBlock_t *)a)->remaining_burst_time == ((ProcessControlBlock_t *)b)->remaining_burst_time)
    {
        return 0;
    }

    return 1;
}



bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (ready_queue == NULL || result == NULL)
    {
        return false;
    }
    
    if (!dyn_array_sort(ready_queue, cmpfuncArrival)) // Sort array by arrival time
    {
        return false;
    };

     float waittime = 0;
    unsigned long runtime = 0;
    float total_wait_time = 0;
    int n = dyn_array_size(ready_queue);

    for (int i = 0; i < n; i++) // loop through the ready queue
    {
        total_wait_time += ((ProcessControlBlock_t *)dyn_array_at(ready_queue, i))->arrival;
        int x = n - i;                                                                            // set the wait time multiplier
        runtime += ((ProcessControlBlock_t *)dyn_array_at(ready_queue, i))->remaining_burst_time; // add the burst time to the runtime
        if (i > 0)
        {
            waittime += ((ProcessControlBlock_t *)dyn_array_at(ready_queue, i - 1))->remaining_burst_time * x; // add the wait time to the waittime
        }
    }

    for (int i = 0; i < n; i++)
    {
        ProcessControlBlock_t *current = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);

        while (current->remaining_burst_time > 0) // run the process until the burst time is 0
        {
            virtual_cpu(current);
        }
    }

    dyn_array_destroy(ready_queue); // cleanup

    result->average_waiting_time = (waittime - total_wait_time) / n; // set the result
    result->average_turnaround_time = ((float)runtime + waittime - total_wait_time) / n;
    result->total_run_time = runtime;
    return true; // return true
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (ready_queue == NULL || result == NULL || dyn_array_empty(ready_queue))
    {
        return false;
    }

    float total_wait_time = 0;
    unsigned long total_run_time = 0;

    // Sort the ready queue by remaining burst time (shortest job first)
    if (!dyn_array_sort(ready_queue, cmpfuncRemainingTime))
    {
        return false;
    }

    int n = dyn_array_size(ready_queue);

    // Check if the ready queue is empty after sorting
    if (n == 0)
    {
        return false;
    }

    for (int i = 0; i < n; i++) 
    {
        ProcessControlBlock_t *current = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);

        // Calculate wait time for each process
        total_wait_time += total_run_time - current->arrival;

        // Update total run time
        total_run_time += current->remaining_burst_time;
    }

    // Check if the result pointer is not NULL before updating
    if (result != NULL)
    {
        result->average_waiting_time = total_wait_time / n;
        result->average_turnaround_time = (float)(total_wait_time + total_run_time) / n;
        result->total_run_time = total_run_time;
    }

    dyn_array_destroy(ready_queue);

    return true;
}
/*
bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (ready_queue == NULL || result == NULL) 
    {
        return false;
    }

    // Implement Priority scheduling logic here

    return false;     
}*/

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
   if (ready_queue == NULL || result == NULL || quantum == 0 || dyn_array_empty(ready_queue))
    {
        return false;
    }

    result->average_turnaround_time = 0;
    result->average_waiting_time = 0;
    result->total_run_time = 0;

    size_t total_waiting_time = 0;
    size_t total_turnaround_time = 0;
    size_t current_time = 0;

    while (!dyn_array_empty(ready_queue)) 
    {
        for (size_t i = 0; i < dyn_array_size(ready_queue); ++i) 
        {
            ProcessControlBlock_t *pcb = dyn_array_at(ready_queue, i);

            if (pcb->started) 
            {
                size_t remaining_burst = pcb->remaining_burst_time;

                if (remaining_burst <= quantum) 
                {
                    // Process completes within this quantum
                    virtual_cpu(pcb);
 
                    // Update result and current time
                    result->total_run_time += remaining_burst;
                    size_t wait_time = current_time - pcb->arrival;
                    total_waiting_time += wait_time;
                    total_turnaround_time += wait_time + remaining_burst;

                    // Remove the completed process from the ready queue
                    dyn_array_erase(ready_queue, i);
                    i--;  // Adjust i to account for the removed element
                } 
                else 
                {
                    // Quantum time slice is not sufficient to complete the process
                    virtual_cpu(pcb);

                    // Update result and current time
                    result->total_run_time += quantum;
                    size_t wait_time = current_time - pcb->arrival;
                    total_waiting_time += wait_time;

                    // Update the remaining burst time for the process
                    pcb->remaining_burst_time -= quantum;

                    // Move the process to the end of the queue
                    dyn_array_erase(ready_queue, i);
                    dyn_array_push_back(ready_queue, pcb);
                }
            }
            else 
            {
                // If the process has not started, mark it as started
                pcb->started = true;
            }
        }

        current_time += quantum;
    }

    // Check if dyn_array_size(ready_queue) is zero to avoid division by zero
    size_t queue_size = dyn_array_size(ready_queue);
    if (queue_size > 0) 
    {
        // Calculate average waiting time and average turnaround time
        result->average_waiting_time = (float)total_waiting_time / queue_size;
        result->average_turnaround_time = (float)total_turnaround_time / queue_size;
    }

    return true;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    if (!input_file) 
    {
        return NULL;
    }

    FILE *fp = fopen(input_file, "rb");  // Open the file in binary mode
    if (!fp) 
    {
        return NULL;  // Return null if the file is not found or cannot be opened
    }

    uint32_t num_PCB;
    if (fread(&num_PCB, sizeof(uint32_t), 1, fp) != 1) 
    {
        fclose(fp);
        return NULL;  // Return null if an error reading the number of PCBs
    }

    ProcessControlBlock_t *buffer = malloc(num_PCB * sizeof(ProcessControlBlock_t));
    if (!buffer) 
    {
        fclose(fp);
        return NULL;  // Return null if memory allocation fails
    }

    if (fread(buffer, sizeof(ProcessControlBlock_t), num_PCB, fp) != num_PCB) 
    {
        free(buffer);
        fclose(fp);
        return NULL;  // Return null if an error reading PCBs
    }

    fclose(fp);

    dyn_array_t *PCB_array = dyn_array_create(num_PCB, sizeof(ProcessControlBlock_t), NULL);
    if (!PCB_array) 
    {
        free(buffer);
        return NULL;  // Return null if creating dyn_array fails
    }

    // Copy PCBs from buffer to dynamic array
    for (uint32_t i = 0; i < num_PCB; i++) 
    {
        dyn_array_push_back(PCB_array, &buffer[i]);
    }

    free(buffer);

    return PCB_array;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (ready_queue == NULL || result == NULL)
    {
        return false;
    } // Check for invalid param
    if (!dyn_array_sort(ready_queue, cmpfuncArrival))
    {
        return false;
    }; // Sort array by arrival time

    float total_run_time = 0;       
    float wait_time = 0; 
    size_t n = dyn_array_size(ready_queue);
    size_t current_ready_queue_index = 0;
    dyn_array_t *run_que = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    // Calculate the total run time
    for (size_t i = 0; i < dyn_array_size(ready_queue); i++) // loop through the ready queue
    {
        total_run_time += ((ProcessControlBlock_t *)dyn_array_at(ready_queue, i))->remaining_burst_time; // add the remaining burst time to the total run time
    }
    // loop through every unit time
    for (size_t i = 0; i < (size_t)total_run_time; i++) // loop through the total run time
    {
        // Add every element to the run que that has an arrival time less or equal to current time
        if (dyn_array_size(ready_queue) > current_ready_queue_index)
        {
            if (((ProcessControlBlock_t *)dyn_array_at(ready_queue, current_ready_queue_index))->arrival <= (uint32_t)i) // check if the arrival time is less than or equal to the current time
            {
                dyn_array_push_back(run_que, dyn_array_at(ready_queue, current_ready_queue_index)); // add the PCB to the run queue
                current_ready_queue_index++;                                                        // increment the current ready queue index
            }
        }
        // Process any PCBs in the run queue
        if (dyn_array_size(run_que) != 0)
        {
            if (!dyn_array_sort(run_que, cmpfuncRemainingTime))
            {
                return false; // sort the run queue by remaining burst time
            }
            wait_time += dyn_array_size(run_que) - 1;                                           // add the number of PCBs in the run queue to the wait time
            virtual_cpu((ProcessControlBlock_t *)dyn_array_at(run_que, 0));                     // run the PCB at the front of the run queue
            if (((ProcessControlBlock_t *)dyn_array_at(run_que, 0))->remaining_burst_time == 0) // check if the PCB is done
            {
                dyn_array_erase(run_que, 0); // remove the PCB from the run queue
            }
        }
    }

    // printf("wait time: %f", wait_time / (float)n);

    result->average_waiting_time = wait_time / n;                       // calculate the average waiting time
    result->average_turnaround_time = (total_run_time + wait_time) / n; // calculate the average turnaround time
    result->total_run_time = (unsigned long)total_run_time;             // set the total run time

    dyn_array_destroy(ready_queue); // cleanup
    dyn_array_destroy(run_que);

    return true; // return true
}


