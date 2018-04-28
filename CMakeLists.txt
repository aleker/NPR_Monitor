cmake_minimum_required(VERSION 3.9)
project(NPR_Monitor)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -lboost_system -lboost_serialization -pthread")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

find_package(MPI REQUIRED)

file(GLOB NPR_source_SRC "src/*.h" "src/*.cpp")
file(GLOB NPR_source_CONNECTION "src/connection/*.h" "src/connection/*.cpp")
file(GLOB NPR_source_EXAMPLES "src/examples/*.h" "src/examples/*.cpp")
file(GLOB NPR_source_MUTEX "src/mutex/*.h" "src/mutex/*.cpp")

set(MPI_LIB ~/.openmpi/lib)

add_executable(NPR_Monitor
        ${NPR_source_SRC}
        ${NPR_source_CONNECTION}
        ${NPR_source_EXAMPLES}
        ${NPR_source_MUTEX}
        )

target_link_libraries(NPR_Monitor ${MPI_CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
