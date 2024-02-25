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

    size_t num_processes = dyn_array_size(ready_queue);

    if (num_processes == 0) {
        return false; // No processes to schedule
    }

    // Sort the ready queue based on remaining burst time (shortest job first)
    dyn_array_sort(ready_queue, compare_remaining_burst_time);

    unsigned long total_waiting_time = 0;
    unsigned long total_turnaround_time = 0;
    unsigned long total_run_time = 0;

    // Initialize the waiting time and turnaround time for the first process
    ProcessControlBlock_t *pcb = dyn_array_at(ready_queue, 0);
    pcb->started = true; // Mark the process as started
    pcb->remaining_burst_time = pcb->remaining_burst_time;
    total_run_time += pcb->remaining_burst_time;

    // Calculate waiting time, turnaround time, and total run time for subsequent processes
    for (size_t i = 1; i < num_processes; ++i) {
        pcb = dyn_array_at(ready_queue, i);

        if (!pcb->started) {
            pcb->started = true; // Mark the process as started
            pcb->remaining_burst_time = pcb->remaining_burst_time;
            total_run_time += pcb->remaining_burst_time;
        }

        pcb->remaining_burst_time = total_run_time;
        total_waiting_time += pcb->remaining_burst_time;

        pcb->remaining_burst_time = pcb->remaining_burst_time;
        total_turnaround_time += pcb->remaining_burst_time;

        total_run_time += pcb->remaining_burst_time;
    }

    // Calculate averages
    result->average_waiting_time = (float)total_waiting_time / num_processes;
    result->average_turnaround_time = (float)total_turnaround_time / num_processes;
    result->total_run_time = total_run_time;

    return true;
}


bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
    if (!ready_queue || !result || quantum <= 0) {
        return false;
    }

    size_t num_processes = dyn_array_size(ready_queue);

    if (num_processes == 0) {
        return false; // No processes to schedule
    }

    unsigned long total_waiting_time = 0;
    unsigned long total_turnaround_time = 0;
    unsigned long total_run_time = 0;

    // Calculate waiting time, turnaround time, and total run time for each process
    for (size_t i = 0; i < num_processes; ++i) {
        ProcessControlBlock_t *pcb = dyn_array_at(ready_queue, i);

        if (!pcb->started) {
            pcb->started = true; // Mark the process as started
            pcb->remaining_burst_time = fmin(quantum, pcb->remaining_burst_time);
            total_run_time += pcb->remaining_burst_time;
        }

        pcb->remaining_burst_time -= quantum;
        total_waiting_time += total_run_time;
        total_turnaround_time += total_run_time;

        total_run_time += pcb->remaining_burst_time;
    }

    // Calculate averages
    result->average_waiting_time = (float)total_waiting_time / num_processes;
    result->average_turnaround_time = (float)total_turnaround_time / num_processes;
    result->total_run_time = total_run_time;

    return true;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
     if (input_file == NULL) // Check for invalid param
    {
        return NULL;
    }
    FILE *fp;
    fp = fopen(input_file, "rb"); // open the file
    if (fp == NULL)
    {
        return NULL; // return null if the file is not found
    }
    uint32_t num_PCB[1];
    if (fread(num_PCB, sizeof(uint32_t), 1, fp) != 1) // read the number of PCBs
    {
        return NULL; // return null if the file is not found
    }
    uint32_t buffer[num_PCB[0] * 3];                // create a buffer to hold the PCBs
    if (fseek(fp, sizeof(uint32_t), SEEK_SET) != 0) // seek to the beginning of the PCBs
    {
        return NULL;
    }
    if (fread(buffer, sizeof(uint32_t), (size_t)(num_PCB[0] * 3), fp) != (num_PCB[0] * 3)) // read the PCBs
    {
        return false;
    }
    dyn_array_t *PCB_array = dyn_array_create((size_t)num_PCB[0], sizeof(uint32_t), NULL); // create a dynamic array to hold the PCBs
    if (PCB_array == NULL)                                                                 // check if the dynamic array was created
    {
        return NULL;
    }

    // Loop through the file creating dynamic array of specified PCBs
    for (uint32_t i = 0; i < num_PCB[0] * 3; i += 3)
    {
        ProcessControlBlock_t *PCB = (ProcessControlBlock_t *)malloc(sizeof(ProcessControlBlock_t)); // create a PCB
        PCB->remaining_burst_time = buffer[i];                                                       // set the PCB's remaining burst time
        PCB->priority = buffer[i + 1];                                                               // set the PCB's priority
        PCB->arrival = buffer[i + 2];                                                                // set the PCB's arrival time
        PCB->started = false;                                                                        // set the PCB's started flag to false
        dyn_array_push_back(PCB_array, PCB);                                                         // add the PCB to the dynamic array
    }
    fclose(fp);       // close the file
    return PCB_array; // return the dynamic array
}


bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    // Check for NULL inputs
    if (!ready_queue || !result) {
        return false;
    }

    // Ensure that ready_queue is not empty
    if (dyn_array_size(ready_queue) == 0) {
        return false;
    }

    // Sorting the ready_queue based on remaining_burst_time (shortest remaining time first)
    dyn_array_sort(ready_queue, compare_remaining_burst_time);

    unsigned long current_time = 0;
    float total_waiting_time = 0;
    float total_turnaround_time = 0;

    for (size_t i = 0; i < dyn_array_size(ready_queue); ++i) {
        ProcessControlBlock_t *pcb = dyn_array_at(ready_queue, i);

        // Check if the process has arrived
        if (pcb->arrival > current_time) {
            current_time = pcb->arrival;
        }

        // Simulate running the process for its burst time
        current_time += pcb->remaining_burst_time;

        // Update waiting and turnaround times
        total_waiting_time += (current_time - pcb->arrival - pcb->remaining_burst_time);
        total_turnaround_time += (current_time - pcb->arrival);

        // Update remaining burst time
        pcb->remaining_burst_time = 0;
    }

    // Calculate averages
    result->average_waiting_time = total_waiting_time / dyn_array_size(ready_queue);
    result->average_turnaround_time = total_turnaround_time / dyn_array_size(ready_queue);
    result->total_run_time = current_time;

    return true;
}


