#include "ccore/safe/Log.h"

namespace cc {
    namespace safe{
        //__FILE__,__LINE__,__func__
        Log::Log(){
            log_fp_ = fopen(log_path_, "ab+"); //附加读写方式打开
            if(fseek(log_fp_, 0, SEEK_END) == 0){
                long f_size = ftell(log_fp_);
                if(f_size != -1 && f_size > 16000000){
                    if(fseek(log_fp_, -log_end_max_, SEEK_END) == 0){
                        std::string tmp_path = log_path_;tmp_path=tmp_path+".tmp";
                        FILE *tmp_fp = fopen(tmp_path.c_str(), "wb");
                        while(true){
                            uint16_t buf_size = 128;
                            char buf[buf_size];
                            size_t read_size = fread(&buf[0],1,buf_size,log_fp_);
                            if(fwrite(buf,1, read_size, tmp_fp) != read_size){
                                perror("写入日志文件时发生错误");
                                exit(-1);
                            }
                            if(read_size != buf_size){
                                break;
                            }
                        }
                        fclose(log_fp_);
                        remove(log_path_);
                        rename(tmp_path.c_str(), log_path_);
                        log_fp_ = tmp_fp;
                    }else{
                        perror("日志文件定位时发生错误，log_end_max_不能大于16000000");
                        exit(-1);
                    }
                }
            }
        }
        Log::~Log(){
            fclose(log_fp_);
        }
        Log &Log::obj(){
            static Log alias;
            return alias;
        }
        void Log::writeF_(std::string &msg,const size_t &msg_size){
            if(fwrite(msg.c_str(),1, msg_size, log_fp_) != msg_size){
                perror("写入日志文件时发生错误");
                exit(-1);
            }
        }
        //一般信息
        void Log::info(std::string msg){
            msg = "提示: "+msg + "\t时间:("+cc::Time::getDate()+")\n";
            writeF_(msg,msg.size());
            printf("%s",msg.c_str());
        }
        void Log::info(std::string msg,const std::string &file_path){
            msg = "提示: "+msg + "\t时间:("+cc::Time::getDate()+"),位置:("+file_path+")\n";
            writeF_(msg,msg.size());
            printf("%s",msg.c_str());
        }
        void Log::info(std::string msg,const std::string &file_path,const int &line){
            msg = "提示: "+msg + "\t时间:("+cc::Time::getDate()+"),位置:("+file_path+"),line:"+std::to_string(line)+")\n";
            writeF_(msg,msg.size());
            printf("%s",msg.c_str());
        }
        //警告信息
        void Log::warn(std::string msg){
            msg = "警告: "+msg + "\t时间:("+cc::Time::getDate()+")\n";
            writeF_(msg,msg.size());
            perror(msg.c_str());
        }
        void Log::warn(std::string msg,const std::string &file_path){
            msg = "警告: "+msg + "\t时间:("+cc::Time::getDate()+"),故障点:("+file_path+")\n";
            writeF_(msg,msg.size());
            perror(msg.c_str());
        }
        void Log::warn(std::string msg,const std::string &file_path,const int &line){
            msg = "警告: "+msg + "\t时间:("+cc::Time::getDate()+"),故障点:("+file_path+"),line:"+std::to_string(line)+")\n";
            writeF_(msg,msg.size());
            perror(msg.c_str());
        }
        //错误信息
        void Log::error(std::string msg){
            msg = "错误:"+msg + "\t时间:("+cc::Time::getDate()+")\n";
            writeF_(msg,msg.size());
            perror(msg.c_str());
            exit(EXIT_FAILURE);
        }
        void Log::error(std::string msg,const std::string &file_path){
            msg = "错误:"+msg + "\t时间:("+cc::Time::getDate()+"),故障点:("+file_path+")\n";
            writeF_(msg,msg.size());
            perror(msg.c_str());
            exit(EXIT_FAILURE);
        }
        void Log::error(std::string msg,const std::string &file_path,const int &line){
            msg = "错误: "+msg + "\t时间:("+cc::Time::getDate()+"),故障点:("+file_path+"),line:"+std::to_string(line)+")\n";
            writeF_(msg,msg.size());
            perror(msg.c_str());
            exit(EXIT_FAILURE);
        }
    }
}