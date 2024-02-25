#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
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


// Comparison function for sorting based on remaining burst time
int compare_remaining_burst_time(const void *a, const void *b) {
    const ProcessControlBlock_t *pcb_a = (const ProcessControlBlock_t *)a;
    const ProcessControlBlock_t *pcb_b = (const ProcessControlBlock_t *)b;

    // Compare remaining burst times
    if (pcb_a->remaining_burst_time < pcb_b->remaining_burst_time) {
        return -1;
    } else if (pcb_a->remaining_burst_time > pcb_b->remaining_burst_time) {
        return 1;
    } else {
        return 0;
    }
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
    if (!ready_queue || !result) {
        return false;
    }

    unsigned long current_time = 0;
    unsigned long total_waiting_time = 0;

    // Sort the ready queue based on remaining burst time (shortest first)
    dyn_array_sort(ready_queue, compare_remaining_burst_time);

    while (dyn_array_size(ready_queue) > 0) {
        // Get the PCB with the shortest remaining burst time
        ProcessControlBlock_t *pcb = dyn_array_at(ready_queue, 0);

        // Execute the PCB
        current_time += pcb->remaining_burst_time;
        total_waiting_time += current_time - pcb->arrival - pcb->remaining_burst_time;

        // Remove the completed PCB from the ready queue
        dyn_array_erase(ready_queue, pcb);

        // Update the result values
        result->average_waiting_time = (float)total_waiting_time / dyn_array_size(ready_queue);
        result->average_turnaround_time = result->average_waiting_time + pcb->remaining_burst_time;
        result->total_run_time = current_time;
    }

    return true;
}


bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
    if (!ready_queue || !result) {
        return false;
    }

    unsigned long current_time = 0;  // Initialize the current time
    unsigned long total_waiting_time = 0;  // Initialize the total waiting time

    // Process each PCB in the ready queue
    while (dyn_array_size(ready_queue) > 0) {
        // Get the PCB at the front of the queue
        ProcessControlBlock_t *pcb = dyn_array_front(ready_queue);

        // Increment the current time by the minimum of quantum and remaining burst time
        unsigned int time_slice = fmin(quantum, pcb->remaining_burst_time);
        current_time += time_slice;

        // Update the remaining burst time for the current PCB
        pcb->remaining_burst_time -= time_slice;

        // If the PCB has completed its execution, calculate waiting time
        if (pcb->remaining_burst_time == 0) {
            total_waiting_time += (current_time - pcb->arrival - pcb->remaining_burst_time);

            // Remove the completed PCB from the ready queue
            dyn_array_pop_front(ready_queue);
        } else {
            // Move the PCB to the back of the ready queue
            dyn_array_pop_front(ready_queue);
            dyn_array_push_back(ready_queue, pcb);
        }
    }

    // Calculate the average waiting time and turnaround time
    result->average_waiting_time = (float)total_waiting_time / dyn_array_size(ready_queue);
    result->average_turnaround_time = result->average_waiting_time + quantum;
    result->total_run_time = current_time;

    return true;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    //Error handling to ensure file is present
    if (!input_file) {
        return NULL;
    }

    //Error handling for empty files
    FILE *file = fopen(input_file, "rb");
    if (!file) {
        return NULL;
    }

    dyn_array_t *pcb_array = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
    if (!pcb_array) {
        fclose(file);
        return NULL;
    }

    ProcessControlBlock_t pcb;
    size_t read_size;

    while ((read_size = fread(&pcb, sizeof(ProcessControlBlock_t), 1, file)) > 0) { //Reads one line at a time
        dyn_array_push_back(pcb_array, &pcb);
    }

    fclose(file);

    return pcb_array;
}


bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (!ready_queue || !result) {
        return false;
    }

    unsigned long current_time = 0;
    unsigned long total_waiting_time = 0;

    while (dyn_array_size(ready_queue) > 0) {
        // Find the PCB with the shortest remaining burst time
        ProcessControlBlock_t *shortest_pcb = dyn_array_at(ready_queue, 0);

        for (size_t i = 1; i < dyn_array_size(ready_queue); ++i) {
            ProcessControlBlock_t *pcb = dyn_array_at(ready_queue, i);
            if (pcb->remaining_burst_time < shortest_pcb->remaining_burst_time) {
                shortest_pcb = pcb;
            }
        }

        // Execute the PCB
        current_time += shortest_pcb->remaining_burst_time;
        total_waiting_time += current_time - shortest_pcb->arrival - shortest_pcb->remaining_burst_time;

        // Remove the completed PCB from the ready queue
        dyn_array_erase(ready_queue, 0);

        // Update the result values
        result->average_waiting_time = (float)total_waiting_time / dyn_array_size(ready_queue);
        result->average_turnaround_time = result->average_waiting_time + shortest_pcb->remaining_burst_time;
        result->total_run_time = current_time;
    }

    return true;
}


