#include "ccore/net/http/Serve.h"

namespace cc{
    namespace net{
        namespace http{
            Serve::Serve(std::map<std::string,std::map<std::string, void (*)(std::map<std::string,std::string> &,std::string &,size_t &)>> &route){
                try {
                    tcp_ = new cc::net::IpTcp(cc::net::http::HttpConfig::obj().PORT_,4,100,1);
                    auto epollThreadF = [this,&route](int socket){
                        auto handleF = [this,&route](int client_fd)->Task{
                            char buf[1024];
                            ssize_t buf_size;
                            while ((buf_size = read(client_fd, buf, sizeof(buf))) > 0) {
                                //printf("buf: %s\n",buf);
                                if(buf_size > 0){
                                    Analyze info(buf,buf_size); //分析出来的http头信息
                                    std::string s_data{}; //要发送的内容
                                    size_t s_size{}; //要发送的长度
                                    if (info.method == "GET") {
                                        if (route["get"].count(info.path)) {
                                            route["get"][info.path](info.get_map,s_data, s_size);
                                            sendHeadF(client_fd,info,"get","html","200 OK",&s_data[0],s_size,false,false);
                                        }else{
                                            sendFileF(client_fd,info); //网页 或 文件请求
                                        }
                                    }else if(info.method == "POST"){
                                        if (route["post"].count(info.path)) {
                                            if(info.head_map["content-type"] == "application/json"){
                                                std::map<std::string, std::string> r_data;
                                                if(info.getPost(r_data)){ //获取post数据
                                                    route["post"][info.path](r_data,s_data, s_size);
                                                    sendHeadF(client_fd,info,"post","html","200 OK",&s_data[0],s_size,false,false);
                                                }else{
                                                    sendHeadF(client_fd,info,"get","html","400 Not Post",nullptr,0,false,false);
                                                }
                                            }else{
                                                sendHeadF(client_fd,info,"get","html","400 content-type != application/json",nullptr,0,false,false);
                                            }
                                        }else{
                                            sendHeadF(client_fd,info,"get","html","400 Not Route",nullptr,0,false,false);
                                        }
                                    } else if (info.method == "OPTIONS") {
                                        sendHeadF(client_fd,info,"get","html","401 Not Supported",nullptr,0,false,false);
                                    }else{
                                        sendHeadF(client_fd,info,"get","html",("400 Not Supported: "+info.method),nullptr,0,false,false);
                                    }
                                }
                            }
                            close(client_fd);
                            co_return;
                        };

                        IpTcpEpoll ip_tcp_epoll;
                        ip_tcp_epoll.addEventF(socket, EPOLLIN);
                        //epoll(内核事件表)里的所有事件
                        struct epoll_event epoll_event_all[ip_tcp_epoll.max_e_];
                        while (true) {
                            int events_active = ip_tcp_epoll.waitF(epoll_event_all, ip_tcp_epoll.max_e_, -1);
                            for (int i = 0; i < events_active; ++i) {
                                if (epoll_event_all[i].events & EPOLLIN) {
                                    if (epoll_event_all[i].data.fd == socket) { //建立新连接(主socket触发的事件)
                                        struct sockaddr_in client_addr;
                                        socklen_t addr_in_size = sizeof(client_addr);
                                        int new_client_fd = accept(socket, (struct sockaddr *)&client_addr, &addr_in_size);
                                        if (new_client_fd == -1) {
                                            cc::safe::Log::obj().warn("accept 建立连接的时候发生错误");
                                            continue;
                                        }

                                        //将新连接添加到epoll监听
                                        ip_tcp_epoll.addEventF(new_client_fd, EPOLLIN);
                                    } else { //处理已连接客户端的数据
                                        Task task = handleF(epoll_event_all[i].data.fd);
                                        task.resume();
                                    }
                                }
                            }
                        }
                    };

                    //创建多个线程来处理epoll事件
                    std::vector<std::jthread> t;
                    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
                        //按引用传递ip_tcp_epoll与sock
                        t.emplace_back(epollThreadF, std::ref(tcp_->socket_));
                    }
                    // 主线程等待线程结束
                    for (auto& v : t) {
                        v.join();
                    }
                } catch (const std::runtime_error& e) { //处理程序异常（也就是自己抛出的异常） std::runtime_error
                    cc::safe::Log::obj().warn(e.what());
                } catch (const std::exception& e) { //处理基类异常 std::exception
                    cc::safe::Log::obj().warn(e.what());
                } catch (...) { //捕获所有其他类型的异常
                    cc::safe::Log::obj().warn("Unknown exception caught");
                }
            }
            Serve::~Serve(){
                delete tcp_;
            }
            bool Serve::sendHeadF(const int32_t &client_fd,Analyze &info,const std::string &method,const std::string &type,const std::string &state,char *data,const size_t &size,const bool &is_gzip,const bool &is_client_cache){
                std::string head_str = "HTTP/1.1 "+state+"\r\n"
                    "Content-Type: "+HttpConfig::obj().content_type_[type].alias+"\r\n"
                    "Access-Control-Allow-Method: "+method+"\r\n" //GET,POST,OPTIONS,PUT,DELETE,PATCH,HEAD(发PUT请求前会先发OPTIONS请求进行预检)
                    "Access-Control-Allow-Headers: x-requested-with,content-type\r\n" //x-requested-with,content-type包含Access-Control-Request-Headers附带的内容//info.head_data["Access-Control-Request-Headers"]
                    "Server: 13535_web_serve_"+_PROJECT_VERSION+"\r\n"; //服务器名称
                //如果使用epoll必须指定文件长度，否则会一直等待，超时才会断开链接，浏览器才会请求下一个文件
                head_str+="Content-Length: "+std::to_string(size)+"\r\n";
                //文件续传(分段请求功能)
                //head_str+="Accept-Ranges: bytes\r\n"; //告诉客户端支持bytes，或none不支持
                //head_str+="Content-Range: bytes 0-1023/"+std::to_string(size)+"\r\n"; //当客户端发送 Range: bytes=0-1023 时才响应
                //head_str+="Content-Length: 1024\r\n"; //本次发送的长度0-1023=1024个字节
                //压缩与本地缓存
                if(is_gzip){head_str+="Content-Encoding: gzip\r\n";}
                if(is_client_cache){head_str+="Cache-Control: max-age=28800\r\n";}
                //跨域
                if(HttpConfig::obj().ORIGIN_.count("*") || HttpConfig::obj().ORIGIN_.count(info.head_map["origin"])){
                    head_str+="Access-Control-Allow-Origin: "+info.head_map["origin"]+"\r\n"; //等于*或Origin附带的内容(cors,如果请求中附带有Origin表示有跨域验证)
                    head_str+="Access-Control-Allow-Credentials: true\r\n"; //允许客户端携带验证信息(例如 cookie 之类的,Access-Control-Allow-Origin: 需要返回对应的ip，如果返回*这里就不能为true)
                }
                head_str += "\r\n";
                if (!tcp_->sendF(client_fd,&head_str[0],head_str.size())) {
                    return false;
                }else{
                    if(data){
                        return sendBodyF(client_fd,data,size);
                    }else{
                        return true;
                    }
                }
            }
            bool Serve::sendBodyF(const int32_t &client_fd,char *data,const size_t &size){
                size_t start_size{}; //用于记录开始位置
                size_t surplus_size = size; //用于记录剩余大小
                while (surplus_size > IpTcpConfig::obj().MAX_SEND_SIZE_) {
                    if(!tcp_->sendF(client_fd,&(data)[start_size],size_t(IpTcpConfig::obj().MAX_SEND_SIZE_))){
                        return false;
                    }
                    start_size += IpTcpConfig::obj().MAX_SEND_SIZE_;
                    surplus_size -= IpTcpConfig::obj().MAX_SEND_SIZE_;
                }
                if(surplus_size > 0){
                    if(!tcp_->sendF(client_fd,&(data)[start_size],surplus_size)){
                        return false;
                    }
                }
                return true;
            }
            bool Serve::sendFileF(const int32_t &client_fd,Analyze &info){
                std::string file_path = "./"+HttpConfig::obj().WEB_DIR_ + info.path;

                //文件类型
                std::string file_type{};
                size_t file_type_suffix = info.path.rfind('.');
                if(file_type_suffix == std::string::npos){ //表示没有后缀，可能是目录
                    file_type = "html";
                    if(file_path.back() == '/'){
                        file_path += "index.html";
                    }else{
                        file_path += "/index.html";
                    }
                }else{
                    file_type = info.path.substr(file_type_suffix + 1);
                }

                //是否支持gzip压缩
                bool is_gzip = HttpConfig::obj().IS_GZIP_;
                if(is_gzip && HttpConfig::obj().content_type_[file_type].is_gzip && info.head_map["accept-encoding"].find("gzip") != std::string::npos){
                    file_path = file_path+".gz";
                }else{
                    is_gzip = false;
                }
                
                //请求的文件类型是否在config.hpp里面配置了(也就是content_type，支持的文件类型)
                if (HttpConfig::obj().content_type_[file_type].alias.empty()) {
                    sendHeadF(client_fd,info,"get","html","405 content-type Not allowed",nullptr,0,false,false);
                    return false;
                }
                //获取文件描述符与状态
                int file_fd = open(file_path.c_str(), O_RDONLY);
                if(file_fd == -1){ //文件不存在
                    sendHeadF(client_fd,info,"get","html","402 not file",nullptr,0,false,false);
                    return false;
                }
                struct stat file_stat;
                if(fstat(file_fd, &file_stat) == -1){ //文件状态异常
                    sendHeadF(client_fd,info,"get","html","401 not file not directory",nullptr,0,false,false);
                    return false;
                }
                //200响应头
                if(!sendHeadF(client_fd,info,"get",file_type,"200 OK",nullptr,file_stat.st_size,is_gzip,true)){
                    return false;
                }
                //发送文件
                return tcp_->sendFileF(client_fd,file_fd,0,file_stat.st_size);
            }
        }
    }
}