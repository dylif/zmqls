#ifndef ZMQLS_JSON_H
#define ZMQLS_JSON_H

#include <zmqls/zmqls.hpp>

#include <fstream>
#include <string>
#include <vector>

#include <json/json.hpp>

namespace zmqls {
        namespace json {
                using string_t = ::std::string;
                using json_lib_t = ::nlohmann::json;
                namespace wrapper {
                        class basic {
                        public:
                                basic() = default;
                                explicit basic(const json_lib_t &j): json(j) {}
                                void set_json(const json_lib_t &j) { this->json = j; }
                                bool exists(const string_t &key) const { return this->json.find(key) != this->json.end(); }
                                template <typename T> T get(const string_t &key, const T &def, bool (*check)(const json_lib_t &)) const
                                {
                                        T ret = def;
                                        if (this->exists(key) && check(this->json.at(key)))
                                                ret = this->json.at(key).get<T>();

                                        return ret;
                                }
                                json_lib_t const *get(const string_t &key) const 
                                {
                                        if (this->exists(key))
                                                return &(this->json.at(key));
                                        
                                        return nullptr;
                                }
                                json_lib_t *get(const string_t &key) 
                                {
                                        if (this->exists(key))
                                                return &(this->json.at(key));

                                        return nullptr;
                                }
                        protected:
                                json_lib_t json;
                        };
                        constexpr bool is_primitive(const json_lib_t &j) { return j.is_primitive(); }
                        constexpr bool is_structured(const json_lib_t &j) { return j.is_structured(); }
                        constexpr bool is_null(const json_lib_t &j) { return j.is_null(); }
                        constexpr bool is_boolean(const json_lib_t &j) { return j.is_boolean(); }
                        constexpr bool is_number(const json_lib_t &j) { return j.is_number(); }
                        constexpr bool is_number_integer(const json_lib_t &j) { return j.is_number_integer(); }
                        constexpr bool is_number_unsigned(const json_lib_t &j) { return j.is_number_unsigned(); }
                        constexpr bool is_number_float(const json_lib_t &j) { return j.is_number_float(); }
                        constexpr bool is_object(const json_lib_t &j) { return j.is_object(); }
                        constexpr bool is_array(const json_lib_t &j) { return j.is_array(); }
                        constexpr bool is_string(const json_lib_t &j) { return j.is_string(); }
                }
        }
}

#endif // ZMQLS_JSON_H