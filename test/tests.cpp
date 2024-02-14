#include <fcntl.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <pthread.h>
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function managling
extern "C" 
{
#include <dyn_array.h>
}


#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit

unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment 
{
    public:
        virtual void SetUp() 
        {
            score = 0;
            total = 210;
        }

        virtual void TearDown()
        {
            ::testing::Test::RecordProperty("points_given", score);
            ::testing::Test::RecordProperty("points_total", total);
            std::cout << "SCORE: " << score << '/' << total << std::endl;
        }
};

TEST(SchedulingTests, FCFS) 
{
    dyn_array_t* ready_queue = load_process_control_blocks("PCBs.bin");
    ScheduleResult_t result;

    ASSERT_TRUE(first_come_first_serve(ready_queue, &result));

    // Perform assertions on result and other checks

    // Cleanup
    dyn_array_destroy(ready_queue);
}

TEST(SchedulingTests, SJF) 
{
    dyn_array_t* ready_queue = load_process_control_blocks("PCBs.bin");
    ScheduleResult_t result;

    ASSERT_TRUE(shortest_job_first(ready_queue, &result));

    // Perform assertions on result and other checks

    // Cleanup
    dyn_array_destroy(ready_queue);
}


int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
    return RUN_ALL_TESTS();
}
