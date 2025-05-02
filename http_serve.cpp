#include <iostream>
#include "ccore/net/http/Serve.h"

int main(int argc, char **argv) {
    std::map<std::string,std::map<std::string, void (*)(std::map<std::string,std::string> &,std::string &,size_t &)>> route;
    route["get"] = {
        {
            "/test",[](std::map<std::string,std::string> &r_data,std::string &s_data,size_t &s_size) {
                s_data.resize(1024);
                for(auto &v:r_data){
                    printf("%s:%s\n",v.first.c_str(),v.second.c_str());
                }
                s_size = sprintf(&s_data[0], R"({"state":%d,"msg":"%s"})", 0, "post成功");
                s_data.resize(s_size);
            }
        }
    };

    cc::net::http::Serve http_serve(route);

    return 0;
}