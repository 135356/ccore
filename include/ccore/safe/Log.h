// Created by 邦邦 on 2022/4/22.
#pragma once
#include <string>
#include <mutex>
#include <exception>
#include "ccore/Time.h"

namespace cc {
    namespace safe{
        class Log{
            FILE *log_fp_; //日志文件的指针
            const char *log_path_ = "./13535.log"; //日志文件保存路径
            int32_t log_end_max_ = 2000000; //日志文件末尾保留最大字节（如果错误日记大于16m,将清理前面的内容只保留尾部log_end_max_字节2000000~=2m）
            Log();
            ~Log();
            void writeF_(std::string &msg,const size_t &msg_size);
        public:
            static Log &obj();
        public:
            //一般信息
            void info(std::string msg);
            void info(std::string msg,const std::string &file_path);
            void info(std::string msg,const std::string &file_path,const int &line);
            //警告信息
            void warn(std::string msg);
            void warn(std::string msg,const std::string &file_path);
            void warn(std::string msg,const std::string &file_path,const int &line);
            //错误信息
            void error(std::string msg);
            void error(std::string msg,const std::string &file_path);
            void error(std::string msg,const std::string &file_path,const int &line);
        };
    }
}