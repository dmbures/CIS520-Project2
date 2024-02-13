cmake_minimum_required(VERSION 3.5)

project(hw2)

# Set the language mode for C files
set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror")

# Set the language mode for C++ files
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add our include directory to CMake's search paths.
include_directories(${CMAKE_SOURCE_DIR}/include)

# Create library from dyn_array for C code
add_library(dyn_array_lib_c src/dyn_array.c)

# Create library from process_scheduling for C++ code
add_library(process_scheduling_lib src/process_scheduling.c)

# Compile the analysis executable
add_executable(analysis src/analysis.c)

# Link the dyn_array library with the analysis executable
target_link_libraries(analysis PRIVATE dyn_array_lib_c)

# Compile the tester executable
add_executable(${PROJECT_NAME}_test test/tests.cpp src/dyn_array.c)

# Link the dyn_array library and processing_scheduling library with the tester executable
target_link_libraries(${PROJECT_NAME}_test dyn_array_lib_c processing_scheduling_lib_cpp gtest pthread)
