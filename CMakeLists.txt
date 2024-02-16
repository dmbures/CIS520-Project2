cmake_minimum_required(VERSION 2.8)

# Set the project name variable to assignment3.
project(hw2)

# Add your CMAKE_C_FLAGS AND CMAKE_CXX_FLAGS here
set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror")

# Add our include directory to CMake's search paths.
include_directories(include)

# Create library from dyn_array so we can use it later.
add_library(dyn_array src/dyn_array.c)
add_library(process_scheduling src/process_scheduling.c)

# Compile the analysis executable.
add_executable(analysis src/analysis.c)

# Link the dyn_array library we compiled against our analysis executable.
target_link_libraries(analysis dyn_array process_scheduling)

# Compile the tester executable.
add_executable(hw2_test test/tests.cpp src/analysis.c)

# Link ${PROJECT_NAME}_test with dyn_array, process_scheduling, gtest, and pthread libraries
target_link_libraries(hw2_test gtest pthread dyn_array process_scheduling)

enable_testing()
add_test(NAME hw2_test COMMAND hw2_test)