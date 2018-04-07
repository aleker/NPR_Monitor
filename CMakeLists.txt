cmake_minimum_required(VERSION 3.9)
project(NPR_Monitor)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -lboost_system -lboost_serialization -pthread")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

find_package(MPI REQUIRED)

set(SOURCE_FILES
        src/test/main.cpp
        src/DistributedMonitor.cpp
        src/DistributedMonitor.h
        src/Mutex.cpp
        src/Mutex.h
        src/MutexWrapper.h
        src/test/Test.cpp
        src/test/Test.h
        src/ConditionalVariable.cpp
        src/ConditionalVariable.h
        src/connection/ConnectionManager.h
        src/connection/MPI_Connection.cpp
        src/connection/MPI_Connection.h
        src/connection/Message.h
        src/connection/MPI_Msg.h
        src/MultiprocessDebugHelper.cpp
        src/MultiprocessDebugHelper.h
        )

set(MPI_LIB ~/.openmpi/lib)
add_executable(NPR_Monitor ${SOURCE_FILES})
target_link_libraries(NPR_Monitor ${MPI_CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
