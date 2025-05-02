#pragma once
#include <map>
#include <string>
#include <regex>
#include <fcntl.h>
#include "ccore/File.h"
#include "ccore/safe/Log.h"
#include "ccore/net/IpTcp.h"
#include "ccore/net/IpTcpConfig.hpp"
#include "HttpConfig.h"
#include "Analyze.h"

namespace cc{
    namespace net{
        namespace http{
            struct Serve{
                IpTcp *tcp_; //tcp协议
                Serve(std::map<std::string,std::map<std::string, void (*)(std::map<std::string,std::string> &,std::string &,size_t &)>> &route);
                ~Serve();
                //发送头
                bool sendHeadF(const int32_t &client_fd,Analyze &info,const std::string &method,const std::string &type,const std::string &state="200 NULL STATE",char *data={},const size_t &size={},const bool &is_gzip=false,const bool &is_client_cache=false);
                //发送数据
                bool sendBodyF(const int32_t &client_fd,char *data,const size_t &size);
                //发送文件（零拷贝方法）
                bool sendFileF(const int32_t &client_fd,Analyze &info);
            };
        }
    }
}