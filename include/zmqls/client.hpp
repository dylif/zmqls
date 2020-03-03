#ifndef ZMQLS_CLIENT_H
#define ZMQLS_CLIENT_H

#include <zmq.hpp>

#include <zmqls/stream.hpp>

namespace zmqls {
        namespace client {
                using base_stream_t = ::zmqls::stream;

                class stream : public base_stream_t {
                public:
                        using base_stream_t::base_stream_t;
                        int start(::zmq::context_t &ctx);
                };
        }
}

#endif // ZMQLS_CLIENT_H