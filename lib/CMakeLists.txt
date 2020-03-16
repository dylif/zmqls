add_library(zmqls_lib STATIC cl_args.cpp zmqls.cpp)

target_include_directories(zmqls_lib PUBLIC ../include ${OpenCV_INCLUDE_DIRS} ${ZeroMQ_INCLUDE_DIR})
target_compile_features(zmqls_lib PUBLIC cxx_std_17)