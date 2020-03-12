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

static cv::Mat gamma_image(const cv::Mat &m, const double &g)
{
        // Create lookup table
        cv::Mat t(1, 256, CV_8U);
        uint8_t *p = t.ptr();
        for(int i = 0; i < 256; ++i)
                p[i] = cv::saturate_cast<uint8_t>(pow(i / 255.0, g) * 255.0);

        // Apply the lookup table transform
        cv::Mat f = m.clone();
        cv::LUT(m, t, f);
        
        return f;
}

static cv::Mat rotate_image(const cv::Mat &m, const int &a)
{
        // Get centre point of image
        cv::Point2f c((m.cols - 1) / 2.0, (m.rows - 1) / 2.0);

        // Get rotation matrix
        cv::Mat r = cv::getRotationMatrix2D(c, a, 1.0);

        // Get bounding rectangle
        cv::Rect2f b = cv::RotatedRect(cv::Point2f(), m.size(), a)
                .boundingRect2f();
        
        // Adjust transformation matrix
        r.at<double>(0, 2) += b.width / 2.0 - m.cols / 2.0;
        r.at<double>(1, 2) += b.height / 2.0 - m.rows / 2.0;

        // Apply rotation
        cv::Mat f;
        cv::warpAffine(m, f, r, b.size());

        return f;
}

static cv::Mat flip_image(const cv::Mat &m, const string &s)
{
        bool h = false;
        bool v = false;

        // Parse the input string
        if (s.find('h') != string::npos)
                h = true;
        if (s.find('v') != string::npos)
                v = true;

        // Determine the flip mode
        int c = (h != v ? (c = h ? 1 : 0) : -1);

        // Apply the flip
        cv::Mat f;
        cv::flip(m, f, c);

        return f;
}

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
        auto gamma = this->m_json.get<double>(
                "gamma", -1, &zmqls::json::wrapper::is_number);
        auto angle = this->m_json.get<int>(
                "angle", 0, &zmqls::json::wrapper::is_number_integer);
        auto flip = this->m_json.get<string_t>(
                "flip", "", &zmqls::json::wrapper::is_string);

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
                        << ": FPS limit: " << (fps ? to_string(fps) : "N/A") << endl;
                cout << this->m_name 
                        << ": Custom width: " 
                        << (custom_width ? to_string(custom_width) : "N/A") << endl;
                cout << this->m_name 
                        << ": Custom height: " 
                        << (custom_height ? to_string(custom_height) : "N/A") << endl;
                cout << this->m_name 
                        << ": Gamma adjustment: " 
                        << (gamma >= 0 ? to_string(gamma) : "N/A") << endl;
                cout << this->m_name
                        << ": Rotation angle: "
                        << (angle ? to_string(angle) : "N/A") << endl;
                cout << this->m_name
                        << ": Flip string: "
                        << (!flip.empty() ? flip : "N/A") << endl;
        }

        // Create the display window
        cv::namedWindow(this->m_name, cv::WINDOW_AUTOSIZE);

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
                cv::Mat raw(1, msg.size() - prefix.length(), 
                        CV_8UC1, zmqls::get_beg(msg, prefix));
                cv::Mat frame = imdecode(raw, cv::IMREAD_COLOR);

                // Skip erroneous data
                if (frame.size().width == 0)
                        continue;

                // If given a custom width and height, resize the image
                // Do this before any other manipulations to save time
                if (custom_width && custom_height) {
                        cv::Mat tmp;
                        resize(frame, tmp, cv::Size(custom_width, custom_height), 
                                0, 0, cv::INTER_LINEAR);
                        frame = tmp;
                }

                // If given a flip string, flip the image accordingly
                if (!flip.empty()) {
                        cv::Mat tmp = flip_image(frame, flip);
                        frame = tmp;
                }

                // If given an angle, rotate the image
                if (angle) {
                        cv::Mat tmp = rotate_image(frame, angle);
                        frame = tmp;
                }

                // If given a gamma value, correct the image
                if (gamma >= 0) {
                        cv::Mat tmp = gamma_image(frame, gamma);
                        frame = tmp;
                }

                // Skip erroneous data after transformations
                if (frame.size().width > 0 && frame.size().height > 0)
                        cv::imshow(this->m_name, frame);

                // Show the frame for a total of 1 millisecond
                // Quit if escape key is pressed
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
        zmq::context_t ctx(args.threads);

        // Try starting the stream by parsing the input file
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
