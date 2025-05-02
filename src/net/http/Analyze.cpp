#include "ccore/net/http/Analyze.h"

namespace cc{
    namespace net{
        namespace http{
            Analyze::Analyze(char *buf,ssize_t &size):r_buf(buf),r_size(size){
                bool is_first{},is_key{},is_space{},is_get{};
                std::string key_str{},str{};
                firstF_(); //解析第一行(与众不同的)
                for(start_index=0;start_index<r_size;start_index++){
                    if(r_buf[start_index] == '\r' || r_buf[start_index] == '\n'){
                        //两个换行表示头信息结束
                        if(n_type_ == 1){
                            if(r_buf[start_index+1] == '\r'){ //下一个位置==\r
                                start_index+=2; //\r\r
                                break;
                            }
                        }else if(n_type_ == 11){
                            if(r_buf[start_index+1] == '\n'){ //下一个位置==\n
                                start_index+=2; //\n\n
                                break;
                            }
                        }else{ //不是1与11就是12
                            if(r_buf[start_index+2] == '\r'){
                                start_index+=4; //\r\n\r\n
                                break;
                            }
                        }
                        if(is_key){
                            is_key=false;
                            head_map[key_str] = str;
                        }
                        str={};
                        is_space=false; //每一行都要重制空格
                    }else if(r_buf[start_index] == ':'){
                        if(!is_key){ //key不存在则赋值
                            key_str = str;
                            str={};is_key=true;
                        }else{ //否则把:拼接到str里面去
                            str+=r_buf[start_index];
                        }
                    }else{
                        if(is_key && (r_buf[start_index] == ' ' && !is_space)){ //跳过head_map冒号后面的空格
                            is_space=true;
                            continue;
                        }
                        if(!is_key){
                            //大写字符转成小写(只对head的键进行转换)
                            if(r_buf[start_index]>='A' && r_buf[start_index]<='Z'){
                                str+=char(r_buf[start_index]+32);
                            }else{
                                str+=r_buf[start_index];
                            }
                        }else{
                            str+=r_buf[start_index];
                        }
                    }
                }
            }
            void Analyze::firstF_(){
                bool is_get{},is_key{};
                std::string key{},str{};
                for(start_index=0;start_index<r_size;start_index++){
                    if(r_buf[start_index] == '\r'){
                        treaty = str; //结束的时候赋值协议treaty部分
                        n_type_ = 1;
                        start_index++;
                        if(r_buf[start_index] == '\n'){ //下一个位置==\n
                            start_index++;
                            n_type_ = 12;
                            break;
                        }
                        break;
                    }else if(r_buf[start_index] == '\n'){
                        treaty = str;
                        n_type_ = 11;
                        start_index++;
                        break;
                    }else if(r_buf[start_index] == ' '){
                        if(is_get){
                            is_get = false;
                            if(!key.empty()){
                                get_map[key] = str;
                            }
                            key = str = {};
                        }else{
                            if(method.empty()){ //第一个空格进来肯定是方法method
                                method = str;
                            }else{ //方法有值之后就是路径path
                                path = str;
                            }
                            str = {};
                        }
                    }else if(r_buf[start_index] == '?'){ //如果有?号表示有get传参，路径是?号前面那一段
                        path = str;
                        str = {};
                        is_get = true;
                    }else if(r_buf[start_index] == '.' && r_buf[start_index+1] == '.'){ //如果第一行里面连续出现两个点就是非法请求
                        cc::safe::Log::obj().warn("非法请求，可能path地址问题");
                        break;
                    }else if(r_buf[start_index] < 32 || r_buf[start_index] > 126){ //除了\r\n空格以外，任何控制字符都不要
                        cc::safe::Log::obj().warn("非法请求，头信息里面不应包含非法制表符");
                        break;
                    }else{
                        if(is_get){
                            if(is_key){ //一旦遇到=符号表示key结束开始接收值，值里面允许有=符号，只有遇到&表示值接收完成。最后一个key的值由空格结束
                                if(r_buf[start_index] == '&'){
                                    is_key = false;
                                    if(!key.empty()){
                                        get_map[key] = str;
                                    }
                                    key = str = {};
                                }else{
                                    str += r_buf[start_index];
                                }
                            }else{
                                if(r_buf[start_index] == '='){
                                    is_key = true;
                                    key = str;
                                    str = {};
                                }else{
                                    str += r_buf[start_index];
                                }
                            }
                        }else{
                            str += r_buf[start_index];
                        }
                    }
                }
            }
            bool Analyze::getPost(std::map<std::string, std::string> &r_data,char *r_buf_1,ssize_t &start_index_1){
                size_t size = r_size;
                if(r_buf_1[start_index_1] == '{'){
                    for(;size>0;size--){
                        if(r_buf_1[size] == '}'){break;} //定位到结束位置
                    }
                }else if(r_buf_1[start_index_1] == '['){
                    for(;size>0;size--){
                        if(r_buf_1[size] == ']'){break;}
                    }
                }else{
                    return false;
                }
                //创建body_content
                size -= start_index_1+1; //结束位置-post数据开始的位置+1个空格 = post数据的长度
                char body_content[size];
                memmove(&body_content[0],&r_buf_1[start_index_1+1],size); //从开始位置+1(如:{),截取size个字符表示post字符串
                body_content[size] = '\0';
                if(size < 4){return false;} //最少会有4位"":1
                //解析body_content
                uint16_t big_brackets_i{},small_brackets_i{}; //大括号、小括号
                bool is_key{},is_colon{}; //冒号
                std::string str,key;size_t arr_i{};
                for(int i=0;i<size;i++){
                    //不过滤大括号、小括号、冒号，里面的分隔符{"aaa":123,"bbb":["aaa","aaa","aaa"]}结果:"aaa"=123,"bbb"=["aaa","aaa","aaa"]
                    if(body_content[i] == '{'){
                        big_brackets_i++;
                    }else if(body_content[i] == '['){
                        small_brackets_i++;
                    }else if(body_content[i] == '}'){
                        big_brackets_i--;
                    }else if(body_content[i] == ']'){
                        small_brackets_i--;
                    }else if(body_content[i] == '"'){
                        if(is_colon){
                            is_colon = false;
                        }else{
                            is_colon = true;
                        }
                    }
                    if(is_colon){
                        if(big_brackets_i || small_brackets_i){
                            str += body_content[i];
                        }else{
                            if(body_content[i] != '"'){
                                str += body_content[i];
                            }
                        }
                    }else{
                        if(big_brackets_i || small_brackets_i){
                            str += body_content[i];
                        }else{
                            if(body_content[i] == ':'){
                                key = str;
                                str = {};is_key = true;
                            }else if(body_content[i] == ','){
                                if(r_buf_1[start_index_1] == '{'){
                                    if(is_key){r_data[key] = str;}
                                }else{
                                    r_data[std::to_string(arr_i++)] = str;
                                }
                                str = {};is_key = false;
                            }else if(body_content[i] != '"'){
                                str += body_content[i];
                            }
                        }
                    }
                }

                if(r_buf_1[start_index_1] == '{'){
                    if(is_key){r_data[key] = str;}
                }else{
                    r_data[std::to_string(arr_i++)] = str;
                }

                return true;
            }
            bool Analyze::getPost(std::map<std::string, std::string> &r_data){
                return getPost(r_data,r_buf,start_index);
            }
        }
    }
}
