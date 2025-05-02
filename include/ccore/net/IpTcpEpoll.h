#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <coroutine>
#include <mutex>
#include "ccore/safe/Log.h"
#include "IpTcpConfig.hpp"

namespace cc{
    namespace net{
        //协程任务
        struct Task{
            //定义一类协程的行为
            struct promise_type {
                //返回给 caller 一个对象
                Task get_return_object() {
                    return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
                }
                //创建后，suspend_always为挂起协程
                std::suspend_always initial_suspend() { return {}; }
                //释放前，suspend_always为不自动销毁
                std::suspend_always final_suspend() noexcept { return {}; }
                //异常后
                void unhandled_exception() {}
                //没返回值，协程执行完后 或 调用 co_return 时被调用
                void return_void() {}
            };

            //协程
            std::coroutine_handle<promise_type> h;

            //恢复一个挂起的协程
            void resume() {
                if (h &&!h.done()) {
                    h.resume();
                }
            }

            Task(std::coroutine_handle<promise_type> h) : h(h) {}
            ~Task() {
                if (h) h.destroy();
            }
        };
        //事件队列
        class IpTcpEpoll{
        private:
            int epoll_fd_{}; //队列fd
            std::mutex mux_;
        public:
            int max_e_{100}; //最大排队任务
            IpTcpEpoll();
            ~IpTcpEpoll();
            //添加事件EPOLL_CTL_ADD
            void addEventF(int fd, uint32_t events);
            //删除事件EPOLL_CTL_DEL（不用删除，而且效率低）
            //修改事件EPOLL_CTL_MOD
            void modEventF(int fd, uint32_t events);
            //等待事件，直到有事件触发或超时 epoll_wait
            int waitF(struct epoll_event *events, int max_events, int timeout);
        };        
    }
}