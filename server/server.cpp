#include <zmqls/server.hpp>

#include <iostream>
#include <string>
#include <thread>
using namespace std;

#include <chrono>
using namespace std::chrono;

#include <opencv2/opencv.hpp>
using namespace cv;

#include <zmq.hpp>
using namespace zmq;

#include <zmqls/zmqls.hpp>
#include <zmqls/cl_args.hpp>
#include <zmqls/json.hpp>

void zmqls::server::stream::update_all_settings(ostream &os, bool verbose)
{
        // Loop through settings and update the device
        auto zd_beg = begin(zmqls::server::device_settings::zero_default);
        auto zd_end = end(zmqls::server::device_settings::zero_default);
        for (const auto &s : zmqls::server::device_settings::lookup) {
                using ur = update_result_t;
                ur result;
                if (find(zd_beg, zd_end, s.id) != zd_end) {
                        result = this->update_device(
                                s.id, [](double d){ return d > 0; });
                } else {
                        result = this->update_device(s.id);
                }

                if (result == ur::OK && verbose) {
                        os << this->m_name << ": " << s.name << " OK" << endl;
                } else {
                        switch (result) {
                        case ur::USING_DEFAULT:
                                if (verbose) {
                                        os << this->m_name 
                                                << ": Using default value for "
                                                << s.name << endl;
                                }
                                break;
                        case ur::NOT_FOUND:
                                if (verbose) {
                                        os << this->m_name 
                                                << ": Unable to find " 
                                                << s.name << endl;
                                }
                                break;
                        case ur::NOT_SUPPORTED:
                                os << this->m_name 
                                        << ": Device does not support " 
                                        << s.name << endl;
                                break;
                        case ur::NOT_OPEN:
                                os << this->m_name 
                                        << ": Device is not open" << endl;
                                break;
                        }
                }
        }
}

int zmqls::server::stream::start(zmq::context_t &ctx)
{
        // To prevent searching of JSON data
        auto address = this->m_json.get<string_t>(
                "address", "", &zmqls::json::wrapper::is_string);
        auto prefix = this->m_json.get<string_t> (
                "prefix", "", &zmqls::json::wrapper::is_string);
        auto fps = this->m_json.get<uint>(
                "fps", 0, &zmqls::json::wrapper::is_number_unsigned);
        auto verbose = this->m_json.get<bool>(
                "verbose", false, &zmqls::json::wrapper::is_boolean);
        auto encode = this->m_json.get<uint>(
                "encode", 80, &zmqls::json::wrapper::is_number_unsigned);

        // Sanity check
        if (address.empty()) {
                cerr << this->m_name << ": No address specified" << endl;
                return EXIT_FAILURE;
        }
        if (prefix.empty()) {
                cerr << this->m_name << ": No prefix specified" << endl;
                return EXIT_FAILURE;
        }

        // Create the socket (ZMQ sockets are NOT thread-safe)
        zmq::socket_t pub(ctx, ZMQ_PUB);

        // Try binding to the address given to us 
        // (will fail if not enough permission, invalid, etc.)
        try {
                pub.bind(address.c_str());
        } catch (const zmq::error_t &e) {
                cerr << this->m_name 
                        << ": Failed to bind to given address: " 
                        << address << endl;
                cerr << this->m_name << ": " << e.what() << endl;
                return EXIT_FAILURE;
        }

        // Try opening the device specified
        if (!this->open_device()) {
                cerr << this->m_name << ": Failed to open device" << endl;
                return EXIT_FAILURE;
        }

        // Update the device settings with given values or defaults
        this->update_all_settings(cerr, verbose);

        // Matrices for raw and encoded frames
        Mat frame;
        // Vector to store encoded frame's data
        vector<uint8_t> encoded;     
        
        // For FPS limiter
        auto last_frame = steady_clock::now();

        while (1) {
                // For FPS limiter
                time_point<steady_clock> wait_until;
                if (fps > 0)
                        wait_until = last_frame + milliseconds(1000 / fps);

                // Read raw from camera
                this->device >> frame;

                // Skip erroneous data
                if (frame.size().width == 0)
                        continue;

                // Resize raw, compress, and encode it
                vector<int> params(2);
                params.push_back(IMWRITE_JPEG_QUALITY);
                params.push_back(encode);
                imencode(".jpg", frame, encoded, params);

                // Setup ZMQ message, put data inside of it, and send it
                zmq::message_t msg;
                data_to_msg(msg, prefix, encoded);
                pub.send(msg);

                // For FPS limiter
                if (fps > 0)
                        this_thread::sleep_until(wait_until);
                auto next_frame = steady_clock::now();

                // Print stats if verbose
                if (verbose) {
                        double fps = (double) 1000 / (double) 
                                duration_cast<milliseconds>(
                                next_frame - last_frame).count();
                        cout << this->m_name << ": FPS: " << fps << endl;
                }

                last_frame = next_frame;
        }
}

int main(int argc, char **argv)
{
        // Setup the cl_args object
        zmqls::cl_args args(zmqls::cl_args::SERVER);

        // Parse and check command-line arguments
        int check;
        if ((check = args.parse(argc, argv)) != 0 || args.help)
                return check;

        // Create ZMQ context
        if (args.verbose) cout << args.name 
                << ": Creating ZMQ context with " << args.threads 
                << " threads" << endl;
        context_t ctx(args.threads);

        try {
                zmqls::server::stream stream(args.file);
                stream.start(ctx);
        } catch (const nlohmann::json::parse_error &e) {
                cerr << args.name 
                        << ": Failed to parse input file: " << e.what() << endl;
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS; 
}
