#include <zmqls/client.hpp>

#include <iostream>
#include <thread>
#include <chrono>

#include <zmq.hpp>
#include <opencv2/opencv.hpp>

#include <zmqls/zmqls.hpp>
#include <zmqls/cl_args.hpp>
#include <zmqls/json.hpp>

using namespace std;
using namespace std::chrono;
using namespace zmq;
using namespace cv;

int zmqls::client::stream::start(zmq::context_t &ctx)
{
        // Escape key constant
        int ESC = 27;

        // To prevent over-searching of JSON data
        auto address = this->m_json.get<string_t>(
                "address", "", &zmqls::json::wrapper::is_string);
        auto prefix = this->m_json.get<string_t>(
                "prefix", "", &zmqls::json::wrapper::is_string);
        auto fps = this->m_json.get<uint>(
                "fps", 0, &zmqls::json::wrapper::is_number_unsigned);
        auto verbose = this->m_json.get<bool>(
                "verbose", false, &zmqls::json::wrapper::is_boolean);
        auto custom_width = this->m_json.get<uint>(
                "width", 0, &zmqls::json::wrapper::is_number_unsigned);
        auto custom_height = this->m_json.get<uint>(
                "height", 0, &zmqls::json::wrapper::is_number_unsigned);

        // Sanity check
        if (address.empty()) {
                cerr << this->m_name << ": No address specified" << endl;
                return EXIT_FAILURE;
        }
        if (prefix.empty()) {
                cerr << this->m_name << ": No prefix specified" << endl;
                return EXIT_FAILURE;
        }

        // Create subscriber socket and attempt to connect to given address
        // Also set the prefix
        zmq::socket_t sub(ctx, ZMQ_SUB);
        try {
                sub.connect(address.c_str());
        } catch (const zmq::error_t &e) {
                cerr << this->m_name 
                        << ": Failed to connect to given address" << endl;
                cerr << this->m_name << ": " << e.what() << endl;
                return EXIT_FAILURE;
        }
        sub.setsockopt(ZMQ_SUBSCRIBE, prefix.c_str(), prefix.length());
        
        // If verbose, print settings
        if (verbose) {
                cout << this->m_name << ": Address: " << address << endl;
                cout << this->m_name << ": Prefix: " << prefix << endl;
                cout << this->m_name 
                        << ": FPS: " << (fps ? to_string(fps) : "N/A") << endl;
                cout << this->m_name 
                        << ": Custom width: " 
                        << (custom_width ? to_string(custom_width) : "N/A") << endl;
                cout << this->m_name 
                        << ": Custom height: " 
                        << (custom_height ? to_string(custom_height) : "N/A") << endl;
        }

        // Create the display window
        namedWindow(this->m_name, WINDOW_AUTOSIZE);

        // For FPS limiter
        auto last_frame = steady_clock::now();

        // Main non-terminating loop
        while (true) {
                // For FPS limiter
                time_point<steady_clock> wait_until;
                if (fps > 0)
                        wait_until = last_frame + milliseconds(1000 / fps);
                
                // Wait to receive the message
                if (verbose) cout << this->m_name 
                        << ": Waiting for " << address << "..." << endl;
                zmq::message_t msg;
                sub.recv(&msg);

                // Decode data
                Mat raw(1, msg.size() - prefix.length(), 
                        CV_8UC1, zmqls::get_beg(msg, prefix));
                Mat frame = imdecode(raw, IMREAD_COLOR);

                // Skip erroneous data
                if (frame.size().width == 0)
                        continue;

                // If we are given a custom width and height, resize the image
                if (custom_width && custom_height) {
                        Mat tmp;
                        resize(frame, tmp, Size(custom_width, custom_height), 
                                0, 0, INTER_LINEAR);
                        frame = tmp;
                }

                // Show the frame for a total of 1 millisecond
                // Quit if escape key is pressed
                cv::imshow(this->m_name, frame);
                if (cv::waitKey(1) == ESC) {
                        cout << this->m_name << ": Quitting..." << endl;
                        return EXIT_SUCCESS;
                }

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

        return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
        // Setup the cl_args object
        zmqls::cl_args args(zmqls::cl_args::CLIENT);

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
                zmqls::client::stream stream(args.file);
                stream.start(ctx);
        } catch (const nlohmann::json::parse_error &e) {
                cerr << args.name 
                        << ": Failed to parse input file: " << e.what() << endl;
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS; 
}
