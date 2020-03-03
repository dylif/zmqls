#include <zmqls/zmqls.hpp>

#include <string>
#include <vector>
#include <cstdint>

#include <zmq.hpp>
#include <opencv2/opencv.hpp>

std::size_t zmqls::data_to_msg(
        zmq::message_t &m, 
        const char *p, const size_t &p_sz, 
        const uint8_t *d, const size_t &d_sz
)
{
        // Create temporary message
        size_t sz = p_sz + d_sz;
        zmq::message_t msg(sz);

        // Copy the data
        memcpy(msg.data(), p, p_sz);
        memcpy(((uint8_t *) msg.data()) + p_sz, d, d_sz);

        // Move our message into the one provided
        m.move(&msg);

        return sz;
}

std::size_t zmqls::data_to_msg(
        zmq::message_t &m, 
        const std::string &p, 
        const std::vector<uint8_t> &d
)
{
        return zmqls::data_to_msg(m, p.c_str(), p.length(), d.data(), d.size());
}

uint8_t *zmqls::get_beg(const zmq::message_t &m, const size_t &p_sz)
{
        return (uint8_t *) m.data() + p_sz;
}

uint8_t *zmqls::get_beg(const zmq::message_t &m, const std::string &p)
{
        return zmqls::get_beg(m, p.length());
}