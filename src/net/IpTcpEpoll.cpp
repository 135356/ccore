#include "ccore/net/IpTcpEpoll.h"

namespace cc{
    namespace net{
        IpTcpEpoll::IpTcpEpoll() : epoll_fd_(epoll_create1(0)) {
            if (epoll_fd_ == -1) {
                throw std::runtime_error("epoll_create1创建时发生错误");
            }
        }
        IpTcpEpoll::~IpTcpEpoll() {
            close(epoll_fd_);
        }
        //添加事件EPOLL_CTL_ADD
        void IpTcpEpoll::addEventF(int fd, uint32_t events) {
            struct epoll_event event;
            event.data.fd = fd;
            event.events = events;
    
            std::lock_guard<std::mutex> guard(mux_);
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == -1) {
                throw std::runtime_error("epoll_ctl添加事件时发生错误");
            }
        }
        //修改事件EPOLL_CTL_MOD
        void IpTcpEpoll::modEventF(int fd, uint32_t events) {
            struct epoll_event event;
            event.data.fd = fd;
            event.events = events;
    
            std::lock_guard<std::mutex> guard(mux_);
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) == -1) {
                throw std::runtime_error("epoll_ctl修改事件时发生错误");
            }
        }
        //等待事件，直到有事件触发或超时 epoll_wait
        int IpTcpEpoll::waitF(struct epoll_event *events, int max_events, int timeout) {
            std::lock_guard<std::mutex> guard(mux_);
            return epoll_wait(epoll_fd_, events, max_events, timeout);
        }
    }
}