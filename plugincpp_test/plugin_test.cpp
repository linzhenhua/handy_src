#include <string>
#include <functional>
#include <iostream>
#include <sstream>

#include <msgpack.hpp>

#include "plugin_test.h"
#include "router.hpp"
#include "msg_codec.h"

using namespace callback_container;

std::string hello() {
    return "hello plugin cpp";
}

int increment(int i) {
    return i + 1;
}

int plus(int a, int b) {
    return a + b;
}

struct dummy_t {
    std::string echo(std::string str) {
        return str;
    }

    int add(int a, int b) {
        return a + b;
    }
};

template<typename... Args>
msgpack::sbuffer mock_client_request_buffer(std::string key, Args... args) {
    return msg_codec::pack_args(std::move(key), std::move(args)...);
}

void callback_container::test() {
    //插入普通函数
    router::get().register_handler("hello", &hello);
    router::get().register_handler("increment", &increment);
    router::get().register_handler("plus", &plus);

    {
        auto hello_buf = mock_client_request_buffer("hello");
        std::string hello_str = router::get().route(hello_buf.data(), hello_buf.size());
        msg_codec codec;
        if (codec.error_code(hello_str.c_str(), hello_str.size()) != error_code::FAIL) {
            std::cout << codec.result<std::string>(hello_str.c_str(), hello_str.size()) << std::endl;
        }
    }

    {
        auto plus_buf = mock_client_request_buffer("plus", 96, 1);
        std::string plus_str = router::get().route(plus_buf.data(), plus_buf.size());
        msg_codec codec;
        if (codec.error_code(plus_str.c_str(), plus_str.size()) != error_code::FAIL) {
            std::cout << codec.result<int>(plus_str.c_str(), plus_str.size()) << std::endl;
        }
    }

    {
        auto increment_buf = mock_client_request_buffer("increment", 3);
        auto increment_str = router::get().route(increment_buf.data(), increment_buf.size());
        msg_codec codec;
        if (codec.error_code(increment_str.c_str(), increment_str.size()) != error_code::FAIL) {
            std::cout << codec.result<int>(increment_str.c_str(), increment_str.size()) << std::endl;
        }
    }

    {
        dummy_t dummy;

        //插入成员函数
        router::get().register_handler("dummy_t::add", &dummy_t::add, &dummy);
        router::get().register_handler("dummy_t::echo", &dummy_t::echo, &dummy);

        auto add_buf = mock_client_request_buffer("dummy_t::add", 3, 4);
        auto add_str = router::get().route(add_buf.data(), add_buf.size());
        msg_codec codec;
        if (codec.error_code(add_str.c_str(), add_str.size()) != error_code::FAIL) {
            std::cout << codec.result<int>(add_str.c_str(), add_str.size()) << std::endl;
        }

        auto echo_buf = mock_client_request_buffer("dummy_t::echo", std::string("linzhenhua"));
        auto echo_str = router::get().route(echo_buf.data(), echo_buf.size());
        if (codec.error_code(echo_str.data(), echo_str.size()) != error_code::FAIL) {
            std::cout << codec.result<std::string>(echo_str.data(), echo_str.size()) << std::endl;
        }
    }
}