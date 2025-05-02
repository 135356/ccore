//http配置
#pragma once
#include <functional>
#include <netinet/in.h> //网络结构体
#include <cstdint> //C++标准库(UINT16_MAX)

namespace cc{
    namespace net{
        class IpTcpConfig{
            IpTcpConfig() = default;
            ~IpTcpConfig() = default;
        public:
            static const size_t MAX_SEND_SIZE_ = 1024; //每次最大发送字节
            static const size_t MAX_READ_SIZE_ = 1024; //每次最大接收字节
            static auto &obj(){
                static IpTcpConfig alias;
                return alias;
            }
        };
    }
}