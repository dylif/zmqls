#ifndef ZMQLS_STREAM_H
#define ZMQLS_STREAM_H

#include <fstream>
#include <string>

#include <json/json.hpp>
#include <zmq.hpp>

#include <zmqls/zmqls.hpp>
#include <zmqls/json.hpp>

namespace zmqls {
      class stream {
      protected:
            using string_t = ::std::string;
            using json_t = ::zmqls::json::wrapper::basic;

            json_t m_json;
            string_t m_name;

            ::nlohmann::json from_file(const string_t &f)
            {
                  ::nlohmann::json j;

                  ::std::ifstream in(f);
                  if (in.is_open())
                        in >> j;

                  return j;
            }
      public:
            stream() = delete;
            stream(const json_t &j): m_json(j), m_name(
                  m_json.get<string_t>("name", "unnamed stream", 
                  &::zmqls::json::wrapper::is_string)
            ) { }
            stream(const ::nlohmann::json &j): stream(json_t(j)) { }
            stream(const string_t &f): stream(from_file(f)) { }

            const string_t &name() const { return this->m_name; }

            virtual int start(zmq::context_t &ctx) = 0;
      };
}

#endif // ZMQLS_STREAM_H