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

/*
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
};*/


int main() 
{
    //::testing::InitGoogleTest(&argc, argv);
    //::testing::AddGlobalTestEnvironment(new GradeEnvironment);
    return RUN_ALL_TESTS();
}

//Checks ScheduleResult is NULL error handling
TEST(first_come_first_serve, ScheduleResultIsNULL){
    dyn_array *temp_array = dyn_array_create(5, sizeof(ProcessControlBlock_t), NULL);
    bool result = first_come_first_serve(temp_array, NULL);
    EXPECT_EQ(false, result);
}

//Checks Dynamic Array is NULL error handling
TEST(first_come_first_serve, NULL_Dyanamic_Array){
    ScheduleResult_t result = {0, 0, 0};
    dyn_array_t *temp_array = dyn_array_create(32, sizeof(ProcessControlBlock_t), NULL);
    bool test_result = first_come_first_serve(temp_array, &result);

    EXPECT_EQ(false, test_result);
}

//Checks successful PCB
TEST(first_come_first_serve, PCBIsValid){
    dyn_array_t *temp_array = dyn_array_create(32, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result = {0, 0, 0};
    ProcessControlBlock_t pcb1 = {4, 0, 2, false};
    ProcessControlBlock_t pcb2 = {9, 0, 3, false};
    ProcessControlBlock_t pcb3 = {2, 0, 0, false};
    ProcessControlBlock_t pcb4 = {12, 0, 6, false};
    ProcessControlBlock_t pcb5 = {6, 0, 1, false};
    
    dyn_array_push_back(temp_array, &pcb1);
    dyn_array_push_back(temp_array, &pcb2);
    dyn_array_push_back(temp_array, &pcb3);
    dyn_array_push_back(temp_array, &pcb4);
    dyn_array_push_back(temp_array, &pcb5);

    bool test_result = first_come_first_serve(temp_array, &result);

    EXPECT_EQ(true, test_result);
    EXPECT_TRUE((float)6.2 == (float)result.average_waiting_time);
    EXPECT_TRUE((float)12.8 ==  (float)result.average_turnaround_time);
    EXPECT_EQ((unsigned long)33, result.total_run_time);
}

//Checks NULL File name error handling
TEST(load_process_control_blocks, NULL_File_Name){
    dyn_array_t* temp_array = load_process_control_blocks(NULL);
    EXPECT_TRUE(temp_array == NULL);
    dyn_array_destroy(temp_array);
}

//Checks NULL File error handling
TEST(load_process_control_blocks, NULL_File){
    dyn_array_t* temp_array = load_process_control_blocks("test.txt");
    EXPECT_TRUE(temp_array == NULL);
}

//Checks with a Valid File
TEST(load_process_control_blocks, Valid_File)
{
    dyn_array_t *temp_array = load_process_control_blocks("../pcb.bin");
    EXPECT_EQ((size_t)4, dyn_array_size(temp_array));
}