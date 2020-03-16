add_executable(server server.cpp)
target_link_libraries(server PRIVATE zmqls_lib ${OpenCV_LIBS} ${ZeroMQ_LIBRARY})
