#include "ccore/net/http/HttpConfig.h"

namespace cc{
    namespace net{
        namespace http{
            //在内存里面存储html
            struct ST_html{
                char *buf{};
                size_t size{}; //每次的字节大小
            };
            struct ST_html_arr{
                std::vector<ST_html> body{};
                size_t size{}; //总字节大小
            };
            HttpConfig::HttpConfig(){
                //创建web目录
                File::mkDir(WEB_DIR_);
                //读取配置文件
                FILE *config_fp = fopen(config_path_.c_str(), "rb");
                if (!config_fp) {
                    config_fp = fopen(config_path_.c_str(), "wb");
                    fputs((R"("IS_GZIP": 1,"IS_COROUTINE": 0,"IS_MTB_FILE": 1,"WEB_DIR": "html","PORT": 80,"ORIGIN": "*")"),config_fp);
                    fclose(config_fp);
                }else{
                    getConfigF_(config_fp);
                    WEB_DIR_ = config_info_["WEB_DIR"];
                    if(config_info_["ORIGIN"] != "*"){
                        ORIGIN_ = {};
                        std::string str{};
                        for(auto &v:config_info_["ORIGIN"]){
                            if(v == ';'){
                                ORIGIN_.insert(str);
                                str = {};
                            }else{
                                str += v;
                            }
                        }
                        if(!str.empty()){
                            ORIGIN_.insert(str);
                        }
                    }
                    try{
                        PORT_ = std::stoi(config_info_["PORT"]);
                        IS_GZIP_ = std::stoi(config_info_["IS_GZIP"]);
                    }catch(...){
                        safe::Log::obj().error("配置的参数格式不正确");
                    }
                    fclose(config_fp);
                }
            };
            void HttpConfig::getConfigF_(FILE *config_fp){
                int8_t is_str_state{};
                bool is_k=true;
                std::string key_0, str_0;
                char c{};
                while (true) {
                    c = fgetc(config_fp);
                    if(c == EOF){
                        break;
                    }else if(c == '\r' || c == '\n'){
                        getConfigF_(config_fp);break;
                    }else if(c == '\''){
                        if(is_str_state == 0){
                            is_str_state = 1;
                        }else if(is_str_state == 1){
                            is_str_state = 0;
                        }
                    }else if(c == '"'){
                        if(is_str_state == 0){
                            is_str_state = 2;
                        }else if(is_str_state == 2){
                            is_str_state = 0;
                        }
                    }else if(c != ' '){
                        if(is_str_state == 0){
                            if(c == ','){
                                config_info_[key_0] = str_0;
                                is_k=true;key_0 = str_0 = {};
                            }else if(c == ':'){
                                key_0 = str_0;
                                str_0 = {};
                            }else{
                                str_0 += c;
                            }
                        }else{
                            str_0 += c;
                        }
                    }
                }
                if(is_str_state == 0){
                    if(!key_0.empty()){
                        config_info_[key_0] = str_0;
                        is_k=true;key_0 = str_0 = {};
                    }
                }
            }
        }
    }
}