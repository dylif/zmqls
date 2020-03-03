cmake_minimum_required(VERSION 3.5)
project(zmq_live_stream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules)

find_package(OpenCV REQUIRED)
find_package(ZMQ REQUIRED)

include_directories(../include ${OpenCV_INCLUDE_DIR} ${ZMQ_INCLUDE_DIR})

add_executable(client client.cpp ../lib/cl_args.cpp ../lib/zmqls.cpp)
target_include_directories(client PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(client PUBLIC ${OpenCV_LIBS} ${ZMQ_LIBRARY})