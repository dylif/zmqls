#ifndef ZMQLS_H
#define ZMQLS_H

#include <string>
#include <vector>

#include <zmq.hpp>
#include <opencv2/opencv.hpp>

typedef unsigned int uint;

namespace zmqls {
        std::size_t data_to_msg(
                zmq::message_t &m, 
                const char *p, const size_t &p_sz, 
                const uint8_t *d, const size_t &d_sz
        );
        std::size_t data_to_msg(
                zmq::message_t &m, 
                const std::string &p, 
                const std::vector<uint8_t> &d
        );
        void image_to_data(std::vector<uint8_t> &v, const cv::Mat &m);
        uint8_t *get_beg(const zmq::message_t &m, const size_t &p_sz);
        uint8_t *get_beg(const zmq::message_t &m, const std::string &p);
}

#endif // ZMQLS_H