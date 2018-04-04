cmake_minimum_required(VERSION 3.9)
project(NPR_Monitor)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

find_package(MPI REQUIRED)
#include_directories(~/.openmpi/include)

set(SOURCE_FILES
        src/main.cpp
        src/DistributedMonitor.cpp
        src/DistributedMonitor.h
        src/Mutex.cpp
        src/Mutex.h
        src/MutexWrapper.h
        src/Test.cpp
        src/Test.h
        src/ConditionalVariable.cpp
        src/ConditionalVariable.h
        src/ConnectionManager.cpp
        src/ConnectionManager.h
        )

# link_directories(~/.openmpi/lib)
set(MPI_LIB ~/.openmpi/lib)
add_executable(NPR_Monitor ${SOURCE_FILES})
target_link_libraries(NPR_Monitor ${MPI_CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
