cmake_minimum_required(VERSION 3.15)

project(zmqls VERSION 0.1
              DESCRIPTION "zmqls"
              LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(ZeroMQ REQUIRED)
find_package(OpenCV REQUIRED)

add_subdirectory(lib)
add_subdirectory(client)
add_subdirectory(server)
        