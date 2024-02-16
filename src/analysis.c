#include <stdio.h>
#include <stdlib.h>

#include "../include/dyn_array.h"
#include "../include/processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"
#define SRT "SRT"

int analysis_main(int argc, char **argv) 
{
    if (argc < 3) 
    {
        printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *pcb_file = argv[1];
    const char *algorithm = argv[2];
    size_t quantum = 0;

    // Load process control blocks from the binary file
    ScheduleResult_t result = {0, 0, 0};
    
    dyn_array_t *ready_queue = load_process_control_blocks(pcb_file);

    

    // Execute the specified scheduling algorithm
    if (strcmp(algorithm, FCFS) == 0) 
    {
        if (first_come_first_serve(ready_queue, &result)) 
        {
            // Print or store the scheduling results
            printf("FCFS scheduling results:\n");
            printf("Average Turnaround Time: %f\n", result.average_turnaround_time);
            printf("Average Waiting Time: %f\n", result.average_waiting_time);
            printf("Total Run Time: %lu\n", result.total_run_time);
            // Output additional results if needed
        }
        else 
        {
            fprintf(stderr, "Error executing FCFS scheduling algorithm\n");
            EXIT_FAILURE;
        }
    }
    else if (strcmp(algorithm, SJF) == 0) 
    {
        if (shortest_job_first(ready_queue, &result)) 
        {
            // Print or store the scheduling results
            printf("Shortest Job First scheduling results:\n");
            printf("Average Turnaround Time: %f\n", result.average_turnaround_time);
            printf("Average Waiting Time: %f\n", result.average_waiting_time);
            printf("Total Run Time: %lu\n", result.total_run_time);
            // Output additional results if needed
        }
        else 
        {
            fprintf(stderr, "Error executing Shortest Job First scheduling algorithm\n");
            EXIT_FAILURE;
        }
    }
    else if (strcmp(algorithm, P) == 0) 
    {
        if (priority(ready_queue, &result)) 
        {
            // Print or store the scheduling results
            printf("Priority scheduling results:\n");
            printf("Average Turnaround Time: %f\n", result.average_turnaround_time);
            printf("Average Waiting Time: %f\n", result.average_waiting_time);
            printf("Total Run Time: %lu\n", result.total_run_time);
            // Output additional results if needed
        }
        else 
        {
            fprintf(stderr, "Error executing Priority scheduling algorithm\n");
            EXIT_FAILURE;
        }
    }
    else if (strcmp(algorithm, RR) == 0) 
    {
        if (round_robin(ready_queue, &result, quantum)) 
        {
            // Print or store the scheduling results
            printf("Round Robin scheduling results:\n");
            printf("Average Turnaround Time: %f\n", result.average_turnaround_time);
            printf("Average Waiting Time: %f\n", result.average_waiting_time);
            printf("Total Run Time: %lu\n", result.total_run_time);
            // Output additional results if needed
        }
        else 
        {
            fprintf(stderr, "Error executing Round Robin scheduling algorithm\n");
            EXIT_FAILURE;
        }
    }
    else if (strcmp(algorithm, SRT) == 0) 
    {
        if (shortest_remaining_time_first(ready_queue, &result)) 
        {
            // Print or store the scheduling results
            printf("Shortest Remaining Time scheduling results:\n");
            printf("Average Turnaround Time: %f\n", result.average_turnaround_time);
            printf("Average Waiting Time: %f\n", result.average_waiting_time);
            printf("Total Run Time: %lu\n", result.total_run_time);
        }
        else 
        {
            fprintf(stderr, "Error executing Shortest Remaining Time scheduling algorithm\n");
            EXIT_FAILURE;
        }
    }
    else 
    {
        fprintf(stderr, "Unknown scheduling algorithm: %s\n", algorithm);
        EXIT_FAILURE;
    }

    // Clean up allocated memory
    dyn_array_destroy(ready_queue);

    return EXIT_SUCCESS;    
}
