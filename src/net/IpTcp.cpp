#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include "ccore/safe/Log.h"
#include "ccore/net/IpTcp.h"

namespace cc{
    namespace net{
        void IpTcp::addrF(const int &port, const int &type, const time_t &second, const int &addr_on){
            //创建套接字
            socket_ = socket(AF_INET,SOCK_STREAM,0);
            if(socket_ == -1){
                throw std::runtime_error("socket错误");
            }
            //socket超时设置 时间
            if(second > 0){
                struct timeval timeout{};
                timeout.tv_sec = second; //秒
                //timeout.tv_usec = 0; //微秒
                if(setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){
                    throw std::runtime_error("setsockopt 设置超时时间错误");
                }
            }
            //地址复用SO_REUSEADDR,端口复用SO_REUSEPORT
            if(addr_on){
                if(setsockopt(socket_, SOL_SOCKET, SO_REUSEPORT, &addr_on, sizeof(addr_on)) < 0){
                    throw std::runtime_error("setsockopt 设置地址复用错误");
                }
            }
            //结构体设置
            if(type==4){
                addr_.sin_family = AF_INET; //ipv4网络协议
                addr_.sin_addr.s_addr = htonl(INADDR_ANY); //自动获取IP地址
                addr_.sin_port=htons(port); //默认是主机字节序 所以要转成网络字节序
                if(bind(socket_,(struct sockaddr *)&addr_,sizeof(addr_)) == 0){}
            }else if(type==6){
                throw std::runtime_error("暂时不支持ipv6");
            }
            //将套接字设置为监听模式
            if(listen(socket_,SOMAXCONN) == -1){
                throw std::runtime_error("监听失败");
            }
        }
        void IpTcp::connectF(const std::string &ip,const int &port,const int &type){
            //创建套接字
            socket_ = socket(AF_INET,SOCK_STREAM,0);
            if(socket_ == -1){
                throw std::runtime_error("socket错误");
            }
            //结构体设置
            if(type==4){
                addr_.sin_family=AF_INET; //ipv4网络协议
                addr_.sin_addr.s_addr=inet_addr(ip.c_str()); //ip地址，将字符ip地址转换成计算机能识别的int
                addr_.sin_port = htons(port);
            }else if(type==6){}
            //发送连接请求
            if(connect(socket_,(struct sockaddr *)&addr_,sizeof(addr_)) == -1){
                cc::safe::Log::obj().warn("连接失败");
            }
        }
        bool IpTcp::sendF(const int32_t &client_fd,char *data,const size_t &size){
            size_t start_size{}; //用于记录开始位置
            size_t surplus_size = size; //用于记录剩余大小
            while (surplus_size > IpTcpConfig::obj().MAX_SEND_SIZE_) {
                if(send(client_fd, &(data)[start_size], IpTcpConfig::obj().MAX_SEND_SIZE_, 0) == -1){
                    return false;
                }
                start_size += IpTcpConfig::obj().MAX_SEND_SIZE_;
                surplus_size -= IpTcpConfig::obj().MAX_SEND_SIZE_;
            }
            if(surplus_size > 0){
                if(send(client_fd, &(data)[start_size], surplus_size, 0) == -1){
                    return false;
                }
            }
            return true;
        }
        bool IpTcp::sendFileF(const int32_t &client_fd,const int32_t &file_fd,const long &start_size,const size_t &end_size){
            //移动文件读取的位置
            if(start_size > 0){
                if(lseek(file_fd, start_size, SEEK_SET) == -1){
                    close(file_fd);
                    return false;
                }
            }
            if(end_size - start_size > 10240000){
               auto aF = std::async([](const int32_t &client_fd,const int32_t &file_fd,const long &start_size,const size_t &end_size){
                    if(sendfile(client_fd, file_fd, (off_t *)&start_size, end_size) == -1){
                        close(file_fd);
                        return false;
                    }else{
                        close(file_fd);
                        return true;
                    }
                },client_fd, file_fd,start_size,end_size);
                //return aF.get();
                return true;
            }else{
                if(sendfile(client_fd, file_fd, (off_t *)&start_size, end_size) == -1){
                    close(file_fd);
                    return false;
                }else{
                    close(file_fd);
                    return true;
                }
            }
        }
        void IpTcp::recvF(const std::function<void(const int32_t &,char *,ssize_t &)> &serveF){
            while(true) {
                struct sockaddr_in addr_client;
                socklen_t addr_in_size = sizeof(addr_client); //结构体大小
                int new_client_fd = accept(socket_, (struct sockaddr *)&addr_client, &addr_in_size);
                if(new_client_fd == -1) {
                    cc::safe::Log::obj().warn("建立连接的时候发生错误");
                }else{
                    char buf[IpTcpConfig::obj().MAX_READ_SIZE_];
                    ssize_t buf_size = recv(new_client_fd,buf,IpTcpConfig::obj().MAX_READ_SIZE_, 0);
                    serveF(new_client_fd,buf,buf_size);
                    close(new_client_fd);
                }
                //不管成功与否必须断开，即使不断开每一次循环都会创建新的连接，因为旧的client_fd没有添加到epoll队列
                close(new_client_fd);
            }
        }
    }
}