# General version specs for CMake.
cmake_minimum_required(VERSION 3.4)

# Define the project name.
project(itor-2021-brkga-mp-ipr-hhcrsp)

# General project definitions. Assumes g++ being used to compile the code.
add_definitions(
   -std=c++17
   -pipe
   -march=native -mtune=native
   -Wall -Wextra -Wshadow 
   -fopenmp
   -fno-omit-frame-pointer
)

# Set some include directories to the compiler look for header files of project dependencies.
include_directories(brkga_mp_ipr_cpp/brkga_mp_ipr)

# Add the source code directory to the compiler include path.
include_directories(src)

# Definitions for Debug and Release compilation.
# To set the build mode, specify the value of var `CMAKE_BUILD_TYPE` when running CMake.
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

# Fallback to Debug build type automatically if no compilation mode was specified.
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
endif()

message("-- Using compilation flags of mode \"" ${CMAKE_BUILD_TYPE} "\"")

# Add the main executable dependencies.
add_executable(
   # Binary name
   brkga
   
   # Project source code.
   # We use a slightly modified versions of Solution and SortingDecoder
   # than GECCO source code.
   src/mainBrkgaMpIpr.cpp
   src/Solution.cpp
   src/SortingDecoder.cpp
   src/Instance.cpp      
   src/Task.cpp
)

# Link the main binary against its dependency libraries.
target_link_libraries(
   brkga 

   # Link with the  GNU OpenMP library.
   gomp

   # Also link with the Boost Program Options library, which we use to parse 
   # the command line parameters.
   boost_program_options
)
