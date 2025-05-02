// 分析http头信息
#pragma once
#include <cstring>
#include <string>
#include <map>
#include <utility>
#include "ccore/safe/Log.h"

namespace cc{
    namespace net{
        namespace http{
            class Analyze{
                int8_t n_type_{}; //换行符类型
                //解析第一行(与众不同的),获取method、path、treaty，get_map数据
                void firstF_();
            public:
                char *r_buf; //接收到的原始数据
                ssize_t &r_size; //接收到的数据大小
                ssize_t start_index; //数据内容在字符串中的开始位置，数据如果是数组用body_arr_index标记下标
                std::string method,path,treaty; //GET,/a1_get?aaa=123&bbb=bbb,HTTP/1.1
                std::map<std::string, std::string> head_map; //头信息
                std::map<std::string, std::string> get_map; //get传递的数据
                //对数据进行分割
                explicit Analyze(char *r_buf,ssize_t &r_size);
                //分析出post数据
                bool getPost(std::map<std::string, std::string> &r_data,char *r_buf_1,ssize_t &start_index_1);
                bool getPost(std::map<std::string, std::string> &r_data);
            };
        }
    }
}