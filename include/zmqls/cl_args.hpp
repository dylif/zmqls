#ifndef ZMQLS_CL_ARGS_H
#define ZMQLS_CL_ARGS_H

#include <cxxopts/cxxopts.hpp>

#include <zmqls/zmqls.hpp>

// Common argument descriptions
#define ZMQLS_COMMON_HELP_DESC          "Print help message"
#define ZMQLS_COMMON_THREADS_DESC       "Number of threads to use for ZMQ"
#define ZMQLS_COMMON_FILE_DESC          "JSON file containing stream configuration(s)"

// Common default arguments
#define ZMQLS_COMMON_THREADS_DEF        "1"
#define ZMQLS_COMMON_FILE_DEF           "stream.json"

// Client argument descriptions
#define ZMQLS_CLIENT_NAME               "zmqls-client"
#define ZMQLS_CLIENT_DESC               "Receives video from a ZMQLS server"
#define ZMQLS_CLIENT_HELP_DESC          ZMQLS_COMMON_HELP_DESC
#define ZMQLS_CLIENT_THREADS_DESC       ZMQLS_COMMON_THREADS_DESC
#define ZMQLS_CLIENT_FILE_DESC          ZMQLS_COMMON_FILE_DESC

// Client default arguments
#define ZMQLS_CLIENT_THREADS_DEF        ZMQLS_COMMON_THREADS_DEF
#define ZMQLS_CLIENT_FILE_DEF           ZMQLS_COMMON_FILE_DEF

// Server argument descriptions
#define ZMQLS_SERVER_NAME               "zmqls-server"
#define ZMQLS_SERVER_DESC               "Encodes and sends video from a capture device across the network using ZMQ"
#define ZMQLS_SERVER_HELP_DESC          ZMQLS_COMMON_HELP_DESC
#define ZMQLS_SERVER_THREADS_DESC       ZMQLS_COMMON_THREADS_DESC
#define ZMQLS_SERVER_FILE_DESC          ZMQLS_COMMON_FILE_DESC

// Server default arguments
#define ZMQLS_SERVER_THREADS_DEF        ZMQLS_COMMON_THREADS_DEF
#define ZMQLS_SERVER_FILE_DEF           ZMQLS_COMMON_FILE_DEF

namespace zmqls {
        struct cl_args {
                typedef enum {
                        CLIENT, SERVER
                } model;

                const char *name;
                const char *desc;
                const char *help_desc;
                const char *threads_desc;
                const char *file_desc;
                const char *threads_def;
                const char *file_def;

                bool help;
                uint threads;
                std::string file;

                cl_args() = delete;
                cl_args(const model &m):
                        name((m == CLIENT)
                                ? ZMQLS_CLIENT_NAME : ZMQLS_SERVER_NAME),
                        desc((m == CLIENT)
                                ? ZMQLS_CLIENT_DESC : ZMQLS_SERVER_DESC),
                        help_desc((m == CLIENT)
                                ? ZMQLS_CLIENT_HELP_DESC
                                : ZMQLS_SERVER_HELP_DESC),
                        threads_desc((m == CLIENT)
                                ? ZMQLS_CLIENT_THREADS_DESC 
                                : ZMQLS_SERVER_THREADS_DESC),
                        file_desc((m == CLIENT) 
                                ? ZMQLS_CLIENT_FILE_DESC 
                                : ZMQLS_SERVER_FILE_DESC),
                        threads_def((m == CLIENT) 
                                ? ZMQLS_CLIENT_THREADS_DEF 
                                : ZMQLS_SERVER_THREADS_DEF),
                        file_def((m == CLIENT)
                                ? ZMQLS_CLIENT_FILE_DEF
                                : ZMQLS_SERVER_FILE_DEF) { }

                int parse(int argc, char **argv);
        private:
                cxxopts::Options add_options();
        };
}

#endif // ZMQLS_CL_ARGS_H