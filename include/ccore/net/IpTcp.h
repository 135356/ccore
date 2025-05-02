// 2024 LongBang
#pragma once
#include <future> //异步
#include <functional>
#include <unistd.h> //关闭文件描述符（close）
#include <arpa/inet.h> //字节转换
#include <netinet/in.h> //网络结构体
#include "ccore/safe/Log.h"
#include "IpTcpConfig.hpp"
#include "IpTcpEpoll.h"

namespace cc {
    namespace net {
        class IpTcp{
        public:
            int socket_{}; //顶部套接字
            struct sockaddr_in addr_{}; //网络结构体
            //这是服务端
            IpTcp(const int &port,const int &type=4,const time_t &second=0,const int &addr_on=0){
                addrF(port,type,second,addr_on);
            }
            //这是客户端
            IpTcp(const std::string &ip,const int &port,const int &type=4){
                connectF(ip,port,type);
            }
            ~IpTcp(){
                if(socket_ > 0){
                    close(socket_); //关闭socket
                }
            }
            /** 结构体设置
             * ip={}为本机ip
             * port=0为不绑定端口，port=1为绑定预设端口,port>1绑定指定端口
             * type=4为ipV4协议类型，type=6为ipV6协议类型
             * second>0设置超时时间
             * addr_on=0不允许端口复用,addr_on=1允许端口复用
             * */
            void addrF(const int &port,const int &type=4,const time_t &second=0,const int &addr_on=0);
            //客户端向服务端发起连接请求
            void connectF(const std::string &ip,const int &port,const int &type=4);
            //发送数据
            static bool sendF(const int32_t &client_fd,char *data,const size_t &size);
            //发送文件(零拷贝需要跨平台)
            static bool sendFileF(const int32_t &client_fd,const int32_t &file_fd,const long &start_size=0,const size_t &end_size=0);
            //接收数据(没有任务队列的简单实现)
            void recvF(const std::function<void(const int32_t &,char *,ssize_t &)> &serveF);
        };
    }
}
