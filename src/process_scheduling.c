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

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (!ready_queue || !result) 
    {
        return false;
    }

    result->average_turnaround_time = 0;
    result->average_waiting_time = 0;
    result->total_run_time = 0;

    size_t total_waiting_time = 0;
    size_t total_turnaround_time = 0;
    size_t current_time = 0;

    for (size_t i = 0; i < dyn_array_size(ready_queue); ++i) 
    {
        ProcessControlBlock_t *pcb = dyn_array_at(ready_queue, i);

        // Waiting time for the current process
        size_t wait_time = current_time - pcb->arrival;
        total_waiting_time += wait_time;

        // Turnaround time for the current process
        size_t turnaround_time = wait_time + pcb->remaining_burst_time;
        total_turnaround_time += turnaround_time;

        // Update result and current time
        result->total_run_time += pcb->remaining_burst_time;
        result->average_waiting_time = (float)total_waiting_time / dyn_array_size(ready_queue);
        result->average_turnaround_time = (float)total_turnaround_time / dyn_array_size(ready_queue);

        // Move to the next process
        current_time += pcb->remaining_burst_time;
    }

    return true;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (!ready_queue || !result) 
    {
        return false;
    }

    // Implement SJF scheduling logic here

    return false; 
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (!ready_queue || !result) 
    {
        return false;
    }

    // Implement Priority scheduling logic here

    return false;     
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
    if (!ready_queue || !result) 
    {
        return false;
    }

    result->average_turnaround_time = 0;
    result->average_waiting_time = 0;
    result->total_run_time = 0;

    size_t total_waiting_time = 0;
    size_t total_turnaround_time = 0;

    dyn_array_t *temp_queue = dyn_array_create(dyn_array_capacity(ready_queue), sizeof(ProcessControlBlock_t), NULL);

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
                    dyn_array_push_back(temp_queue, pcb);
                    virtual_cpu(pcb);

                    // Update result and current time
                    result->total_run_time += remaining_burst;
                    size_t wait_time = current_time - pcb->arrival;
                    total_waiting_time += wait_time;
                    total_turnaround_time += wait_time + remaining_burst;

                    // Remove the completed process from the ready queue
                    dyn_array_erase(ready_queue, i);
                } 
                else 
                {
                    // Quantum time slice is not sufficient to complete the process
                    dyn_array_push_back(temp_queue, pcb);
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
        }
    }

    result->average_waiting_time = (float)total_waiting_time / dyn_array_size(temp_queue);
    result->average_turnaround_time = (float)total_turnaround_time / dyn_array_size(temp_queue);

    // Clean up
    dyn_array_destroy(temp_queue);

    return true;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    if (!input_file) 
    {
        return NULL;
    }

    FILE *file = fopen(input_file, "rb");
    if (!file) 
    {
        fprintf(stderr, "Error opening file: %s\n", input_file);
        return NULL;
    }

    // Read the number of process control blocks
    size_t num_blocks;
    if (fread(&num_blocks, sizeof(size_t), 1, file) != 1) 
    {
        fclose(file);
        fprintf(stderr, "Error reading number of process control blocks\n");
        return NULL;
    }

    // Create a dynamic array to store the process control blocks
    dyn_array_t *process_blocks = dyn_array_create(num_blocks, sizeof(ProcessControlBlock_t), NULL);

    // Read process control blocks from the file
    for (size_t i = 0; i < num_blocks; ++i) 
    {
        ProcessControlBlock_t pcb;

        if (fread(&pcb.priority, sizeof(uint32_t), 1, file) != 1 ||
            fread(&pcb.arrival, sizeof(uint32_t), 1, file) != 1) 
        {
            fclose(file);
            fprintf(stderr, "Error reading process control block data\n");
            dyn_array_destroy(process_blocks);
            return NULL;
        }

        // Initialize remaining burst time to 0
        pcb.remaining_burst_time = 0;
        pcb.started = false;

        // Add the process control block to the dynamic array
        dyn_array_push_back(process_blocks, &pcb);
    }

    fclose(file);
    return process_blocks;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;
}
