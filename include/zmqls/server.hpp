#ifndef ZMQLS_SERVER_H
#define ZMQLS_SERVER_H

#include <iostream>
#include <string>

#include <json/json.hpp>
#include <opencv2/opencv.hpp>
#include <zmq.hpp>

#include <zmqls/zmqls.hpp>
#include <zmqls/stream.hpp>

namespace zmqls {
        namespace server {
                typedef ::zmqls::stream base_stream_t;

                namespace device_settings {
                        struct setting_t {
                                int id;
                                const char *name;
                        };

                        constexpr int zero_default[] = {
                                cv::CAP_PROP_FRAME_WIDTH,
                                cv::CAP_PROP_FRAME_HEIGHT,
                                cv::CAP_PROP_FPS
                        };

                        constexpr setting_t lookup[] = {
                                {cv::CAP_PROP_FRAME_WIDTH, "width"},
                                {cv::CAP_PROP_FRAME_HEIGHT, "height"},
                                {cv::CAP_PROP_FPS, "fps"},
                                {cv::CAP_PROP_BRIGHTNESS, "brightness"},
                                {cv::CAP_PROP_CONTRAST, "contrast"},
                                {cv::CAP_PROP_SATURATION, "saturation"},
                                {cv::CAP_PROP_HUE, "hue"},
                                {cv::CAP_PROP_GAIN, "gain"},
                                {cv::CAP_PROP_EXPOSURE, "exposure"}
                        };

                        const char *get_name(int id)
                        {
                                auto it = ::std::begin(zmqls::server::device_settings::lookup);
                                auto end = ::std::end(zmqls::server::device_settings::lookup);
                                for (; it != end && it->id != id; ++it)
                                        ;

                                return (it == end) ? nullptr : it->name;
                        }

                        typedef enum {
                                OK = 0, USING_DEFAULT, NOT_FOUND, NOT_SUPPORTED, NOT_OPEN
                        } update_result;
                }

                class stream : public base_stream_t {
                private:
                        using device_t =  ::cv::VideoCapture;
                        using update_result_t = ::zmqls::server::device_settings::update_result;

                        device_t device;

                        void update_all_settings(::std::ostream &os, bool verbose);
                public:
                        using base_stream_t::base_stream_t;

                        bool open_device()
                        {
                                bool ret = false;
                                auto odevice = this->m_json.get("device");
                                if (odevice) {
                                        if (odevice->is_number_unsigned())
                                                ret = this->device.open(odevice->get<uint>());
                                        else if (odevice->is_string())
                                                ret = this->device.open(odevice->get<string_t>());
                                }

                                return ret;
                        }

                        update_result_t update_device(int id, bool (*check)(double))
                        {
                                if (!this->device.isOpened())
                                        return update_result_t::NOT_OPEN;

                                auto id_name = ::zmqls::server::device_settings::get_name(id);
                                if (!id_name)
                                        return update_result_t::NOT_FOUND;
                                
                                auto id_obj = this->m_json.get(id_name);
                                if (!id_obj)
                                        return update_result_t::NOT_FOUND;

                                double val = id_obj->get<double>();
                                if (!check(val))
                                        return update_result_t::USING_DEFAULT;
                                
                                return this->device.set(id, val) ? update_result_t::OK : update_result_t::NOT_SUPPORTED;
                        }

                        update_result_t update_device(int id)
                        {
                                return this->update_device(id, [](double d){ return true; });
                        }

                        int start(::zmq::context_t &ctx);
                };
        }
}

#endif // ZMQLS_SERVER_H