// Created by 邦邦 on 2022/7/8.
#pragma once
#include <string>
#include <vector>
#include <sys/stat.h> //S_IRWXU
#include <unistd.h> //关闭文件描述符（close）

namespace cc{
    class File{
    public:
        static size_t mkDir(const std::string &dir);
        static size_t mkFile(const std::string &dir,const std::string &name,const std::string &form,std::string &path);
    };
}